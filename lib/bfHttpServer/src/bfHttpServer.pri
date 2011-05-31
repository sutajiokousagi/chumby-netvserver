INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/httplistener.h $$PWD/httpconnectionhandler.h $$PWD/httpconnectionhandlerpool.h $$PWD/httprequest.h $$PWD/httpresponse.h $$PWD/httpcookie.h $$PWD/httprequesthandler.h
HEADERS += $$PWD/httpsession.h $$PWD/httpsessionstore.h
HEADERS += $$PWD/controller/staticfilecontroller.h
HEADERS += $$PWD/controller/scriptcontroller.h
HEADERS += $$PWD/controller/cursorcontroller.h

SOURCES += $$PWD/httplistener.cpp $$PWD/httpconnectionhandler.cpp $$PWD/httpconnectionhandlerpool.cpp $$PWD/httprequest.cpp $$PWD/httpresponse.cpp $$PWD/httpcookie.cpp $$PWD/httprequesthandler.cpp
SOURCES += $$PWD/httpsession.cpp $$PWD/httpsessionstore.cpp
SOURCES += $$PWD/controller/staticfilecontroller.cpp
SOURCES += $$PWD/controller/scriptcontroller.cpp
SOURCES += $$PWD/controller/cursorcontroller.cpp

OTHER_FILES += $$PWD/../doc/readme.txt
