/**
  @file
  @author Stefan Frings
  @version $Id: startup.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include <QDir>
#include "static.h"
#include "startup.h"
#include "dualfilelogger.h"
#include "httplistener.h"
#include "flashpolicyserver.h"
#include "xmlsocketserver.h"
#include "requestmapper.h"
#include "controller/staticfilecontroller.h"
#include "controller/scriptcontroller.h"

#if defined Q_OS_UNIX && !defined Q_OS_MAC
    #include "controller/cursorcontroller.h"
#endif

/** Name of this application */
#define APPNAME "NeTVServer"

/** Publisher of this application */
#define ORGANISATION "Chumby Industries"

/** Short description of this application */
#define DESCRIPTION "Customized web server based on Qt"

/** The special string used to split & join arguements */
#define ARGS_SPLIT_TOKEN    "|~|"

Startup::Startup(int argc, char *argv[])
    : QtSingleApplication(argc, argv)
{
    //Check if another instance is already running by sending a message to it
    QStringList argsList = this->arguments();
    QString argsString = argsList.join(ARGS_SPLIT_TOKEN);

    if (this->sendMessage(argsString))
    {
        printf("Sending '%s' to running NeTVServer instance\n", argsString.toLatin1().constData());
        exit(0);
    }

    printf("Starting new NeTVServer with args: %s\n", argsString.toLatin1().constData());

    start();
    this->receiveArgs(argsString);
    QObject::connect(this, SIGNAL(messageReceived(const QString&)), this, SLOT(receiveArgs(const QString&)));
}

void Startup::start()
{
    // Initialize the core application
    QtSingleApplication* app = this;
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

    // Configure bridge controller
    Static::bridgeController=new BridgeController(fileSettings,app);

    // Configure cursor controller
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    Static::cursorController=new CursorController(fileSettings,app);
#endif

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat,app);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings, app);

    // Configure XML socket server
    QSettings* xmlserverSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    xmlserverSettings->beginGroup("xml-socket-server");
    new XmlSocketServer(xmlserverSettings, app);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    listenerSettings->beginGroup("listener");
    new HttpListener(listenerSettings,new RequestMapper(app),app);

    qDebug("ServiceHelper: Application has started");
}

void Startup::receiveArgs(const QString &argsString)
{
    QStringList argsList = argsString.split(ARGS_SPLIT_TOKEN);
    int argCount = argsList.count();
    if (argCount < 2)
        return;

    QString execPath = argsList[0];
    QString command = argsList[1];
    argsList.removeFirst();
    argsList.removeFirst();
    argCount = argsList.count();

    printf("Received argument: %s", command.toLatin1().constData());

    /*
    if (command == "SetUrl" && argCount >= 1)
    {
        QString param = argsList[0];
        //myWebView->load( QUrl(param) );
    }

    else if (command == "SetHtml" && argCount >= 1)
    {
        QString param = argsList[0];
        //myWebView->setHtml(param, QUrl("http://localhost"));
    }

    else if (command == "Javascript" && argCount >= 1)
    {
        QString param = argsList.join(" ");
        //myWebView->page()->mainFrame()->evaluateJavaScript(param);
    }

    else if (command == "InvertColor" && argCount >= 1)
    {
        QString param = argsList[0].toUpper();
        //myWebView->setInvertColor( param == "TRUE" || param == "YES" );
    }
    */
}
