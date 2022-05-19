//
// Created by hexiufeng on 2022/5/18.
//
#include "common.h"
#include "log.h"


FilterContext *create_filter_context() {
    FilterContext *fc = av_malloc(sizeof(FilterContext));
    memset(fc, 0, sizeof(FilterContext));
    fc->filter_frame = av_frame_alloc();
    return fc;
}

void filter_context_free(FilterContext *filter_context) {
    if (filter_context->filter_graph) {
        avfilter_graph_free(&filter_context->filter_graph);
    }

    if (filter_context->filter_frame) {
        av_frame_free(&(filter_context->filter_frame));
    }
    av_free(filter_context);
}

Parameters *create_parameters() {
    Parameters *parameters = av_malloc(sizeof(Parameters));
    memset(parameters, 0, sizeof(Parameters));

    parameters->r = UNKNOWN;
    parameters->ar = UNKNOWN;
    parameters->ac = UNKNOWN;
    parameters->vcodec = UNKNOWN;
    parameters->acodec = UNKNOWN;
    return parameters;
}

void set_file_names(Parameters *parameters, const char *in, const char *out) {
    if (!parameters) {
        return;
    }
    if (parameters->in_name) {
        av_freep(&parameters->in_name);
    }

    if (in) {
        parameters->in_name = av_malloc(strlen(in) + 1);
        strcpy(parameters->in_name, in);
    }
    if (parameters->out_name) {
        av_freep(&parameters->out_name);
    }
    if (out) {
        parameters->out_name = av_malloc(strlen(out) + 1);
        strcpy(parameters->out_name, out);
    }
}

void set_file_filters(Parameters *parameters, const char *audio, const char *video) {
    if (!parameters) {
        return;
    }
    if (parameters->audio_filter) {
        av_freep(&parameters->audio_filter);

    }
    if (audio) {
        parameters->audio_filter = av_malloc(strlen(audio) + 1);
        strcpy(parameters->in_name, audio);
    }
    if (parameters->video_filter) {
        av_freep(&parameters->video_filter);

    }
    if (video) {
        parameters->video_filter = av_malloc(strlen(video) + 1);
        strcpy(parameters->video_filter, video);
    }
}

void reset_parameters_r(Parameters *parameters) {
    if (parameters->video_filter && strstr(parameters->video_filter, "fps=")) {
        parameters->r = -101;
    }
//    if(parameters->audio_filter && strstr(parameters->audio_filter,"aresample="))
//    {
//        parameters->ar = -101;
//    }
}

void free_parameters(Parameters *p) {
    if (p != NULL) {
        if (p->in_name) {
            av_freep(&p->in_name);
        }
        if (p->out_name) {
            av_freep(&p->out_name);
        }
        if (p->audio_filter) {
            av_freep(&p->audio_filter);
        }
        if (p->video_filter) {
            av_freep(&p->video_filter);
        }
        av_free(p);
    }
}


/**
 * @brief 创建输出context
 *
 * @param parameters
 * @return InContext*
 */
InContext *create_in_context(Parameters *parameters) {
    InContext *in_ctx = av_malloc(sizeof(InContext));
    if (!in_ctx) {
        return NULL;
    }
    memset(in_ctx, 0, sizeof(InContext));

    in_ctx->in_packet = av_packet_alloc();

    int ret;
    if ((ret = avformat_open_input(&in_ctx->ic, parameters->in_name, NULL, NULL)) < 0) {
        ff_log("create_in_context", AV_LOG_ERROR, ret, "Cannot open input file", 1);
        in_context_free(in_ctx);
        return NULL;
    }

    if ((ret = avformat_find_stream_info(in_ctx->ic, NULL)) < 0) {
        ff_log("create_in_context", AV_LOG_ERROR, ret, "Cannot find stream information", 1);
        in_context_free(in_ctx);
        return NULL;
    }

    in_ctx->in_stream_ctx = av_mallocz_array(in_ctx->ic->nb_streams, sizeof(InStreamContext));
    if (!in_ctx->in_stream_ctx) {
        in_context_free(in_ctx);
        return NULL;
    }
    in_ctx->stream_count = in_ctx->ic->nb_streams;


    for (int i = 0; i < in_ctx->ic->nb_streams; i++) {
        AVStream *in_stream = in_ctx->ic->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        in_ctx->in_stream_ctx[i].stream = in_stream;
        in_ctx->in_stream_ctx[i].codec_type = in_codecpar->codec_type;
        in_ctx->in_stream_ctx[i].frame = av_frame_alloc();
    }
    return in_ctx;
}


void in_context_free(InContext *in_ctx) {
    if (!in_ctx) {
        return;
    }
    if (in_ctx->in_packet) {
        av_packet_free(&in_ctx->in_packet);
    }
    if (in_ctx->ic) {
        avformat_close_input(&in_ctx->ic);
    }
    if (in_ctx->stream_count) {
        for (int i = 0; i < in_ctx->stream_count; i++) {
            in_stream_context_free(in_ctx->in_stream_ctx + i);
        }
        av_freep(&(in_ctx->in_stream_ctx));
        in_ctx->stream_count = 0;
    }

    av_freep(&in_ctx);
}

void in_stream_context_free(InStreamContext *in_stream_ctx) {
    if (in_stream_ctx->codec_ctx) {
        avcodec_free_context(&in_stream_ctx->codec_ctx);
    }
    if (in_stream_ctx->frame) {
        av_frame_free(&in_stream_ctx->frame);
    }
    if (in_stream_ctx->filter_context) {
        filter_context_free(in_stream_ctx->filter_context);
        in_stream_ctx->filter_context = NULL;
    }
}

void set_audio_context_param(AVCodecContext *enc_ctx, AVCodecContext *dec_ctx, Parameters *parameters) {

    enc_ctx->sample_rate = (parameters->ar == UNKNOWN || parameters->ar == -101) ? dec_ctx->sample_rate
                                                                                 : parameters->ar;
    enc_ctx->channel_layout = dec_ctx->channel_layout;
    enc_ctx->channels = dec_ctx->channels;
    enc_ctx->sample_fmt = dec_ctx->sample_fmt;
    enc_ctx->time_base = (AVRational) {1, enc_ctx->sample_rate};
}

void set_video_context_param(AVCodecContext *enc_ctx, AVCodecContext *dec_ctx, Parameters *parameters) {
    enc_ctx->height = dec_ctx->height;
    enc_ctx->width = dec_ctx->width;
    enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    enc_ctx->pix_fmt = dec_ctx->pix_fmt;
    enc_ctx->framerate = dec_ctx->framerate;
    enc_ctx->time_base = dec_ctx->time_base;
//    if (parameters->r == UNKNOWN) {
//        enc_ctx->framerate = dec_ctx->framerate;
//        enc_ctx->time_base = dec_ctx->time_base;
//    } else {
//        enc_ctx->framerate = av_make_q(parameters->r, 1);
//        enc_ctx->time_base = av_inv_q(enc_ctx->framerate);
//    }


    //av_make_q(1,parameters->r == UNKNOWN?dec_ctx->framerate.num: parameters->r * 2);
}

OutContext *create_out_context(Parameters *parameters, const InContext *in_ctx) {
    AVFormatContext *ic = in_ctx->ic;

    OutContext *out_ctx = av_malloc(sizeof(OutContext));
    if (!out_ctx) {
        return NULL;
    }

    memset(out_ctx, 0, sizeof(OutContext));

    out_ctx->out_packet = av_packet_alloc();

    int ret;
    if ((ret = avformat_alloc_output_context2(&out_ctx->oc, NULL, NULL, parameters->out_name)) < 0) {
        ff_log("create_out_context", AV_LOG_ERROR, ret, "Cannot open out file", 1);
        out_context_free(out_ctx);
        return NULL;
    }
    if (!(out_ctx->oc->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&out_ctx->oc->pb, parameters->out_name, AVIO_FLAG_WRITE);
        if (ret < 0) {
            ff_log("create_out_context.open.output", AV_LOG_ERROR, ret, parameters->out_name, 1);
            out_context_free(out_ctx);
            return NULL;
        }
    }

    out_ctx->out_stream_ctx = av_mallocz_array(ic->nb_streams, sizeof(OutStreamContext));
    if (!out_ctx->out_stream_ctx) {
        out_context_free(out_ctx);
        return NULL;
    }
    out_ctx->stream_count = ic->nb_streams;


    int out_stream_index = 0;

    for (int i = 0; i < ic->nb_streams; i++) {
        AVStream *in_stream = ic->streams[i];
        AVCodecParameters *in_codec_par = in_stream->codecpar;
        OutStreamContext *out_stream_ctx = out_ctx->out_stream_ctx + i;
        set_para_callback callback = NULL;
        if (in_codec_par->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (parameters->vn) {
                out_stream_ctx->out = 0;
            } else {
                out_stream_ctx->out = 1;
                if (parameters->vcodec == COPY_VALUE
                    || (parameters->r == UNKNOWN
                        && (parameters->vcodec == UNKNOWN
                            || parameters->vcodec == in_stream->codecpar->codec_id))) {
                    out_stream_ctx->stream_copy = 1;
                }
                out_stream_ctx->out_stream_index = out_stream_index++;
                callback = set_video_context_param;
            }

        } else if (in_codec_par->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (parameters->an) {
                out_stream_ctx->out = 0;
            } else {
                out_stream_ctx->out = 1;
                if (parameters->acodec == COPY_VALUE
                    || (parameters->ac == UNKNOWN && parameters->ar == UNKNOWN
                        && (parameters->acodec == UNKNOWN || parameters->acodec == in_stream->codecpar->codec_id))) {
                    out_stream_ctx->stream_copy = 1;
                }
                callback = set_audio_context_param;
                out_stream_ctx->out_stream_index = out_stream_index++;
            }
        } else if (in_codec_par->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            if (parameters->sn) {
                out_stream_ctx->out = 0;
            } else {
                out_stream_ctx->out = 1;
                out_stream_ctx->stream_copy = 1;

                out_stream_ctx->out_stream_index = out_stream_index++;
            }
        } else {
            out_stream_ctx->out = 1;
            out_stream_ctx->stream_copy = 1;
            out_stream_ctx->out_stream_index = out_stream_index++;
        }
        if (open_out_stream(in_ctx, out_ctx, parameters, i, callback)) {
            out_context_free(out_ctx);
            return NULL;
        }
        if (out_stream_ctx->out) {
            copy_stream_meta(in_stream, out_stream_ctx->stream);
        }
    }

    if (out_stream_index == 0) {
        ff_log("create_out_context", AV_LOG_INFO, 0, "no stream to output", 0);
        out_context_free(out_ctx);
        return NULL;
    }
    return out_ctx;
}

void out_context_free(OutContext *out_ctx) {
    if (!out_ctx) {
        return;
    }
    if (out_ctx->out_packet) {
        av_packet_free(&out_ctx->out_packet);
    }
    if (out_ctx->oc && !(out_ctx->oc->flags & AVFMT_NOFILE)) {
        avio_closep(&out_ctx->oc->pb);
    }

    if (out_ctx->oc) {
        avformat_free_context(out_ctx->oc);
        out_ctx->oc = NULL;
    }
    if (out_ctx->stream_count) {
        for (int i = 0; i < out_ctx->stream_count; i++) {
            OutStreamContext *p = out_ctx->out_stream_ctx + i;
            out_stream_context_free(p);
        }
        av_freep(&(out_ctx->out_stream_ctx));
        out_ctx->stream_count = 0;
    }


    av_freep(&out_ctx);
}

void out_stream_context_free(OutStreamContext *out_stream_ctx) {
    if (out_stream_ctx->codec_ctx) {
        avcodec_free_context(&out_stream_ctx->codec_ctx);
    }
}

// 0 is sucess
int open_out_stream(InContext *in_ctx, OutContext *out_ctx, Parameters *parameters, int stream_index,
                    set_para_callback set_param) {
    OutStreamContext *out_stream_ctx = out_ctx->out_stream_ctx + stream_index;
    InStreamContext *in_stream_ctx = in_ctx->in_stream_ctx + stream_index;
    AVFormatContext *ic = in_ctx->ic;
    AVFormatContext *oc = out_ctx->oc;


    if (!out_stream_ctx->out) {
        return 0;
    }
    if (out_stream_ctx->stream_copy) {
        AVStream *out_stream = avformat_new_stream(oc, NULL);
        out_stream_ctx->stream = out_stream;
        if (!out_stream) {
            ff_log("open_out_stream", AV_LOG_ERROR, 1, "Failed allocating output stream", 1);
            return 1;
        }

        int ret = avcodec_parameters_copy(out_stream->codecpar, in_stream_ctx->stream->codecpar);
        if (ret < 0) {
            ff_log("open_out_stream", AV_LOG_ERROR, ret, "Failed to copy codec parameters", 1);
            return 1;
        }
        out_stream->codecpar->codec_tag = 0;
        return 0;
    }

    int ret = open_in_coder_ctx(ic, oc, in_stream_ctx);
    if (ret) {
        return ret;
    }
    AVStream *out_stream = avformat_new_stream(oc, NULL);
    if (!out_stream) {
        ff_log("open_out_stream", AV_LOG_ERROR, 1, "Failed allocating output stream", 1);
        return 1;
    }
    out_stream_ctx->stream = out_stream;
    int encoder_id;
    if (in_stream_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        encoder_id = parameters->vcodec == UNKNOWN ? in_stream_ctx->codec_ctx->codec_id : parameters->vcodec;
    } else {
        encoder_id = parameters->acodec == UNKNOWN ? in_stream_ctx->codec_ctx->codec_id : parameters->acodec;
    }


    AVCodec *encoder = avcodec_find_encoder(encoder_id);
    if (!encoder) {
        ff_log("open_out_stream", AV_LOG_FATAL, 1, "Necessary encoder not found", 1);
        return 1;
    }
    AVCodecContext *enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        ff_log("open_out_stream", AV_LOG_FATAL, 1, "Failed to allocate the encoder context", 1);
        return 1;
    }

    set_param(enc_ctx, in_stream_ctx->codec_ctx, parameters);
    out_stream_ctx->codec_ctx = enc_ctx;

    if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /* Third parameter can be used to pass settings to encoder */
    ret = avcodec_open2(enc_ctx, encoder, NULL);
    if (ret < 0) {
        ff_log("open_out_stream", AV_LOG_ERROR, ret, "Cannot open  encoder context", 1);
        return ret;
    }
    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if (ret < 0) {
        ff_log("open_out_stream", AV_LOG_ERROR, ret, "avcodec_parameters_from_context error", 1);
        return ret;
    }

    out_stream->time_base = enc_ctx->time_base;
    return 0;
}


void copy_stream_meta(AVStream *in_stream, AVStream *out_stream) {
    AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(in_stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        // printf("************%s=%s\n", tag->key, tag->value);
        av_dict_set(&out_stream->metadata, tag->key, tag->value, AV_DICT_MATCH_CASE);
    }

    for (int j = 0; j < in_stream->nb_side_data; j++) {
        AVPacketSideData *sd = &in_stream->side_data[j];
        if (sd->type == AV_PKT_DATA_DISPLAYMATRIX) {
            uint8_t *dst = av_stream_new_side_data(out_stream, sd->type, sd->size);
            if (!dst) {
                return;
            }
            memcpy(dst, sd->data, sd->size);
        }
    }
}


// 0 is success
int open_in_coder_ctx(AVFormatContext *ic, AVFormatContext *oc, InStreamContext *in_stream_ctx) {
    AVStream *in_stream = in_stream_ctx->stream;
    AVCodec *dec = avcodec_find_decoder(in_stream->codecpar->codec_id);
    AVCodecContext *dec_ctx;
    if (!dec) {
        ff_log("open_in_coder_ctx", AV_LOG_ERROR, 1, "avcodec_find_decoder error", 1);
        return 1;
    }
    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx) {
        ff_log("open_in_coder_ctx", AV_LOG_ERROR, 1, "avcodec_alloc_context3 for dec error", 1);
        return 1;
    }
    in_stream_ctx->codec_ctx = dec_ctx;
    int ret = avcodec_parameters_to_context(dec_ctx, in_stream->codecpar);
    if (ret < 0) {
        ff_log("open_in_coder_ctx", AV_LOG_ERROR, ret, "avcodec_parameters_to_context for dec_ctx error", 1);
        return ret;
    }
    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        dec_ctx->framerate = av_guess_frame_rate(ic, in_stream, NULL);
    }
    /* Open decoder */
    ret = avcodec_open2(dec_ctx, dec, NULL);
    if (ret < 0) {
        ff_log("open_in_coder_ctx", AV_LOG_ERROR, ret, "avcodec_open2 for dec_ctx error", 1);
        return ret;
    }
    return 0;
}


// 0 is success
int copy_packet(AVFormatContext *oc, InStreamContext *in_stream_ctx, OutStreamContext *out_stream_ctx,
                AVPacket *in_packet) {
    AVStream *out_stream = out_stream_ctx->stream;
    in_packet->pts = av_rescale_q_rnd(in_packet->pts, in_stream_ctx->stream->time_base, out_stream->time_base,
                                      AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
    in_packet->dts = av_rescale_q_rnd(in_packet->dts, in_stream_ctx->stream->time_base, out_stream->time_base,
                                      AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
    in_packet->duration = av_rescale_q(in_packet->duration, in_stream_ctx->stream->time_base, out_stream->time_base);
    in_packet->pos = -1;
    // log_packet(ofmt_ctx, &pkt, "out");
    in_packet->stream_index = out_stream_ctx->out_stream_index;

    int ret = av_interleaved_write_frame(oc, in_packet);

    if (ret < 0) {
        ff_log("copy_packet", AV_LOG_ERROR, ret, "to av_interleaved_write_frame error", 1);
        return ret;
    }
    return 0;
}