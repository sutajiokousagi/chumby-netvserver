#ifndef BridgeController_H
#define BridgeController_H

#include <QWSServer>
#include "httprequesthandler.h"
#include "socketrequesthandler.h"

class BridgeController : public QObject, public HttpRequestHandler, public SocketRequestHandler, public QWSServer::KeyboardFilter
{
    Q_OBJECT
    //Q_DISABLE_COPY(BridgeController);
public:

    /** Constructor */
    BridgeController(QSettings* settings, QObject* parent = 0);

    /** Receive & response to HTTP requests */
    void service(HttpRequest& request, HttpResponse& response);

    /** Receive & response to socket requests */
    void service(SocketRequest& request, SocketResponse& response);

public slots:

    // From dbusmonitor.h
    void slot_StateChanged(uint state);
    void slot_PropertiesChanged(QByteArray prop_name, QByteArray prop_value);
    void slot_DeviceAdded(QByteArray objPath);
    void slot_DeviceRemoved(QByteArray objPath);

private:

    /** Long polling HTTP response */
    QList<HttpResponse*> longPollResponses;

    /** Encoding of text files */
    QString encoding;

    /** Root directory of documents */
    QString docroot;

    /** Process Utilities */
    QByteArray Execute(const QString &fullPath);
    QByteArray Execute(const QString &fullPath, QStringList args);

    /** Helper */
    bool SetNetworkConfig(QHash<QString, QString> parameters);

    /** File Utilities */
    bool FileExists(const QString &fullPath);
    bool FileExecutable(const QString &fullPath);
    qint64  GetFileSize(const QString &fullPath);
    QByteArray GetFileContents(const QString &fullPath);
    bool SetFileContents(const QString &fullPath, QByteArray data);
    bool SetFileExecutable(const QString &fullPath);
    bool UnlinkFile(const QString &fullPath);

    /** Custom keyboard filter */
    bool filter( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat );
};

#endif // BridgeController_H
