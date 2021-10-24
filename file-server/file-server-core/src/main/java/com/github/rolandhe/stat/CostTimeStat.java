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

    public void endStat(String prefix) {
        logger.info("{}, cost {} ms", prefix, System.currentTimeMillis() - startTime);
    }
}
