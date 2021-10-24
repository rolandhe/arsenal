package com.github.rolandhe.fileserver.ctrl.service;

public enum UploadProcStatus {
    INIT(0),FINISH(1);
    private final int status;

    UploadProcStatus(int status) {
        this.status = status;
    }

    public int getStatus() {
        return status;
    }
}
