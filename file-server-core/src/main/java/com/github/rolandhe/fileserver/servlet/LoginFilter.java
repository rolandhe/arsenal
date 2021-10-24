package com.github.rolandhe.fileserver.servlet;

import com.github.rolandhe.fileserver.consts.ConstValues;
import com.github.rolandhe.fileserver.login.LoginValidatorProvider;
import org.apache.commons.lang3.StringUtils;

import javax.annotation.Resource;
import javax.servlet.*;
import javax.servlet.annotation.WebFilter;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

@WebFilter(urlPatterns = "/fs/*", asyncSupported = true)
public class LoginFilter implements Filter {
    @Resource
    private LoginValidatorProvider loginValidatorProvider;

    @Override
    public void doFilter(ServletRequest servletRequest, ServletResponse servletResponse, FilterChain filterChain) throws IOException, ServletException {
        String user = loginValidatorProvider.getActiveLoginValidator().getLoginUser();
        if (!StringUtils.isEmpty(user)) {
            servletRequest.setAttribute(ConstValues.LOGIN_VAR_NAME, user);
            try {
                filterChain.doFilter(servletRequest, servletResponse);
            } finally {
                servletRequest.removeAttribute(ConstValues.LOGIN_VAR_NAME);
            }
        } else {
            loginValidatorProvider.getActiveLoginValidator().loginNext((HttpServletResponse) servletResponse);
        }
    }
}
