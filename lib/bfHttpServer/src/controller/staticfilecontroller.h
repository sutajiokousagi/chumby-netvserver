/**
  @file
  @author Stefan Frings
  @version $Id: staticfilecontroller.h 938 2010-12-05 14:29:58Z stefan $
*/

#ifndef STATICFILECONTROLLER_H
#define STATICFILECONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"
#include <QCache>

/**
  The static file controller delivers static files.
  It is usually called by the application-specific request mapper.
  <p>
  The following settings are required:
  <code><pre>
  path=docroot
  encoding=UTF-8
  maxAge=60000
  cacheTime=60000
  cacheSize=1000000
  maxCachedFileSize=65536
  </pre></code>
  The path is relative to the directory of the config file. In case of windows, if the
  settings are in the registry, the path is relative to the current working directory.
  <p>
  The encoding is sent to the web browser in case of text and html files.
  <p>
  The cache improves performance of small files when loaded from a network
  drive. Large files are not cached. Files are cached as long as possible,
  when cacheTime=0. The maxAge value (in msec!) controls the remote browsers cache.
*/

class StaticFileController : public HttpRequestHandler
{

public:

    /** Constructor */
    StaticFileController(QSettings* settings);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:

    /** Encoding of text files */
    QString encoding;

    /** Root directory of documents */
    QString docroot;

    /** Maximum age of files in the browser cache */
    int maxAge;    

    struct CacheEntry {
        QByteArray document;
        qint64 created;
    };

    /** Timeout for each cached file */
    int cacheTimeout;

    /** Maximum size of files in cache, larger files are not cached */
    int maxCachedFileSize;

    /** Cache storage */
    QCache<QString,CacheEntry> cache;

    /** Set a content-type header in the response depending on the ending of the filename */
    void setContentType(QString file, HttpResponse& response) const;
};

#endif // STATICFILECONTROLLER_H
