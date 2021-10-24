package com.github.rolandhe.fileserver.ctrl.service;

import com.github.rolandhe.fileserver.std.StdPath;

import javax.servlet.http.HttpServletResponse;

public interface ResultOutput {
    void doFailResult(HttpServletResponse response, StdPath stdPath);
    void doExceedMaxConcurrentResult(HttpServletResponse response, StdPath stdPath);
    void doInternalResult(HttpServletResponse response, StdPath stdPath);
    void doExceedResult(HttpServletResponse response, StdPath stdPath);
    void doSuccessResult(HttpServletResponse response, StdPath stdPath, String targetName);
}
