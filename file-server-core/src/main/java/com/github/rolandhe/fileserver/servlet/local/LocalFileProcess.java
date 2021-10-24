package com.github.rolandhe.fileserver.servlet.local;

import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.UUID;

public class LocalFileProcess implements FileProcess {
    private static final Logger logger = LoggerFactory.getLogger(LocalFileProcess.class);

    private final String root;
    private final String remoteFileName;
    private FileChannel fileChannel;
    private String localFilePath;

    public LocalFileProcess(String root,String remoteFileName) {
        this.root = root;
        this.remoteFileName = remoteFileName;
    }

    @Override
    public void accept(ByteBuffer byteBuffer) throws IOException {
        if (fileChannel == null) {
            Path path = Paths.get(root, UUID.randomUUID().toString() + getExtName());
            fileChannel = FileChannel.open(path, StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.APPEND);
            localFilePath = path.toString();
        }
        fileChannel.write(byteBuffer);
    }

    @Override
    public void complete() throws IOException {
        fileChannel.close();
    }

    @Override
    public void clean() {
        if (localFilePath == null) {
            return;
        }
        try {
            FileUtils.forceDelete(new File(localFilePath));
        } catch (IOException e) {
            logger.info(localFilePath, e);
        }
        localFilePath = null;
    }

    @Override
    public String getLocalFileName() {
        return localFilePath;
    }

    private String getExtName() {
        if(remoteFileName.endsWith(".tar.gz")){
            return ".tar.gz";
        }
        int pos = remoteFileName.lastIndexOf('.');
        if(pos < 0) {
            return "";
        }
        return remoteFileName.substring(pos);
    }
}
