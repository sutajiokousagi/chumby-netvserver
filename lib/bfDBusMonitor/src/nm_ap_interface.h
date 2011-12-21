#ifndef NM_AP_INTERFACE_H_1264772822
#define NM_AP_INTERFACE_H_1264772822

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
class OrgFreedesktopNetworkManagerAPInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManager.AccessPoint"; }

    static inline const char *staticServiceName()
    { return "org.freedesktop.NetworkManager.AccessPoint"; }

    static inline const char *staticPathName()
    { return "/org/freedesktop/NetworkManager/AccessPoint"; }

public:
    OrgFreedesktopNetworkManagerAPInterface(QObject *parent = 0);

    ~OrgFreedesktopNetworkManagerAPInterface();

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
    void PropertiesChanged(QVariantMap changed_properties);
};

namespace org {
  namespace freedesktop {
    typedef ::OrgFreedesktopNetworkManagerAPInterface NetworkManagerAPInterface;
  }
}
#endif
