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

private slots:

    void StateChanged(const uint &state);
    //void PropertiesChanged();
    void DeviceAdded(const QDBusObjectPath &objPath);
    void DeviceRemoved(const QDBusObjectPath &objPath);
};

#endif // DBUSMONITOR_H
