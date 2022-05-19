package com.github.rolandhe.fileserver.ctrl.service.impl;

import com.github.rolandhe.fileserver.config.ConfigProvider;
import com.github.rolandhe.fileserver.ctrl.entity.UploadFile;
import com.github.rolandhe.fileserver.ctrl.mapper.UploadFileMapper;
import com.github.rolandhe.fileserver.ctrl.service.UploadFlow;
import com.github.rolandhe.fileserver.ctrl.service.UploadProcStatus;
import com.github.rolandhe.fileserver.sender.SenderProvider;
import com.github.rolandhe.fileserver.std.StdPath;
import com.github.rolandhe.fileserver.std.sender.FileSender;
import com.github.rolandhe.hash.Hashable;
import com.github.rolandhe.hash.cityhash.CityHash;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import javax.annotation.Resource;
import java.nio.file.Paths;
import java.time.LocalDateTime;

@Service
public class UploadFlowImpl implements UploadFlow {
    private static final Logger logger = LoggerFactory.getLogger(UploadFlowImpl.class);

    private static final Hashable HASHABLE = new CityHash();

    @Resource
    private ConfigProvider configProvider;
    @Resource
    private SenderProvider senderProvider;

    @Resource
    private UploadFileMapper uploadFileMapper;

    @Override
    public String control(StdPath stdPath, String localFile, String uploadUser) {
        UploadFile uploadFile = initFile(stdPath, localFile, uploadUser);
        logger.info("insert db to init upload file,{}, from {}", localFile, stdPath.getUri());
        String senderRole = configProvider.getActiveConfig().getConfigSenderRuleName(stdPath);
        FileSender fileSender = senderProvider.matchFileSender(senderRole);
        String target = null;
        try {
            target = fileSender.send(localFile);
        } catch (RuntimeException e) {
            uploadFileMapper.deleteLogic(uploadFile.getId());
            throw e;
        }
        String rawFileName = Paths.get(localFile).getFileName().toString();
        logger.info("send {} to target,{}, from {}", localFile, target, stdPath.getUri());
        if (!rawFileName.equals(target)) {
            uploadFileMapper.updateStatusWithName(target, HASHABLE.hash64(target), uploadFile.getId(), UploadProcStatus.FINISH.getStatus());
        } else {
            uploadFileMapper.updateStatus(uploadFile.getId(), UploadProcStatus.FINISH.getStatus());
        }
        logger.info("update status {} to target,{}, from {}", localFile, target, stdPath.getUri());
        return target;
    }

    private UploadFile initFile(StdPath stdPath, String localPath, String uploadUser) {
        UploadFile uploadFile = new UploadFile();
        uploadFile.setProduct(stdPath.getUri());
        uploadFile.setFileName(localPath);
        uploadFile.setUploadUser(uploadUser);
        uploadFile.setNameHash(HASHABLE.hash64(localPath));
        uploadFile.setUploadTime(LocalDateTime.now());
        uploadFile.setProcTime(LocalDateTime.now());
        uploadFile.setProcStatus(UploadProcStatus.INIT.getStatus());
        uploadFile.setDeleted(false);
        uploadFileMapper.addUploadFile(uploadFile);
        return uploadFile;
    }
}
