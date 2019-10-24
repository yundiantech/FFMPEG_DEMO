#-------------------------------------------------
#
# Project created by QtCreator 2019-10-17T14:42:56
#
#-------------------------------------------------
 
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoDecode
TEMPLATE = app

CONFIG += c++11

UI_DIR  = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj


#将输出文件直接放到源码目录下的bin目录下，将dll都放在了次目录中，用以解决运行后找不到dll的问
#DESTDIR=$$PWD/bin/
contains(QT_ARCH, i386) {
    message("32-bit")
    DESTDIR = $${PWD}/bin32
} else {
    message("64-bit")
    DESTDIR = $${PWD}/bin64
}

#包含视频解码的代码
include(VideoDecoder/VideoDecoder.pri)

SOURCES += \
    src/Video/ShowVideoWidget.cpp \
    src/main.cpp \
    src/Base/FunctionTransfer.cpp \
    src/AppConfig.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/AppConfig.h \
    src/Base/FunctionTransfer.h \
    src/Video/ShowVideoWidget.h \
    src/mainwindow.h

FORMS += \
    src/Video/ShowVideoWidget.ui \
    src/mainwindow.ui

INCLUDEPATH += $$PWD/src
