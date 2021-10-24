package com.github.rolandhe.fileserver.config;

import com.github.rolandhe.fileserver.std.config.FileServerConfig;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Component;

@Component
public class ConfigProvider implements ApplicationContextAware {

    private ApplicationContext applicationContext;

    @Value("${file-server.config-bean-name}")
    private String activeConfigBeanName;

    private volatile FileServerConfig activeConfig;


    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.applicationContext = applicationContext;
    }

    public FileServerConfig getActiveConfig() {
        if(activeConfig != null) {
            return activeConfig;
        }
        activeConfig = applicationContext.getBean(activeConfigBeanName,FileServerConfig.class);
        return activeConfig;
    }
}
