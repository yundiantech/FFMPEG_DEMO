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
    DESTDIR = $${PWD}/bin/bin32
} else {
    message("64-bit")
    DESTDIR = $${PWD}/bin/bin64
}

include($$PWD/lib/lib.pri)

INCLUDEPATH += $$PWD/src

win32{
    INCLUDEPATH += $$PWD/src/compat/atomics/win32
}

HEADERS += \
    src/cmdutils.h \
    src/config.h \
    src/ffmpeg.h

SOURCES += \
    src/cmdutils.c \
    src/ffmpeg.c \
    src/ffmpeg_filter.c \
    src/ffmpeg_hw.c \
    src/ffmpeg_opt.c

win32{
LIBS += Shell32.lib
}


