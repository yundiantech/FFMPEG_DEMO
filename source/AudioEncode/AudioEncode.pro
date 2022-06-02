TARGET = VideoEncode
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

UI_DIR  = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj


#将输出文件直接放到源码目录下的bin目录下，将dll都放在了次目录中，用以解决运行后找不到dll的问
#DESTDIR=$$PWD/bin/
contains(QT_ARCH, i386) {
    message("32-bit")
    DESTDIR = $${PWD}/../../bin/win32
} else {
    message("64-bit")
    DESTDIR = $${PWD}/../../bin/win64
}

include($$PWD/../../lib/lib.pri)

INCLUDEPATH += $$PWD/src

SOURCES += \
        src/AppConfig.cpp \
        src/Audio/AudioEncoder.cpp \
        src/Audio/AudioFrame/AACFrame.cpp \
        src/Audio/AudioFrame/PCMFrame.cpp \
        src/Audio/GetAudioThread.cpp \
        src/AudioRecordManager.cpp \
        src/Mix/PcmMix.cpp \
        src/Mutex/Cond.cpp \
        src/Mutex/Mutex.cpp \
        src/main.cpp

HEADERS += \
    src/AppConfig.h \
    src/Audio/AudioEncoder.h \
    src/Audio/AudioFrame/AACFrame.h \
    src/Audio/AudioFrame/PCMFrame.h \
    src/Audio/GetAudioThread.h \
    src/AudioRecordManager.h \
    src/Mix/PcmMix.h \
    src/Mutex/Cond.h \
    src/Mutex/Mutex.h
