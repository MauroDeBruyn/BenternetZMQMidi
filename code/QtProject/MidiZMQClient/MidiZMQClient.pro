DEFINES += ZMQ_STATIC __WINDOWS_MM__

LIBS += -L$$PWD/../lib -lws2_32 -lpthread -lIphlpapi -lzmq -lnzmqt
win32:LIBS += -lwinmm

SOURCES += main.cpp RtMidi.cpp

INCLUDEPATH += $$PWD/../include
HEADERS += RtMidi.h

QT += core
CONFIG += console c++11
CONFIG -= app_bundle

QT += core gui widgets network
CONFIG += c++11
INCLUDEPATH += path/to/nzmqt/include
LIBS += -Lpath/to/nzmqt/lib -lnzmqt
