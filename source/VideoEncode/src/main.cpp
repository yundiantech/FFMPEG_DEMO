#include <iostream>

using namespace std;

#include "AppConfig.h"
#include "Video/GetVideoThread.h"

int main()
{
    cout << "Hello World!" << endl;

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    GetVideoThread *getVideoThread = new GetVideoThread();
    getVideoThread->setQuantity(10);

    if (getVideoThread->init() == SUCCEED)
    {
        getVideoThread->startRecord();

        while(1)
        {
            AppConfig::mSleep(10000);
        }
    }
    else
    {
        fprintf(stderr, "init device failed! \n");
    }

    return 0;
}
