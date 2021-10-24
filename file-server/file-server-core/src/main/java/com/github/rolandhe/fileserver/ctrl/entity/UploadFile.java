package com.github.rolandhe.fileserver.ctrl.entity;

import java.time.LocalDateTime;

public class UploadFile {
    private long id;
    private String product;
    private String fileName;
    private String uploadUser;
    private long nameHash;
    private LocalDateTime uploadTime;
    private LocalDateTime procTime;
    private int procStatus;
    private boolean deleted;

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public String getProduct() {
        return product;
    }

    public void setProduct(String product) {
        this.product = product;
    }

    public String getFileName() {
        return fileName;
    }

    public void setFileName(String fileName) {
        this.fileName = fileName;
    }

    public String getUploadUser() {
        return uploadUser;
    }

    public void setUploadUser(String uploadUser) {
        this.uploadUser = uploadUser;
    }

    public LocalDateTime getUploadTime() {
        return uploadTime;
    }

    public void setUploadTime(LocalDateTime uploadTime) {
        this.uploadTime = uploadTime;
    }

    public LocalDateTime getProcTime() {
        return procTime;
    }

    public void setProcTime(LocalDateTime procTime) {
        this.procTime = procTime;
    }



    public boolean isDeleted() {
        return deleted;
    }

    public void setDeleted(boolean deleted) {
        this.deleted = deleted;
    }

    public long getNameHash() {
        return nameHash;
    }

    public void setNameHash(long nameHash) {
        this.nameHash = nameHash;
    }

    public int getProcStatus() {
        return procStatus;
    }

    public void setProcStatus(int procStatus) {
        this.procStatus = procStatus;
    }
}
