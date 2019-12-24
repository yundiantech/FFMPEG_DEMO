/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef GetAudioThread_H
#define GetAudioThread_H

#include <functional>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
    #include "libavdevice/avdevice.h"
    #include "libavutil/imgutils.h"
}

#include "AudioEncoder.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

/**
 * @brief The GetVideoThread class  此类主要负责采集屏幕
 */

class GetAudioThread
{

public:
    explicit GetAudioThread();
    ~GetAudioThread();

    bool init(const char * const deviceName);
    void deInit();

    void startRecord(int outOneFrameSize, std::function<void (PCMFramePtr pcmFrame, int index)> func, int funcIndex);
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

protected:
    void run();

private:
    int mONEFrameSize; //输出的一帧数据大小

    int mFuncIndex;
    std::function<void (PCMFramePtr pcmFrame, int index)> mCallBackFunc; //回调函数

    AVFormatContext	*pFormatCtx;
    int             audioStream;
    AVCodecContext	*aCodecCtx;

    AVFrame	*aFrame;

    bool m_isRun;
    bool m_pause;
    bool m_getFirst; //是否获取到了时间基准

    ///以下变量用于音频重采样
    /// 由于ffmpeg编码aac需要输入FLTP格式的数据。
    /// 因此这里将音频重采样成44100 双声道  AV_SAMPLE_FMT_FLTP
    AVFrame *aFrame_ReSample;
    SwrContext *swrCtx;

    enum AVSampleFormat in_sample_fmt; //输入的采样格式
    enum AVSampleFormat out_sample_fmt;//输出的采样格式 16bit PCM
    int in_sample_rate;//输入的采样率
    int out_sample_rate;//输出的采样率
    int audio_tgt_channels; ///av_get_channel_layout_nb_channels(out_ch_layout);
    int out_ch_layout;

    ///用于存储读取到的音频数据
    /// 由于平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，因此这里需要将左右声道数据分开存放
    DECLARE_ALIGNED(16, uint8_t, audio_buf_L) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
    unsigned int audio_buf_size_L;
    DECLARE_ALIGNED(16, uint8_t, audio_buf_R) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
    unsigned int audio_buf_size_R;

    bool initResample();

    void dealWithAudioFrame(const int &OneChannelDataSize);
};

#endif // GetVideoThread_H
