#include "AudioPlayer.h"

#include "PcmVolumeControl.h"

#include <stdio.h>

AudioPlayer::AudioPlayer()
{
    mCond = new Cond();
}

AudioPlayer::~AudioPlayer()
{

}

bool AudioPlayer::startPlay()
{
    bool isSucceed = openDevice();

    return isSucceed;
}

bool AudioPlayer::stopPlay()
{
    bool isSucceed = closeDevice();

    return isSucceed;
}

void AudioPlayer::inputPCMFrame(PCMFramePtr framePtr)
{
    mCond->Lock();
    mPcmFrameList.push_back(framePtr);
    mCond->Unlock();
    mCond->Signal();
}

int AudioPlayer::getPcmFrameSize()
{
//    mCond->Lock();
    int size = mPcmFrameList.size();
//    mCond->Unlock();

    return size;
}

void AudioPlayer::playAudioBuffer(void *stream, int len)
{
    PCMFramePtr pcmFramePtr = nullptr;
//fprintf(stderr, "%s %d %d \n", __FUNCTION__, len, mPcmFrameList.size());
    mCond->Lock();
    if (!mPcmFrameList.empty())
    {
        pcmFramePtr = mPcmFrameList.front();
        mPcmFrameList.pop_front();
    }
    mCond->Unlock();

    if (pcmFramePtr != nullptr)
    {
        /// 这里我的PCM数据都是AV_SAMPLE_FMT_S16 44100 双声道的，且采样都是1024，因此pcmFramePtr->getSize()和len值相等都为4096.
        /// 所以这里直接拷贝过去就好了。
//        fprintf(stderr, "%s %d %d \n", __FUNCTION__, pcmFramePtr->getSize(), len);
        memcpy(stream, (uint8_t *)pcmFramePtr->getBuffer(), pcmFramePtr->getSize());
        len -= pcmFramePtr->getSize();
    }

}
