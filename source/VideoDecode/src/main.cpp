/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */
#include "mainwindow.h"
#include <QApplication>

#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);

    MainWindow w;
    w.show();

    return a.exec();
}
