SOURCES += \
    main.cpp

DEFINES += ZMQ_STATIC
# DEFINES += NZMQT_LIB

LIBS += -L$$PWD/../lib -lws2_32 -lpthread -lIphlpapi -lzmq -lnzmqt
INCLUDEPATH += $$PWD/../include

SOURCES += main.cpp
