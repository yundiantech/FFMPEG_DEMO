/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "crop.h"

//#ifdef _WIN32
//    #ifndef snprintf
//    #define snprintf _snprintf
//    #endif
//#endif

#ifdef __cplusplus
extern "C"
{
#endif
    #include <libavcodec/avcodec.h>
//    #include <libavfilter/avfiltergraph.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

int doCrop()
{
    int ret;
    AVFrame *frame_in;
    AVFrame *frame_out;
    unsigned char *frame_buffer_in;
    unsigned char *frame_buffer_out;

    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;

    //Input YUV
    FILE *fp_in=fopen("../data/in.yuv","rb+");
    if(fp_in==NULL){
        printf("Error open input file.\n");
        return -1;
    }
    int in_width=176;
    int in_height=144;

    //Output YUV
    FILE *fp_out=fopen("../data/output.yuv","wb+");
    if(fp_out==NULL){
        printf("Error open output file.\n");
        return -1;
    }

    //const char *filter_descr = "lutyuv='u=128:v=128'";
    //const char *filter_descr = "boxblur";
    //const char *filter_descr = "hflip";
    //const char *filter_descr = "hue='h=60:s=-3'";
//    const char *filter_descr = "crop=2/3*in_w:2/3*in_h";
    const char *filter_descr = "crop=out_w=100:out_h=100:x=10:y=10";
//    const char *filter_descr = "drawbox=x=100:y=100:w=100:h=100:color=pink@0.5";
    //const char *filter_descr = "drawtext=fontfile=arial.ttf:fontcolor=green:fontsize=30:text='Lei Xiaohua'";

    avfilter_register_all();

    char args[512] = {0};
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
//    const AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");新版的ffmpeg这里要改成buffersink
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
    AVBufferSinkParams *buffersink_params;

    filter_graph = avfilter_graph_alloc();

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    sprintf(args,
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        in_width,in_height,AV_PIX_FMT_YUV420P,
        1, 25,1,1);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
        args, NULL, filter_graph);
    if (ret < 0) {
        printf("Cannot create buffer source\n");
        return ret;
    }

    /* buffer video sink: to terminate the filter chain. */
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
        NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0) {
        char str[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, ret);
        printf("Cannot create buffer sink %d %s\n", ret, str);
        return ret;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
        &inputs, &outputs, NULL)) < 0)
        return ret;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        return ret;

    frame_in=av_frame_alloc();
    frame_buffer_in=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width,in_height,1));
    av_image_fill_arrays(frame_in->data, frame_in->linesize,frame_buffer_in,
        AV_PIX_FMT_YUV420P,in_width, in_height,1);

    frame_out=av_frame_alloc();
    frame_buffer_out=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width,in_height,1));
    av_image_fill_arrays(frame_out->data, frame_out->linesize,frame_buffer_out,
        AV_PIX_FMT_YUV420P,in_width, in_height,1);

    frame_in->width=in_width;
    frame_in->height=in_height;
    frame_in->format=AV_PIX_FMT_YUV420P;

    while (1) {

        if(fread(frame_buffer_in, 1, in_width*in_height*3/2, fp_in)!= in_width*in_height*3/2){
            break;
        }
        //input Y,U,V
        frame_in->data[0]=frame_buffer_in;
        frame_in->data[1]=frame_buffer_in+in_width*in_height;
        frame_in->data[2]=frame_buffer_in+in_width*in_height*5/4;

        if (av_buffersrc_add_frame(buffersrc_ctx, frame_in) < 0) {
            printf( "Error while add frame.\n");
            break;
        }

        /* pull filtered pictures from the filtergraph */
        ret = av_buffersink_get_frame(buffersink_ctx, frame_out);
        if (ret < 0)
            break;

        //output Y,U,V
        if(frame_out->format==AV_PIX_FMT_YUV420P){
            for(int i=0;i<frame_out->height;i++){
                fwrite(frame_out->data[0]+frame_out->linesize[0]*i,1,frame_out->width,fp_out);
            }
            for(int i=0;i<frame_out->height/2;i++){
                fwrite(frame_out->data[1]+frame_out->linesize[1]*i,1,frame_out->width/2,fp_out);
            }
            for(int i=0;i<frame_out->height/2;i++){
                fwrite(frame_out->data[2]+frame_out->linesize[2]*i,1,frame_out->width/2,fp_out);
            }
        }
        printf("Process 1 frame!\n");
        av_frame_unref(frame_out);
    }

    fclose(fp_in);
    fclose(fp_out);

    av_frame_free(&frame_in);
    av_frame_free(&frame_out);
    avfilter_graph_free(&filter_graph);

    return 0;
}
