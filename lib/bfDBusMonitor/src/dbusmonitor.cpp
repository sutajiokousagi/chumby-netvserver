#include "static.h"
#include "dbusmonitor.h"

DBusMonitor::DBusMonitor(QObject *parent) : QObject(parent)
{
    nm_interface = new org::freedesktop::NetworkManagerInterface(this);

    if (!nm_interface->isValid())
    {
        qDebug("DBusMonitor: failed to start NM interface");
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

    //-------------

    nm_ap_interface = new org::freedesktop::NetworkManagerAPInterface(this);

    if (!nm_ap_interface->isValid())
    {
        qDebug("DBusMonitor: failed to start NM AP interface");
        qDebug("%s", qPrintable(nm_ap_interface->connection().lastError().message()));
    }
    else
    {
        nm_ap_interface->connection().connect(nm_ap_interface->service(), nm_ap_interface->path(), nm_ap_interface->interface(), "PropertiesChanged", this, SLOT(PropertiesChanged(QVariantMap)));
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

void DBusMonitor::PropertiesChanged(QVariantMap changed_properties)
{
    qDebug() << "DBusMonitor: [PropertiesChanged] " << changed_properties;
    //emit signal_PropertiesChanged("abc", "123");
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
