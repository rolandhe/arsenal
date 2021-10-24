package com.github.rolandhe.fileserver.ctrl.mapper;

import com.github.rolandhe.fileserver.ctrl.entity.UploadFile;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

@Mapper
public interface UploadFileMapper {
    void addUploadFile(UploadFile uploadFile);

    int deleteLogic(@Param("id") long id);

    int updateStatus(@Param("id") long id, @Param("status") int status);

    int updateStatusWithName(@Param("fileName") String fileName, @Param("nameHash") long nameHash, @Param("id") long id, @Param("status") int status);

    UploadFile findById(@Param("id") long id);

    UploadFile findByNameHash(@Param("nameHash") long hash);
}
