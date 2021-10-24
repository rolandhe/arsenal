package com.github.rolandhe.fileserver.sender;

import com.github.rolandhe.fileserver.consts.ConstValues;
import com.github.rolandhe.fileserver.std.sender.FileSender;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.File;
import java.io.IOException;
import java.nio.file.Paths;

@Service
public class DefaultFileSender implements FileSender {
    private static final Logger logger = LoggerFactory.getLogger(DefaultFileSender.class);

    @Value("${file-server.default-store-root}")
    private String storeRoot;

    @Override
    public String getSenderRuleName() {
        return ConstValues.DEFAULT_ROLE_NAME;
    }

    @Override
    public String send(String fileName) {
        String rawFileName = Paths.get(fileName).getFileName().toString();
        String targetPath = String.format("%s/%s", storeRoot, rawFileName);
        try {
            FileUtils.copyFile(new File(fileName), new File(targetPath));
        } catch (IOException e) {
            logger.info(fileName, e);
            return null;
        }
        return rawFileName;
    }
}
