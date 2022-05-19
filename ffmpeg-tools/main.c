
#include "transcoding.h"
#include "log.h"

int main(int argc, char **argv) {
    const char *in_filename = "/Users/hexiufeng/ffmpeg/v/t1.mp4";
    const char *out_filename = "/Users/hexiufeng/ffmpeg/v/g9.mp4";

    Parameters *parameters = create_parameters();
    set_file_names(parameters,in_filename,out_filename);

    parameters->acodec = COPY_VALUE;
//    parameters->acodec = AV_CODEC_ID_MP3;
//    parameters->vcodec = AV_CODEC_ID_HEVC;

//    parameters->ar = 44100;

    set_file_filters(parameters,NULL,"fps=20");
//    parameters->vn = 1;

    reset_parameters_r(parameters);

    InContext *in_ctx = NULL;
    OutContext *out_ctx = NULL;

    in_ctx = create_in_context(parameters);
    if (in_ctx == NULL) {
        goto end;
    }

    out_ctx = create_out_context(parameters, in_ctx);
    if (out_ctx == NULL) {
        goto end;
    }

    int ret = do_process(in_ctx, out_ctx, parameters);
    ff_log("main",AV_LOG_INFO,ret,"all complete...",0);
    end:

    free_parameters(parameters);
    in_context_free(in_ctx);
    out_context_free(out_ctx);

}


