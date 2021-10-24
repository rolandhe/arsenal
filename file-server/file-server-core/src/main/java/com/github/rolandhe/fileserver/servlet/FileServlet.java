package com.github.rolandhe.fileserver.servlet;

import com.fasterxml.jackson.annotation.JsonTypeInfo;
import com.github.rolandhe.fileserver.config.ConfigProvider;
import com.github.rolandhe.fileserver.consts.ConstValues;
import com.github.rolandhe.fileserver.ctrl.service.ResultOutput;
import com.github.rolandhe.fileserver.ctrl.service.UploadFlow;
import com.github.rolandhe.fileserver.login.LoginValidatorProvider;
import com.github.rolandhe.fileserver.servlet.local.FileProcess;
import com.github.rolandhe.fileserver.servlet.local.LocalFileProcess;
import com.github.rolandhe.fileserver.std.StdPath;
import com.github.rolandhe.stat.CostTimeStat;
import org.apache.commons.fileupload.FileItemIterator;
import org.apache.commons.fileupload.FileItemStream;
import org.apache.commons.fileupload.FileUploadException;
import org.apache.commons.fileupload.servlet.ServletFileUpload;
import org.apache.commons.lang3.StringUtils;
import org.apache.tomcat.util.http.fileupload.util.Streams;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.server.ServletServerHttpAsyncRequestControl;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import javax.annotation.Resource;
import javax.servlet.AsyncContext;
import javax.servlet.AsyncEvent;
import javax.servlet.AsyncListener;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;
import java.util.concurrent.ThreadFactory;

@WebServlet(name = "fileUpServlet", urlPatterns = "/fs/up/*", asyncSupported = true)
public class FileServlet extends HttpServlet {
    private static final Logger logger = LoggerFactory.getLogger(FileServlet.class);

    private static final String MEET_ERROR_OF_GET_USER = new String();

    @Value("${file-server.max-threads}")
    private int threadCount;

    @Value("${file-server.max-upload-concurrent}")
    private int maxUploadConcurrent;

    @Value("${file-server.temp-root}")
    private String tempRoot;

    @Resource
    private ConfigProvider configProvider;

    @Resource
    private ResultOutput resultOutput;

    @Resource
    private UploadFlow uploadFlow;

    @Resource
    private LoginValidatorProvider loginValidatorProvider;

    private ExecutorService executorService;
    private Semaphore limitCtrl;

    @PostConstruct
    public void before() {
        limitCtrl = new Semaphore(maxUploadConcurrent);
        executorService = Executors.newFixedThreadPool(threadCount, new ThreadFactory() {
            @Override
            public Thread newThread(Runnable r) {
                Thread thread = new Thread(r);
                thread.setName("thread-fileServlet-" + thread.getId());
                return thread;
            }
        });
    }

    @PreDestroy
    public void after() {
        executorService.shutdown();
    }

    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        CostTimeStat costTimeStat = CostTimeStat.startStat(logger);
        String uri = req.getRequestURI();
        checked(req, uri);
        StdPath stdPath = StdPath.parse(uri);
        if (!limitCtrl.tryAcquire()) {
            logger.info("fast ending,because exceed max concurrent of {}, {}", maxUploadConcurrent, uri);
            resultOutput.doExceedMaxConcurrentResult(resp, stdPath);
            return;
        }
        AsyncContext asyncContext = req.startAsync();
        asyncContext.setTimeout(0L);
        addListener(asyncContext, stdPath, costTimeStat);
        executorService.execute(() -> {
            String uploadUser = safeGetUploadUser();
            if (uploadUser == MEET_ERROR_OF_GET_USER) {
                resultOutput.doInternalResult((HttpServletResponse) asyncContext.getResponse(), stdPath);
                asyncContext.complete();
                return;
            }
            if (StringUtils.isEmpty(uploadUser)) {
                logger.info("no login,please login,{}", stdPath.getUri());
                loginValidatorProvider.getActiveLoginValidator().loginNext((HttpServletResponse) asyncContext.getResponse());
                asyncContext.complete();
                return;
            }
            FileProcess fileProcess = process(asyncContext, stdPath);
            if (fileProcess != null) {
                doSend(resp, fileProcess, stdPath, uploadUser);
            }
            asyncContext.complete();
        });
    }

    private void addListener(AsyncContext asyncContext, StdPath stdPath, CostTimeStat costTimeStat) {
        asyncContext.addListener(new AsyncListener() {
            @Override
            public void onComplete(AsyncEvent asyncEvent) throws IOException {
                limitCtrl.release();
                costTimeStat.endStat("upload file from " + stdPath.getUri());
            }

            @Override
            public void onTimeout(AsyncEvent asyncEvent) throws IOException {

            }

            @Override
            public void onError(AsyncEvent asyncEvent) throws IOException {

            }

            @Override
            public void onStartAsync(AsyncEvent asyncEvent) throws IOException {

            }
        });
    }

    private String safeGetUploadUser() {
        try {
            return loginValidatorProvider.getActiveLoginValidator().getLoginUser();
        } catch (RuntimeException e) {
            logger.info("get login user failed.", e);
            return MEET_ERROR_OF_GET_USER;
        }
    }

    private void checked(HttpServletRequest req, String uri) throws ServletException {
        if (!ServletFileUpload.isMultipartContent(req)) {
            logger.warn("{} is not multiple file", uri);
            throw new ServletException("don't support request,you should upload file.");
        }

        if (!StdPath.validPath(uri)) {
            logger.warn("{} is not std format.", uri);
            throw new ServletException("invalid url std format");
        }
    }

    private void doSend(HttpServletResponse response, FileProcess fileProcess, StdPath stdPath, String uploadUser) {
        logger.info("begin to send local file to target,url: {}, local file {},upload user: {}", stdPath.getUri(), fileProcess.getLocalFileName(), uploadUser);
        CostTimeStat costTimeStat = CostTimeStat.startStat(logger);
        try {
            String targetUrl = uploadFlow.control(stdPath, fileProcess.getLocalFileName(), uploadUser);
            if (StringUtils.isEmpty(targetUrl)) {
                resultOutput.doFailResult(response, stdPath);
            } else {
                resultOutput.doSuccessResult(response, stdPath, targetUrl);
            }
            logger.info("end to send local file to target,url: {}, local file {},upload user: {},target:{}",
                    stdPath.getUri(), fileProcess.getLocalFileName(), uploadUser, targetUrl);
        } catch (RuntimeException e) {
            logger.info(stdPath.getUri(), e);
            resultOutput.doInternalResult(response, stdPath);
        } finally {
            fileProcess.clean();
            costTimeStat.endStat("send file " + stdPath.getUri());
        }
    }

    private FileProcess process(AsyncContext asyncContext, StdPath stdPath) {
        HttpServletRequest request = (HttpServletRequest) asyncContext.getRequest();
        HttpServletResponse response = (HttpServletResponse) asyncContext.getResponse();
        ServletFileUpload servletFileUpload = new ServletFileUpload();
        FileProcess fileProcess = null;
        CostTimeStat costTimeStat = CostTimeStat.startStat(logger);
        try {
            FileItemIterator itemIterator = servletFileUpload.getItemIterator(request);
            while (itemIterator.hasNext()) {
                FileItemStream itemStream = itemIterator.next();
                String name = itemStream.getFieldName();
                logger.info("upload file:{}, name:{},from {}", name, itemStream.getName(), stdPath.getUri());
                InputStream stream = itemStream.openStream();
                if (!itemStream.isFormField()) {
                    fileProcess = readContent(stream, response, stdPath, itemStream.getName());
                    if (null == fileProcess) {
                        break;
                    }
                } else {
                    logger.info("field info:{},{}", name, Streams.asString(stream));
                }
            }
        } catch (FileUploadException e) {
            logger.info(stdPath.getUri(), e);
            resultOutput.doFailResult(response, stdPath);
            return null;
        } catch (IOException e) {
            logger.info(stdPath.getUri(), e);
        } finally {
            costTimeStat.endStat("read file and store from" + stdPath.getUri());
        }
        return fileProcess;
    }

    private FileProcess readContent(InputStream inputStream, HttpServletResponse response, StdPath stdPath, String remoteName) throws IOException {
        logger.info("begin to read file from http request:{}, remote file {}", stdPath.getUri(), remoteName);
        byte[] buff = new byte[8096];
        int size = 0;
        int maxLimit = configProvider.getActiveConfig().getMaxFileLimit(stdPath);
        FileProcess fileProcess = new LocalFileProcess(tempRoot, remoteName);
        boolean fastEnd = false;
        while (true) {
            int len = inputStream.read(buff);
            if (len == -1) {
                break;
            }
            if (len > 0) {
                size += len;
            }
            if (maxLimit > 0 && size > maxLimit) {
                fastEnd = true;
                break;
            }
            ByteBuffer byteBuffer = ByteBuffer.wrap(buff, 0, len);
            fileProcess.accept(byteBuffer);
        }
        if (fastEnd) {
            fileProcess.clean();
            logger.info("exceed max size file from http request:{}, remote file {}", stdPath.getUri(), remoteName);
            resultOutput.doExceedResult(response, stdPath);
            return null;
        }
        fileProcess.complete();
        logger.info("end to read file from http request:{}, remote file {},local file: {}", stdPath.getUri(), remoteName, fileProcess.getLocalFileName());
        return fileProcess;
    }
}
