QT -= gui

QT += network xml multimedia

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
        main.cpp \
        tcpserver.cpp \
        tcpsocket.cpp \
        testsender.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    common.h \
    tcpserver.h \
    tcpsocket.h \
    testsender.h
