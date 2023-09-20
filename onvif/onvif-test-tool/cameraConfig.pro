#-------------------------------------------------
#
# Project created by QtCreator 2019-01-10T17:46:10
#
#-------------------------------------------------

#QT += core network
#QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app

TARGET = cameraConfig
CONFIG   += arm

TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11 -fpermissive -O0

QMAKE_CXXFLAGS += -fpermissive
DEFINES += WITH_OPENSSL

arm{
LIBS += -L/opt/fsl-imx-fb/4.9.88-2.0.0/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/lib -lcrypto -lssl \
        -L/home/sad/res/log4cplus-1.1.2-arm/lib/ -llog4cplus

INCLUDEPATH += /opt/fsl-imx-fb/4.9.88-2.0.0/recipe-sysroot/usr/include/gstreamer-1.0
INCLUDEPATH += /opt/fsl-imx-fb/4.9.88-2.0.0/recipe-sysroot/usr/lib/glib-2.0/include
INCLUDEPATH += /opt/fsl-imx-fb/4.9.88-2.0.0/recipe-sysroot/usr/include/glib-2.0
INCLUDEPATH += /opt/fsl-imx-fb/4.9.88-2.0.0/recipe-sysroot/usr/include

INCLUDEPATH += /opt/fsl-imx-fb/4.9.88-2.0.0/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/include/ \
                /opt/fsl-imx-fb/4.9.88-2.0.0/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/include/c++/7.3.0/
}

INCLUDEPATH += \
    log/

SOURCES += main.cpp \
#    config/util.cpp \
    onvif/wsseapi.cpp \
    onvif/wsaapi.cpp \
    onvif/threads.cpp \
    onvif/stdsoap2.cpp \
    onvif/soapClient.cpp \
    onvif/soapC.cpp \
    onvif/smdevp.cpp \
    onvif/mecevp.cpp \
    onvif/duration.cpp \
    onvif/dom.cpp \
    onvif/CameraConfig.cpp \
    log/Log4c.cpp \
    onvif/onvif_comm.c \
    onvif/onvif_dump.c
#    config/platform.cpp

HEADERS += \
#    config/util.h \
    onvif/wsseapi.h \
    onvif/wsdd.h \
    onvif/wsaapi.h \
    onvif/threads.h \
    onvif/stdsoap2.h \
    onvif/soapStub.h \
    onvif/soapH.h \
    onvif/smdevp.h \
    onvif/mecevp.h \
    onvif/CameraConfig.h \    
    log/Log4c.h \
    onvif/onvif_comm.h \
    onvif/onvif_dump.h

#    config/platform.h


FORMS +=
