package com.github.rolandhe.fileserver.std.login;

import javax.servlet.http.HttpServletResponse;

public interface LoginValidator {
    String getLoginUser();
    String loginNext(HttpServletResponse response);
}
