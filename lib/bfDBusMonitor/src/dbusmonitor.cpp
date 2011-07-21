#include "static.h"
#include "dbusmonitor.h"

DBusMonitor::DBusMonitor(QObject *parent) : QObject(parent)
{
    nm_interface = new org::freedesktop::NetworkManagerInterface("org.freedesktop.NetworkManager", "/", QDBusConnection::systemBus(), this);

    if (!nm_interface->isValid())
    {
        qDebug("DBusMonitor: error");
        qDebug("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
    else
    {
        QDBusConnection::sessionBus().connect(QString(), QString(), "org.freedesktop.NetworkManager", "StateChanged", this, SLOT(StateChanged(uint)));
        QDBusConnection::sessionBus().connect(QString(), QString(), "org.freedesktop.NetworkManager", "DeviceAdded", this, SLOT(DeviceAdded(QDBusObjectPath)));
        QDBusConnection::sessionBus().connect(QString(), QString(), "org.freedesktop.NetworkManager", "DeviceRemoved", this, SLOT(DeviceRemoved(QDBusObjectPath)));
        qDebug("DBusMonitor: started");
    }
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
