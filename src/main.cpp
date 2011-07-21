/**
  @file
  @author Stefan Frings
  @version $Id: main.cpp 938 2010-12-05 14:29:58Z stefan $
*/

/**
  @mainpage
  This project provies libraries for writing server-side web application in C++
  based on the Qt toolkit. It is a light-weight implementation that works like 
  Java Servlets from the programmers point of view.
  <p>
  Features:

  - HTTP 1.1 web server
      - persistent connections
      - chunked and non-chunked transfer
      - file uploads (multipart encoded forms)
      - cookies
      - sessions
      - uses dynamic thread pool
      - optional file cache
  - Template engine 
      - multi-language
      - optional file cache
  - Logger
      - configurable without program restart
      - automatic backup and file rotation
      - configurable message format
      - messages may contain thread-local variables
      - optional buffer for writing history details in case of an error
  - Example application
      - Install and run as Windows service, unix daemon or at the command-line
      - Search config file in multiple common directories
      - Demonstrates how to write servlets for common use-cases

  If you write a real application based on this source, take a look into startup.cpp,
  which contains startup and shutdown procedures. The example source sets
  up a single listener on port 8080, however multiple listeners with individual
  configuration could be set up.
  <p>
  Incoming requests are mapped to controller classes in requestmapper.cpp, based on the
  requested path. If you want to learn form the example, then focus on these classes.
  <p>
  High-availability and HTTPS encryption can be easily added by putting an Apache HTTP server
  in front of the self-written web application using the mod-proxy module with sticky sessions.
*/

#include "startup.h"
#include <unistd.h>
#include <QtGui/QApplication>
#include <QWSServer>
#include <QBrush>
#include <qtsingleapplication.h>
#include "stdio.h"

int main(int argc, char *argv[])
{
    //Check if another instance is already running by sending a message to it
    QtSingleApplication instance(argc, argv, QApplication::GuiServer);
    instance.setApplicationName(APPNAME);
    instance.setOrganizationName(ORGANISATION);

    QStringList argsList = instance.arguments();
    QString argsString = argsList.join(ARGS_SPLIT_TOKEN);

    if (instance.sendMessage(argsString))
    {
        printf("Sending arguments to running NeTVServer instance: %s\n", argsString.toLatin1().constData());
        return 0;
        printf("Quitting... This should not be seen!!!!!");
    }

    // If the args list contains "-d", then daemonize
    if (argsList.contains("-d"))
        daemon(0, 0);

    printf("Starting new NeTVServer with args:");
    printf("%s", argsString.toLatin1().constData());

    //Pink background (temp fix. We should completely prevent it from drawing in non-window area)
#ifdef ENABLE_QWS_STUFF
    QWSServer *qserver = QWSServer::instance();
    qserver->setBackground(QBrush(QColor(240,0,240)));
    qserver->setCursorVisible(false);
    qserver->enablePainting(false);
#endif

    Startup startup;
    startup.receiveArgs(argsString);

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)), &startup, SLOT(receiveArgs(const QString&)));
#ifdef ENABLE_QWS_STUFF
    QObject::connect(qserver, SIGNAL(windowEvent(QWSWindow*, QWSServer::WindowEvent)), &startup, SLOT(windowEvent(QWSWindow*, QWSServer::WindowEvent)));
#endif
    return instance.exec();
}
