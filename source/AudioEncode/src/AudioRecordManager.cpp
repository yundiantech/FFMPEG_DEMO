#include "AudioRecordManager.h"

#if defined(WIN32)
    #include<direct.h>
#else
    #include<unistd.h>
#endif

#include "AppConfig.h"
#include "Mix/PcmMix.h"

#if defined(WIN32)
std::string UTF8ToGB(const char* str)
{
    std::string result;
    WCHAR *strSrc;
    LPSTR szRes;

    //获得临时变量的大小
    int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    strSrc = new WCHAR[i + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

    //获得临时变量的大小
    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    szRes = new CHAR[i + 1];
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

    result = szRes;
    delete[]strSrc;
    delete[]szRes;

    return result;
}
#endif

AudioRecordManager::AudioRecordManager()
{
    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    std::list<DeviceNode> videoDeviceList;
    std::list<DeviceNode> audioDeviceList;

#if defined(WIN32)
    getDeviceList(videoDeviceList, audioDeviceList);
#else
    audioDeviceList.push_back(DeviceNode{"default", "default"});
#endif

    mIsStop = true;

    mCond = new Cond();

    mAudioEncoder = new AudioEncoder();
    mAudioEncoder->openEncoder();

    auto inputPcmBufferFunc = [&](PCMFramePtr pcmFrame, int index)
    {
        mCond->Lock();
        ///将数据存入队列，然后再后面的线程中，取出数据，混音后再拿去编码。
        int i=0;
        std::list<AudioManagerNode>::iterator iter;
        for (iter=mAudioManagerList.begin();iter!=mAudioManagerList.end();iter++)
        {
            if (i++ == index)
            {
               (*iter).pcmFrameList.push_back(pcmFrame);
               (*iter).lastGetFrameTime = AppConfig::getTimeStamp_MilliSecond();
            }
        }

//        fprintf(stderr, "%s %d %d \n", __FUNCTION__, index, pcmFrame->getSize());

        mCond->Unlock();
        mCond->Signal();
    };

fprintf(stderr, "%s audioDeviceList.size=%d \n", __FUNCTION__, audioDeviceList.size());

    int i=0;
    for (DeviceNode deviceNode : audioDeviceList)
    {
        GetAudioThread *getAudioThread = new GetAudioThread();

        AudioManagerNode node;
        node.thread = getAudioThread;
        node.pcmFrameList.clear();
        node.lastGetFrameTime = AppConfig::getTimeStamp_MilliSecond();

        mAudioManagerList.push_back(node);

        if (getAudioThread->init(deviceNode.deviceID.c_str()))
        {
            getAudioThread->startRecord(mAudioEncoder->getONEFrameSize(), inputPcmBufferFunc, i++);

            fprintf(stderr, "init device [%s] succeed! \n", deviceNode.deviceName.c_str());
        }
        else
        {
            fprintf(stderr, "init device [%s] failed! \n", deviceNode.deviceName.c_str());
        }
    }


    std::thread([=]
    {
        mIsStop = false;

        this->run();

    }).detach();

}

void AudioRecordManager::run()
{

    while(!mIsStop)
    {
        std::list<PCMFramePtr> waitEncodeFrameList;

        mCond->Lock();

        do
        {
            ///判断队列里面是否有数据
            bool hasBuffer = true;

            for (std::list<AudioManagerNode>::iterator iter =mAudioManagerList.begin(); iter!=mAudioManagerList.end(); iter++)
            {
                ///由于读取声卡数据的时候，当声卡没有输出的时候，采集就不会获取到数据，因此需要判断采集声卡的线程是否有数据。
                if ( ((AppConfig::getTimeStamp_MilliSecond() - (*iter).lastGetFrameTime) < 1000)
                     && ((*iter).pcmFrameList.size() <= 0)) //一秒内获取过数据，且队列是空的，那么继续等待
                {
                    hasBuffer = false;
                    break;
                }
            }

            if (hasBuffer)
            {
                waitEncodeFrameList.clear();
                for (std::list<AudioManagerNode>::iterator iter =mAudioManagerList.begin(); iter!=mAudioManagerList.end(); iter++)
                {
                    std::list<PCMFramePtr> &tmpFrameList = (*iter).pcmFrameList;
                    if (!tmpFrameList.empty())
                    {
                        waitEncodeFrameList.push_back(tmpFrameList.front());
                        tmpFrameList.pop_front();
                    }
                }
//                fprintf(stderr, "%s waitEncodeFrameList size = %d \n",__FUNCTION__, waitEncodeFrameList.size());
                break;
            }
            else
            {
                mCond->Wait(1000);
            }

        }while(1);

        mCond->Unlock();

        if (waitEncodeFrameList.size() > 0)
        {
            ///这里的PCM数据格式为：AV_SAMPLE_FMT_FLTP

            ///实现混音
            float *srcData[10] = {0};
            int number=0;
            int bufferSize = 0;
            for (PCMFramePtr & pcmFrame : waitEncodeFrameList)
            {
                srcData[number++] = (float*)pcmFrame->getBuffer();
                bufferSize = pcmFrame->getSize(); //由于采集的时候做了处理，因此这里每一帧的size都是一样的。
            }

            uint8_t * pcmBuffer = (uint8_t*)malloc(bufferSize);
            PcmMix::NormalizedRemix(srcData, number, bufferSize, (float*)pcmBuffer);
            AACFramePtr aacFrame = mAudioEncoder->encode(pcmBuffer, bufferSize);
            free(pcmBuffer);

#if 1 ///写入aac文件
            if (aacFrame != nullptr && aacFrame.get() != nullptr)
            {
                static FILE *aacFp = fopen("out.aac", "wb");
                fwrite(aacFrame->getBuffer(), 1, aacFrame->getSize(), aacFp);
            }
#endif

        }
    }

    mAudioEncoder->closeEncoder();
}

bool AudioRecordManager::getDeviceList(std::list<DeviceNode> &videoDeviceList, std::list<DeviceNode> &audioDeviceList)
{
    bool isSucceed = false;


    /// 执行ffmpeg命令行 获取音视频设备
    /// 请将ffmpeg.exe和程序放到同一个目录下

    char dirPath[512] = {0};
    getcwd(dirPath, sizeof (dirPath));

#ifdef WIN32

    std::string ffmpegPath = std::string(dirPath) + "/ffmpeg.exe";
    ffmpegPath = AppConfig::stringReplaceAll(ffmpegPath, "/","\\\\");

    #if 0
        std::string cmdStr = AppConfig::stringFormat(" /c \"%s\" -list_devices true -f dshow -i dummy 2>ffmpeg_device_out.txt", ffmpegPath.c_str());

        std::wstring str;
        {
            char * c = (char*)cmdStr.c_str();
            size_t m_encode = CP_ACP;
            int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0);
            wchar_t*	m_wchar = new wchar_t[len + 1];
            MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len);
            m_wchar[len] = '\0';
            str = m_wchar;
            delete m_wchar;
        }

        fprintf(stderr, "%s %s \n", __FUNCTION__, str.c_str());

        SHELLEXECUTEINFO ShExecInfo = {0};
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = L"cmd.exe";//调用的程序名
    //    ShExecInfo.lpParameters = L" /c ffmpeg.exe -list_devices true -f dshow -i dummy 2>D:/a.txt";//调用程序的命令行参数
        ShExecInfo.lpParameters = str.data();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_SHOWMINIMIZED;//窗口状态为隐藏
        ShExecInfo.hInstApp = NULL;
        int ret = ShellExecuteEx(&ShExecInfo);
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);////等到该进程结束
    #else
        std::string cmdStr = AppConfig::stringFormat("cmd.exe /c \"%s\" -list_devices true -f dshow -i dummy 2>ffmpeg_device_out.txt", ffmpegPath.c_str());

        int ret = WinExec(cmdStr.c_str(), SW_SHOWMINIMIZED);
    #endif

#else

//    int ret = system(cmdStr.c_str());
#endif

    for (int i=0;i<10;i++)
    {

        std::string deviceName;
        std::string deviceID;

        FILE *fp = fopen("ffmpeg_device_out.txt", "r");
        if (fp != nullptr)
        {
            bool isVideoBegin = false;
            bool isAudioBegin = false;

            while (!feof(fp))
            {
                char ch[1024] = {0};
                char*p = fgets(ch, 1024, fp);

#if defined(WIN32)
                std::string str = UTF8ToGB(ch); //ffmpeg生成的文件是UTF8编码的
#else
                std::string str = std::string(ch);
#endif
//                fprintf(stderr, "[%s] %s [end]\n", str.c_str(), ch);

                if ((str.find("DirectShow video devices") != std::string::npos) && (str.find("[dshow @") != std::string::npos))
                {
                    isVideoBegin = true;
                    isAudioBegin = false;
                    continue;
                }

                if ((str.find("DirectShow audio devices") != std::string::npos) && (str.find("[dshow @") != std::string::npos))
                {
                    isAudioBegin = true;
                    isVideoBegin = false;
                    continue;
                }

                if (str.find("[dshow @") != std::string::npos)
                {
                    std::string tmpStr = str;

                    int index = str.find("\"");
                    str = str.erase(0, index);

                    str = AppConfig::stringReplaceAll(str, "\"", "");
                    str = AppConfig::stringReplaceAll(str, "\n", "");
                    str = AppConfig::stringReplaceAll(str, "\r", "");

                    if (tmpStr.find("Alternative name") == std::string::npos)
                    {
                        ///是设备名字
//                        if (str.find("virtual-audio-capturer") != std::string::npos)
                        deviceName = str;
                    }
                    else
                    {
                        deviceID = str;

                        DeviceNode deviceNode{deviceName, deviceID};

                        ///是设备ID
                        if (isVideoBegin)
                        {
    //                        fprintf(stderr, ">>>>>>>video>>>>>>> %s\n", str.c_str());
                            if (!deviceName.empty())
                                videoDeviceList.push_back(deviceNode);
                        }
                        else if (isAudioBegin)
                        {
//                            fprintf(stderr, ">>>>>>>audio>>>>>>> %s\n", str.c_str());
                            if (!deviceName.empty())
                                audioDeviceList.push_back(deviceNode);
                        }
                    }
                }
            }
            fclose(fp);

            break;
        }
        else
        {
            AppConfig::mSleep(1000); //等待一秒再试一次
        }
//        fprintf(stderr, "####=======================###\n");
    }

    AppConfig::mSleep(500);
    AppConfig::removeFile("ffmpeg_device_out.txt");

    return isSucceed;
}
