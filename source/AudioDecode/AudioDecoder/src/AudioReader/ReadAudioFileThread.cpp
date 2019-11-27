
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include <thread>

#include "MoudleConfig.h"
#include "ReadAudioFileThread.h"

#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <direct.h>
#include <io.h> //C (Windows)    access
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds)
{
    usleep(mSeconds * 1000);
}

#endif

#define PCMTYPE short
#define MAXPCMVALUE 32767

ReadAudioFileThread::ReadAudioFileThread()
{
    mAudioCallBack = nullptr;

    mAACReader    = new AACReader();
    mAudioDecoder = new AudioDecoder();

#if 1
    mAudioPlayer  = new AudioPlayer_RtAudio();
#else
    mAudioPlayer  = new AudioPlayer_SDL();
#endif
}

ReadAudioFileThread::~ReadAudioFileThread()
{

}

std::list<AudioDevice> ReadAudioFileThread::getAudiDeviceList()
{
    return  mAudioPlayer->getAudiDeviceList();
}

void ReadAudioFileThread::startRead(char* filePath)
{
    strcpy(mFileName, filePath);

    //启动新的线程实现读取视频文件
    std::thread([&](ReadAudioFileThread *pointer)
    {
        pointer->run();

    }, this).detach();
}

void ReadAudioFileThread::run()
{

    char *fileName = mFileName;
    FILE *fp = fopen(fileName, "rb");
    if (fp == nullptr)
    {
        fprintf(stderr, "AAC file not exist! \n");
        return;
    }

    mAudioDecoder->openDecoder();
    mAudioPlayer->startPlay();

    int totalFrameNum = 0; //总帧数
    int frameNum = 0; //当前播放的帧序号
    uint32_t totalTime = 0; //总时长（毫秒）
    uint32_t currentTime = 0; //当前播放时间(毫秒)

    ///获取音频强度数据，用于绘制波形图
    {
        std::list<float> mLeftChannelDbValues;
        std::list<float> mRighttChannelDbValues;

        while(!feof(fp))
        {
            char buf[10240];
            int size = fread(buf, 1, 1024, fp);//从h264文件读1024个字节 (模拟从网络收到h264流)
            int nCount = mAACReader->inputAACData((uint8_t*)buf,size);

            while(1)
            {
                //从前面读到的数据中获取一个nalu
                AACFramePtr framePtr = mAACReader->getNextFrame();
                if (framePtr == nullptr || framePtr.get() == nullptr) break;

                AACFrame *aacFrame = framePtr.get();

                PCMFramePtr pcmFramePtr = mAudioDecoder->decode(aacFrame->getBuffer(), aacFrame->getSize());

                PCMTYPE *buffer = (PCMTYPE *)pcmFramePtr->getBuffer();

                /// 这里直接使用前两个字节转成short，然后作为纵坐标来绘制波形图。
                /// PS:这里的PCM音频数据排列方式为，左右左右左右左右...
                mLeftChannelDbValues.push_back(buffer[0] * 1.0 / MAXPCMVALUE);
                mRighttChannelDbValues.push_back(buffer[1] * 1.0 / MAXPCMVALUE);

                totalFrameNum++;
            }
        }

        if (mAudioCallBack != nullptr)
        {
            mAudioCallBack->onGetPcmWaveValues(mLeftChannelDbValues, mRighttChannelDbValues);
        }

        fseek(fp, 0, SEEK_SET);
    }

    ///计算总时长
    {
        /// 这里一帧音频的采样是1024，音频的采样率是一秒钟44100次，总时长便很容易就算。
        /// 当然也可以通过直接计算数据总大小，原理都是类似。
        totalTime = totalFrameNum * 1024.0 / 44100 * 1000; //单位是毫秒
    }

    while(!feof(fp))
    {
        char buf[10240];
        int size = fread(buf, 1, 1024, fp);//从h264文件读1024个字节 (模拟从网络收到h264流)
        int nCount = mAACReader->inputAACData((uint8_t*)buf,size);

        while(1)
        {
            //从前面读到的数据中获取一个nalu
            AACFramePtr framePtr = mAACReader->getNextFrame();
            if (framePtr == nullptr || framePtr.get() == nullptr) break;

            frameNum++;

            AACFrame *aacFrame = framePtr.get();

            PCMFramePtr pcmFramePtr = mAudioDecoder->decode(aacFrame->getBuffer(), aacFrame->getSize());

            ///延时等待
            while(1)
            {
                if (mAudioPlayer->getPcmFrameSize() <= 3)
                {
                    mAudioPlayer->inputPCMFrame(pcmFramePtr);
                    break;
                }
                else
                {
                    MoudleConfig::mSleep(50);
                }
            }

            if (mAudioCallBack != nullptr)
            {
                PCMTYPE *buffer = (PCMTYPE *)pcmFramePtr->getBuffer();

                ///记录当前帧的所有采样点db值
                std::list<float> leftChannelDbValues;
                std::list<float> righttChannelDbValues;

                ///记录当前帧的所有采样点db值总和
                uint64_t leftChannelTotal = 0;
                uint64_t rightChannelTotal = 0;

                /// 计算分贝 音频数据与大小
                /// 首先我们分别累加每个采样点的数值，除以采样个数，得到声音平均能量值。
                /// 然后再将其做100与32767之间的等比量化。得到1-100的量化值。
                /// 通常情况下，人声分布在较低的能量范围，这样就会使量化后的数据大致分布在1-20的较小区间，不能够很敏感的感知变化。
                /// 所以我们将其做了5倍的放大，当然计算后大于100的值，我们将其赋值100.
                /// PS:这里的PCM音频数据排列方式为，左右左右左右左右..
                int nums = pcmFramePtr->getSize() / sizeof (PCMTYPE);
                for (int i=0;i<nums;)
                {
                    leftChannelTotal += abs(buffer[i]);
                    rightChannelTotal += abs(buffer[i+1]);

                    ///左声道数据放大5倍展示
                    leftChannelDbValues.push_back(buffer[i] * 1.0 / MAXPCMVALUE);

                    if (i % 500 == 0)
                    {
                        righttChannelDbValues.push_back(buffer[i+1] * 1.0 / MAXPCMVALUE);
                    }

                    i+=2;
                }

                ///记录当前帧的所有采样点db值平均值
                PCMTYPE leftChannels  = leftChannelTotal / nums;
                PCMTYPE rightChannels = rightChannelTotal / nums;

                float leftChannelDb = leftChannels * 5.0 / MAXPCMVALUE;
                if (leftChannelDb > 1.0f)
                {
                    leftChannelDb = 1.0f;
                }

                float rightChannelDb = rightChannels * 5.0 / MAXPCMVALUE;
                if (rightChannelDb > 1.0f)
                {
                    rightChannelDb = 1.0f;
                }

                float progress = frameNum * 1.0 / totalFrameNum;
                currentTime = frameNum * 1024.0 / 44100 * 1000; //单位是毫秒

                mAudioCallBack->onGetPcmFrame(pcmFramePtr);
                mAudioCallBack->onUpdatePlayingTime(totalTime, currentTime);
                mAudioCallBack->onUpdatePlayingValue(leftChannelDb, rightChannelDb, leftChannelDbValues, righttChannelDbValues, progress);
            }
        }
    }

    mAudioDecoder->closeDecoder();
}
