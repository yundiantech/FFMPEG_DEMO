/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef NALUPARSING_H
#define NALUPARSING_H

#include <stdint.h>
#include <stdlib.h>

#include "h264.h"
#include "h265.h"

enum T_NALU_TYPE
{
    T_NALU_H264 = 0,
    T_NALU_H265,
};

typedef struct
{
    T_NALU_TYPE type;

    union{
        T_H264_NALU h264Nalu;
        T_H265_NALU h265Nalu;
    }nalu;

} T_NALU;

class NALUParsing
{
public:
    NALUParsing();

    void setVideoType(T_NALU_TYPE type){mVideoType = type;}

    int inputH264Data(uint8_t *buf,int len); //输入h264数据

    ///从H264数据中查找出一帧数据
    T_NALU* getNextFrame();

private:
    uint8_t *mH264Buffer;
    int mBufferSize;

    T_NALU_TYPE mVideoType; //类型 区分是264还是265

public:
    static T_NALU *AllocNALU(int buffersize, T_NALU_TYPE type);

    static void FreeNALU(T_NALU *n);
};

#endif // H264READER_H
