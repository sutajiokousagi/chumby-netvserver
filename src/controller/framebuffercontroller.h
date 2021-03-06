#ifndef FRAMEBUFFERCONTROLLER_H
#define FRAMEBUFFERCONTROLLER_H

#include "httprequesthandler.h"

/**
  This controller displays a image of the current framebuffer.
*/


class FramebufferController : public HttpRequestHandler
{

public:

    /** Constructor */
    FramebufferController(QSettings* settings);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:

    /** Default size of output image */
    int width, height;

    /** Default quality of ouput image */
    int quality;

    /** Default format of ouput image */
    QByteArray format;

    /** Default smooth/fast transformation used in scaling */
    bool smooth;
};

#endif // FRAMEBUFFERCONTROLLER_H
