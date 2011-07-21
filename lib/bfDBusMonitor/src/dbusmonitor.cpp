#include "static.h"
#include "dbusmonitor.h"

DBusMonitor::DBusMonitor(QObject *parent) : QObject(parent)
{
    nm_interface = new org::freedesktop::NetworkManagerInterface("org.freedesktop.NetworkManager", "/", QDBusConnection::systemBus(), this);
    qDebug("DBusMonitor: started");

    QDBusConnection::sessionBus().connect("org.freedesktop.NetworkManager", "/", "org.freedesktop.NetworkManager", "StateChanged", this, SLOT(StateChanged(uint)));
    QDBusConnection::sessionBus().connect("org.freedesktop.NetworkManager", "/", "org.freedesktop.NetworkManager", "DeviceAdded", this, SLOT(DeviceAdded(QDBusObjectPath)));
    QDBusConnection::sessionBus().connect("org.freedesktop.NetworkManager", "/", "org.freedesktop.NetworkManager", "DeviceRemoved", this, SLOT(DeviceRemoved(QDBusObjectPath)));
}

DBusMonitor::~DBusMonitor()
{
    qDebug("DBusMonitor: closed");
}


void DBusMonitor::StateChanged(const uint &state)
{
    qDebug() << "DBusMonitor: [StateChanged] " << state;
}

void DBusMonitor::DeviceAdded(const QDBusObjectPath & /* objPath */)
{
    qDebug() << "DBusMonitor: [DeviceAdded] ";
}

void DBusMonitor::DeviceRemoved(const QDBusObjectPath & /* objPath */)
{
    qDebug() << "DBusMonitor: [DeviceRemoved] ";
}
