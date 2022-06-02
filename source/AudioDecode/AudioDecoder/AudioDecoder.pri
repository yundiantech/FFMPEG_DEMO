CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += $$PWD/src

SOURCES += \
        $$PWD/src/MoudleConfig.cpp \
        $$PWD/src/AudioDecoder/AACDecoder.cpp \
        $$PWD/src/AudioFrame/AACFrame.cpp \
        $$PWD/src/AudioFrame/PCMFrame.cpp \
        $$PWD/src/AudioPlayer/AudioPlayer.cpp \
        $$PWD/src/AudioPlayer/AudioPlayer_RtAudio.cpp \
        $$PWD/src/AudioPlayer/AudioPlayer_SDL.cpp \
        $$PWD/src/AudioReader/AAC/AACReader.cpp \
        $$PWD/src/AudioReader/ReadAACFileThread.cpp \
        $$PWD/src/AudioReader/ReadAudioFileThread.cpp \
        $$PWD/src/EventHandle/AudioPlayerEventHandle.cpp \
        $$PWD/src/Mutex/Cond.cpp \
        $$PWD/src/Mutex/Mutex.cpp

HEADERS += \
        $$PWD/src/MoudleConfig.h \
        $$PWD/src/AudioDecoder/AACDecoder.h \
        $$PWD/src/AudioFrame/AACFrame.h \
        $$PWD/src/AudioFrame/PCMFrame.h \
        $$PWD/src/AudioPlayer/AudioPlayer.h \
        $$PWD/src/AudioPlayer/AudioPlayer_RtAudio.h \
        $$PWD/src/AudioPlayer/AudioPlayer_SDL.h \
        $$PWD/src/AudioReader/AAC/AACReader.h \
        $$PWD/src/AudioReader/ReadAACFileThread.h \
        $$PWD/src/AudioReader/ReadAudioFileThread.h \
        $$PWD/src/EventHandle/AudioPlayerEventHandle.h \
        $$PWD/src/Mutex/Cond.h \
        $$PWD/src/Mutex/Mutex.h
