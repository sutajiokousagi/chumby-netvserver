#ifndef NETVSERVERAPPLICATION_H
#define NETVSERVERAPPLICATION_H

#include <QCoreApplication>
#include <QHash>
#include <QString>
#include <QDir>
#include <QSettings>
#include <QList>
#include <QTimer>

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"

#define BRIDGE_RETURN_STATUS_UNIMPLEMENTED  "0"
#define BRIDGE_RETURN_STATUS_SUCCESS        "1"
#define BRIDGE_RETURN_STATUS_ERROR          "2"
#define BRIDGE_RETURN_STATUS_UNAUTHORIZED   "3"

#define NETV_STATUS_UNIMPLEMENTED  0
#define NETV_STATUS_SUCCESS        1
#define NETV_STATUS_ERROR          2
#define NETV_STATUS_UNAUTHORIZED   3

class NeTVServerApplication;
class NHttpRequest;
class NHttpResponse;

#define DECLARE_NETV_HANDLER(name) \
    int (name)(const QString &, NeTVServerApplication *, \
               const NHttpRequest *, NHttpResponse *)

#define BEGIN_NETV_HANDLER(name, cmd, app, req, res) \
    int (name)(const QString & (cmd), NeTVServerApplication * (app), \
               const NHttpRequest * (request), NHttpResponse * (response)) { \
    Q_UNUSED(cmd); \
    Q_UNUSED(app); \
    Q_UNUSED(request);
#define END_NETV_HANDLER() return 0; }


// Callback function that handles a particular request
typedef int (*handleBridgeCallbackType)(const QString & cmd, NeTVServerApplication * app,
                                        const NHttpRequest * request, NHttpResponse * response);

class NeTVServerApplication : public QCoreApplication
{
    Q_OBJECT
public:
    NeTVServerApplication(int & argc, char **argv);
    int registerBridgeFunction(const QString & cmd, handleBridgeCallbackType callback);

    const QDir & setStaticDocRoot(const QString & newRoot);
    const QDir & staticDocRoot(void);
    const QDir widgetRoot(void);
    const QVariant setting(const QString &key, QVariant defaultValue = QVariant());
    void setSetting(const QString &key, const QVariant &value);
    handleBridgeCallbackType callback(const QString &cmd);

    void sendBroadcast(const QString &str);
    void handleBroadcast(NHttpResponse *response);

private:
    qhttp::server::QHttpServer                  *server;
    QHash<QString, handleBridgeCallbackType>    callbacks;
    QDir                                        docRoot;
    QSettings                                   settings;
    QList<QString>                              broadcastQueue;
    NHttpResponse                              *waitingResponse;
    QTimer                                      broadcastTimeout;

public slots:
    void broadcastTimedOut(void);
};

#endif // NETVSERVERAPPLICATION_H
