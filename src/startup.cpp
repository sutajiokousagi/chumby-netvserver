#include <QDir>
#include <QByteArray>
#include <QStringList>
#include "startup.h"
#include "static.h"
#include "flashpolicyserver.h"
#include "tcpsocketserver.h"
#include "udpsocketserver.h"
#include "requestmapper.h"

#define UNIMPLEMENTED       "Un1mPl3m3nT3D"

Startup::Startup(QObject* parent) : QObject(parent)
{
    //-------------------------------------------------------------------------
    // Handlers

    // Initialize the core application
    QString configFileName=Static::getConfigDir()+"/"+APPNAME+".ini";

    // Configure bridge controller
    QSettings* bridgeSettings=new QSettings(configFileName,QSettings::IniFormat);
    bridgeSettings->beginGroup("bridge-controller");
    Static::bridgeController=new BridgeController(bridgeSettings,this);

    RequestMapper *requestMapper = new RequestMapper();
    Static::requestMapper = requestMapper;

    //-------------------------------------------------------------------------
    // Servers

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings, this);

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

