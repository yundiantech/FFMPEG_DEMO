/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include <thread>

#include "FFmpegVideoParsing.h"

#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <direct.h>
#include <io.h> //C (Windows)    access
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds)
{
    usleep(mSeconds * 1000);
}

#endif

#include <QDebug>
#include "AppConfig.h"

int read_buffer(void *opaque, uint8_t *buf, int buf_size)
{
    FFmpegVideoParsing *pointer = (FFmpegVideoParsing*)opaque;

    int readsize = pointer->readVideoBuffer(buf, buf_size);
//fprintf(stderr, "%s %d readsize=%d \n", __FUNCTION__, buf_size, readsize);
    if(readsize > 0)
    {
        return readsize;
    }
    else
    {
        return AVERROR_EOF;
    }
}

int MAX_FRAME_SIZE = 1024 * 1024 * 10;

FFmpegVideoParsing::FFmpegVideoParsing()
{
    mCallBackFunc = nullptr;
    mCallBackFuncParam = nullptr;

    mCond = new Cond();
    mVideoBuffer = (uint8_t*)malloc(MAX_FRAME_SIZE);
    mVideoBufferSize = 0;
}

FFmpegVideoParsing::~FFmpegVideoParsing()
{

}

void FFmpegVideoParsing::start(std::function<void (uint8_t *buffer, const int &size, void *param)> callBackFunc, void *callBackFuncParam)
{
    mIsStop = false;
    mCallBackFunc = callBackFunc;
    mCallBackFuncParam = callBackFuncParam;

    mVideoBufferSize = 0;

    //启动新的线程实现读取视频文件
    std::thread([=]
    {
        this->run();

    }).detach();
}

void FFmpegVideoParsing::stop()
{
    mIsStop = true;

    while (mIsThreadRunning)
    {
        AppConfig::mSleep(100);
    }
}

int FFmpegVideoParsing::inputVideoBuffer(const uint8_t * buffer, const int &size)
{
    mCond->Lock();

    while ((mVideoBufferSize + size) > MAX_FRAME_SIZE)
    {
        mCond->Wait(100);
    }

    memcpy(mVideoBuffer + mVideoBufferSize, buffer, size);
    mVideoBufferSize += size;
    int totalSize = mVideoBufferSize;
    mCond->Unlock();

    return totalSize;
}

int FFmpegVideoParsing::readVideoBuffer(uint8_t * bufferOut, const int &size)
{
    mCond->Lock();

    while (mVideoBufferSize <= 0)
    {
        if (mIsStop)
        {
            break;
        }

        mCond->Wait(100);
    }

    int bufSize = 0;

    if (mVideoBufferSize > 0)
    {
        bufSize = mVideoBufferSize > size ? size : mVideoBufferSize;
        memcpy(bufferOut, mVideoBuffer, bufSize);

        /// 把后面的数据覆盖上来
        int leftSize = mVideoBufferSize - bufSize;
        memmove(mVideoBuffer, mVideoBuffer + bufSize, leftSize);
        mVideoBufferSize = leftSize;
    }

    mCond->Unlock();

    return bufSize;
}

void FFmpegVideoParsing::run()
{
    mIsThreadRunning = true;

    #define IO_CTX_BUFFER_SIZE 4096 * 4

    unsigned char *aviobuffer=(unsigned char *)av_malloc(IO_CTX_BUFFER_SIZE);//申请自定义IO的缓冲区，必备，4k对齐
    AVIOContext *pIOCtx  = avio_alloc_context(aviobuffer, IO_CTX_BUFFER_SIZE, 0, this, read_buffer, NULL, NULL);//opaque为NULL即可

    AVFormatContext *pFormatCtx = nullptr;

do
{

    AVInputFormat *piFmt = NULL;

    //step2:探测流格式
    int ret = av_probe_input_buffer(pIOCtx, &piFmt, NULL, NULL, 0, 0);

    if (ret < 0)
    {
        fprintf(stderr, "probe failed!\n");
        break;
    }
    else
    {
        fprintf(stdout, "probe success!\n");
        fprintf(stdout, "format: %s[%s]\n", piFmt->name, piFmt->long_name);
    }

    pFormatCtx = avformat_alloc_context();//建立fmt_ctx格式上下文（统领全文的结构体）

    // Set the IOContext:
    pFormatCtx->pb = pIOCtx; //关键赋值 ，将自定义的IO赋予fmt_ctx接口
    pFormatCtx->flags = AVFMT_FLAG_CUSTOM_IO;


    //step4:打开流
    if (avformat_open_input(&pFormatCtx, "", piFmt, NULL) < 0)
    {
        fprintf(stderr, "avformat open failed.\n");
        break;
    }
    else
    {
        fprintf(stdout, "open stream success!\n");
    }

    if (avformat_find_stream_info(pFormatCtx, NULL)<0)
    {
        fprintf(stdout, "av_find_stream_info error \n");
        break;
    }

//    if (ret != 0)
//    {
//       char buffer[1024] = {0};
//       av_strerror(ret, buffer, 1024);
//       fprintf(stderr, "avformat_open_input ret=%d %s \n", ret, buffer);
//    }

    fprintf(stderr, "... %d \n", ret);

    while (1)
    {
        AVPacket packet;
        int ret = av_read_frame(pFormatCtx, &packet);

        if (ret != 0 && mIsStop)
        {
            break;
        }

        if (mCallBackFunc != nullptr)
        {
            mCallBackFunc((uint8_t*)packet.data, packet.size, mCallBackFuncParam);
        }

        av_packet_unref(&packet);
    }


}while(0);

    av_free(pIOCtx->buffer);

    avio_context_free(&pIOCtx);

    if (pFormatCtx != nullptr)
    {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }

    mIsThreadRunning = false;

    fprintf(stdout, " read finished!");

}

