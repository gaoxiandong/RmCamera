#-------------------------------------------------
#
# Project created by QtCreator 2017-10-19T15:32:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = RmCamera
TEMPLATE = lib

DEFINES += RMCAMERA_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        rmcamera.cpp \
    dhcamera.cpp \
    hkcamera.cpp

HEADERS += \
        rmcamera.h \
    dhcamera.h \
    dhnetsdk.h \
    hkcamera.h

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}

unix: LIBS += -L/usr/local/lib/ \
            -lopencv_core \
            -lopencv_imgproc \
            -lopencv_highgui \
            -lopencv_video

unix: LIBS += -L/usr/lib/ \
              -L/usr/lib/x86_64-linux-gnu \
              -llapack \
              -lpthread



unix:LIBS += -L/home/gxd/Downloads/General_PlaySDK_Eng_Linux64_IS_V3.39.0.R.161116/Bin \
        -ldhplay

unix:LIBS  += -L/home/gxd/Downloads/NetSDK_Chn_Bin/libs -ldhnetsdk -lpthread

LIBS += -L/home/gxd/rmface/CH_HCNetSDK_V5.2.7.4_build20170606_Linux64/lib/ \
        -lhcnetsdk \
        -lHCCore \
        -lhpr \
        -lPlayCtrl \
        -lSuperRender \
        -lAudioRender


INCLUDEPATH += /home/gxd/rmface/CH_HCNetSDK_V5.2.7.4_build20170606_Linux64/incCn

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += /home/gxd/Downloads/General_PlaySDK_Eng_Linux64_IS_V3.39.0.R.161116/Bin
INCLUDEPATH += /home/gxd/Downloads/NetSDK_Chn_Bin/libs
