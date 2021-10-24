package com.github.rolandhe.fileserver.ctrl.service.impl;

import com.github.rolandhe.fileserver.config.ConfigProvider;
import com.github.rolandhe.fileserver.ctrl.service.ResultOutput;
import com.github.rolandhe.fileserver.std.ResultParams;
import com.github.rolandhe.fileserver.std.StdPath;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

@Service
public class ResultOutputImpl implements ResultOutput {
    private static final Logger logger = LoggerFactory.getLogger(ResultOutputImpl.class);

    @Resource
    private ConfigProvider configProvider;
    @Override
    public void doFailResult(HttpServletResponse response, StdPath stdPath) {
        String message = configProvider.getActiveConfig().getFailReturnMessage(stdPath);
        try {
            response.getWriter().append(message);
            response.getWriter().close();
        } catch (IOException e) {
            logger.info(stdPath.getUri(), e);
        }
    }

    @Override
    public void doExceedMaxConcurrentResult(HttpServletResponse response, StdPath stdPath) {
        String message = configProvider.getActiveConfig().getExceedMaxConcurrentReturnMessage(stdPath);
        try {
            response.getWriter().append(message);
            response.getWriter().close();
        } catch (IOException e) {
            logger.info(stdPath.getUri(), e);
        }
    }

    @Override
    public void doInternalResult(HttpServletResponse response, StdPath stdPath) {
        String message = configProvider.getActiveConfig().getInternalErrReturnMessage(stdPath);
        try {
            response.getWriter().append(message);
        } catch (IOException e) {
            logger.info(stdPath.getUri(), e);
        }
    }

    @Override
    public void doExceedResult(HttpServletResponse response, StdPath stdPath) {
        String message = configProvider.getActiveConfig().getExceedReturnMessage(stdPath);
        try {
            response.getWriter().append(message);
            response.getWriter().close();
        } catch (IOException e) {
            logger.info(stdPath.getUri(), e);
        }
    }

    @Override
    public void doSuccessResult(HttpServletResponse response, StdPath stdPath, String targetName) {
        String message = configProvider.getActiveConfig().getSuccessReturnMessage(stdPath);
        message = StringUtils.replace(message, ResultParams.UPLOADED_FILE_URL,targetName);
        try {
            response.getWriter().append(message);
        } catch (IOException e) {
            logger.info(stdPath.getUri(), e);
        }
    }
}
