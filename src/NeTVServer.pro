#
# This project extends the functionality of a simple web service
# by adding 'controllers' classes
#

QT = core network
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
    mot_ctl.h \
    controller/fileuploadcontroller.h \
    controller/sessioncontroller.h

SOURCES = main.cpp \
        static.cpp \
        startup.cpp \
        requestmapper.cpp \
        mot_ctl.cpp \
        controller/fileuploadcontroller.cpp \
        controller/sessioncontroller.cpp

# Include DBus support only on bitbake environment
!exists( /home/torin ) {
    message("DBus enabled")
    QT += dbus
}

# Shell script execution (cgi-like)
HEADERS += $$PWD/controller/scriptcontroller.h
SOURCES += $$PWD/controller/scriptcontroller.cpp

# Hardware bridge
HEADERS += $$PWD/controller/bridgecontroller.h
SOURCES += $$PWD/controller/bridgecontroller.cpp \
           $$PWD/controller/bridgecontroller_utils.cpp \
           $$PWD/controller/bridgecontroller_slots.cpp


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
    ../etc/docroot/scripts/xmlbridge.sh

include(../lib/bfHttpServer/src/bfHttpServer.pri)
include(../lib/bfFlashPolicyServer/src/bfFlashpolicyserver.pri)
include(../lib/bfSocketServer/src/bfSocketserver.pri)

# QDBus
contains( QT, dbus ) {
    CONFIG += qdbus
    DEFINES += ENABLE_DBUS_STUFF
    include(../lib/bfDBusMonitor/src/bfDBusMonitor.pri)
}

# Change the name of the target, when is debug mode
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

# FastCGI library
LIBS += -lfcgi

# Temporary folders for the auxiliar files
INCLUDEPATH += $$PWD/tmp/$$BUILD_NAME $$PWD/tmp
OBJECTS_DIR = $$PWD/tmp/$$BUILD_NAME
MOC_DIR = $$PWD/tmp/$$BUILD_NAME
UI_DIR = $$PWD/tmp/$$BUILD_NAME
RCC_DIR = $$PWD/tmp/$$BUILD_NAME
DESTDIR = $$PWD/bin

# Hacks for qmake compiling on Torin's development Ubuntu
exists( /home/torin ) {
    message("Cross compiling on Torin's Ubuntu development machine")
    INCLUDEPATH += /mnt/storage/fcgi/include
    LIBS += -L/mnt/storage/fcgi/lib
}
