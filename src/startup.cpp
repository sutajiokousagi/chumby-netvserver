/**
  @file
  @author Stefan Frings
  @version $Id: startup.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "static.h"
#include "startup.h"
#include "dualfilelogger.h"
#include "httplistener.h"
#include "requestmapper.h"
#include "controller/staticfilecontroller.h"
#include "controller/scriptcontroller.h"
#include "controller/cursorcontroller.h"
#include <QDir>

/** Name of this application */
#define APPNAME "NeTVServer"

/** Publisher of this application */
#define ORGANISATION "Chumby Industries"

/** Short description of this application */
#define DESCRIPTION "Customized web server based on Qt"


void Startup::start()
{
    // Initialize the core application
    QApplication* app = this;
    app->setApplicationName(APPNAME);
    app->setOrganizationName(ORGANISATION);
    QString configFileName=Static::getConfigDir()+"/"+QCoreApplication::applicationName()+".ini";

    // Configure logging into files
    QSettings* mainLogSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    mainLogSettings->beginGroup("mainLogFile");
    QSettings* debugLogSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    debugLogSettings->beginGroup("debugLogFile");
    Logger* logger=new DualFileLogger(mainLogSettings,debugLogSettings,10000,app);
    logger->installMsgHandler();

    // Configure template loader and cache
    QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    templateSettings->beginGroup("templates");
    Static::templateLoader=new TemplateCache(templateSettings,app);

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    sessionSettings->beginGroup("sessions");
    Static::sessionStore=new HttpSessionStore(sessionSettings,app);

    // Configure static file controller
    QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    fileSettings->beginGroup("docroot");
    Static::staticFileController=new StaticFileController(fileSettings,app);

    // Configure script controller
    Static::scriptController=new ScriptController(fileSettings,app);

    // Configure cursor controller
    Static::cursorController=new CursorController(fileSettings,app);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    listenerSettings->beginGroup("listener");
    new HttpListener(listenerSettings,new RequestMapper(app),app);

    qDebug("ServiceHelper: Application has started");
}

Startup::Startup(int argc, char *argv[])
    : QApplication(argc, argv)
{
    start();

    //setServiceDescription(DESCRIPTION);
    //setStartupType(AutoStartup);
}



