package com.github.rolandhe.fileserver.std.config;


import com.github.rolandhe.fileserver.std.StdPath;

public interface FileServerConfig {
    String getFailReturnMessage(StdPath stdPath);
    String getExceedMaxConcurrentReturnMessage(StdPath stdPath);
    String getInternalErrReturnMessage(StdPath stdPath);
    String getExceedReturnMessage(StdPath stdPath);
    String getSuccessReturnMessage(StdPath stdPath);
    int getMaxFileLimit(StdPath stdPath);
    String getConfigSenderRuleName(StdPath stdPath);
}