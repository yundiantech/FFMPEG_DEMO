
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef READAUDIOFILETHREAD_H
#define READAUDIOFILETHREAD_H

#include "AudioPlayer/AudioPlayer_SDL.h"
#include "AudioPlayer/AudioPlayer_RtAudio.h"

#include "EventHandle/AudioPlayerEventHandle.h"

#include <thread>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/time.h>
    #include <libavutil/pixfmt.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

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

    void startRead(const std::string &filePath);

    std::list<AudioDevice> getAudiDeviceList();

protected:
    void run();

private:
    std::string mFilePath;

    AudioPlayer *mAudioPlayer;

    ///播放控制相关
    bool mIsNeedPause; //暂停后跳转先标记此变量
    bool mIsPause;  //暂停标志
    bool mIsQuit;   //停止
    bool mIsReadFinished; //文件读取完毕
    bool mIsReadThreadFinished;
    bool mIsVideoThreadFinished; //视频解码线程
    bool mIsAudioThreadFinished; //音频播放线程

    ///音视频同步相关
    uint64_t mVideoStartTime; //开始播放视频的时间
    uint64_t mPauseStartTime; //暂停开始的时间
    double audio_clock; ///音频时钟
    double video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    AVStream *mVideoStream; //视频流
    AVStream *mAudioStream; //音频流

    ///视频相关
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    ///音频相关
    AVCodecContext *aCodecCtx;
    AVCodec *aCodec;
    AVFrame *aFrame;

    ///以下变量用于音频重采样
    /// 由于ffmpeg解码出来后的pcm数据有可能是带平面的pcm，因此这里统一做重采样处理，
    /// 重采样成44100的16 bits 双声道数据(AV_SAMPLE_FMT_S16)
    AVFrame *aFrame_ReSample;
    SwrContext *swrCtx;

    enum AVSampleFormat in_sample_fmt; //输入的采样格式
    enum AVSampleFormat out_sample_fmt;//输出的采样格式 16bit PCM
    int in_sample_rate;//输入的采样率
    int out_sample_rate;//输出的采样率
    int audio_tgt_channels; ///av_get_channel_layout_nb_channels(out_ch_layout);
    unsigned int audio_buf_size;
    unsigned int audio_buf_index;
    DECLARE_ALIGNED(16,uint8_t,audio_buf) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];


    ///回调函数相关，主要用于输出信息给界面
private:
    ///回调函数
    AudioPlayerCallBack *mAudioCallBack;

//    ///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
//    void doDisplayVideo(const uint8_t *yuv420Buffer, const int &width, const int &height, const int &frameNum);

};

#endif // READAUDIOFILETHREAD_H
