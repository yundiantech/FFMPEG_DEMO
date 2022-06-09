INCLUDEPATH += $$PWD

include($$PWD/ffmpeg/ffmpeg.pri)
include($$PWD/RtAudio/RtAudio.pri)
include($$PWD/SDL2/SDL2.pri)
#include($$PWD/libyuv/libyuv.pri)


# git submodule add -b master --force --name lib/libyuv https://gitee.com/devlib/libyuv-dev.git lib/libyuv
# git submodule add -b master --force --name lib/RtAudio https://gitee.com/devlib/RtAudio-dev.git lib/RtAudio
# git submodule add -b master --force --name lib/SDL2 https://gitee.com/devlib/SDL2-dev.git lib/SDL2
# git submodule add -b V4.3.1 --force --name lib/ffmpeg https://gitee.com/huihui765/ffmpeg-dev.git lib/ffmpeg



#win32{

#    DEFINES += NDEBUG WIN32 _CONSOLE __WINDOWS_ASIO__ __WINDOWS_DS__ __WINDOWS_WASAPI__

#    contains(QT_ARCH, i386) {
#        message("32-bit")
#        INCLUDEPATH += $$PWD/lib/win32/ffmpeg/include \
#                       $$PWD/lib/win32/SDL2/include \
#                       $$PWD/src

#        LIBS += -L$$PWD/lib/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
#        LIBS += -L$$PWD/lib/win32/SDL2/lib -lSDL2
#    } else {
#        message("64-bit")
#        INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include \
#                       $$PWD/lib/win64/SDL2/include \
#                       $$PWD/src

#        LIBS += -L$$PWD/lib/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
#        LIBS += -L$$PWD/lib/win64/SDL2/lib -lSDL2
#    }

#    LIBS += -lmfplat -lmfuuid -lksuser -lwinmm -lwmcodecdspuuid
#}

#unix{

#    DEFINES += NDEBUG _CONSOLE __LINUX_ALSA__

#    contains(QT_ARCH, i386) {
#        message("32-bit, 请自行编译32位库!")
#    } else {
#        message("64-bit")
#        INCLUDEPATH += $$PWD/lib/linux/ffmpeg/include \
#                       $$PWD/lib/linux/SDL2/include/SDL2 \
#                       $$PWD/lib/linux/alsa/include \
#                       $$PWD/src

#        LIBS += -L$$PWD/lib/linux/ffmpeg/lib  -lavformat  -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale -lpostproc
#        LIBS += -L$$PWD/lib/linux/SDL2/lib -lSDL2
#        LIBS += -L$$PWD/lib/linux/alsa/lib -lasound
#        LIBS += -lpthread -ldl
#    }

#    #QMAKE_POST_LINK 表示编译后执行内容
#    #QMAKE_PRE_LINK 表示编译前执行内容

#    #解压库文件
#    #QMAKE_PRE_LINK += "cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz "

#    system("cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz")
#    system("cd $$PWD/lib/linux && tar xvzf SDL2.tar.gz")
#    system("cd $$PWD/lib/linux && tar xvzf alsa.tar.gz")
#}
