INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/udpsocketserver.h
SOURCES += $$PWD/udpsocketserver.cpp

HEADERS += $$PWD/tcpsocketserver.h
SOURCES += $$PWD/tcpsocketserver.cpp

HEADERS += $$PWD/socketrequesthandler.h $$PWD/socketrequest.h $$PWD/socketresponse.h
SOURCES += $$PWD/socketrequesthandler.cpp $$PWD/socketrequest.cpp $$PWD/socketresponse.cpp
