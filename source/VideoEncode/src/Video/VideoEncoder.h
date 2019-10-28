/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VIDEOENCORDER_H
#define VIDEOENCORDER_H

#include <list>
#include <thread>
#include "Mutex/Cond.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
}

struct FrameDataNode
{
    uint8_t * buffer;
    int size;
    int64_t time;

    FrameDataNode()
    {
        buffer = nullptr;
        size = 0;
    }
};

/// 编码h.264的线程  这里把编码和采集分开 放到单独的线程 是因为编码也比较耗时
class VideoEncoder
{

public:
    explicit VideoEncoder();
    ~VideoEncoder();

    void setWidth(int w, int h);//设置编码后的图像高宽(这个必须和输入的yuv图像数据一样 且必须是偶数)
    void setQuantity(int value);// 设置编码质量

    void startEncode();
    void stopEncode();

    /**
     * @brief inputYuvBuffer 输入需要编码的YUV数据
     * @param buffer
     * @param size
     * @param time
     */
    void inputYuvBuffer(uint8_t *buffer, int size, int64_t time);

protected:
    void run();

private:
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;

    uint8_t* picture_buf;
    AVFrame* picture;

    std::list<FrameDataNode> mYuvBufferList; //YUV数据队列
    Cond *mCond;

    int mBitRate; //video bitRate
    int mWidth;
    int mHeight;

    bool mIsStop;

    bool openEncoder(); //打开编码器
    bool closeEncoder(); //关闭编码器

};

#endif // VIDEOENCORDER_H
