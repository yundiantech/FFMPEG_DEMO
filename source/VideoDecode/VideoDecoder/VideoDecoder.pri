CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    $$PWD/src/Video/VideoEventHandle.cpp \
    $$PWD/src/Video/VideoFrame.cpp \
    $$PWD/src/Mutex/Cond.cpp \
    $$PWD/src/Mutex/Mutex.cpp \
    $$PWD/src/VideoDecoder/VideoDecoder.cpp \
    $$PWD/src/VideoReader/NALUParsing.cpp \
    $$PWD/src/VideoReader/ReadVideoFileThread.cpp

HEADERS += \
    $$PWD/src/Mutex/Cond.h \
    $$PWD/src/Mutex/Mutex.h \
    $$PWD/src/Video/VideoEventHandle.h \
    $$PWD/src/Video/VideoFrame.h \
    $$PWD/src/VideoDecoder/VideoDecoder.h \
    $$PWD/src/VideoReader/h264.h \
    $$PWD/src/VideoReader/h265.h \
    $$PWD/src/VideoReader/NALUParsing.h \
    $$PWD/src/VideoReader/ReadVideoFileThread.h

win32{

    contains(QT_ARCH, i386) {
        message("32-bit")
        INCLUDEPATH += $$PWD/lib/win32/ffmpeg/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

    }

}


unix{
    contains(QT_ARCH, i386) {
        message("32-bit, 请自行编译32位库!")
    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/linux/ffmpeg/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/linux/ffmpeg/lib  -lavformat  -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale

        LIBS += -lpthread -ldl
    }

#QMAKE_POST_LINK 表示编译后执行内容
#QMAKE_PRE_LINK 表示编译前执行内容

#解压库文件
#QMAKE_PRE_LINK += "cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz "
system("cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz")

}
