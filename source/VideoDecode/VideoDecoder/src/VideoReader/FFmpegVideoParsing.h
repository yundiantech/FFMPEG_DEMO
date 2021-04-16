/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef FFMPEGVIDEOPARSING_H
#define FFMPEGVIDEOPARSING_H

#include <thread>
#include <functional>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include "Mutex/Cond.h"

class FFmpegVideoParsing
{
public:
    FFmpegVideoParsing();
    ~FFmpegVideoParsing();

    void start(std::function<void (uint8_t *buffer, const int &size, void *param)> callBackFunc, void *callBackFuncParam);
    void stop();

    int inputVideoBuffer(const uint8_t *buffer, const int &size);
    int readVideoBuffer(uint8_t *bufferOut, const int &size);

protected:
    void run();

private:
    bool mIsStop = false;
    bool mIsThreadRunning = false;

    Cond *mCond;
    uint8_t * mVideoBuffer;
    int mVideoBufferSize;

    ///回调函数,回传读到的视频数据
    std::function<void (uint8_t *buffer, const int &size, void *param)> mCallBackFunc;
    void *mCallBackFuncParam;

};

#endif // FFMPEGVIDEOPARSING_H
