#include "bridgecontroller.h"
#include "../static.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUrl>
#include <QXmlStreamWriter>


//----------------------------------------------------------------------------------------------------
// Public slots
//----------------------------------------------------------------------------------------------------

// From dbusmonitor.h
void BridgeController::slot_StateChanged(uint state)
{
    //Forward it to NeTVBrowser
    qDebug() << "BridgeController: [NMStateChanged] " << state;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMStateChanged</cmd><value>") + QString().number(state).toLatin1() + "</value></xml>", "netvbrowser");
}

void BridgeController::slot_PropertiesChanged(QByteArray /* prop_name */, QByteArray /* prop_value */)
{
    //Forward it to NeTVBrowser
    //qDebug() << "BridgeController: [NMPropertiesChanged] " << state;
    //Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMPropertiesChanged</cmd><value>" + objPath + "</value></xml>", "netvbrowser");
}
void BridgeController::slot_DeviceAdded(QByteArray objPath)
{
    //Forward it to NeTVBrowser
    qDebug() << "BridgeController: [NMDeviceAdded] " << objPath;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMDeviceAdded</cmd><value>") + objPath + "</value></xml>", "netvbrowser");
}

void BridgeController::slot_DeviceRemoved(QByteArray objPath)
{
    //Forward it to NeTVBrowser
    qDebug() << "BridgeController: [NMDeviceRemoved] " << objPath;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMDeviceRemoved</cmd><value>") + objPath + "</value></xml>", "netvbrowser");
}

void BridgeController::slot_StopAP()
{
    this->Execute(docroot + "/scripts/stop_ap.sh");
}

void BridgeController::slot_StartAP()
{
    this->Execute(docroot + "/scripts/start_ap.sh");
}

void BridgeController::slot_StartAP_Factory()
{
    this->Execute(docroot + "/scripts/start_ap_factory.sh");
}

void BridgeController::slot_Reboot()
{
    this->Execute("/sbin/reboot");
}
