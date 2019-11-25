/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "AudioFrame/PCMFrame.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
}

typedef unsigned char uchar;

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

class AudioDecoder
{
public:
    AudioDecoder();

    bool openDecoder();
    void closeDecoder();

    PCMFramePtr decode(uint8_t *inputbuf, int frame_size);

private:

    AVCodec         *aCodec;
    AVCodecContext  *aCodecCtx;

    AVFrame *aFrame;

    ///以下变量用于音频重采样
    /// 由于ffmpeg解码出来后的pcm数据是带平面的pcm，因此这里统一做重采样处理，
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

    bool initResample();

};

#endif // AUDIODECORDER_H
