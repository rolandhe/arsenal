package com.github.rolandhe.fileserver.ctrl.service;

import com.github.rolandhe.fileserver.std.StdPath;

public interface UploadFlow {
    String control(StdPath stdPath, String localFile, String uploadUser);
}
