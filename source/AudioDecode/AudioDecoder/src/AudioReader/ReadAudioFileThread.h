
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef READAUDIOFILETHREAD_H
#define READAUDIOFILETHREAD_H

#include "AAC/AACReader.h"
#include "AudioDecoder/AudioDecoder.h"
#include "AudioPlayer/AudioPlayer_SDL.h"
#include "AudioPlayer/AudioPlayer_RtAudio.h"

#include "EventHandle/AudioPlayerEventHandle.h"

class ReadAudioFileThread
{
public:
    ReadAudioFileThread();
    ~ReadAudioFileThread();

    /**
     * @brief setVideoPlayerCallBack 设置播放器回调函数
     * @param pointer
     */
    void setVideoPlayerCallBack(AudioPlayerCallBack *pointer){mAudioCallBack=pointer;}

    void startRead(char* filePath);

    std::list<AudioDevice> getAudiDeviceList();

protected:
    void run();

private:
    char mFileName[256];

    AACReader *mAACReader;
    AudioDecoder *mAudioDecoder;
    AudioPlayer *mAudioPlayer;

    ///回调函数相关，主要用于输出信息给界面
private:
    ///回调函数
    AudioPlayerCallBack *mAudioCallBack;

//    ///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
//    void doDisplayVideo(const uint8_t *yuv420Buffer, const int &width, const int &height, const int &frameNum);

};

#endif // READAUDIOFILETHREAD_H
