package com.github.rolandhe.fileserver.std.sender;

public interface FileSender {
    String getSenderRuleName();
    String send(String fileName);
}
