
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
#endif

#define PCMTYPE short
#define MAXPCMVALUE 32767

ReadAudioFileThread::ReadAudioFileThread()
{
    mAudioCallBack = nullptr;

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

void ReadAudioFileThread::startRead(const std::string &filePath)
{
    mFilePath = filePath;

    //启动新的线程实现读取视频文件
    std::thread([&](ReadAudioFileThread *pointer)
    {
        pointer->run();

    }, this).detach();
}

void ReadAudioFileThread::run()
{
//    mIsReadThreadFinished = false;
//    mIsReadFinished = false;

    mAudioPlayer->startPlay();

    int totalFrameNum = 0; //总帧数
    int frameNum = 0; //当前播放的帧序号
    uint32_t totalTime = 0; //总时长（毫秒）
    uint32_t currentTime = 0; //当前播放时间(毫秒)

    const char * file_path = mFilePath.c_str();

    pFormatCtx = nullptr;
    pCodecCtx = nullptr;
    pCodec = nullptr;

    aCodecCtx = nullptr;
    aCodec = nullptr;
    aFrame = nullptr;

    mAudioStream = nullptr;
    mVideoStream = nullptr;

    audio_clock = 0;
    video_clock = 0;

    int audioStream ,videoStream;

    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, file_path, nullptr, nullptr) != 0)
    {
        fprintf(stderr, "can't open the file. \n");
//        doOpenVideoFileFailed();
        goto end;
    }

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        fprintf(stderr, "Could't find stream infomation.\n");
//        doOpenVideoFileFailed();
        goto end;
    }

    videoStream = -1;
    audioStream = -1;

    ///循环查找视频中包含的流信息，
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
        }
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO  && audioStream < 0)
        {
            audioStream = i;
        }
    }

//    doTotalTimeChanged(getTotalTime());

    ///打开视频解码器，并启动视频线程
    if (videoStream >= 0)
    {
        ///查找视频解码器
        pCodecCtx = pFormatCtx->streams[videoStream]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

        if (pCodec == nullptr)
        {
            fprintf(stderr, "PCodec not found.\n");
//            doOpenVideoFileFailed();
            goto end;
        }

        ///打开视频解码器
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            fprintf(stderr, "Could not open video codec.\n");
//            doOpenVideoFileFailed();
            goto end;
        }

        mVideoStream = pFormatCtx->streams[videoStream];
    }

    if (audioStream >= 0)
    {
        ///查找音频解码器
        aCodecCtx = pFormatCtx->streams[audioStream]->codec;
        aCodec = avcodec_find_decoder(aCodecCtx->codec_id);

        if (aCodec == NULL)
        {
            fprintf(stderr, "ACodec not found.\n");
            audioStream = -1;
        }
        else
        {
            ///打开音频解码器
            if (avcodec_open2(aCodecCtx, aCodec, nullptr) < 0)
            {
                fprintf(stderr, "Could not open audio codec.\n");
//                doOpenVideoFileFailed();
                goto end;
            }

            ///解码音频相关
            aFrame = av_frame_alloc();


            //重采样设置选项-----------------------------------------------------------start
            aFrame_ReSample = nullptr;

            //frame->16bit 44100 PCM 统一音频采样格式与采样率
            swrCtx = nullptr;

            //输入的声道布局
            int in_ch_layout;

            //输出的声道布局
            int out_ch_layout = av_get_default_channel_layout(audio_tgt_channels); ///AV_CH_LAYOUT_STEREO

            out_ch_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;

            /// 这里音频播放使用了固定的参数
            /// 强制将音频重采样成44100 双声道  AV_SAMPLE_FMT_S16
            /// SDL播放中也是用了同样的播放参数
            //重采样设置选项----------------
            //输入的采样格式
            in_sample_fmt = aCodecCtx->sample_fmt;
            //输出的采样格式 16bit PCM
            out_sample_fmt = AV_SAMPLE_FMT_S16;
            //输入的采样率
            in_sample_rate = aCodecCtx->sample_rate;
            //输入的声道布局
            in_ch_layout = aCodecCtx->channel_layout;

            //输出的采样率
            out_sample_rate = 44100;
            //输出的声道布局

            audio_tgt_channels = 2; ///av_get_channel_layout_nb_channels(out_ch_layout);
            out_ch_layout = av_get_default_channel_layout(audio_tgt_channels); ///AV_CH_LAYOUT_STEREO

            out_ch_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;

            /// 2019-5-13添加
            /// wav/wmv 文件获取到的aCodecCtx->channel_layout为0会导致后面的初始化失败，因此这里需要加个判断。
            if (in_ch_layout <= 0)
            {
                in_ch_layout = av_get_default_channel_layout(aCodecCtx->channels);
            }

            swrCtx = swr_alloc_set_opts(nullptr, out_ch_layout, out_sample_fmt, out_sample_rate,
                                                 in_ch_layout, in_sample_fmt, in_sample_rate, 0, nullptr);

            /** Open the resampler with the specified parameters. */
            int ret = swr_init(swrCtx);
            if (ret < 0)
            {
                char buff[128]={0};
                av_strerror(ret, buff, 128);

                fprintf(stderr, "Could not open resample context %s\n", buff);
                swr_free(&swrCtx);
                swrCtx = nullptr;
//                doOpenVideoFileFailed();
                goto end;
            }

            //存储pcm数据
            int out_linesize = out_sample_rate * audio_tgt_channels;

    //        out_linesize = av_samples_get_buffer_size(NULL, audio_tgt_channels, av_get_bytes_per_sample(out_sample_fmt), out_sample_fmt, 1);
            out_linesize = AVCODEC_MAX_AUDIO_FRAME_SIZE;


            mAudioStream = pFormatCtx->streams[audioStream];
        }

    }

    av_dump_format(pFormatCtx, 0, file_path, 0); //输出视频信息

//    mPlayerState = VideoPlayer_Playing;
//    doPlayerStateChanged(VideoPlayer_Playing, mVideoStream != nullptr, mAudioStream != nullptr);

    mVideoStartTime = av_gettime();
fprintf(stderr, "%s mIsQuit=%d mIsPause=%d \n", __FUNCTION__, mIsQuit, mIsPause);
    while (1)
    {
        if (mIsQuit)
        {
            //停止播放了
            break;
        }

//        if (seek_req)
//        {
//            int stream_index = -1;
//            int64_t seek_target = seek_pos;

//            if (videoStream >= 0)
//                stream_index = videoStream;
//            else if (audioStream >= 0)
//                stream_index = audioStream;

//            AVRational aVRational = {1, AV_TIME_BASE};
//            if (stream_index >= 0)
//            {
//                seek_target = av_rescale_q(seek_target, aVRational, pFormatCtx->streams[stream_index]->time_base);
//            }

//            if (av_seek_frame(pFormatCtx, stream_index, seek_target, AVSEEK_FLAG_BACKWARD) < 0)
//            {
//                fprintf(stderr, "%s: error while seeking\n",pFormatCtx->filename);
//            }
//            else
//            {
//                if (audioStream >= 0)
//                {
//                    AVPacket packet;
//                    av_new_packet(&packet, 10);
//                    strcpy((char*)packet.data,FLUSH_DATA);
//                    clearAudioQuene(); //清除队列
//                    inputAudioQuene(packet); //往队列中存入用来清除的包
//                }

//                if (videoStream >= 0)
//                {
//                    AVPacket packet;
//                    av_new_packet(&packet, 10);
//                    strcpy((char*)packet.data,FLUSH_DATA);
//                    clearVideoQuene(); //清除队列
//                    inputVideoQuene(packet); //往队列中存入用来清除的包
//                    video_clock = 0;
//                }

//                mVideoStartTime = av_gettime() - seek_pos;
//                mPauseStartTime = av_gettime();
//            }
//            seek_req = 0;
//            seek_time = seek_pos / 1000000.0;
//            seek_flag_audio = 1;
//            seek_flag_video = 1;

//            if (mIsPause)
//            {
//                mIsNeedPause = true;
//                mIsPause = false;
//            }

//        }

//        //这里做了个限制  当队列里面的数据超过某个大小的时候 就暂停读取  防止一下子就把视频读完了，导致的空间分配不足
//        //这个值可以稍微写大一些
//        if (mAudioPacktList.size() > MAX_AUDIO_SIZE || mVideoPacktList.size() > MAX_VIDEO_SIZE)
//        {
//            mSleep(10);
//            continue;
//        }

//        if (mIsPause == true)
//        {
//            mSleep(10);
//            continue;
//        }

        AVPacket packet;
        if (av_read_frame(pFormatCtx, &packet) < 0)
        {
            mIsReadFinished = true;

            if (mIsQuit)
            {
                break; //解码线程也执行完了 可以退出了
            }

            MoudleConfig::mSleep(10);
            continue;
        }

        if( packet.stream_index == audioStream )
        {
            fprintf(stderr, "%s size=%d pts=%I64d \n", __FUNCTION__, packet.size, packet.pts);


            //解码AVPacket->AVFrame
            if (int ret = avcodec_send_packet(aCodecCtx, &packet) && ret != 0)
            {
               char buffer[1024] = {0};
               av_strerror(ret, buffer, 1024);
               fprintf(stderr, "input AVPacket to decoder failed! ret = %d %s\n", ret, buffer);
            }
            else
            {
            //    while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
                while(1)
                {
                    int ret = avcodec_receive_frame(aCodecCtx, aFrame);
                    if (ret != 0)
                    {
            //            char buffer[1024] = {0};
            //            av_strerror(ret, buffer, 1024);
            //            fprintf(stderr, "avcodec_receive_frame = %d %s\n", ret, buffer);
                        break;
                    }

                    /// ffmpeg解码之后得到的音频数据不是SDL想要的，
                    /// 因此这里需要重采样成44100 双声道 AV_SAMPLE_FMT_S16
                    if (aFrame_ReSample == NULL)
                    {
                        aFrame_ReSample = av_frame_alloc();
                    }

                    if (aFrame_ReSample->nb_samples != aFrame->nb_samples)
                    {
                        aFrame_ReSample->nb_samples = av_rescale_rnd(swr_get_delay(swrCtx, out_sample_rate) + aFrame->nb_samples,
                                    out_sample_rate, in_sample_rate, AV_ROUND_UP);

                        av_samples_fill_arrays(aFrame_ReSample->data, aFrame_ReSample->linesize, audio_buf, audio_tgt_channels, aFrame_ReSample->nb_samples, out_sample_fmt, 0);

                    }

                    int len2 = swr_convert(swrCtx, aFrame_ReSample->data, aFrame_ReSample->nb_samples, (const uint8_t**)aFrame->data, aFrame->nb_samples);
                    int resampled_data_size = len2 * audio_tgt_channels * av_get_bytes_per_sample(out_sample_fmt);

                    frameNum++;
                    fprintf(stderr, "%s [%d] size=%d pts=%I64d %d\n", __FUNCTION__, frameNum, packet.size, packet.pts, resampled_data_size);

                    PCMFramePtr pcmFramePtr = std::make_shared<PCMFrame>();
                    pcmFramePtr->setFrameBuffer(audio_buf, resampled_data_size);

                    mAudioPlayer->inputPCMFrame(pcmFramePtr);

                    static FILE * fp = fopen("out.pcm", "wb");
                    fwrite(audio_buf, 1, resampled_data_size, fp);
                }
            }

    //保存重采样之前的一个声道的数据方法
    //size_t unpadded_linesize = aFrame->nb_samples * av_get_bytes_per_sample((AVSampleFormat) aFrame->format);
    //static FILE * fp = fopen("out.pcm", "wb");
    //fwrite(aFrame->extended_data[0], 1, unpadded_linesize, fp);

            av_packet_unref(&packet);
        }
        else
        {
            // Free the packet that was allocated by av_read_frame
            av_packet_unref(&packet);
        }
    }

    ///文件读取结束 跳出循环的情况
    ///等待播放完毕
    while (!mIsQuit)
    {
        MoudleConfig::mSleep(100);
    }

end:

//    clearAudioQuene();
//    clearVideoQuene();

//    if (mPlayerState != VideoPlayer_Stop) //不是外部调用的stop 是正常播放结束
//    {
//        stop();
//    }

//    while((mVideoStream != nullptr && !mIsVideoThreadFinished) || (mAudioStream != nullptr && !mIsAudioThreadFinished))
//    {
//        mSleep(10);
//    } //确保视频线程结束后 再销毁队列

    if (swrCtx != nullptr)
    {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }

    if (aFrame != nullptr)
    {
        av_frame_free(&aFrame);
        aFrame = nullptr;
    }

    if (aFrame_ReSample != nullptr)
    {
        av_frame_free(&aFrame_ReSample);
        aFrame_ReSample = nullptr;
    }

    if (aCodecCtx != nullptr)
    {
        avcodec_close(aCodecCtx);
        aCodecCtx = nullptr;
    }

    if (pCodecCtx != nullptr)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
    }

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

//    doPlayerStateChanged(VideoPlayer_Stop, mVideoStream != nullptr, mAudioStream != nullptr);

    mIsReadThreadFinished = true;

fprintf(stderr, "%s finished \n", __FUNCTION__);
}

//void ReadAudioFileThread::run()
//{

//    char *fileName = mFileName;
//    FILE *fp = fopen(fileName, "rb");
//    if (fp == nullptr)
//    {
//        fprintf(stderr, "AAC file not exist! \n");
//        return;
//    }

//    mAudioPlayer->startPlay();

//    int totalFrameNum = 0; //总帧数
//    int frameNum = 0; //当前播放的帧序号
//    uint32_t totalTime = 0; //总时长（毫秒）
//    uint32_t currentTime = 0; //当前播放时间(毫秒)

//    ///获取音频强度数据，用于绘制波形图
//    {
//        std::list<float> mLeftChannelDbValues;
//        std::list<float> mRighttChannelDbValues;

//        while(!feof(fp))
//        {
////            char buf[10240];
////            int size = fread(buf, 1, 1024, fp);//从h264文件读1024个字节 (模拟从网络收到h264流)
////            int nCount = mAACReader->inputAACData((uint8_t*)buf,size);

////            while(1)
////            {
////                //从前面读到的数据中获取一个nalu
////                AACFramePtr framePtr = mAACReader->getNextFrame();
////                if (framePtr == nullptr || framePtr.get() == nullptr) break;

////                AACFrame *aacFrame = framePtr.get();

////                PCMFramePtr pcmFramePtr = mAudioDecoder->decode(aacFrame->getBuffer(), aacFrame->getSize());

////                PCMTYPE *buffer = (PCMTYPE *)pcmFramePtr->getBuffer();

////                /// 这里直接使用前两个字节转成short，然后作为纵坐标来绘制波形图。
////                /// PS:这里的PCM音频数据排列方式为，左右左右左右左右...
////                mLeftChannelDbValues.push_back(buffer[0] * 1.0 / MAXPCMVALUE);
////                mRighttChannelDbValues.push_back(buffer[1] * 1.0 / MAXPCMVALUE);

////                totalFrameNum++;
////            }
//        }

//        if (mAudioCallBack != nullptr)
//        {
//            mAudioCallBack->onGetPcmWaveValues(mLeftChannelDbValues, mRighttChannelDbValues);
//        }

//        fseek(fp, 0, SEEK_SET);
//    }

//    ///计算总时长
//    {
//        /// 这里一帧音频的采样是1024，音频的采样率是一秒钟44100次，总时长便很容易就算。
//        /// 当然也可以通过直接计算数据总大小，原理都是类似。
//        totalTime = totalFrameNum * 1024.0 / 44100 * 1000; //单位是毫秒
//    }

//    while(!feof(fp))
//    {
////        char buf[10240];
////        int size = fread(buf, 1, 1024, fp);//从h264文件读1024个字节 (模拟从网络收到h264流)
////        int nCount = mAACReader->inputAACData((uint8_t*)buf,size);

////        while(1)
////        {
////            //从前面读到的数据中获取一个nalu
////            AACFramePtr framePtr = mAACReader->getNextFrame();
////            if (framePtr == nullptr || framePtr.get() == nullptr) break;

////            frameNum++;

////            AACFrame *aacFrame = framePtr.get();

////            PCMFramePtr pcmFramePtr = mAudioDecoder->decode(aacFrame->getBuffer(), aacFrame->getSize());

////            ///延时等待
////            while(1)
////            {
////                if (mAudioPlayer->getPcmFrameSize() <= 3)
////                {
////                    mAudioPlayer->inputPCMFrame(pcmFramePtr);
////                    break;
////                }
////                else
////                {
////                    MoudleConfig::mSleep(50);
////                }
////            }

////            if (mAudioCallBack != nullptr)
////            {
////                PCMTYPE *buffer = (PCMTYPE *)pcmFramePtr->getBuffer();

////                ///记录当前帧的所有采样点db值
////                std::list<float> leftChannelDbValues;
////                std::list<float> righttChannelDbValues;

////                ///记录当前帧的所有采样点db值总和
////                uint64_t leftChannelTotal = 0;
////                uint64_t rightChannelTotal = 0;

////                /// 计算分贝 音频数据与大小
////                /// 首先我们分别累加每个采样点的数值，除以采样个数，得到声音平均能量值。
////                /// 然后再将其做100与32767之间的等比量化。得到1-100的量化值。
////                /// 通常情况下，人声分布在较低的能量范围，这样就会使量化后的数据大致分布在1-20的较小区间，不能够很敏感的感知变化。
////                /// 所以我们将其做了5倍的放大，当然计算后大于100的值，我们将其赋值100.
////                /// PS:这里的PCM音频数据排列方式为，左右左右左右左右..
////                int nums = pcmFramePtr->getSize() / sizeof (PCMTYPE);
////                for (int i=0;i<nums;)
////                {
////                    leftChannelTotal += abs(buffer[i]);
////                    rightChannelTotal += abs(buffer[i+1]);

////                    ///左声道数据放大5倍展示
////                    leftChannelDbValues.push_back(buffer[i] * 1.0 / MAXPCMVALUE);

////                    if (i % 500 == 0)
////                    {
////                        righttChannelDbValues.push_back(buffer[i+1] * 1.0 / MAXPCMVALUE);
////                    }

////                    i+=2;
////                }

////                ///记录当前帧的所有采样点db值平均值
////                PCMTYPE leftChannels  = leftChannelTotal / nums;
////                PCMTYPE rightChannels = rightChannelTotal / nums;

////                float leftChannelDb = leftChannels * 5.0 / MAXPCMVALUE;
////                if (leftChannelDb > 1.0f)
////                {
////                    leftChannelDb = 1.0f;
////                }

////                float rightChannelDb = rightChannels * 5.0 / MAXPCMVALUE;
////                if (rightChannelDb > 1.0f)
////                {
////                    rightChannelDb = 1.0f;
////                }

////                float progress = frameNum * 1.0 / totalFrameNum;
////                currentTime = frameNum * 1024.0 / 44100 * 1000; //单位是毫秒

////                mAudioCallBack->onGetPcmFrame(pcmFramePtr);
////                mAudioCallBack->onUpdatePlayingTime(totalTime, currentTime);
////                mAudioCallBack->onUpdatePlayingValue(leftChannelDb, rightChannelDb, leftChannelDbValues, righttChannelDbValues, progress);
////            }
////        }
//    }

//}
