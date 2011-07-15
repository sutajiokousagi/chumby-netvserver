#include <QScreen>
#include <QColor>
#include <QDir>
#include "startup.h"
#include "static.h"
#include "dualfilelogger.h"
#include "httplistener.h"
#include "flashpolicyserver.h"
#include "tcpsocketserver.h"
#include "udpsocketserver.h"
#include "requestmapper.h"

#define UNIMPLEMENTED       "Un1mPl3m3nT3D"

Startup::Startup(QObject* parent) : QObject(parent)
{
    start();
}

void Startup::start()
{
    // Fill the screen with default chroma key color
    // This will fail during Linking on Qt Desktop version
    QScreen* theScreen = QScreen::instance();
    theScreen->solidFill(QColor(240,0,240), QRegion(0,0, theScreen->width(),theScreen->height()));

    // Initialize the core application
    QString configFileName=Static::getConfigDir()+"/"+APPNAME+".ini";

    // Configure logging into files
    QSettings* mainLogSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    mainLogSettings->beginGroup("mainLogFile");
    QSettings* debugLogSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    debugLogSettings->beginGroup("debugLogFile");
    Logger* logger=new DualFileLogger(mainLogSettings,debugLogSettings,10000,this);
    logger->installMsgHandler();

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    sessionSettings->beginGroup("sessions");
    Static::sessionStore=new HttpSessionStore(sessionSettings,this);

    // Configure static file controller
    QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    fileSettings->beginGroup("docroot");
    Static::staticFileController=new StaticFileController(fileSettings,this);

    // Configure script controller
    Static::scriptController=new ScriptController(fileSettings,this);

    // Configure cursor controller
#if defined (CURSOR_CONTROLLER)
    Static::cursorController=new CursorController(fileSettings,this);
#endif

    // Configure framebuffer controller
    QSettings* fbSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    fbSettings->beginGroup("framebuffer-controller");
    Static::framebufferController=new FramebufferController(fbSettings, this);

    // Configure bridge controller
    Static::bridgeController=new BridgeController(fileSettings,this);

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat,this);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings, this);

    RequestMapper *requestMapper = new RequestMapper(this);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    listenerSettings->beginGroup("listener");
    new HttpListener(listenerSettings, requestMapper, this);

    // Configure TCP socket server
    QSettings* tcpServerSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    tcpServerSettings->beginGroup("tcp-socket-server");
    Static::tcpSocketServer=new TcpSocketServer(tcpServerSettings, requestMapper, this);

    // Configure UDP socket server
    QSettings* udpServerSettings=new QSettings(configFileName,QSettings::IniFormat,this);
    udpServerSettings->beginGroup("udp-socket-server");
    Static::udpSocketServer=new UdpSocketServer(udpServerSettings, requestMapper, this);

    printf("NeTVServer has started");
    qDebug("Startup: Application has started");
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

    QByteArray string = processStatelessCommand(command.toLatin1(), argsList);
    if (string != UNIMPLEMENTED)            printf("%s", string.constData());
    else                                    printf("Invalid argument");
}

QByteArray Startup::processStatelessCommand(QByteArray command, QStringList argsList)
{
    //command name
    if (command.length() < 0)
        return UNIMPLEMENTED;
    command = command.toUpper();

    //arguments
    int argCount = argsList.count();

    //----------------------------------------------------

    if (command == "SETRESOLUTION" && argCount == 1)
    {
        //comma-separated arguments
        QString args = argsList[0];
        QStringList argsLs = args.split(",");
        if (argsLs.count() < 3)
            return UNIMPLEMENTED;

        int w = argsLs[0].toInt();
        int h = argsLs[1].toInt();
        int depth = argsLs[2].toInt();
        QScreen::instance()->setMode(w,h,depth);
    }

    else if (command == "SETRESOLUTION" && argCount >= 3)
    {
        //space-separated arguments
        int w = argsList[0].toInt();
        int h = argsList[1].toInt();
        int depth = argsList[2].toInt();
        QScreen::instance()->setMode(w,h,depth);
    }

    return UNIMPLEMENTED;
}
