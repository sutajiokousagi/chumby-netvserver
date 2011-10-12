#ifndef STATICFILECONTROLLER_H
#define STATICFILECONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  The static file controller delivers static files.
  It is usually called by the application-specific request mapper.
  <p>
  The following settings are required:
  <code><pre>
  path=docroot
  encoding=UTF-8
  </pre></code>
  The path is relative to the directory of the config file. In case of windows, if the
  settings are in the registry, the path is relative to the current working directory.
  <p>
  The encoding is sent to the web browser in case of text and html files.
*/

class StaticFileController : public HttpRequestHandler
{

public:

    /** Constructor */
    StaticFileController(QSettings* settings);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

    /** Set a content-type header in the response depending on the ending of the filename */
    static void SetContentType(QString file, HttpResponse& response);

private:

    /** Encoding of text files */
    QString encoding;

    /** Root directory of documents */
    QString docroot;
};

#endif // STATICFILECONTROLLER_H
