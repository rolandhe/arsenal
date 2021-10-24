package com.github.rolandhe.fileserver.servlet.local;

import java.io.IOException;
import java.nio.ByteBuffer;

public interface FileProcess {
    void accept(ByteBuffer byteBuffer) throws IOException;
    void complete() throws IOException;
    void clean();
    String getLocalFileName();
}
