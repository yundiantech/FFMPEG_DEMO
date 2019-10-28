/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AppConfig.h"
#include "VideoEncoder.h"

//#define ENCODE_H265

VideoEncoder::VideoEncoder()
{
    mBitRate = 600000;
    mIsStop = true;

    pCodecCtx = NULL;
    pCodec = NULL;

    picture_buf = NULL;
    picture = NULL;

    mCond = new Cond();
}

VideoEncoder::~VideoEncoder()
{

}

void VideoEncoder::setQuantity(int value)
{
    mBitRate = 450000 + (value - 5) * 50000;
}

void VideoEncoder::setWidth(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

void VideoEncoder::inputYuvBuffer(uint8_t *buffer, int size, int64_t time)
{
    FrameDataNode node;
    node.size = size;
    node.buffer = buffer;
    node.time = time;

    mCond->Lock();
    mYuvBufferList.push_back(node);
    mCond->Unlock();
    mCond->Signal();
}

void VideoEncoder::startEncode()
{
    mIsStop = false;

    //启动新的线程
    std::thread([&](VideoEncoder *pointer)
    {
        pointer->run();

    }, this).detach();
}

void VideoEncoder::stopEncode()
{
    mIsStop = true;
}

void VideoEncoder::run()
{
    openEncoder();

    int y_size = pCodecCtx->width * pCodecCtx->height;

    AVPacket pkt;
    av_new_packet(&pkt, y_size*3);

#ifdef ENCODE_H265
    FILE *h264Fp = fopen("out.h265","wb");
#else
    FILE *h264Fp = fopen("out.h264","wb");
#endif

    while(1)
    {
        mCond->Lock();

        while(mYuvBufferList.empty())
        {
            mCond->Wait();
        }

        FrameDataNode node = mYuvBufferList.front(); //取出一帧yuv数据
        mYuvBufferList.pop_front();

        mCond->Unlock();

        if (node.buffer != NULL)
        {
//            picture->data[0] = node.buffer;     // 亮度Y
//            picture->data[1] = node.buffer + y_size;  // U
//            picture->data[2] = node.buffer + y_size*5/4; // V

            memcpy(picture_buf, node.buffer, node.size);

            int ret = avcodec_send_frame(pCodecCtx, picture);
            if (ret != 0)
            {
                char buff[128]={0};
                av_strerror(ret, buff, 128);

                fprintf(stderr, "Error sending a frame for encoding! (%s)\n", buff);
                continue;
            }

            while (0 == avcodec_receive_packet(pCodecCtx, &pkt))
            {
                bool isKeyFrame = pkt.flags & AV_PKT_FLAG_KEY; //判断是否关键帧
                int w = fwrite(pkt.data, 1, pkt.size, h264Fp); //写入文件中 (h264的裸数据 直接写入文件 也可以播放  因为这里包含H264关键帧)

                fprintf(stderr, "%s encoded size=%d isKeyFrame=%d\n", __FUNCTION__, pkt.size, isKeyFrame);
            }

            av_packet_unref(&pkt);
            av_free(node.buffer);
        }
    }

//    /*由于编码成H264的时候，一开始输入的几帧数据，不会立马输出
//     *因此停止输入的时候，编码器里面其实还是有数据的，下面的代码就是取出这个数据
//     *这个数据并不多 不处理也是可以的
//     */
//    while(1)
//    {
//         int got_picture=0;
//         int ret = avcodec_encode_video2(pCodecCtx, &pkt,NULL, &got_picture);

//         if (got_picture)
//         {
//               fwrite(pkt.data,1,pkt.size,h264Fp);
//         }
//         else
//         {
//             break;
//         }
//         av_free_packet(&pkt);
//    }

    fclose(h264Fp);

    closeEncoder();

}

AVDictionary *VideoEncoder::setEncoderParam(const AVCodecID &codec_id)
{
    int in_w = mWidth;
    int in_h = mHeight;//宽高

    pCodecCtx->codec_id = codec_id;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 15;//帧率(既一秒钟多少张图片)
    pCodecCtx->bit_rate = mBitRate; //比特率(调节这个大小可以改变编码后视频的质量)
    pCodecCtx->gop_size=12;
    //H264 还可以设置很多参数 自行研究吧
    ////    pCodecCtx->me_range = 16;
    ////    pCodecCtx->max_qdiff = 4;
    ////    pCodecCtx->qcompress = 0.6;
    ////    pCodecCtx->qmin = 10;
    ////    pCodecCtx->qmax = 51;
    //    pCodecCtx->me_range = 16;
    //    pCodecCtx->max_qdiff = 1;
    //    pCodecCtx->qcompress = 0.6;
    //    pCodecCtx->qmin = 10;
    //    pCodecCtx->qmax = 51;
    //    //Optional Param
    //    pCodecCtx->max_b_frames=3;


    // some formats want stream headers to be separate
    if (pCodecCtx->flags & AVFMT_GLOBALHEADER)
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //编码器预设
    AVDictionary *param = 0;
    if(pCodecCtx->codec_id == AV_CODEC_ID_H264)
    {
        av_dict_set(&param, "preset", "medium", 0);
    //        av_dict_set(&param, "preset", "superfast", 0);
        av_dict_set(&param, "tune", "zerolatency", 0); //实现实时编码
        av_dict_set(&param, "profile", "main", 0);
    }
    else if(pCodecCtx->codec_id == AV_CODEC_ID_H265)
    {
        av_dict_set(&param, "preset", "ultrafast", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
        av_dict_set(&param, "profile", "main", 0);
    }

    return param;

}

///打开视频编码器
bool VideoEncoder::openVideoEncoder(const AVCodecID &codec_id)
{
    bool isSucceed = false;
    bool isHardWareEncoderOpened = false;

//    bool mIsSupportHardEncoder = true;
//    if (mIsSupportHardEncoder)
    {
        ///尝试打开cuvid编码器器
        isHardWareEncoderOpened = openHardEncoder_Cuvid(codec_id);

        ///cuvid打开失败了 继续尝试 qsv
        if (!isHardWareEncoderOpened)
        {
            isHardWareEncoderOpened = openHardEncoder_Qsv(codec_id);
        }
    }

    //尝试打开硬件解码器失败了 改用软解码
    if (!isHardWareEncoderOpened)
    {
        isSucceed = openSoftEncoder(codec_id);
    }
    else
    {
        isSucceed = true;
    }

    return isSucceed;
}

///打开硬件编码器（英伟达）
bool VideoEncoder::openHardEncoder_Cuvid(const AVCodecID &codec_id)
{
    bool isSucceed = false;

    fprintf(stderr,"open hardware encoder cuvid...\n");

    ///查找硬件解码器
    char hardWareDecoderName[32] = {0};

    if (AV_CODEC_ID_H264 == codec_id)
    {
        sprintf(hardWareDecoderName, "h264_nvenc");
    }
    else if (AV_CODEC_ID_HEVC == codec_id)
    {
        sprintf(hardWareDecoderName, "hevc_nvenc");
    }

    if (strlen(hardWareDecoderName) > 0)
    {
        pCodec = avcodec_find_encoder_by_name(hardWareDecoderName);

        if (pCodec != NULL)
        {
            pCodecCtx = avcodec_alloc_context3(pCodec);

            AVDictionary *param = setEncoderParam(codec_id);

            ///打开解码器
            if (avcodec_open2(pCodecCtx, pCodec, &param) < 0)
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

///打开硬件编码器（intel）
bool VideoEncoder::openHardEncoder_Qsv(const AVCodecID &codec_id)
{
    bool isSucceed = false;

    fprintf(stderr,"open hardware encoder cuvid...\n");

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

    if (strlen(hardWareDecoderName) > 0)
    {
        pCodec = avcodec_find_encoder_by_name(hardWareDecoderName);

        if (pCodec != NULL)
        {
            pCodecCtx = avcodec_alloc_context3(pCodec);

            AVDictionary *param = setEncoderParam(codec_id);

            ///打开解码器
            if (avcodec_open2(pCodecCtx, pCodec, &param) < 0)
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

///打开软编码器
bool VideoEncoder::openSoftEncoder(const AVCodecID &codec_id)
{
    bool isSucceed = false;

    fprintf(stderr,"open software encoder... \n");

    pCodec = avcodec_find_encoder(codec_id);

    if (pCodec == NULL)
    {
        fprintf(stderr, "Codec not found.\n");
        isSucceed = false;
    }
    else
    {
        pCodecCtx = avcodec_alloc_context3(pCodec);
        pCodecCtx->thread_count = 8;

        AVDictionary *param = setEncoderParam(codec_id);

        ///打开解码器
        if (avcodec_open2(pCodecCtx, pCodec, &param) < 0)
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
        }
    }

    return isSucceed;
}

bool VideoEncoder::openEncoder()
{

#ifdef ENCODE_H265
    AVCodecID codec_id = AV_CODEC_ID_H265;
#else
    AVCodecID codec_id = AV_CODEC_ID_H264;
#endif

    bool isSucceed = openVideoEncoder(codec_id);

    if (isSucceed)
    {
        picture = av_frame_alloc();

        picture->format = pCodecCtx->pix_fmt;
        picture->width  = pCodecCtx->width;
        picture->height = pCodecCtx->height;

        int size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height); //计算需要用到的数据大小
        picture_buf = (uint8_t *)av_malloc(size); //分配空间
        avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

        fprintf(stderr, " 编码器打开成功！pCodecCtx->pix_fmt=%d %d %d %d %d %d\n",
                pCodecCtx->pix_fmt, AV_PIX_FMT_YUV420P, AV_PIX_FMT_NV12, pCodecCtx->width, pCodecCtx->height, size);
    }

    return isSucceed;
}

bool VideoEncoder::closeEncoder()
{
    if (picture != NULL)
        av_free(picture);

    if (picture_buf != NULL)
        av_free(picture_buf);

    if (pCodecCtx != NULL)
        avcodec_close(pCodecCtx);

    pCodecCtx = NULL;
    pCodec = NULL;

    picture_buf = NULL;
    picture = NULL;

    return true;
}
