/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AACDecoder.h"

AACDecoder::AACDecoder()
{
    aCodec = nullptr;
    aCodecCtx = nullptr;

    aFrame = nullptr;
    aFrame_ReSample = nullptr;
    swrCtx = nullptr;
}

bool AACDecoder::openDecoder()
{
    ///打开音频解码器
    //find the decoder
    aCodec = avcodec_find_decoder(AV_CODEC_ID_AAC);

    if (aCodec == nullptr)
    {
        fprintf(stderr, "audio Codec not found.\n");
        return false;
    }
    else
    {
        aCodecCtx = avcodec_alloc_context3(aCodec);
    }

    if(avcodec_open2(aCodecCtx, aCodec, nullptr)<0)
    {
        printf("Could not open audio codec.\n");
        return false;
    }

    ///解码音频相关
    aFrame = av_frame_alloc();

    return true;
}

void AACDecoder::closeDecoder()
{
    avcodec_close(aCodecCtx);
    av_free(aCodecCtx);
    av_free(aFrame);

    aCodec = nullptr;
    aCodecCtx = nullptr;

    aFrame = nullptr;

}

bool AACDecoder::initResample()
{
    //重采样设置选项-----------------------------------------------------------start
    aFrame_ReSample = av_frame_alloc();

    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    swrCtx = nullptr;

    //输入的声道布局
    int in_ch_layout;

    //输出的声道布局
    int out_ch_layout = av_get_default_channel_layout(audio_tgt_channels); ///AV_CH_LAYOUT_STEREO

    out_ch_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;

    /// 由于ffmpeg解码后的数据为FLTP格式的数据，这是平面的PCM数据。
    /// 而SDL播放需要不带平面的AV_SAMPLE_FMT_S16的PCM数据
    /// 因此这里将音频重采样成44100 双声道  AV_SAMPLE_FMT_S16
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

        return false;
    }
    return true;
}
//FILE *fp = fopen("out.pcm", "wb");
PCMFramePtr AACDecoder::decode(uint8_t *inputbuf, int frame_size)
{
    PCMFramePtr framePtr = nullptr;

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = inputbuf;
    pkt.size = frame_size;

    if (int ret = avcodec_send_packet(aCodecCtx, &pkt) && ret != 0)
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

            ///解码一帧后才能获取到采样率等信息，因此将初始化放到这里
            if (aFrame_ReSample == nullptr)
            {
                initResample();
            }

            if (aFrame_ReSample->nb_samples != aFrame->nb_samples)
            {
                fprintf(stderr, "%d %d %d %d\n", out_sample_rate, aFrame->nb_samples, in_sample_rate, aFrame->sample_rate);
                aFrame_ReSample->nb_samples = av_rescale_rnd(swr_get_delay(swrCtx, out_sample_rate) + aFrame->nb_samples,
                                                                            out_sample_rate, in_sample_rate, AV_ROUND_UP);

                av_samples_fill_arrays(aFrame_ReSample->data, aFrame_ReSample->linesize, audio_buf, audio_tgt_channels, aFrame_ReSample->nb_samples, out_sample_fmt, 0);

            }

            ///执行重采样
            int len2 = swr_convert(swrCtx, aFrame_ReSample->data, aFrame_ReSample->nb_samples, (const uint8_t**)aFrame->data, aFrame->nb_samples);
            int resampled_data_size = len2 * audio_tgt_channels * av_get_bytes_per_sample(out_sample_fmt);

            framePtr = std::make_shared<PCMFrame>();
            PCMFrame *frame = framePtr.get();
            frame->setFrameBuffer(audio_buf, resampled_data_size);

    //        fwrite(audio_buf, 1, resampled_data_size, fp);
        }
    }

    av_packet_unref(&pkt);

    return framePtr;
}
