//
// Created by hexiufeng on 2022/5/18.
//
#include "common.h"

#ifndef FFMPEG_TOOLS_TRANSCODING_H
#define FFMPEG_TOOLS_TRANSCODING_H


int init_filters(InContext * in_ctx,OutContext * out_ctx,const char * audio_filter_desc, const char * video_filter_desc);

// 0 is success
int do_process(InContext *in_ctx, OutContext *out_ctx, Parameters *parameters);

// 0 is success
int process_audio(OutContext *out_ctx, InStreamContext *in_stream_ctx,
                  OutStreamContext *out_stream_ctx, AVPacket *in_packet);

// 0 is success
int process_video(OutContext *out_ctx,
                  InStreamContext *in_stream_ctx, OutStreamContext *out_stream_ctx,
                  AVPacket *in_packet);

// 0 is success
int re_codec(AVPacket *in_packet, OutStreamContext *out_stream_ctx, InStreamContext *in_stream_ctx, OutContext *out_ctx);

// 0 is success
int filter_write_encode_frame(InStreamContext *in_stream_ctx, AVFrame *dec_frame, OutContext *out_ctx, OutStreamContext *out_stream_ctx);

int write_encode_frame(FilterContext * filter_ctx,  OutContext *out_ctx, OutStreamContext *out_stream_ctx,int flush);

int flush_encode(InContext * in_ctx, OutContext * out_ctx);

#endif //FFMPEG_TOOLS_TRANSCODING_H
