#ifndef NM_INTERFACE_H_1264772826
#define NM_INTERFACE_H_1264772826

#include <QtDBus>
#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

/*
 * Proxy class for interface org.freedesktop.NetworkManager
 */
class OrgFreedesktopNetworkManagerInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManager"; }

public:
    OrgFreedesktopNetworkManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgFreedesktopNetworkManagerInterface();

public Q_SLOTS: // METHODS
    /*
    inline QDBusPendingReply<> GetDevices()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetDevices"), argumentList);
    }

    inline QDBusPendingReply<> ActivateConnection()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("ActivateConnection"), argumentList);
    }

    inline QDBusPendingReply<> DeactivateConnection()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("DeactivateConnection"), argumentList);
    }

    inline QDBusPendingReply<> Sleep()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Sleep"), argumentList);
    }
    */

Q_SIGNALS: // SIGNALS
    void StateChanged(const uint &state);
    //void PropertiesChanged();
    void DeviceAdded(const QDBusObjectPath &objPath);
    void DeviceRemoved(const QDBusObjectPath &objPath);
};

namespace org {
  namespace freedesktop {
    typedef ::OrgFreedesktopNetworkManagerInterface NetworkManagerInterface;
  }
}
#endif
