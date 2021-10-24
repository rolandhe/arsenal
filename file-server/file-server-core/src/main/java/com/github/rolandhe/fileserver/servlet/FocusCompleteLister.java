package com.github.rolandhe.fileserver.servlet;

import javax.servlet.AsyncEvent;
import javax.servlet.AsyncListener;
import java.io.IOException;

abstract class FocusCompleteLister implements AsyncListener {

    @Override
    public void onTimeout(AsyncEvent asyncEvent) throws IOException {

    }

    @Override
    public void onError(AsyncEvent asyncEvent) throws IOException {

    }

    @Override
    public void onStartAsync(AsyncEvent asyncEvent) throws IOException {

    }
}
