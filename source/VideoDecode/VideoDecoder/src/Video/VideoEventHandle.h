#ifndef VIDEOEVENTHANDLE_H
#define VIDEOEVENTHANDLE_H

#include "Video/VideoFrame.h"

class VideoCallBack
{
public:
    ~VideoCallBack();

    ///播放视频，此函数不宜做耗时操作，否则会影响播放的流畅性。
    virtual void onDisplayVideo(VideoFramePtr videoFrame, int frameNum) = 0;

};

#endif // VIDEOYEVENTHANDLE_H
