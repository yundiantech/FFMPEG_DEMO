#ifndef AUDIOPLAYEREVENTHANDLE_H
#define AUDIOPLAYEREVENTHANDLE_H

#include <stdint.h>
#include <list>

#include "AudioFrame/PCMFrame.h"

class AudioPlayerCallBack
{
public:
    virtual ~AudioPlayerCallBack();

    ///获取到音频波形图数据
    virtual void onGetPcmWaveValues(const std::list<float> &leftChannelValues, const std::list<float> &rightChannelValues) = 0;

    ///当前播放的一帧音频数据
    virtual void onGetPcmFrame(PCMFramePtr pcmFramePtr) = 0;

    /**
     * @brief onUpdatePlayingValue 更新正在播放的音频强度数据
     * @param leftChannel    当前帧左声道所有采样点DB值平均值（放大5倍了）
     * @param rightChannel   当前帧右声道所有采样点DB值平均值（放大5倍了）
     * @param leftChannelDbValues  当前帧左声道所有采样点DB值
     * @param rightChannelDbValues 当前帧右声道所有采样点DB值平均值
     * @param progress      播放进度(0.0~1.0)
     */
    virtual void onUpdatePlayingValue(const float &leftChannel,
                                      const float &rightChannel,
                                      const std::list<float> &leftChannelDbValues,
                                      const std::list<float> &rightChannelDbValues,
                                      const float &progress) = 0;

    ///更新播放时间
    virtual void onUpdatePlayingTime(const uint32_t &totalTime, const uint32_t &currentTime) = 0;

};

#endif // AUDIOPLAYEREVENTHANDLE_H
