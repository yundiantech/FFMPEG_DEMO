#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>

#include "AppConfig.h"
#include "Base/FunctionTransfer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FunctionTransfer::init(QThread::currentThreadId());

    mShowProgressLabel = new QLabel(this);
    mShowProgressLabel->hide();
    mShowProgressLabel->setStyleSheet("background-color: rgb(255, 0, 4);");

    ///创建图表
    auto createView = [=](const QString &name, bool isShowTime)
    {
        QChart      *chart = new QChart();

        QChartView *chartView = new QChartView();
        chartView->setChart(chart);
//        chartView->show();

        QLineSeries *series = new QLineSeries();
        chart->addSeries(series);

        QAbstractAxis *axisX = nullptr;

        if (isShowTime)
        {
            QDateTimeAxis *axis = new QDateTimeAxis;
            axis->setRange(QDateTime::currentDateTime(), QDateTime::currentDateTime().addDays(1));
            axis->setTickCount(8);
            axis->setFormat("mm:ss");

            axisX = axis;
        }
        else
        {
            QValueAxis  *axis = new QValueAxis;
            axis->setRange(0, 20000);
            axis->setLabelFormat("%g");
//            axis->setTitleText("Samples");

            axisX = axis;
        }

        QValueAxis *axisY = new QValueAxis;
        axisY->setRange(-1, 1);
//        axisY->setTitleText("Audio level");
        chart->setAxisX(axisX, series);
        chart->setAxisY(axisY, series);
        chart->legend()->hide();
        chart->setTitle(name);

        return chartView;
    };

    mChartView_Left  = createView(QStringLiteral("左声道"), true);
    mChartView_Right = createView(QStringLiteral("右声道"), true);

    mCurrentChartView_Left  = createView("left channel(one frame)", false);
    mCurrentChartView_Right = createView("right channel(2 seconds)", false);

    ui->horizontalLayout_left->addWidget(mChartView_Left);
    ui->horizontalLayout_right->addWidget(mChartView_Right);

    ui->horizontalLayout_current_left->addWidget(mCurrentChartView_Left);
    ui->horizontalLayout_current_right->addWidget(mCurrentChartView_Right);

    mReadAudioFileThread = new ReadAudioFileThread();
    mReadAudioFileThread->setVideoPlayerCallBack(this);
    mReadAudioFileThread->startRead((char*)"../data/test.aac");

    std::list<AudioDevice> deviceList = mReadAudioFileThread->getAudiDeviceList();

    for (const AudioDevice & device : deviceList)
    {
        ui->comboBox_audioDevice->addItem(QString::fromStdString(device.deviceName));
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onGetPcmWaveValues(const std::list<float> &leftChannelValues, const std::list<float> &rightChannelValues)
{
    qDebug()<<__FUNCTION__<<leftChannelValues.size();

    FunctionTransfer::runInMainThread([=]()
    {
        auto showDbValues = [=](QChartView  *chartView, const std::list<float> &channelValues)
        {
            /// 将横坐标换算成时间
            /// 这里一帧音频的采样是1024，音频的采样率是一秒钟44100次。
            /// 当然也可以通过直接计算数据大小，原理都是类似。
            qint64 oneFrameTime = 1024.0 / 44100 * 1000; //一帧音频时间（秒）

            QList<QPointF> valueList;

            int i=0;
            for(const float value : channelValues)
            {
                valueList.append(QPointF(QDateTime(QDate::currentDate(), QTime(0,0,0).addMSecs(oneFrameTime*i)).toMSecsSinceEpoch(), value));
                i++;
            }

            ((QDateTimeAxis*)chartView->chart()->axisX())->setRange(QDateTime(QDate::currentDate(), QTime(0,0,0)),
                                                                    QDateTime(QDate::currentDate(), QTime(0,0,0).addMSecs(channelValues.size()*oneFrameTime)));

            ((QLineSeries*)chartView->chart()->series().first())->replace(valueList);
        };

        showDbValues(mChartView_Left, leftChannelValues);
        showDbValues(mChartView_Right, rightChannelValues);

    });
}

void MainWindow::onGetPcmFrame(PCMFramePtr pcmFramePtr)
{

}

void MainWindow::onUpdatePlayingValue(const float &leftChannel,
                                      const float &rightChannel,
                                      const std::list<float> &leftChannelDbValues,
                                      const std::list<float> &rightChannelDbValues,
                                      const float &progress)
{
    ///以下图表显示仅做demo展示，效率并未优化

    mLeftChannelDbValueList.append(leftChannelDbValues);
    mRightChannelDbValueList.append(rightChannelDbValues);

    if (mLeftChannelDbValueList.size() >= 2)
    {
        mLeftChannelDbValueList.removeFirst();
    }

    ///右声道展示2秒数据
    if (mRightChannelDbValueList.size() >= 43 * 2)
    {
        mRightChannelDbValueList.removeFirst();
    }

    auto showDbValues = [=](QChartView  *chartView, const QList<std::list<float>> &channelDbValueList)
    {
        QList<QPointF> valueList;

        int i=0;
        for (const std::list<float> &channelDbValues : channelDbValueList)
        {
            for(const float &value : channelDbValues)
            {
                valueList.append(QPointF(i, value));
                i++;
            }
        }

        FunctionTransfer::runInMainThread([=]()
        {
            chartView->chart()->axisX()->setRange(0, i);
            ((QLineSeries*)chartView->chart()->series().first())->replace(valueList);
        });

    };

    showDbValues(mCurrentChartView_Left,  mLeftChannelDbValueList);
    showDbValues(mCurrentChartView_Right, mRightChannelDbValueList);

    FunctionTransfer::runInMainThread([=]()
    {
        ui->progressBar_left->setValue(leftChannel*100);
        ui->progressBar_right->setValue(rightChannel*100);

        ///设置进度
        {
            QChart *chart = mChartView_Left->chart();
            int w = chart->plotArea().width();
            int x = chart->plotArea().x() + ui->widget_charViewContainer->x() + chart->plotArea().width() * progress;
            int y = ui->widget_charViewContainer->y();

            mShowProgressLabel->show();
            mShowProgressLabel->move(x, y);
            mShowProgressLabel->resize(2, ui->widget_charViewContainer->height());
        }
    });
}

void MainWindow::onUpdatePlayingTime(const uint32_t &totalTime, const uint32_t &currentTime)
{
    FunctionTransfer::runInMainThread([=]()
    {
        QString str = QString("%1/%2")
                        .arg(QTime(0,0,0).addMSecs(totalTime).toString("hh:mm:ss"))
                        .arg(QTime(0,0,0).addMSecs(currentTime).toString("hh:mm:ss.zzz"));

        ui->label_time->setText(str);
    });
}
