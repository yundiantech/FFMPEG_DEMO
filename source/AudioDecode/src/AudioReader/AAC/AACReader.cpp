#include "AACReader.h"

AACReader::AACReader()
{
    ///初始化一段内存 用于临时存放AAC数据
    mAACBuffer = (uint8_t*)malloc(1024*1024*10);
    mBufferSize = 0;
}

int AACReader::inputAACData(uint8_t *buf, int len)
{
    memcpy(mAACBuffer + mBufferSize, buf, len);
    mBufferSize += len;

    return mBufferSize;
}

void AACReader::clearAACData()
{
    mBufferSize = 0;
}

bool AACReader::ReadAdtsHeader(unsigned char * adts_headerbuf, ADTS_HEADER *adts)
{
    bool isSucceed = false;

    if ((adts_headerbuf[0] == 0xFF)&&((adts_headerbuf[1] & 0xF0) == 0xF0))    //syncword 12个1
    {
        adts->aac_frame_length = 0;
        adts->adts_buffer_fullness = 0;
        adts->channel_configuration = 0;
        adts->copyright_identification_bit = 0;
        adts->copyright_identification_start = 0;
        adts->home = 0;
        adts->id = 0;
        adts->layer = 0;
        adts->no_raw_data_blocks_in_frame = 0;
        adts->original = 0;
        adts->private_bit = 0;
        adts->profile = 0;
        adts->protection_absent = 0;
        adts->sf_index = 0;
        adts->syncword = 0;

        adts->id = ((unsigned int) adts_headerbuf[1] & 0x08) >> 3;
        //printf("adts:id  %d\n",adts.id);
        adts->layer = ((unsigned int) adts_headerbuf[1] & 0x06) >> 1;
       // printf( "adts:layer  %d\n",adts.layer);
        adts->protection_absent = (unsigned int) adts_headerbuf[1] & 0x01;
       // printf( "adts:protection_absent  %d\n",adts.protection_absent);
        adts->profile = ((unsigned int) adts_headerbuf[2] & 0xc0) >> 6;
       // printf( "adts:profile  %d\n",adts.profile);
        adts->sf_index = ((unsigned int) adts_headerbuf[2] & 0x3c) >> 2;
       // printf( "adts:sf_index  %d\n",adts.sf_index);
        adts->private_bit = ((unsigned int) adts_headerbuf[2] & 0x02) >> 1;
      //  printf( "adts:pritvate_bit  %d\n",adts.private_bit);
        adts->channel_configuration = ((((unsigned int) adts_headerbuf[2] & 0x01) << 2) | (((unsigned int) adts_headerbuf[3] & 0xc0) >> 6));
      //  printf( "adts:channel_configuration  %d\n",adts.channel_configuration);
        adts->original = ((unsigned int) adts_headerbuf[3] & 0x20) >> 5;
       // printf( "adts:original  %d\n",adts.original);
        adts->home = ((unsigned int) adts_headerbuf[3] & 0x10) >> 4;
       // printf( "adts:home  %d\n",adts.home);
        adts->copyright_identification_bit = ((unsigned int) adts_headerbuf[3] & 0x08) >> 3;
      //  printf( "adts:copyright_identification_bit  %d\n",adts.copyright_identification_bit);
        adts->copyright_identification_start = (unsigned int) adts_headerbuf[3] & 0x04 >> 2;
      //  printf( "adts:copyright_identification_start  %d\n",adts.copyright_identification_start);
        adts->aac_frame_length = (((((unsigned int) adts_headerbuf[3]) & 0x03) << 11) | (((unsigned int) adts_headerbuf[4] & 0xFF) << 3)| ((unsigned int) adts_headerbuf[5] & 0xE0) >> 5) ;
      //  printf( "adts:aac_frame_length  %d\n",adts.aac_frame_length);
        adts->adts_buffer_fullness = (((unsigned int) adts_headerbuf[5] & 0x1f) << 6 | ((unsigned int) adts_headerbuf[6] & 0xfc) >> 2);
      //  printf( "adts:adts_buffer_fullness  %d\n",adts.adts_buffer_fullness);
        adts->no_raw_data_blocks_in_frame = ((unsigned int) adts_headerbuf[6] & 0x03);
       // printf( "adts:no_raw_data_blocks_in_frame  %d\n",adts.no_raw_data_blocks_in_frame);

        isSucceed = true;
    }
    else
    {
        //printf("读取adts 头失败 : 第 %d帧 \n",frame_number);
        isSucceed = false;
    }

    return isSucceed;
}

AACFramePtr AACReader::getNextFrame()
{
    ADTS_HEADER adtsHeader;

    int pos = 0; //记录当前处理的数据偏移量

    while(1)
    {
        unsigned char* Buf = mAACBuffer + pos;
        int lenth = mBufferSize - pos; //剩余没有处理的数据长度

        if (lenth <= 7)
        {
            return nullptr;
        }

        ///读取adts头
        if (ReadAdtsHeader(Buf, &adtsHeader))
        {
            int Framelen = adtsHeader.aac_frame_length - 7;

            if ((lenth - 7) < Framelen)
            {
                return nullptr;
            }

            break;
        }
        else
        {
            //否则 往后查找一个字节
            pos++;
        }
    }

    unsigned char* buffer = mAACBuffer + pos;
    int buffersize = adtsHeader.aac_frame_length;

    AACFramePtr framePtr = std::make_shared<AACFrame>();
    AACFrame *frame = framePtr.get();
    frame->setAdtsHeader(adtsHeader);
    frame->setFrameBuffer(buffer, buffersize);

    /// 将这一帧数据去掉
    /// 把后一帧数据覆盖上来
    int pos_2 = pos + buffersize;
    int leftSize = mBufferSize - pos_2;
    memcpy(mAACBuffer, mAACBuffer + pos_2, leftSize);
    mBufferSize = leftSize;

    return framePtr;
}

