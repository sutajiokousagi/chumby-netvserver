#ifndef ScriptController_H
#define ScriptController_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ScriptController : public HttpRequestHandler
{

public:

    /** Constructor */
    ScriptController(QSettings* settings);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:

    /** Encoding of text files */
    QString encoding;

    /** Root directory of documents */
    QString docroot;
};

#endif // ScriptController_H
