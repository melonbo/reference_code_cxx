TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += /mnt/hgfs/share/ffmpeg/lib-ffmpeg_4.1.4-x86-ubuntu/include/
LIBS += -L/mnt/hgfs/share/ffmpeg/lib-ffmpeg_4.1.4-x86-ubuntu/lib/ -lavformat -lavdevice -lavfilter -lavcodec -lavutil -lswresample -lx264 -lz -pthread

SOURCES += main.c
