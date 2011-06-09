QT = core network gui
TARGET = NeTVServer
TEMPLATE = app
CONFIG += CONSOLE

DESTDIR = $$PWD

CONFIG(debug, debug|release) {
  DEFINES += SUPERVERBOSE
}

HEADERS = \
    static.h \
    startup.h \
    requestmapper.h \
    controller/dumpcontroller.h \
    controller/templatecontroller.h \
    controller/formcontroller.h \
    controller/fileuploadcontroller.h \
    controller/sessioncontroller.h \
    controller/framebuffercontroller.h

SOURCES = main.cpp \
    static.cpp \
    startup.cpp \
    requestmapper.cpp \
    controller/dumpcontroller.cpp \
    controller/templatecontroller.cpp \
    controller/formcontroller.cpp \
    controller/fileuploadcontroller.cpp \
    controller/sessioncontroller.cpp \
    controller/framebuffercontroller.cpp

OTHER_FILES += \
    ../etc/NeTVServer.ini \
    ../etc/flashpolicy.xml \
    ../etc/start_netvserver.sh \
    ../etc/docroot/index.html \
    ../etc/docroot/xmlbridge.html \
    ../etc/docroot/favicon.ico \
    ../etc/docroot/images/chumby.png \
    ../etc/docroot/scripts/hello.sh \
    ../etc/docroot/scripts/test_param.sh \
    ../etc/docroot/scripts/xmlbridge.sh \
    ../etc/templates/demo.tpl \
    ../Doxyfile \
    ../doc/license.txt \
    ../doc/releasenotes.txt \
    ../doc/example-response-normal.txt \
    ../doc/example-response-chunked.txt \
    ../doc/example-request-get.txt \
    ../doc/example-request-form-post.txt \
    ../doc/example-request-form-get.txt \
    ../doc/example-request-file-upload.txt

# Singleton & command line argument passing
include(../lib/qtsingleapplication-2.6_1-opensource/src/qtsingleapplication.pri)

include(../lib/bfLogging/src/bfLogging.pri)
include(../lib/bfHttpServer/src/bfHttpServer.pri)
include(../lib/bfFlashPolicyServer/src/bfFlashpolicyserver.pri)
include(../lib/bfXmlSocketServer/src/bfXmlsocketserver.pri)
include(../lib/bfTemplateEngine/src/bfTemplateEngine.pri)

# Changes the name of the target, when is debug mode
CONFIG( debug, debug|release ) {
    TARGET = $${TARGET}_debug
    BUILD_NAME = debug
}
CONFIG( release, debug|release ) {
    TARGET = $${TARGET}
    BUILD_NAME = release
}

# Recommended for Bitbake
target.path = /usr/bin
INSTALLS += target

# Temporary folders for the auxiliar files
INCLUDEPATH += $$PWD/tmp/$$BUILD_NAME $$PWD/tmp
OBJECTS_DIR = $$PWD/tmp/$$BUILD_NAME
MOC_DIR = $$PWD/tmp/$$BUILD_NAME
UI_DIR = $$PWD/tmp/$$BUILD_NAME
RCC_DIR = $$PWD/tmp/$$BUILD_NAME
DESTDIR = $$PWD/bin
