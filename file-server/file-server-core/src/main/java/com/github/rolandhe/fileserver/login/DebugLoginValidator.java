package com.github.rolandhe.fileserver.login;

import com.github.rolandhe.fileserver.std.login.LoginValidator;
import org.springframework.stereotype.Service;

import javax.servlet.http.HttpServletResponse;

@Service("debugLoginValidator")
public class DebugLoginValidator  implements LoginValidator {
    @Override
    public String getLoginUser() {
        return "default-user";
    }

    @Override
    public String loginNext(HttpServletResponse response) {
        return null;
    }
}
