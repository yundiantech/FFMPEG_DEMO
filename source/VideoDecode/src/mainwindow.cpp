/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FunctionTransfer::init(QThread::currentThreadId());

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    mReadVideoFileThread = new ReadVideoFileThread();
    mReadVideoFileThread->setVideoCallBack(this);
//    mReadH264FileThread->startRead((char*)"../data/test.h264");

    connect(ui->pushButton_open, SIGNAL(clicked(bool)), this, SLOT(slotBtnClick(bool)));
    connect(ui->pushButton_play, SIGNAL(clicked(bool)), this, SLOT(slotBtnClick(bool)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotBtnClick(bool isChecked)
{
    if (QObject::sender() == ui->pushButton_open)
    {
        QString s = QFileDialog::getOpenFileName(
                   this, "选择要播放的文件",
                    QCoreApplication::applicationDirPath() + "/../data",//初始目录
                    QStringLiteral("H264/H265 Files (*.264 *.265 *.h264 *.h265);;")
                    +QStringLiteral("All Files (*.*)"));
        if (!s.isEmpty())
        {
            s.replace("/","\\");
            ui->lineEdit->setText(s);
        }
    }
    else if (QObject::sender() == ui->pushButton_play)
    {
        QString filePath = ui->lineEdit->text();

        QString suffix = QFileInfo(filePath).suffix();

        AVCodecID id;

        ///根据后缀判断是264还是265
        if (suffix.toLower() == "h265" || suffix.toLower() == "265")
        {
            id = AV_CODEC_ID_H265;
        }
        else if (suffix.toLower() == "h264" || suffix.toLower() == "264")
        {
            id = AV_CODEC_ID_H264;
        }

        mReadVideoFileThread->startRead(filePath.toUtf8().data(), id);
    }

}

///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
void MainWindow::onDisplayVideo(std::shared_ptr<VideoFrame> videoFrame, int frameNum)
{
    ui->label_frameNum->setText(QString("%1").arg(frameNum));
    ui->widget_videoPlayer->inputOneFrame(videoFrame);
}
