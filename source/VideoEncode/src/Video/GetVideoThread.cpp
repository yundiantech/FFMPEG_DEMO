/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AppConfig.h"
#include "GetVideoThread.h"


//'1' Use Dshow
//'0' Use VFW
#define USE_DSHOW 0

//Show Dshow Device
void show_dshow_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Info=============\n");
    avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
    printf("================================\n");
}

//Show Dshow Device Option
void show_dshow_device_option()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_options","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Option Info======\n");
    avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
    printf("================================\n");
}

//Show VFW Device
void show_vfw_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVInputFormat *iformat = av_find_input_format("vfwcap");
    printf("========VFW Device Info======\n");
    avformat_open_input(&pFormatCtx,"list",iformat,nullptr);
    printf("=============================\n");
}

//Show AVFoundation Device
void show_avfoundation_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    avformat_open_input(&pFormatCtx, "",iformat, &options);
    printf("=============================\n");
}

GetVideoThread::GetVideoThread()
{
    m_isRun = false;

    pFormatCtx = NULL;
    out_buffer = NULL;
    pFrame = NULL;
    pFrameYUV = NULL;
    pCodecCtx = NULL;

    m_pause = false;

    mVideoEncoder = new VideoEncoder();

}

GetVideoThread::~GetVideoThread()
{

}

void GetVideoThread::setQuantity(int value)
{
    mVideoEncoder->setQuantity(value);
}

ErroCode GetVideoThread::init()
{

    AVCodec			*pCodec = nullptr;

    pFormatCtx = avformat_alloc_context();

#if defined(WIN32)

    //Show Dshow Device
    show_dshow_device();
    //Show Device Options
    show_dshow_device_option();
    //Show VFW Options
    show_vfw_device();


    AVInputFormat *ifmt = av_find_input_format("dshow"); //使用dshow

    if(avformat_open_input(&pFormatCtx, "video=screen-capture-recorder", ifmt, nullptr)!=0)
    {
        fprintf(stderr, "Couldn't open input stream video.（无法打开输入流）\n");
        return VideoOpenFailed;
    }
#elif defined __linux
//Linux
//    AVInputFormat *ifmt=av_find_input_format("video4linux2");
//    if(avformat_open_input(&pFormatCtx, "/dev/video0", ifmt, NULL)!=0)
//    {
//        fprintf(stderr, "Couldn't open input stream.\n");
//        return -1;
//    }

    AVDictionary* options = NULL;
//    av_dict_set(&options,"list_devices","true", 0);
    /* set frame per second */
//    av_dict_set( &options,"framerate","30", 0);
    av_dict_set( &options,"show_region","1", 0);
//    av_dict_set( &options,"video_size","1240x480", 0);
//    av_dict_set( &options, "preset", "medium", 0 );

    /*
    X11 video input device.
    To enable this input device during configuration you need libxcb installed on your system. It will be automatically detected during configuration.
    This device allows one to capture a region of an X11 display.
    refer : https://www.ffmpeg.org/ffmpeg-devices.html#x11grab
    */
    AVInputFormat *ifmt = av_find_input_format("x11grab");
    if(avformat_open_input(&pFormatCtx, ":0.0+10,250", ifmt, &options) != 0)
//    if(avformat_open_input(&pFormatCtx, ":0.0", ifmt, &options) != 0)
    {
        fprintf(stderr, "\nerror in opening input device\n");
        return VideoOpenFailed;
    }
#else
    show_avfoundation_device();
    //Mac
    AVInputFormat *ifmt=av_find_input_format("avfoundation");
    //Avfoundation
    //[video]:[audio]
    if(avformat_open_input(&pFormatCtx,"0",ifmt,NULL)!=0)
    {
        fprintf(stderr, "Couldn't open input stream.\n");
        return VideoOpenFailed;
    }
#endif

    videoindex=-1;
    pCodecCtx = NULL;

    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoindex=i;
            break;
        }

    if(videoindex==-1)
    {
        printf("Didn't find a video stream.（没有找到视频流）\n");
        return VideoOpenFailed;
    }

    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if(pCodec == NULL)
    {
        printf("video Codec not found.\n");
        return VideoDecoderOpenFailed;
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
    {
        printf("Could not open video codec.\n");
        return VideoDecoderOpenFailed;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);


    //***************
//    int Screen_W = GetSystemMetrics(SM_CXSCREEN); //获取屏幕宽高
//    int Screen_H = GetSystemMetrics(SM_CYSCREEN);
    mVideoEncoder->setWidth(pCodecCtx->width, pCodecCtx->height);  //设置编码器的宽高

    return SUCCEED;
}

void GetVideoThread::deInit()
{
    if (out_buffer)
    {
        av_free(out_buffer);
        out_buffer = NULL;
    }

    if (pFrame)
    {
        av_free(pFrame);
        pFrame = NULL;
    }

    if (pFrameYUV)
    {
        av_free(pFrameYUV);
        pFrameYUV = NULL;
    }

    if (pCodecCtx)
        avcodec_close(pCodecCtx);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

}

void GetVideoThread::startRecord()
{
    m_isRun = true;

    //启动新的线程
    std::thread([&](GetVideoThread *pointer)
    {
        pointer->run();

    }, this).detach();

    mVideoEncoder->startEncode();
}

void GetVideoThread::pauseRecord()
{
    m_pause = true;
}

void GetVideoThread::restoreRecord()
{
    m_getFirst = false;
    m_pause = false;
}

void GetVideoThread::stopRecord()
{
    m_isRun = false;
}

//FILE *fp = fopen("out.yuv","wb");

void GetVideoThread::run()
{
    struct SwsContext *img_convert_ctx = NULL;

    int y_size = 0;
    int yuvSize = 0;

    if (pCodecCtx)
    {
        y_size = pCodecCtx->width * pCodecCtx->height;
        yuvSize = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
        ///理论上 这里的 size = y_size * 3 / 2

        int numBytes = yuvSize;
        out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
        avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height);

        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                         pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC, NULL, NULL, NULL);
    }

    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));

    int64_t firstTime = AppConfig::getTimeStamp_MilliSecond();
    m_getFirst = false;
    int64_t timeIndex = 0;

    bool m_saveVideoFileThread = true;

    while(m_isRun )
    {
        if (av_read_frame(pFormatCtx, packet)<0)
        {
            fprintf(stderr, "read failed! \n");
            AppConfig::mSleep(10);
            continue;
        }

        if (m_pause)
        {
            av_packet_unref(packet);
            AppConfig::mSleep(10);
            continue;
        }

        if(packet->stream_index==videoindex)
        {
            int64_t time = 0;
            if (m_saveVideoFileThread)
            {
                if (m_getFirst)
                {
                    int64_t secondTime = AppConfig::getTimeStamp_MilliSecond();
                    time = secondTime - firstTime + timeIndex;
                }
                else
                {
                    firstTime = AppConfig::getTimeStamp_MilliSecond();
                    timeIndex = 0;
                    m_getFirst = true;
                }
            }

            if (avcodec_send_packet(pCodecCtx, packet) != 0)
            {
               fprintf(stderr, "input AVPacket to decoder failed!\n");
               av_packet_unref(packet);
               continue;
            }

            while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
            {
                /// 转换成YUV420
                /// 由于解码后的数据不一定是yuv420p，比如硬件解码后会是yuv420sp，因此这里统一转成yuv420p
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

                if (m_saveVideoFileThread)
                {
                    uint8_t * picture_buf = (uint8_t *)av_malloc(yuvSize);
                    memcpy(picture_buf, out_buffer, yuvSize);
//                        fwrite(picture_buf,1,y_size*3/2,fp);
//                        av_free(picture_buf);
                    mVideoEncoder->inputYuvBuffer(picture_buf, yuvSize, time); //将yuv数据添加到h.264编码的线程
                }
            }
        }
        else
        {
            fprintf(stderr, "other %d \n", packet->stream_index);
        }
        av_packet_unref(packet);

    }

    sws_freeContext(img_convert_ctx);

    fprintf(stderr, "record stopping... \n");

    m_pause = false;

    deInit();

    mVideoEncoder->stopEncode();

    fprintf(stderr, "record finished! \n");

}

