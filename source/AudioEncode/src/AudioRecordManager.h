#ifndef AUDIORECORDMANAGER_H
#define AUDIORECORDMANAGER_H

#include <list>
#include <thread>

#include "Mutex/Cond.h"

#include "Audio/GetAudioThread.h"
#include "Audio/AudioEncoder.h"

struct DeviceNode
{
    std::string deviceName;
    std::string deviceID;
};

struct AudioManagerNode
{
    GetAudioThread* thread;
    std::list<PCMFramePtr> pcmFrameList;
    int64_t lastGetFrameTime; //最近一次获取

    AudioManagerNode()
    {
        lastGetFrameTime = 0;
    }
};

class AudioRecordManager
{
public:
    AudioRecordManager();

protected:
    void run();

private:
    bool mIsStop;

    Cond *mCond;
    std::list<AudioManagerNode> mAudioManagerList;

    AudioEncoder *mAudioEncoder;

    bool getDeviceList(std::list<DeviceNode> &videoDeviceList, std::list<DeviceNode> &audioDeviceList);

};

#endif // AUDIORECORDMANAGER_H
