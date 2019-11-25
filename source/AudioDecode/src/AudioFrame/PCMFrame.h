#ifndef PCMFRAME_H
#define PCMFRAME_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory>

//extern "C"
//{
//    #include <libavutil/samplefmt.h>
//}

#define PCMFramePtr std::shared_ptr<PCMFrame>

///本程序固定使用AV_SAMPLE_FMT_S16 44100 双声道
class PCMFrame
{
public:
    PCMFrame();
    ~PCMFrame();

    void setFrameBuffer(const uint8_t * const buffer, const unsigned int &size);

    uint8_t *getBuffer(){return mFrameBuffer;}
    unsigned int getSize(){return  mFrameBufferSize;}

private:
    uint8_t *mFrameBuffer; //pcm数据
    unsigned int mFrameBufferSize; //pcm数据长度

//    enum AVSampleFormat mSampleFmt;//输出的采样格式
//    int mSampleRate;//采样率
//    int mChannels; //声道数

};

#endif // PCMFRAME_H
