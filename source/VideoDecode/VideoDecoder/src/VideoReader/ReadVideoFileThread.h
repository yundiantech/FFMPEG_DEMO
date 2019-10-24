/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef READVIDEOFILETHREAD_H
#define READVIDEOFILETHREAD_H

#include "VideoDecoder/VideoDecoder.h"
#include "VideoReader/NALUParsing.h"

#include "Video/VideoEventHandle.h"

class ReadVideoFileThread
{
public:
    ReadVideoFileThread();
    ~ReadVideoFileThread();

    /**
     * @brief setVideoCallBack 设置视频回调函数
     * @param pointer
     */
    void setVideoCallBack(VideoCallBack *pointer){mVideoCallBack=pointer;}

    void startRead(char* filePath, AVCodecID id = AV_CODEC_ID_H264);

protected:
    void run(AVCodecID id);

private:
    NALUParsing *mNaluParsing;
    VideoDecoder *mVideoDecoder;

    char mFileName[256];

    ///回调函数相关，主要用于输出信息给界面
private:
    ///回调函数
    VideoCallBack *mVideoCallBack;

    ///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
    void doDisplayVideo(const uint8_t *yuv420Buffer, const int &width, const int &height, const int &frameNum);

};

#endif // READH264FILETHREAD_H
