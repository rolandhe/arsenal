//
// Created by hexiufeng on 2022/5/18.
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

#ifndef FFMPEG_TOOLS_DATA_STRUCT_H
#define FFMPEG_TOOLS_DATA_STRUCT_H

#define COPY_VALUE -2
#define UNKNOWN  -1



typedef struct _FilterContext
{
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
    AVFrame  * filter_frame;
} FilterContext;

typedef struct _InStreamContext
{
    AVStream *stream;
    AVCodecContext *codec_ctx;
    enum AVMediaType codec_type;
    AVFrame *frame;
    FilterContext* filter_context;
} InStreamContext;



typedef struct _InContext
{
    AVFormatContext *ic;
    AVPacket *in_packet;
    int stream_count;
    InStreamContext *in_stream_ctx;
    const char *file_name;
} InContext;

typedef struct _OutStreamContext
{
    // 会被自动释放
    AVStream *stream;
    AVCodecContext *codec_ctx;
    int stream_copy;
    int out;
    int out_stream_index;
} OutStreamContext;

typedef struct _OutContext
{
    AVFormatContext *oc;
    int stream_count;
    OutStreamContext *out_stream_ctx;
    const char *file_name;
    AVPacket *out_packet;
} OutContext;



typedef struct _Parameters {
    int r; // 帧率
    int ar; // 音频采样率
    int ac; // 音频channel
    int vcodec;
    int acodec;
    int an;
    int vn;
    int sn;
    const char * in_name;
    const char * out_name;
    const char * audio_filter;
    const char * video_filter;
} Parameters;

typedef void (*set_para_callback)(AVCodecContext *, AVCodecContext *, Parameters *);

FilterContext * create_filter_context();
void filter_context_free(FilterContext* filter_context);

Parameters *create_parameters();
void set_file_names(Parameters * parameters,const char * in,const char * out);

void set_file_filters(Parameters * parameters,const char * audio,const char * video);

void reset_parameters_r(Parameters * parameters);


void free_parameters(Parameters *p);

InContext *create_in_context(Parameters *parameters);

void in_context_free(InContext *in_ctx);

void in_stream_context_free(InStreamContext *in_stream_ctx);

OutContext *create_out_context(Parameters *parameters, const InContext *in_ctx);

void out_context_free(OutContext *out_ctx);

void out_stream_context_free(OutStreamContext *out_stream_ctx);


int open_out_stream(InContext *in_ctx, OutContext *out_ctx, Parameters *parameters, int stream_index,
                    set_para_callback set_param);

void copy_stream_meta(AVStream *in_stream, AVStream *out_stream);

// 0 is success
int open_in_coder_ctx(AVFormatContext *ic, AVFormatContext *oc, InStreamContext *in_stream_ctx);

// 0 is success
int
copy_packet(AVFormatContext *oc, InStreamContext *in_stream_ctx, OutStreamContext *out_stream_ctx, AVPacket *in_packet);

#endif //FFMPEG_TOOLS_DATA_STRUCT_H
