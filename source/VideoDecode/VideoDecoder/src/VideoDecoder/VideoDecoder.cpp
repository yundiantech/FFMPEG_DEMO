/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "VideoDecoder.h"

VideoDecoder::VideoDecoder()
{
    pCodec = nullptr;
    pCodecCtx = nullptr;
    img_convert_ctx = nullptr;
    pFrame = nullptr;

    pFrameYUV = nullptr;
    bufferYUV = nullptr;
}

void VideoDecoder::initWidth(int width, int height)
{
    fprintf(stderr, "%s pCodecCtx->pix_fmt=%d %d %d \n", __FUNCTION__, pCodecCtx->pix_fmt, width, height);

#if 0

    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, width,height);
    bufferYUV = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    avpicture_fill((AVPicture *)pFrameYUV, bufferYUV, AV_PIX_FMT_YUV420P,width, height);

#else

    int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
//    int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 0);  //按0字节进行内存对齐，得到的内存大小是0
//    int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 4);   //按4字节进行内存对齐，得到的内存大小稍微大一些

    unsigned int numBytes = static_cast<unsigned int>(yuvSize);
    bufferYUV = static_cast<uint8_t *>(av_malloc(numBytes * sizeof(uint8_t)));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, bufferYUV, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

#endif

    ///用于将解码后的数据统一转换成YUV420P,因为硬件解码后的数据会是NV12的
    img_convert_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt,
                                     width, height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, nullptr, nullptr, nullptr);

}

#if 0

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

#else

bool VideoDecoder::openDecoder(AVCodecID id)
{
    bool isSucceed = openVideoDecoder(id);

    if (isSucceed)
    {
        // Allocate video frame
        pFrame = av_frame_alloc();
        pFrameYUV = av_frame_alloc();

        if(pFrameYUV == nullptr || pFrame == nullptr)
        {
            isSucceed = false;
        }
    }

    return isSucceed;
}

#endif

///打开视频解码器
bool VideoDecoder::openVideoDecoder(const AVCodecID &codec_id)
{
    bool isSucceed = false;
    bool isHardWareDecoderOpened = false;

//    bool mIsSupportHardDecoder = true;
//    if (mIsSupportHardDecoder)
    {
        ///尝试打开英伟达的硬解
        isHardWareDecoderOpened = openHardDecoder_Cuvid(codec_id);

        ///cuvid打开失败了 继续尝试 qsv
        if (!isHardWareDecoderOpened)
        {
            ///打开intel的硬解
            isHardWareDecoderOpened = openHardDecoder_Qsv(codec_id);
        }
    }

    //尝试打开硬件解码器失败了 改用软解码
    if (!isHardWareDecoderOpened)
    {
        isSucceed = openSoftDecoder(codec_id);
    }
    else
    {
        isSucceed = true;
    }

    return isSucceed;
}

///打开硬件解码器（英伟达）
bool VideoDecoder::openHardDecoder_Cuvid(const AVCodecID &codec_id)
{
    bool isSucceed = false;

    fprintf(stderr,"open hardware decoder cuvid...\n");

    ///查找硬件解码器
    char hardWareDecoderName[32] = {0};

    if (AV_CODEC_ID_H264 == codec_id)
    {
        sprintf(hardWareDecoderName, "h264_cuvid");
    }
    else if (AV_CODEC_ID_HEVC == codec_id)
    {
        sprintf(hardWareDecoderName, "hevc_cuvid");
    }
    else if (AV_CODEC_ID_MPEG1VIDEO == codec_id)
    {
        sprintf(hardWareDecoderName, "mpeg1_cuvid");
    }
    else if (AV_CODEC_ID_MPEG2VIDEO == codec_id)
    {
        sprintf(hardWareDecoderName, "mpeg2_cuvid");
    }
    else if (AV_CODEC_ID_MPEG4 == codec_id)
    {
        sprintf(hardWareDecoderName, "mpeg4_cuvid");
    }

    if (strlen(hardWareDecoderName) > 0)
    {
        pCodec = avcodec_find_decoder_by_name(hardWareDecoderName);

        if (pCodec != nullptr)
        {
            pCodecCtx = avcodec_alloc_context3(pCodec);
            pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

            ///打开解码器
            if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
            {
                avcodec_close(pCodecCtx);
                avcodec_free_context(&pCodecCtx);
                pCodecCtx = nullptr;
                isSucceed = false;

                fprintf(stderr,"Could not open codec %s\n",hardWareDecoderName);
            }
            else
            {
                isSucceed = true;
                fprintf(stderr,"open codec %s succeed! %d %d\n",hardWareDecoderName,pCodec->id,pCodecCtx->codec_id);
            }
        }
        else
        {
            fprintf(stderr,"Codec %s not found.\n",hardWareDecoderName);
        }
    }

    return isSucceed;
}

///打开硬件解码器（intel）
bool VideoDecoder::openHardDecoder_Qsv(const AVCodecID &codec_id)
{
    bool isSucceed = false;

    fprintf(stderr,"open hardware decoder qsv... \n");

    ///查找硬件解码器
    char hardWareDecoderName[32] = {0};

    if (AV_CODEC_ID_H264 == codec_id)
    {
        sprintf(hardWareDecoderName, "h264_qsv");
    }
    else if (AV_CODEC_ID_HEVC == codec_id)
    {
        sprintf(hardWareDecoderName, "hevc_qsv");
    }
    else if (AV_CODEC_ID_MPEG1VIDEO == codec_id)
    {
        sprintf(hardWareDecoderName, "mpeg1_qsv");
    }
    else if (AV_CODEC_ID_MPEG2VIDEO == codec_id)
    {
        sprintf(hardWareDecoderName, "mpeg2_qsv");
    }
    else if (AV_CODEC_ID_MPEG4 == codec_id)
    {
        sprintf(hardWareDecoderName, "mpeg4_qsv");
    }

    /// 在使用 hevc_qsv 编码器的时候
    /// 可能会出现 Error initializing an internal MFX session 错误，目前没有找到具体原因。
    /// 在把 Media SDK 下的libmfxhw32.dll 文件拷贝到执行目录下之后这个问题就消失看

    if (strlen(hardWareDecoderName) > 0)
    {
        pCodec = avcodec_find_decoder_by_name(hardWareDecoderName);

        if (pCodec != nullptr)
        {
            pCodecCtx = avcodec_alloc_context3(pCodec);
            pCodecCtx->pix_fmt = AV_PIX_FMT_NV12;

            ///打开解码器
            if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
            {
                avcodec_close(pCodecCtx);
                avcodec_free_context(&pCodecCtx);
                pCodecCtx = nullptr;
                isSucceed = false;

                fprintf(stderr,"Could not open codec %s\n",hardWareDecoderName);
            }
            else
            {
                isSucceed = true;
                fprintf(stderr,"open codec %s succeed! %d %d\n",hardWareDecoderName,pCodec->id,pCodecCtx->codec_id);
            }
        }
        else
        {
            fprintf(stderr,"Codec %s not found.\n",hardWareDecoderName);
        }
    }

    return isSucceed;

}

///打开软解码器
bool VideoDecoder::openSoftDecoder(const AVCodecID &codec_id)
{
    bool isSucceed = false;

    fprintf(stderr,"open software decoder... \n");

    pCodec = avcodec_find_decoder(codec_id);

    if (pCodec == nullptr)
    {
        fprintf(stderr, "Codec not found.\n");
        isSucceed = false;
    }
    else
    {
        pCodecCtx = avcodec_alloc_context3(pCodec);
        pCodecCtx->thread_count = 8;
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

        ///打开解码器
        if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
        {
            avcodec_close(pCodecCtx);
            avcodec_free_context(&pCodecCtx);
            pCodecCtx = nullptr;
            isSucceed = false;

            fprintf(stderr,"Could not open codec.\n");
        }
        else
        {
            isSucceed = true;
            fprintf(stderr,"open software decoder succeed!\n");
        }
    }

    return isSucceed;
}

void VideoDecoder::closeDecoder()
{
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_free(pFrame);
    av_free(pFrameYUV);

    if (bufferYUV != nullptr)
    {
        av_free(bufferYUV);
    }

    pCodec = nullptr;
    pCodecCtx = nullptr;
    img_convert_ctx = nullptr;
    pFrame = nullptr;

    pFrameYUV = nullptr;
    bufferYUV = nullptr;
}

bool VideoDecoder::decode(uint8_t *inputbuf, int frame_size, uint8_t *&outBuf, int &outWidth, int &outHeight)
{
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = inputbuf;
    pkt.size = frame_size;
//fprintf(stderr, "%s %d %d \n", __FUNCTION__, pCodecCtx, frame_size);
    if (avcodec_send_packet(pCodecCtx, &pkt) != 0)
    {
       fprintf(stderr, "input AVPacket to decoder failed!\n");
       av_packet_unref(&pkt);
       return 0;
    }

//    while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
    while(1)
    {
        int ret = avcodec_receive_frame(pCodecCtx, pFrame);
        if (ret != 0)
        {
//            char buffer[1024] = {0};
//            av_strerror(ret, buffer, 1024);
//            fprintf(stderr, "avcodec_receive_frame = %d %s\n", ret, buffer);
            break;
        }

        //前面初始化解码器的时候 并没有设置视频的宽高信息，
        //因为h264的每一帧数据都带有编码的信息，当然也包括这些宽高信息了，因此解码完之后，便可以知道视频的宽高是多少
        //这就是为什么 初始化编码器的时候 需要初始化高度，而初始化解码器却不需要。
        //解码器可以直接从需要解码的数据中获得宽高信息，这样也才会符合道理。
        //所以一开始没有为bufferYUV分配空间 因为没办法知道 视频宽高
        //一旦解码了一帧之后 就可以知道宽高了  这时候就可以分配了
        if (bufferYUV == nullptr)
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
