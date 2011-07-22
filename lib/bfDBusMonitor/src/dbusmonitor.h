#ifndef DBUSMONITOR_H
#define DBUSMONITOR_H

#include <nm_interface.h>

class DBusMonitor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DBusMonitor)
public:

    /** Constructor */
    DBusMonitor(QObject* parent = 0);

    /** Destructor */
    virtual ~DBusMonitor();

private:

    org::freedesktop::NetworkManagerInterface *nm_interface;

signals:

    void signal_StateChanged(uint state);
    //void signal_PropertiesChanged();
    void signal_DeviceAdded(QDBusObjectPath objPath);
    void signal_DeviceRemoved(QDBusObjectPath objPath);

private slots:

    void StateChanged(uint state);
    //void PropertiesChanged();
    void DeviceAdded(QDBusObjectPath objPath);
    void DeviceRemoved(QDBusObjectPath objPath);
};

#endif // DBUSMONITOR_H
