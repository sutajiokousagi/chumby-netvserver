/**
  @file
  @author Stefan Frings
  @version $Id: staticfilecontroller.h 938 2010-12-05 14:29:58Z stefan $
*/

#ifndef ScriptController_H
#define ScriptController_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ScriptController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptController);
public:

    /** Constructor */
    ScriptController(QSettings* settings, QObject* parent = 0);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:

    /** Encoding of text files */
    QString encoding;

    /** Root directory of documents */
    QString docroot;

    /** Set a content-type header in the response depending on the ending of the filename */
    void setContentType(QString file, HttpResponse& response) const;
};

#endif // ScriptController_H
