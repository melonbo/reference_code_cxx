#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T16:10:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


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
QMAKE_CXXFLAGS += -std=c++11

TARGET = VideoPlayer
TEMPLATE = app

#包含视频播放器的代码
#include(module/VideoPlayer/VideoPlayer.pri)
#包含可拖动窗体的代码
#include(module/DragAbleWidget/DragAbleWidget.pri)

SOURCES += src/main.cpp \
    src/Widget/VideoPlayerWidget.cpp \
    src/Widget/ShowVideoWidget.cpp \
    src/Widget/VideoSlider.cpp  \
    $$PWD/src/AppConfig.cpp \
    $$PWD/src/Mutex/Cond.cpp \
    $$PWD/src/Mutex/Mutex.cpp \
    $$PWD/src/LogWriter/LogWriter.cpp \
    $$PWD/src/VideoPlayer/VideoPlayer.cpp \
    $$PWD/src/VideoPlayer/Video/VideoPlayer_VideoThread.cpp \
    $$PWD/src/VideoPlayer/Audio/VideoPlayer_AudioThread.cpp \
    $$PWD/src/VideoPlayer/Audio/PcmVolumeControl.cpp \
    $$PWD/src/EventHandle/VideoPlayerEventHandle.cpp


HEADERS  += \
    src/Widget/VideoPlayerWidget.h \
    src/Widget/ShowVideoWidget.h \
    src/Widget/VideoSlider.h    \
    $$PWD/src/AppConfig.h \
    $$PWD/src/Mutex/Cond.h \
    $$PWD/src/Mutex/Mutex.h \
    $$PWD/src/LogWriter/LogWriter.h \
    $$PWD/src/VideoPlayer/VideoPlayer.h \
    $$PWD/src/VideoPlayer/Audio/PcmVolumeControl.h \
    $$PWD/src/EventHandle/VideoPlayerEventHandle.h \
    $$PWD/src/types.h


FORMS    += \
    src/Widget/VideoPlayerWidget.ui \
    src/Widget/ShowVideoWidget.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += $$PWD/src

win32:RC_FILE=$$PWD/resources/main.rc

win32{

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
