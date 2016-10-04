#
# This project extends the functionality of a simple web service
# by adding 'controllers' classes
#

QT          += core network
QT          -= gui
CONFIG      += console
osx:CONFIG  -= app_bundle

TARGET       = NeTVServer
TEMPLATE     = app

HEADERS     += \
   netvhandlers.h \
    netvserverapplication.h \
    nhttprequest.h \
    nhttpresponse.h \
    static.h \
    controller/bridgecontroller.h \
    geturlfinisher.h

SOURCES     += main.cpp \
    netvhandlers.cpp \
    netvserverapplication.cpp \
    nhttprequest.cpp \
    nhttpresponse.cpp \
    controller/bridgecontroller.cpp \
    geturlfinisher.cpp

# qhttp defines
DEFINES       *= QHTTP_MEMORY_LOG=0

# Joyent http_parser
SOURCES  += $$PWD/../qhttp/3rdparty/http-parser/http_parser.c
HEADERS  += $$PWD/../qhttp/3rdparty/http-parser/http_parser.h

SOURCES  += \
    $$PWD/../qhttp/src/qhttpabstracts.cpp \
    $$PWD/../qhttp/src/qhttpserverconnection.cpp \
    $$PWD/../qhttp/src/qhttpserverrequest.cpp \
    $$PWD/../qhttp/src/qhttpserverresponse.cpp \
    $$PWD/../qhttp/src/qhttpserver.cpp \
    $$PWD/../qhttp/src/qhttpclient.hpp \
    $$PWD/../qhttp/src/qhttpclientresponse.hpp \
    $$PWD/../qhttp/src/qhttpclientrequest.hpp

HEADERS  += \
    $$PWD/../qhttp/src/qhttpfwd.hpp \
    $$PWD/../qhttp/src/qhttpabstracts.hpp \
    $$PWD/../qhttp/src/qhttpserverconnection.hpp \
    $$PWD/../qhttp/src/qhttpserverrequest.hpp \
    $$PWD/../qhttp/src/qhttpserverresponse.hpp \
    $$PWD/../qhttp/src/qhttpserver.hpp \
    $$PWD/../qhttp/src/qhttpclientrequest.cpp \
    $$PWD/../qhttp/src/qhttpclientresponse.cpp \
    $$PWD/../qhttp/src/qhttpclient.cpp

INCLUDEPATH += $$PWD/../qhttp/src \
               $$PWD/../qhttp/3rdparty
