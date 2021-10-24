package com.github.rolandhe.fileserver.login;

import com.github.rolandhe.fileserver.std.login.LoginValidator;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Component;

@Component
public class LoginValidatorProvider implements ApplicationContextAware {

    private ApplicationContext applicationContext;

    @Value("${file-server.login-validate-bean-name}")
    private String activeLoginValidateBeanName;

    private volatile LoginValidator activeLoginValidator;


    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.applicationContext = applicationContext;
    }

    public LoginValidator getActiveLoginValidator() {
        if(activeLoginValidator != null) {
            return activeLoginValidator;
        }
        activeLoginValidator = applicationContext.getBean(activeLoginValidateBeanName, LoginValidator.class);
        return activeLoginValidator;
    }
}
