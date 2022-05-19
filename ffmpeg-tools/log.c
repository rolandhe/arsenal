//
// Created by hexiufeng on 2022/5/18.
//
#include "log.h"

#define LOCAL 1
void ff_log(const char * src, int level, int code, const char * message,int from_ff)
{
#ifdef LOCAL
        if(AV_LOG_TRACE == level){
            return;
        }
        av_log(NULL, level, "%s,code=%d,from ffmpeg:%d\n",message, code,from_ff);
#else

#endif
}