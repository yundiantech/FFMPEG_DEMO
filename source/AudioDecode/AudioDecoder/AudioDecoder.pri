CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11


SOURCES += \
        $$PWD/src/MoudleConfig.cpp \
        $$PWD/src/AudioDecoder/AudioDecoder.cpp \
        $$PWD/src/AudioFrame/AACFrame.cpp \
        $$PWD/src/AudioFrame/PCMFrame.cpp \
        $$PWD/src/AudioPlayer/AudioPlayer.cpp \
        $$PWD/src/AudioPlayer/AudioPlayer_RtAudio.cpp \
        $$PWD/src/AudioPlayer/AudioPlayer_SDL.cpp \
        $$PWD/src/AudioReader/AAC/AACReader.cpp \
        $$PWD/src/AudioReader/ReadAudioFileThread.cpp \
        $$PWD/src/EventHandle/AudioPlayerEventHandle.cpp \
        $$PWD/src/Mutex/Cond.cpp \
        $$PWD/src/Mutex/Mutex.cpp

HEADERS += \
        $$PWD/src/MoudleConfig.h \
        $$PWD/src/AudioDecoder/AudioDecoder.h \
        $$PWD/src/AudioFrame/AACFrame.h \
        $$PWD/src/AudioFrame/PCMFrame.h \
        $$PWD/src/AudioPlayer/AudioPlayer.h \
        $$PWD/src/AudioPlayer/AudioPlayer_RtAudio.h \
        $$PWD/src/AudioPlayer/AudioPlayer_SDL.h \
        $$PWD/src/AudioReader/AAC/AACReader.h \
        $$PWD/src/AudioReader/ReadAudioFileThread.h \
        $$PWD/src/EventHandle/AudioPlayerEventHandle.h \
        $$PWD/src/Mutex/Cond.h \
        $$PWD/src/Mutex/Mutex.h


include($$PWD/lib/RtAudio/RtAudio.pri)

win32{

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

    LIBS += -lmfplat -lmfuuid -lksuser -lwinmm -lwmcodecdspuuid
}

unix{

    DEFINES += NDEBUG _CONSOLE __LINUX_ALSA__

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
