#-------------------------------------------------
#
# Project created by QtCreator 2019-11-14T10:38:31
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AudioDecode
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CONFIG += c++11

UI_DIR  = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj


#将输出文件直接放到源码目录下的bin目录下，将dll都放在了次目录中，用以解决运行后找不到dll的问
#DESTDIR=$$PWD/bin/
contains(QT_ARCH, i386) {
    message("32-bit")
    DESTDIR = $${PWD}/../..//bin/win32
} else {
    message("64-bit")
    DESTDIR = $${PWD}/../../bin/win64
}

include($$PWD/../../lib/lib.pri)

#包含音频解码的代码
include(AudioDecoder/AudioDecoder.pri)

SOURCES += \
        src/main.cpp \
        src/AppConfig.cpp \
        src/Base/FunctionTransfer.cpp \
        src/MainWindow.cpp

HEADERS += \
        src/AppConfig.h \
        src/Base/FunctionTransfer.h \
        src/MainWindow.h

FORMS += \
        src/MainWindow.ui
