/**
  @file
  @author Stefan Frings
  @version $Id: fileuploadcontroller.h 938 2010-12-05 14:29:58Z stefan $
*/

#ifndef FRAMEBUFFERCONTROLLER_H
#define FRAMEBUFFERCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller displays a image of the current framebuffer.
*/


class FramebufferController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FramebufferController);
public:

    /** Constructor */
    FramebufferController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // FRAMEBUFFERCONTROLLER_H
