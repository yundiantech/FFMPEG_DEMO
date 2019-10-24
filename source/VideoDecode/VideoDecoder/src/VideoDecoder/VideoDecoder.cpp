/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "VideoDecoder.h"

VideoDecoder::VideoDecoder()
{
    pCodec = NULL;
    pCodecCtx = NULL;
    img_convert_ctx = NULL;
    pFrame = NULL;

    pFrameYUV = NULL;
    bufferYUV = NULL;
}

void VideoDecoder::initWidth(int width, int height)
{
    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, width,height);
    bufferYUV = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    avpicture_fill((AVPicture *)pFrameYUV, bufferYUV, AV_PIX_FMT_YUV420P,width, height);

    img_convert_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                                     width, height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, NULL,NULL,NULL);

}

bool VideoDecoder::openDecoder(AVCodecID id)
{
    /* find the h264 video decoder */
    pCodec = avcodec_find_decoder(id);
    if (!pCodec)
    {
        fprintf(stderr, "codec not found\n");
        return false;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);// avcodec_alloc_context2();

    /* open the coderc */
    if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0)
    {
        fprintf(stderr, "could not open codec\n");
        return false;
    }

    // Allocate video frame
    pFrame = av_frame_alloc();

    if(pFrame == NULL)
        return false;

    pFrameYUV = av_frame_alloc();

    if(pFrameYUV == NULL)
        return false;

    return true;
}

void VideoDecoder::closeDecoder()
{
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_free(pFrame);
    av_free(pFrameYUV);

    if (bufferYUV != NULL)
    {
        av_free(bufferYUV);
    }

    pCodec = NULL;
    pCodecCtx = NULL;
    img_convert_ctx = NULL;
    pFrame = NULL;

    pFrameYUV = NULL;
    bufferYUV = NULL;
}

bool VideoDecoder::decode(uint8_t *inputbuf, int frame_size, uint8_t *&outBuf, int &outWidth, int &outHeight)
{
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = inputbuf;
    pkt.size = frame_size;

    if (avcodec_send_packet(pCodecCtx, &pkt) != 0)
    {
       fprintf(stderr, "input AVPacket to decoder failed!\n");
       av_packet_unref(&pkt);
       return 0;
    }

    while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
    {
        //前面初始化解码器的时候 并没有设置视频的宽高信息，
        //因为h264的每一帧数据都带有编码的信息，当然也包括这些宽高信息了，因此解码完之后，便可以知道视频的宽高是多少
        //这就是为什么 初始化编码器的时候 需要初始化高度，而初始化解码器却不需要。
        //解码器可以直接从需要解码的数据中获得宽高信息，这样也才会符合道理。
        //所以一开始没有为bufferYUV分配空间 因为没办法知道 视频宽高
        //一旦解码了一帧之后 就可以知道宽高了  这时候就可以分配了
        if (bufferYUV == NULL)
        {
            initWidth(pCodecCtx->width, pCodecCtx->height);
        }

        //格式转换 解码之后的数据是yuv420p的 把她转换成 rgb的图像数据
        sws_scale(img_convert_ctx,
                 (uint8_t const * const *) pFrame->data, pFrame->linesize,
                  0, pCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);

        outBuf = bufferYUV;
        outWidth = pCodecCtx->width;
        outHeight = pCodecCtx->height;
    }

    av_packet_unref(&pkt);

    return true;
}

int VideoDecoder::getFrameRate()
{
    int den = pCodecCtx->framerate.den;
    int num = pCodecCtx->framerate.num;

    int frameRate = 0;

    if (den > 0)
    {
        frameRate = num / den;
    }

    return frameRate;
}
