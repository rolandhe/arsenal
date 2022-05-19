//
// Created by hexiufeng on 2022/5/18.
//

#include "transcoding.h"
#include "log.h"

// av_buffersink_get_frame_rate
static int init_filter(FilterContext *filter_context, AVCodecContext *dec_ctx, OutStreamContext *out_stream_ctx,
                       const char *filter_spec) {
    char args[512];

    const AVFilter *buffersrc = NULL;
    const AVFilter *buffersink = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVFilterGraph *filter_graph = avfilter_graph_alloc();
    AVCodecContext *enc_ctx = out_stream_ctx->codec_ctx;

    int ret = 0;
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }


    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        buffersrc = avfilter_get_by_name("buffer");
        buffersink = avfilter_get_by_name("buffersink");
        if (!buffersrc || !buffersink) {
            ff_log("init_filter",AV_LOG_ERROR,AVERROR_UNKNOWN,"filtering source or sink element not found",1);
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                 dec_ctx->time_base.num, dec_ctx->time_base.den,
                 dec_ctx->sample_aspect_ratio.num,
                 dec_ctx->sample_aspect_ratio.den);

        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                           args, NULL, filter_graph);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot create buffer source",1);
            goto end;
        }

        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                           NULL, NULL, filter_graph);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot create buffer sink",1);
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
                             (uint8_t *) &enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),
                             AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot set output pixel format",1);
            goto end;
        }


    } else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        buffersrc = avfilter_get_by_name("abuffer");
        buffersink = avfilter_get_by_name("abuffersink");
        if (!buffersrc || !buffersink) {
            ff_log("init_filter",AV_LOG_ERROR,AVERROR_UNKNOWN,"filtering source or sink element not found",1);
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        if (!dec_ctx->channel_layout)
            dec_ctx->channel_layout =
                    av_get_default_channel_layout(dec_ctx->channels);
        snprintf(args, sizeof(args),
                 "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
                 dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
                 av_get_sample_fmt_name(dec_ctx->sample_fmt),
                 dec_ctx->channel_layout);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                           args, NULL, filter_graph);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot create audio buffer source",1);
            goto end;
        }

        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                           NULL, NULL, filter_graph);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot create audio buffer sink",1);
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
                             (uint8_t *) &enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
                             AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot set output sample format",1);
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
                             (uint8_t *) &enc_ctx->channel_layout,
                             sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot set output channel layout",1);
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
                             (uint8_t *) &enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
                             AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            ff_log("init_filter",AV_LOG_ERROR,ret,"Cannot set output sample rate",1);
            goto end;
        }
    } else {
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    /* Endpoints for the filter graph. */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if (!outputs->name || !inputs->name) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;


    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

    if (filter_graph->nb_filters) {
        for (int i = 0; i < filter_graph->nb_filters; i++) {
            AVFilterContext *afc = filter_graph->filters[i];
            if (strcmp(afc->filter->name, "fps") == 0) {
                for (int j = 0; j < afc->nb_outputs; j++) {
                    AVRational frame_rate = afc->outputs[j]->frame_rate;
                    AVRational time_base = afc->outputs[j]->time_base;
                    out_stream_ctx->codec_ctx->framerate = frame_rate;
                    out_stream_ctx->codec_ctx->time_base = time_base;
                    out_stream_ctx->stream->time_base = time_base;
                }
                continue;
            }

            if (strcmp(afc->filter->name, "aresample") == 0 && !strstr(afc->name,"auto_resampler_")) {
                for (int j = 0; j < afc->nb_outputs; j++) {
                    int sample_rate = afc->outputs[j]->sample_rate;
                    int chs = afc->outputs[j]->channels;
                    int ch_layout = afc->outputs[j]->channel_layout;
                    AVRational time_base = afc->outputs[j]->time_base;
                    out_stream_ctx->codec_ctx->sample_rate = sample_rate;
                    out_stream_ctx->codec_ctx->time_base = time_base;
                    out_stream_ctx->stream->time_base = time_base;
                    out_stream_ctx->codec_ctx->channel_layout = ch_layout;
                    out_stream_ctx->codec_ctx->channels = chs;

                }
            }
        }
    }
    end:
    /* Fill FilteringContext */
    filter_context->filter_graph = filter_graph;
    filter_context->buffersrc_ctx = buffersrc_ctx;
    filter_context->buffersink_ctx = buffersink_ctx;

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

int init_filters(InContext *in_ctx, OutContext *out_ctx, const char *audio_filter_desc, const char *video_filter_desc) {
    if (!in_ctx || !in_ctx->ic || !in_ctx->stream_count) {
        return -1;
    }
    for (int i = 0; i < in_ctx->stream_count; i++) {
        InStreamContext *in_stream_ctx = in_ctx->in_stream_ctx + i;
        OutStreamContext *out_stream_ctx = out_ctx->out_stream_ctx + i;
        if (!out_stream_ctx->out || out_stream_ctx->stream_copy) continue;
        if (in_stream_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            in_stream_ctx->filter_context = create_filter_context();
            init_filter(in_stream_ctx->filter_context, in_stream_ctx->codec_ctx, out_stream_ctx,
                        !video_filter_desc ? "null" : video_filter_desc);
            continue;
        }

        if (in_stream_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            in_stream_ctx->filter_context = create_filter_context();
            init_filter(in_stream_ctx->filter_context, in_stream_ctx->codec_ctx, out_stream_ctx,
                        !audio_filter_desc ? "anull" : audio_filter_desc);
            continue;
        }
    }

    return 0;
}

// 0 is success
int do_process(InContext *in_ctx, OutContext *out_ctx, Parameters *parameters) {
    int ret = init_filters(in_ctx, out_ctx, parameters->audio_filter, parameters->video_filter);
    if (ret < 0) {
        ff_log("do_process",AV_LOG_ERROR,ret,"Error occurred when init_filters",0);
        return ret;
    }
    ret = avformat_write_header(out_ctx->oc, NULL);
    if (ret < 0) {
        ff_log("do_process",AV_LOG_ERROR,ret,"Error occurred when opening output file",0);
        return ret;
    }

    while (1) {
        AVPacket *in_packet = in_ctx->in_packet;
        ret = av_read_frame(in_ctx->ic, in_packet);
        if (ret < 0) {
            break;
        }

        InStreamContext *in_stream_ctx = in_ctx->in_stream_ctx + in_packet->stream_index;
        OutStreamContext *out_stream_ctx = out_ctx->out_stream_ctx + in_packet->stream_index;
        if (!out_stream_ctx->out) {
            av_packet_unref(in_packet);
            continue;
        }
        if (in_stream_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            ret = process_audio( out_ctx, in_stream_ctx, out_stream_ctx, in_packet);
            av_packet_unref(in_packet);
            if (ret) {
                return ret;
            }
            continue;
        }

        if (in_stream_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            ret = process_video(out_ctx, in_stream_ctx, out_stream_ctx, in_packet);
            av_packet_unref(in_packet);
            if (ret) {
                return ret;
            }
            continue;
        }

        if (in_stream_ctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            ret = copy_packet(out_ctx->oc, in_stream_ctx, out_stream_ctx, in_packet);
            if (ret) {
                av_packet_unref(in_packet);
                return ret;
            }

            av_packet_unref(in_packet);
            continue;
        }
        ret = copy_packet(out_ctx->oc, in_stream_ctx, out_stream_ctx, in_packet);
        if (ret) {
            av_packet_unref(in_packet);
            return ret;
        }
        av_packet_unref(in_packet);
    }

    ret = flush_encode(in_ctx, out_ctx);
    if (ret) {
        return ret;
    }
    av_write_trailer(out_ctx->oc);
    return ret;
}

// 0 is success
int process_audio(OutContext *out_ctx, InStreamContext *in_stream_ctx,
                  OutStreamContext *out_stream_ctx, AVPacket *in_packet) {
    if (out_stream_ctx->stream_copy) {
        return copy_packet(out_ctx->oc, in_stream_ctx, out_stream_ctx, in_packet);
    }

    return re_codec(in_packet, out_stream_ctx, in_stream_ctx, out_ctx);
}


// 0 is success
int process_video(OutContext *out_ctx,
                  InStreamContext *in_stream_ctx, OutStreamContext *out_stream_ctx,
                  AVPacket *in_packet) {
    if (out_stream_ctx->stream_copy) {
        return copy_packet(out_ctx->oc, in_stream_ctx, out_stream_ctx, in_packet);
    }

    return re_codec(in_packet, out_stream_ctx, in_stream_ctx, out_ctx);
}

// 0 is success
int re_codec( AVPacket *in_packet, OutStreamContext *out_stream_ctx, InStreamContext *in_stream_ctx,
             OutContext *out_ctx) {
    AVStream *in_stream = in_stream_ctx->stream;
    AVCodecContext *dec_ctx = in_stream_ctx->codec_ctx;

    // 解码
    av_packet_rescale_ts(in_packet, in_stream->time_base, dec_ctx->time_base);
    int ret = avcodec_send_packet(dec_ctx, in_packet);
    if (ret < 0) {
        ff_log("re_codec",AV_LOG_ERROR,ret,"Decoding failed",1);
        return ret;
    }

    AVFrame *dec_frame = in_stream_ctx->frame;

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, dec_frame);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
            return 0;
        } else if (ret < 0) {
            return ret;
        }
        dec_frame->pts = dec_frame->best_effort_timestamp;
        // printf("got decode frame of %d\n",packet_index);
        ret = filter_write_encode_frame(in_stream_ctx, dec_frame, out_ctx, out_stream_ctx);
        av_frame_unref(dec_frame);
    }
    return ret;
}

// 0 is success
int filter_write_encode_frame(InStreamContext *in_stream_ctx, AVFrame *dec_frame, OutContext *out_ctx,
                              OutStreamContext *out_stream_ctx) {

    FilterContext *filter_ctx = in_stream_ctx->filter_context;

    int ret = av_buffersrc_add_frame_flags(filter_ctx->buffersrc_ctx,
                                           dec_frame, 0);
    if (ret < 0) {
        ff_log("filter_write_encode_frame",AV_LOG_ERROR,ret,"Error while feeding the filtergraph",1);
        return ret;
    }

    while (1) {
        ff_log("filter_write_encode_frame",AV_LOG_TRACE,0,"Pulling filtered frame from filters",0);
        ret = av_buffersink_get_frame(filter_ctx->buffersink_ctx,
                                      filter_ctx->filter_frame);
        if (ret < 0) {
            /* if no more frames for output - returns AVERROR(EAGAIN)
             * if flushed and no more frames for output - returns AVERROR_EOF
             * rewrite retcode to 0 to show it as normal procedure completion
             */
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                ret = 0;
            break;
        }

        if (in_stream_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            filter_ctx->filter_frame->pict_type = AV_PICTURE_TYPE_NONE;
        }
        ret = write_encode_frame(filter_ctx, out_ctx, out_stream_ctx, 0);
        av_frame_unref(filter_ctx->filter_frame);
        if (ret < 0)
            break;
    }

    return ret;


}

int write_encode_frame(FilterContext *filter_ctx, OutContext *out_ctx, OutStreamContext *out_stream_ctx, int flush) {

    AVCodecContext *enc_ctx = out_stream_ctx->codec_ctx;
    AVFrame *filter_frame = flush ? NULL : filter_ctx->filter_frame;

    // encode
    int ret = avcodec_send_frame(enc_ctx, filter_frame);

    if (ret) {
        return ret;
    }

    AVPacket *out_packet = out_ctx->out_packet;

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, out_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }

        out_packet->stream_index = out_stream_ctx->out_stream_index;

        av_packet_rescale_ts(out_packet,
                             enc_ctx->time_base,
                             out_stream_ctx->stream->time_base);

        ff_log("write_encode_frame",AV_LOG_TRACE,0,"Muxing frame",0);
        /* mux encoded frame */
        ret = av_interleaved_write_frame(out_ctx->oc, out_packet);
        av_packet_unref(out_packet);
    }

    return ret;
}


int flush_encode(InContext *in_ctx, OutContext *out_ctx) {
    int ret = 0;
    for (int i = 0; i < in_ctx->stream_count; i++) {
        /* flush filter */
        OutStreamContext *out_stream_ctx = out_ctx->out_stream_ctx + i;
        InStreamContext *in_stream_ctx = in_ctx->in_stream_ctx + i;
        if (out_stream_ctx->stream_copy || !out_stream_ctx->out) {
            continue;
        }
        ret = filter_write_encode_frame(in_stream_ctx, NULL, out_ctx, out_stream_ctx);
        if (ret < 0) {
            ff_log("write_encode_frame",AV_LOG_ERROR,0,"Flushing filter failed",1);
            return ret;
        }

        if (!(out_stream_ctx->codec_ctx->codec->capabilities & AV_CODEC_CAP_DELAY)) {
            continue;
        }

        ret = write_encode_frame(in_stream_ctx->filter_context, out_ctx, out_stream_ctx, 1);

        if (ret < 0) {
            ff_log("write_encode_frame",AV_LOG_ERROR,0,"Flushing encoder failed",1);
            return ret;
        }
    }
    return ret;
}