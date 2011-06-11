#
# This project only contains the core functionality of a web service
# that is handling connections & serving static files.
# Other fancy functionality are to be extended in wrapper project.
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Handle TCP socket connections
HEADERS += $$PWD/httplistener.h $$PWD/httpconnectionhandler.h $$PWD/httpconnectionhandlerpool.h $$PWD/httprequest.h $$PWD/httpresponse.h $$PWD/httpcookie.h $$PWD/httprequesthandler.h
SOURCES += $$PWD/httplistener.cpp $$PWD/httpconnectionhandler.cpp $$PWD/httpconnectionhandlerpool.cpp $$PWD/httprequest.cpp $$PWD/httpresponse.cpp $$PWD/httpcookie.cpp $$PWD/httprequesthandler.cpp

# Session
HEADERS += $$PWD/httpsession.h $$PWD/httpsessionstore.h
SOURCES += $$PWD/httpsession.cpp $$PWD/httpsessionstore.cpp

# File content dump
HEADERS += $$PWD/controller/staticfilecontroller.h
SOURCES += $$PWD/controller/staticfilecontroller.cpp

OTHER_FILES += $$PWD/../doc/readme.txt
