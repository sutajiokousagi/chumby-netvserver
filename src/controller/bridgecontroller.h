#ifndef BridgeController_H
#define BridgeController_H

#include "httprequesthandler.h"
#include "socketrequesthandler.h"

class BridgeController : public HttpRequestHandler, public SocketRequestHandler
{
    //Q_OBJECT
    //Q_DISABLE_COPY(BridgeController);
public:

    /** Constructor */
    BridgeController(QSettings* settings, QObject* parent = 0);

    /** Receive & response to HTTP requests */
    void service(HttpRequest& request, HttpResponse& response);

    /** Receive & response to socket requests */
    void service(SocketRequest& request, SocketResponse& response);

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

    /** File Utilities */
    bool FileExists(const QString &fullPath);
    qint64  GetFileSize(const QString &fullPath);
    QByteArray GetFileContents(const QString &fullPath);
    bool SetFileContents(const QString &fullPath, QByteArray data);
    bool UnlinkFile(const QString &fullPath);
};

#endif // BridgeController_H
