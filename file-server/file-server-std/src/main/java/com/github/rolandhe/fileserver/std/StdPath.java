package com.github.rolandhe.fileserver.std;

import org.apache.commons.lang3.StringUtils;

public class StdPath {
    private String product;
    private String subCate;

    private final String uri;

    private StdPath(String uri) {
        this.uri = uri;
    }

    public String getProduct() {
        return product;
    }

    private void setProduct(String product) {
        this.product = product;
    }

    public String getSubCate() {
        return subCate;
    }

    private void setSubCate(String subCate) {
        this.subCate = subCate;
    }

    public String getUri() {
        return uri;
    }

    public static boolean validPath(String path) {
        String[] items = StringUtils.split(path, '/');
        if (items.length < 2) {
            return false;
        }
        return true;
    }

    public static StdPath parse(String path) {
        String[] items = StringUtils.split(path, '/');
        StdPath stdPath = new StdPath(path);
        if (items.length == 2) {
            stdPath.setProduct("default-product");
        }
        if (items.length >= 3) {
            stdPath.setProduct(items[3]);
        }
        if (items.length >= 4) {
            stdPath.setSubCate(items[4]);
        }
        return stdPath;
    }
}
