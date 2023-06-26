TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += debug

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /mnt/hgfs/share/ffmpeg/lib-ffmpeg_4.1.4-x86-ubuntu/include/
LIBS += -L/mnt/hgfs/share/ffmpeg/lib-ffmpeg_4.1.4-x86-ubuntu/lib/ -lavformat -lavdevice -lavfilter -lavcodec -lavutil -lswresample -lx264 -lz -pthread


SOURCES += main.cpp \
    ffmux.cpp

HEADERS += \
    ffmux.h
