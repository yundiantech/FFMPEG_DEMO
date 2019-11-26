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
    DESTDIR = $${PWD}/bin/bin32
} else {
    message("64-bit")
    DESTDIR = $${PWD}/bin/bin64
}

SOURCES += \
        src/AppConfig.cpp \
        src/AudioDecoder/AudioDecoder.cpp \
        src/AudioFrame/AACFrame.cpp \
        src/AudioFrame/PCMFrame.cpp \
        src/AudioPlayer/AudioPlayer.cpp \
        src/AudioPlayer/AudioPlayer_RtAudio.cpp \
        src/AudioPlayer/AudioPlayer_SDL.cpp \
        src/AudioReader/AAC/AACReader.cpp \
        src/AudioReader/ReadAudioFileThread.cpp \
        src/Base/FunctionTransfer.cpp \
        src/EventHandle/AudioPlayerEventHandle.cpp \
        src/Mutex/Cond.cpp \
        src/Mutex/Mutex.cpp \
        src/main.cpp \
        src/MainWindow.cpp

HEADERS += \
        src/AppConfig.h \
        src/AudioDecoder/AudioDecoder.h \
        src/AudioFrame/AACFrame.h \
        src/AudioFrame/PCMFrame.h \
        src/AudioPlayer/AudioPlayer.h \
        src/AudioPlayer/AudioPlayer_RtAudio.h \
        src/AudioPlayer/AudioPlayer_SDL.h \
        src/AudioReader/AAC/AACReader.h \
        src/AudioReader/ReadAudioFileThread.h \
        src/Base/FunctionTransfer.h \
        src/EventHandle/AudioPlayerEventHandle.h \
        src/MainWindow.h \
        src/Mutex/Cond.h \
        src/Mutex/Mutex.h

FORMS += \
        src/MainWindow.ui

include($$PWD/lib/RtAudio/RtAudio.pri)

win32{

QMAKE_CFLAGS_DEBUG += -MT
QMAKE_CXXFLAGS_DEBUG += -MT

QMAKE_CFLAGS_RELEASE += -MT
QMAKE_CXXFLAGS_RELEASE += -MT

DEFINES += NDEBUG WIN32 _CONSOLE __WINDOWS_ASIO__ __WINDOWS_DS__ __WINDOWS_WASAPI__

    contains(QT_ARCH, i386) {
        message("32-bit")
        INCLUDEPATH += $$PWD/lib/win32/ffmpeg/include \
                       $$PWD/lib/win32/SDL2/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
        LIBS += -L$$PWD/lib/win32/SDL2/lib -lSDL2
    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include \
                       $$PWD/lib/win64/SDL2/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
        LIBS += -L$$PWD/lib/win64/SDL2/lib -lSDL2
    }

}

DEFINES += NDEBUG _CONSOLE __LINUX_ALSA__
unix{
    contains(QT_ARCH, i386) {
        message("32-bit, 请自行编译32位库!")
    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/linux/ffmpeg/include \
                       $$PWD/lib/linux/SDL2/include/SDL2 \
                       $$PWD/lib/linux/alsa/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/linux/ffmpeg/lib  -lavformat  -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale -lpostproc
        LIBS += -L$$PWD/lib/linux/SDL2/lib -lSDL2
        LIBS += -L$$PWD/lib/linux/alsa/lib -lasound
        LIBS += -lpthread -ldl
    }

#QMAKE_POST_LINK 表示编译后执行内容
#QMAKE_PRE_LINK 表示编译前执行内容

#解压库文件
#QMAKE_PRE_LINK += "cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz "
system("cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz")
system("cd $$PWD/lib/linux && tar xvzf SDL2.tar.gz")
system("cd $$PWD/lib/linux && tar xvzf alsa.tar.gz")
}
