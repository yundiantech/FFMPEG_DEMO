#include "AudioPlayer_RtAudio.h"

#include "PcmVolumeControl.h"

#include <stdio.h>

int AudioPlayer_RtAudio::AudioCallBackFunc(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
                                      double /*streamTime*/, RtAudioStreamStatus /*status*/, void *userdata)
{
    AudioPlayer_RtAudio *player = (AudioPlayer_RtAudio*)userdata;
    player->playAudioBuffer(outputBuffer, nBufferFrames);

    return 0;
}

AudioPlayer_RtAudio::AudioPlayer_RtAudio()
{
    mDeviceId = -1;
}

AudioPlayer_RtAudio::~AudioPlayer_RtAudio()
{

}

std::list<AudioDevice> AudioPlayer_RtAudio::getAudiDeviceList()
{
    std::list<AudioDevice> deviceList;

    for (int i=0;i<dac.getDeviceCount();i++)
    {
        RtAudio::DeviceInfo deviceInfo = dac.getDeviceInfo(i);

        if (deviceInfo.outputChannels > 0)
        {
            AudioDevice device;

            device.deviceId = i;
            device.deviceName = deviceInfo.name.c_str();

            deviceList.push_back(device);
        }

        fprintf(stderr, "device name = %s %d %d\n", deviceInfo.name.c_str(), deviceInfo.outputChannels, deviceInfo.inputChannels);
    }

    return deviceList;
}

bool AudioPlayer_RtAudio::openDevice()
{
    bool isSucceed = false;

    if ( dac.getDeviceCount() < 1 )
    {
        std::cout << "\nNo audio devices found!\n";
        isSucceed = false;
    }

    for (int i=0;i<dac.getDeviceCount();i++)
    {
        RtAudio::DeviceInfo deviceInfo = dac.getDeviceInfo(i);
    }

    RtAudio::StreamParameters oParams;
    oParams.deviceId = dac.getDefaultOutputDevice();
    oParams.nChannels = 2;
//    oParams.firstChannel = offset;

    unsigned int bufferFrames = 1024;
    try
    {
        dac.openStream( &oParams, NULL, RTAUDIO_SINT16, 44100, &bufferFrames, &AudioCallBackFunc, (void *)this );
        dac.startStream();

        mDeviceId = oParams.deviceId;
    }
    catch ( RtAudioError& e )
    {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        return false;
    }

    return true;
}

bool AudioPlayer_RtAudio::closeDevice()
{
    if (mDeviceId > 0)
    {
        dac.stopStream();
        dac.closeStream();
    }

    mDeviceId = -1;

    return true;
}
