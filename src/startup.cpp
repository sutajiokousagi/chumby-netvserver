#include <QDir>
#include "startup.h"
#include "static.h"
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
    //-------------------------------------------------------------------------
    // Handlers

    // Initialize the core application
    QString configFileName=Static::getConfigDir()+"/"+APPNAME+".ini";

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat);
    sessionSettings->beginGroup("sessions");
    Static::sessionStore=new HttpSessionStore(sessionSettings, this);

    // Configure static file controller
    QSettings* docrootSettings=new QSettings(configFileName,QSettings::IniFormat);
    docrootSettings->beginGroup("docroot");
    Static::staticFileController=new StaticFileController(docrootSettings);

    // Configure script controller
    Static::scriptController=new ScriptController(docrootSettings);

    // Configure bridge controller
    QSettings* bridgeSettings=new QSettings(configFileName,QSettings::IniFormat);
    bridgeSettings->beginGroup("bridge-controller");
    Static::bridgeController=new BridgeController(bridgeSettings,this);

    RequestMapper *requestMapper = new RequestMapper();

    //-------------------------------------------------------------------------
    // Servers

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings, this);

    // Configure and start the TCP listener
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat);
    listenerSettings->beginGroup("listener");
    HttpListener *httpListener = new HttpListener(listenerSettings, requestMapper, this);
    if (!httpListener->isListening()) {
        delete httpListener;
        Static::httpListener = NULL;
    }

    // Configure TCP socket server
    QSettings* tcpServerSettings=new QSettings(configFileName,QSettings::IniFormat);
    tcpServerSettings->beginGroup("tcp-socket-server");
    Static::tcpSocketServer = new TcpSocketServer(tcpServerSettings, requestMapper, this);
    if (!Static::tcpSocketServer->isListening()) {
        delete Static::tcpSocketServer;
        Static::tcpSocketServer = NULL;
    }

    // Configure UDP socket server
    QSettings* udpServerSettings=new QSettings(configFileName,QSettings::IniFormat);
    udpServerSettings->beginGroup("udp-socket-server");
    Static::udpSocketServer = new UdpSocketServer(udpServerSettings, requestMapper, this);

    // DBus monitor
#ifdef ENABLE_DBUS_STUFF
    Static::dbusMonitor = new DBusMonitor(this);
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_StateChanged(uint)), Static::bridgeController, SLOT(slot_StateChanged(uint)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_PropertiesChanged(QByteArray,QByteArray)), Static::bridgeController, SLOT(slot_PropertiesChanged(QByteArray,QByteArray)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_DeviceAdded(QByteArray)), Static::bridgeController, SLOT(slot_DeviceAdded(QByteArray)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_DeviceRemoved(QByteArray)), Static::bridgeController, SLOT(slot_DeviceRemoved(QByteArray)));
#endif
}

bool Startup::receiveArgs(const QString &argsString)
{
    QStringList argsList = argsString.split(ARGS_SPLIT_TOKEN);
    int argCount = argsList.count();
    if (argCount < 2)
        return false;

    QString execPath = argsList[0];
    QString command = argsList[1];
    argsList.removeFirst();
    argsList.removeFirst();
    argCount = argsList.count();

    qDebug("Received argument: %s", qPrintable(command));

    QByteArray string = processStatelessCommand(command.toLatin1(), argsList);
    if (string != UNIMPLEMENTED)            printf("NeTVServer: %s", string.constData());
    else                                    printf("NeTVServer: Invalid argument");
    return false;
}

QByteArray Startup::processStatelessCommand(QByteArray command, QStringList argsList)
{
    //command name
    if (command.length() < 0)
        return UNIMPLEMENTED;
    command = command.toUpper();

    //arguments
    int argCount = argsList.count();

    if (command == "HELLO")
    {
        //Just a dummy command
        qDebug("argCount %d", argCount);
    }

    //----------------------------------------------------

    return UNIMPLEMENTED;
}
