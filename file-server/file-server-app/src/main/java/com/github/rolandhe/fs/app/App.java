package com.github.rolandhe.fs.app;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.web.servlet.ServletComponentScan;
import org.springframework.context.annotation.ComponentScan;

/**
 * Hello world!
 *
 */
@SpringBootApplication
@MapperScan("com.github.rolandhe.fileserver.ctrl.mapper")
@ServletComponentScan("com.github.rolandhe.fileserver")
@ComponentScan("com.github.rolandhe.fileserver")
public class App {
    public static void main( String[] args ) {
        SpringApplication.run(App.class, args);
    }
}
