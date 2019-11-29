#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QLabel>

#include "AudioReader/ReadAudioFileThread.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public AudioPlayerCallBack
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    ReadAudioFileThread *mReadAudioFileThread;

    QLabel *mShowProgressLabel; //显示进度用的

    ///显示左声道波形图的图表
    QChartView  *mChartView_Left;

    ///显示右声道波形图的图表
    QChartView  *mChartView_Right;

    ///当前播放的数据
    QChartView  *mCurrentChartView_Left;
    QChartView  *mCurrentChartView_Right;

    QList<std::list<float>> mLeftChannelDbValueList;
    QList<std::list<float>> mRightChannelDbValueList;

    ///以下为回调函数，用于接受ReadAudioFileThread类传给界面的数据
protected:
    ///获取到音频波形图数据
    void onGetPcmWaveValues(const std::list<float> &leftChannelValues, const std::list<float> &rightChannelValues) override;

    ///当前播放的一帧音频数据
    void onGetPcmFrame(PCMFramePtr pcmFramePtr) override;

    void onUpdatePlayingValue(const float &leftChannel,
                              const float &rightChannel,
                              const std::list<float> &leftChannelDbValues,
                              const std::list<float> &rightChannelDbValues,
                              const float &progress) override;

    void onUpdatePlayingTime(const uint32_t &totalTime, const uint32_t &currentTime) override;
};

#endif // MAINWINDOW_H
