INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Handle raw TCP socket connections
HEADERS += $$PWD/httplistener.h $$PWD/httpconnectionhandler.h $$PWD/httpconnectionhandlerpool.h $$PWD/httprequest.h $$PWD/httpresponse.h $$PWD/httpcookie.h $$PWD/httprequesthandler.h
SOURCES += $$PWD/httplistener.cpp $$PWD/httpconnectionhandler.cpp $$PWD/httpconnectionhandlerpool.cpp $$PWD/httprequest.cpp $$PWD/httpresponse.cpp $$PWD/httpcookie.cpp $$PWD/httprequesthandler.cpp

# Session
HEADERS += $$PWD/httpsession.h $$PWD/httpsessionstore.h
SOURCES += $$PWD/httpsession.cpp $$PWD/httpsessionstore.cpp

# File dump
HEADERS += $$PWD/controller/staticfilecontroller.h
SOURCES += $$PWD/controller/staticfilecontroller.cpp

# Shell script execution (cgi-like)
HEADERS += $$PWD/controller/scriptcontroller.h
SOURCES += $$PWD/controller/scriptcontroller.cpp

# Hardware bridge
HEADERS += $$PWD/controller/bridgecontroller.h
SOURCES += $$PWD/controller/bridgecontroller.cpp

# UDP cursor
unix:!mac:HEADERS += $$PWD/controller/cursorcontroller.h
unix:!mac:SOURCES += $$PWD/controller/cursorcontroller.cpp

OTHER_FILES += $$PWD/../doc/readme.txt
