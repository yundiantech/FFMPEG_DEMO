/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VIDEODECODER_H
#define VIDEODECODER_H

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

typedef unsigned char uchar;

class VideoDecoder
{
public:
    VideoDecoder();

    bool openDecoder(enum AVCodecID id = AV_CODEC_ID_H264);
    void closeDecoder();

    bool decode(uint8_t *inputbuf, int frame_size, uint8_t * &outBuf, int &outWidth, int &outHeight);
    int getFrameRate(); //获取帧率

private:

    AVCodec         *pCodec;
    AVCodecContext  *pCodecCtx;
    SwsContext      *img_convert_ctx;
    AVFrame         *pFrame;

    AVFrame *pFrameYUV;
    uint8_t *bufferYUV;

    void initWidth(int width, int height);

};

#endif // H264DECORDER_H
