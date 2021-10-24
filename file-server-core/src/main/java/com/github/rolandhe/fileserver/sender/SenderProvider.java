package com.github.rolandhe.fileserver.sender;

import com.github.rolandhe.fileserver.std.sender.FileSender;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import javax.annotation.Resource;
import java.util.HashMap;
import java.util.Map;

@Component
public class SenderProvider {
    @Resource
    private FileSender[] fileSenders;

    @Resource

    private Map<String,FileSender> fileSenderMap = new HashMap<>();

    @PostConstruct
    public void init() {
        for(FileSender fileSender : fileSenders) {
            fileSenderMap.put(fileSender.getSenderRuleName(),fileSender);
        }
    }
    public FileSender matchFileSender(String senderRole) {
        return fileSenderMap.get(senderRole);
    }
}
