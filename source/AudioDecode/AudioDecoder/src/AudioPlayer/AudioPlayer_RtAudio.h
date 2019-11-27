#ifndef AUDIOPLAYER_RTAUDIO_H
#define AUDIOPLAYER_RTAUDIO_H

#include <thread>
#include <list>

#include <RtAudio.h>

#include "AudioPlayer/AudioPlayer.h"

class AudioPlayer_RtAudio : public AudioPlayer
{
public:
    AudioPlayer_RtAudio();
    ~AudioPlayer_RtAudio();

    std::list<AudioDevice> getAudiDeviceList(); //获取音频设备列表

private:
    RtAudio dac;
    int mDeviceId;

protected:
    bool openDevice();
    bool closeDevice();

    void AudioCallBack(void *stream, int len);

protected:
    static int AudioCallBackFunc(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
                                 double /*streamTime*/, RtAudioStreamStatus /*status*/, void *userdata);

};

#endif // AUDIOPLAYER_RTAUDIO_H
