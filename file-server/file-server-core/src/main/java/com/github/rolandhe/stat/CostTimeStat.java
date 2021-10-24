package com.github.rolandhe.stat;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class CostTimeStat {
    private static final Logger owerLogger = LoggerFactory.getLogger(CostTimeStat.class);
    private final long startTime = System.currentTimeMillis();
    private final Logger logger;

    private CostTimeStat() {
        logger = owerLogger;
    }

    private CostTimeStat(Logger logger) {
        this.logger = logger;
    }

    public static CostTimeStat startStat() {
        return new CostTimeStat();
    }

    public static CostTimeStat startStat(Logger logger) {
        return new CostTimeStat(logger);
    }

    public void endStat(String... prefix) {
        if(prefix.length == 0) {
            logger.info("cost {} ms", prefix, System.currentTimeMillis() - startTime);
            return;
        }
        Object[] vars = new Object[prefix.length + 1];
        int index = 0;
        StringBuilder stringBuilder = new StringBuilder();
        for(String p: prefix) {
            stringBuilder.append("{} ");
            vars[index++] = p;
        }
        vars[index] = System.currentTimeMillis() - startTime;
        stringBuilder.deleteCharAt(stringBuilder.length() - 1);
        stringBuilder.append(",").append("cost {} ms.");
        logger.info(stringBuilder.toString(), vars);
    }
}
