/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QMainWindow>

#include "VideoReader/ReadVideoFileThread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public VideoCallBack
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    ReadVideoFileThread *mReadVideoFileThread;

private slots:
    void slotBtnClick(bool isChecked);

    ///以下函数，是播放器的回调函数，用于输出信息给界面
protected:
    ///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
    void onDisplayVideo(VideoFramePtr videoFrame, int frameNum);
};

#endif // MAINWINDOW_H
