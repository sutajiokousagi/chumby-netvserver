/**
  @file
  @author Stefan Frings
  @version $Id: staticfilecontroller.h 938 2010-12-05 14:29:58Z stefan $
*/

#ifndef BridgeController_H
#define BridgeController_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class BridgeController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(BridgeController);
public:

    /** Constructor */
    BridgeController(QSettings* settings, QObject* parent = 0);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:

    /** Encoding of text files */
    QString encoding;

    /** Root directory of documents */
    QString docroot;

    /** Process Utilities */
    QByteArray Execute(const QString &fullPath, QStringList args);

    /** File Utilities */
    bool FileExists(const QString &fullPath);
    qint64  GetFileSize(const QString &fullPath);
    QByteArray GetFileContents(const QString &fullPath);
    bool SetFileContents(const QString &fullPath, QByteArray data);
    bool UnlinkFile(const QString &fullPath);
};

#endif // BridgeController_H
