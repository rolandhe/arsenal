package com.github.rolandhe.fileserver.config;

import com.github.rolandhe.fileserver.consts.ConstValues;
import com.github.rolandhe.fileserver.std.StdPath;
import com.github.rolandhe.fileserver.std.config.FileServerConfig;
import org.springframework.stereotype.Service;

@Service("simpleFsConfig")
public class SimpleFileServerConfig implements FileServerConfig {
    @Override
    public String getFailReturnMessage(StdPath stdPath) {
        return "meet error with " + stdPath.getUri();
    }

    @Override
    public String getExceedMaxConcurrentReturnMessage(StdPath stdPath) {
        return  "exceed max concurrent error with " + stdPath.getUri();
    }

    @Override
    public String getInternalErrReturnMessage(StdPath stdPath) {
        return "meet internal error with " + stdPath.getUri();
    }

    @Override
    public String getExceedReturnMessage(StdPath stdPath) {
        return "meet exceed max file with " + stdPath.getUri();
    }

    @Override
    public String getSuccessReturnMessage(StdPath stdPath) {
        return "${uploaded.file.url}";
    }

    @Override
    public int getMaxFileLimit(StdPath stdPath) {
        return -1;
    }

    @Override
    public String getConfigSenderRuleName(StdPath stdPath) {
        return ConstValues.DEFAULT_ROLE_NAME;
    }
}
