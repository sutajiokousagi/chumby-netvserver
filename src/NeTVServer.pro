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

PRJDIR       = ../..
include($$PRJDIR/commondir.pri)

HEADERS     +=

SOURCES     += main.cpp

LIBS += -lqhttp