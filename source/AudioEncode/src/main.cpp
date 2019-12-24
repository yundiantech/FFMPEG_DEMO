#include <iostream>

using namespace std;

#include "AppConfig.h"
#include "AudioRecordManager.h"

int main()
{
    cout << "Hello World!" << endl;

    AudioRecordManager * mAudioRecordManager = new AudioRecordManager();

    while(1) AppConfig::mSleep(600000);

    return 0;
}
