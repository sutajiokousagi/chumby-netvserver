#ifndef STATICFILECONTROLLER_H
#define STATICFILECONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#define HOMEPAGE_PAGE_FILE      "/psp/homepage"

class StaticFileController : public HttpRequestHandler
{

public:

    /** Constructor */
    StaticFileController(QSettings* settings);

    /** Handle the request */
    void service(HttpRequest& request, HttpResponse& response);

    /** Set a content-type header in the response depending on the ending of the filename */
    static void SetContentType(QString file, HttpResponse& response);

    /** Dynamic docroot */
    QString readPspHomepage();
    QString setDocroot(QString newPath);
    QString getDocroot();

    /** File Utilities */
    bool DirExists(const QString &fullPath);
    bool FileExists(const QString &fullPath);
    bool FileExecutable(const QString &fullPath);
    qint64  GetFileSize(const QString &fullPath);
    QByteArray GetFileContents(const QString &fullPath);

private:

    /** Root directory for life support stuff */
    QString docroot;

    /** Root directory for high level UI */
    QString docrootDynamic;
};

#endif // STATICFILECONTROLLER_H
