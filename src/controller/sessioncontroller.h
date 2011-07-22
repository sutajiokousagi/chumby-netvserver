/**
  @file
  @author Stefan Frings
  @version $Id: sessioncontroller.h 938 2010-12-05 14:29:58Z stefan $
*/

#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include "httprequesthandler.h"

/**
  This controller demonstrates how to use sessions.
*/

class SessionController : public HttpRequestHandler
{

public:

    /** Constructor */
    SessionController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // SESSIONCONTROLLER_H
