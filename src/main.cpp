
#include "netvserverapplication.h"
#include "nhttprequest.h"
#include "netvhandlers.h"

/** Name of this application */
#define APPNAME "NeTVServer"

#define THREAD_COUNT 20

#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

/*
 * Create handlers, listeners and assign them to global pointers in Static class
 */
static int loadConfiguration(void)
{
#if 0
    // Configuration file
    QString configFileName=Static::getConfigDir()+"/"+APPNAME+".ini";

    // Configure bridge controller
    QSettings* bridgeSettings=new QSettings(configFileName,QSettings::IniFormat);
    bridgeSettings->beginGroup("bridge-controller");
    Static::bridgeController=new BridgeController(bridgeSettings);

    RequestMapper *requestMapper = new RequestMapper();
    Static::requestMapper = requestMapper;

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings);

    // Configure TCP socket server
    QSettings* tcpServerSettings=new QSettings(configFileName,QSettings::IniFormat);
    tcpServerSettings->beginGroup("tcp-socket-server");
    Static::tcpSocketServer = new TcpSocketServer(tcpServerSettings, requestMapper);
    if (!Static::tcpSocketServer->isListening()) {
        delete Static::tcpSocketServer;
        Static::tcpSocketServer = NULL;
    }

    // Configure UDP socket server
    QSettings* udpServerSettings=new QSettings(configFileName,QSettings::IniFormat);
    udpServerSettings->beginGroup("udp-socket-server");
    Static::udpSocketServer = new UdpSocketServer(udpServerSettings, requestMapper);

    // DBus monitor
#ifdef ENABLE_DBUS_STUFF
    Static::dbusMonitor = new DBusMonitor();
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_StateChanged(uint)), Static::bridgeController, SLOT(slot_StateChanged(uint)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_PropertiesChanged(QByteArray,QByteArray)), Static::bridgeController, SLOT(slot_PropertiesChanged(QByteArray,QByteArray)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_DeviceAdded(QByteArray)), Static::bridgeController, SLOT(slot_DeviceAdded(QByteArray)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_DeviceRemoved(QByteArray)), Static::bridgeController, SLOT(slot_DeviceRemoved(QByteArray)));
#endif
#endif
    return 0;
}


int main(int argc, char ** argv) {
    NeTVServerApplication app(argc, argv);
#if defined(Q_OS_UNIX)
    catchUnixSignals({ SIGQUIT, SIGINT, SIGTERM, SIGHUP });
#endif

    app.registerBridgeFunction("geturl", handleGetUrl);
    app.registerBridgeFunction("getjpeg", handleGetJpeg);
    app.registerBridgeFunction("getjpg", handleGetJpeg);
    app.registerBridgeFunction("initialhello", handleInitialHello);
    app.registerBridgeFunction("getparam", handleGetParam);

    app.registerBridgeFunction("getlocalwidgets", handleGetLocalWidgets);
    app.registerBridgeFunction("getlocalwidgetconfig", handleGetLocalWidgetConfig);

    app.registerBridgeFunction("", handleDefault);
    app.setStaticDocRoot("D:\\Code\\netv-controlpanel");
    app.exec();
}
