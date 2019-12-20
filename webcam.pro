QT -= gui

QT += network xml multimedia

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
        camerastream.cpp \
        main.cpp \
        tcpserver.cpp \
        tcpsocket.cpp \
        testsender.cpp \
        videosurface.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    camerastream.h \
    common.h \
    tcpserver.h \
    tcpsocket.h \
    testsender.h \
    videosurface.h

#!win32{
#    LIBS += -lx264
#}

include(ffmpeg/ffmpeg.pri)
