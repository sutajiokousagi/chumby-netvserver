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
    //Forward it to browser
    qDebug() << "BridgeController: [NMStateChanged] " << state;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMStateChanged</cmd><data><value>") + QString().number(state).toLatin1() + "</value></data></xml>", "netvbrowser");
}

void BridgeController::slot_PropertiesChanged(QByteArray /* prop_name */, QByteArray /* prop_value */)
{
    //Forward it to browser
    //qDebug() << "BridgeController: [NMPropertiesChanged] " << state;
    //Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMPropertiesChanged</cmd><data><value>" + objPath + "</value></data></xml>", "netvbrowser");
}
void BridgeController::slot_DeviceAdded(QByteArray objPath)
{
    //Forward it to browser
    qDebug() << "BridgeController: [NMDeviceAdded] " << objPath;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMDeviceAdded</cmd><data><value>") + objPath + "</value></data></xml>", "netvbrowser");
}

void BridgeController::slot_DeviceRemoved(QByteArray objPath)
{
    //Forward it to browser
    qDebug() << "BridgeController: [NMDeviceRemoved] " << objPath;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMDeviceRemoved</cmd><data><value>") + objPath + "</value></data></xml>", "netvbrowser");
}

void BridgeController::slot_StopAP()
{
    this->Execute(docroot + "/scripts/stop_ap.sh");
}

void BridgeController::slot_StartAP()
{
    this->Execute(docroot + "/scripts/start_ap.sh");
}
