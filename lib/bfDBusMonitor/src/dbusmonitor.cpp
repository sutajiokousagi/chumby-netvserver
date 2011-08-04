#include "static.h"
#include "dbusmonitor.h"

DBusMonitor::DBusMonitor(QObject *parent) : QObject(parent)
{
    nm_interface = new org::freedesktop::NetworkManagerInterface(this);

    if (!nm_interface->isValid())
    {
        qDebug("DBusMonitor: failed to start");
        qDebug("%s", qPrintable(nm_interface->connection().lastError().message()));
    }
    else
    {
        nm_interface->connection().connect(nm_interface->service(), nm_interface->path(), nm_interface->interface(), "StateChanged", this, SLOT(StateChanged(uint)));
        nm_interface->connection().connect(nm_interface->service(), nm_interface->path(), nm_interface->interface(), "DeviceAdded", this, SLOT(DeviceAdded(QDBusObjectPath)));
        nm_interface->connection().connect(nm_interface->service(), nm_interface->path(), nm_interface->interface(), "DeviceRemoved", this, SLOT(DeviceRemoved(QDBusObjectPath)));
        qDebug("DBusMonitor: started");

        //QVariant state = nm_interface->property("State");
        //qDebug() << "DBusMonitor: NetworkManager State: " << state.toUInt();
    }
}

DBusMonitor::~DBusMonitor()
{
    delete nm_interface;
    qDebug("DBusMonitor: closed");
}


void DBusMonitor::StateChanged(uint state)
{
    qDebug() << "DBusMonitor: [StateChanged] " << state;
    emit signal_StateChanged(state);
}

void DBusMonitor::DeviceAdded(QDBusObjectPath objPath)
{
    qDebug() << "DBusMonitor: [DeviceAdded] ";
    emit signal_DeviceAdded(objPath.path().toLatin1());
}

void DBusMonitor::DeviceRemoved(QDBusObjectPath objPath)
{
    qDebug() << "DBusMonitor: [DeviceRemoved] ";
    emit signal_DeviceRemoved(objPath.path().toLatin1());
}
