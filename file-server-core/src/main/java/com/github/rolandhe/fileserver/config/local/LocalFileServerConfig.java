package com.github.rolandhe.fileserver.config.local;

import com.github.rolandhe.fileserver.std.StdPath;
import com.github.rolandhe.fileserver.std.config.FileServerConfig;
import org.springframework.stereotype.Service;

@Service("localFsConfig")
public class LocalFileServerConfig implements FileServerConfig {
    @Override
    public String getFailReturnMessage(StdPath stdPath) {
        return null;
    }

    @Override
    public String getInternalErrReturnMessage(StdPath stdPath) {
        return null;
    }

    @Override
    public String getExceedReturnMessage(StdPath stdPath) {
        return null;
    }

    @Override
    public String getSuccessReturnMessage(StdPath stdPath) {
        return null;
    }

    @Override
    public int getMaxFileLimit(StdPath stdPath) {
        return 0;
    }

    @Override
    public String getConfigSenderRoleName(StdPath stdPath) {
        return null;
    }
}
