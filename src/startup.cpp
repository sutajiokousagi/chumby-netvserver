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

/** Name of this application */
#define APPNAME "NeTVServer"

/** Publisher of this application */
#define ORGANISATION "Chumby Industries"

/** Short description of this application */
#define DESCRIPTION "Customized web server based on Qt"

/** The special string used to split & join arguements */
#define ARGS_SPLIT_TOKEN    "|~|"

Startup::Startup(int argc, char *argv[]) : QtSingleApplication(argc, argv, QApplication::GuiServer)
{
    //Check if another instance is already running by sending a message to it
    QStringList argsList = this->arguments();
    QString argsString = argsList.join(ARGS_SPLIT_TOKEN);

    if (this->sendMessage(argsString))
    {
        printf("Sending '%s' to running NeTVServer instance\n", argsString.toLatin1().constData());
        exit(0);
    }

#ifdef Q_WS_QWS
    //QWSServer::setCursorVisible(false);
    //QWSServer::setBackground(QBrush(QColor(240,0,240)));
#endif

    printf("Starting new NeTVServer with args: %s\n", argsString.toLatin1().constData());

    start();
    this->receiveArgs(argsString);
    QObject::connect(this, SIGNAL(messageReceived(const QString&)), this, SLOT(receiveArgs(const QString&)));
}

void Startup::start()
{
    // Fill the screen with default chroma key color
    QScreen* theScreen = QScreen::instance();
    theScreen->solidFill(QColor(240,0,240), QRegion(0,0, theScreen->width(),theScreen->height()));

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
#if defined (CURSOR_CONTROLLER)
    Static::cursorController=new CursorController(fileSettings,app);
#endif

    // Configure framebuffer controller
    QSettings* fbSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    fbSettings->beginGroup("framebuffer-controller");
    Static::framebufferController=new FramebufferController(fbSettings, app);

    // Configure bridge controller
    Static::bridgeController=new BridgeController(fileSettings,app);

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat,app);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings, app);

    RequestMapper *requestMapper = new RequestMapper(app);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    listenerSettings->beginGroup("listener");
    new HttpListener(listenerSettings, requestMapper, app);

    // Configure TCP socket server
    QSettings* tcpServerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    tcpServerSettings->beginGroup("tcp-socket-server");
    Static::tcpSocketServer=new TcpSocketServer(tcpServerSettings, requestMapper, app);

    // Configure UDP socket server
    QSettings* udpServerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    udpServerSettings->beginGroup("udp-socket-server");
    Static::udpSocketServer=new UdpSocketServer(udpServerSettings, requestMapper, app);

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

    //QByteArray string = processStatelessCommand(command.toLatin1(), argsList);
    //if (string != UNIMPLEMENTED)            printf("%s", string.constData());
    //else                                    printf("Invalid argument");
}

QByteArray Startup::processStatelessCommand(QByteArray command, QStringList argsList)
{
    /*
    //command name
    if (command.length() < 0)
        return UNIMPLEMENTED;
    command = command.toUpper();

    //arguments
    int argCount = argsList.count();

    if (command == "QUIT" || command == "EXIT" || command == "TERMINATE")
    {
        QApplication::exit(0);
        return command;
    }

    else if (command == "RESTART")
    {
        //This will just ignore the 'singleton' class behaviour. Awesome!
        QProcess::startDetached( QApplication::applicationFilePath() );
        QApplication::exit(0);
        return command;
    }

    //----------------------------------------------------

    else if (command == "SETRESOLUTION" && argCount == 1)
    {
        //comma-separated arguments
        QString args = argsList[0];
        QStringList argsLs = args.split(",");
        if (argsLs.count())
        int w = argsLs[0].toInt();
        int h = argsLs[1].toInt();
        int depth = argsLs[2].toInt();
        QScreen().instance()->setMode(w,h,depth);
    }

#ifdef Q_WS_QWS
    else if (command == "SETRESOLUTION" && argCount >= 3)
    {
        //space-separated arguments
        int w = argsList[0].toInt();
        int h = argsList[1].toInt();
        int depth = argsList[2].toInt();
        QScreen().instance()->setMode(w,h,depth);
    }
#endif

    return UNIMPLEMENTED;
    */
    return "";
}
