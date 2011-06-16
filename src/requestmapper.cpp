/**
  @file
  @author Stefan Frings
  @version $Id: requestmapper.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "requestmapper.h"
#include "static.h"
#include "controller/dumpcontroller.h"
#include "controller/templatecontroller.h"
#include "controller/formcontroller.h"
#include "controller/fileuploadcontroller.h"
#include "controller/framebuffercontroller.h"
#include "controller/sessioncontroller.h"
#include "controller/staticfilecontroller.h"
#include "controller/bridgecontroller.h"
#include "controller/scriptcontroller.h"


#if defined CURSOR_CONTROLLER
    #include "controller/cursorcontroller.h"
#endif

//Constructor
RequestMapper::RequestMapper(QObject* parent) : HttpRequestHandler(parent), SocketRequestHandler(parent)
{

}

//Dispatch different type of HTTP request to different controller
void RequestMapper::service(HttpRequest& request, HttpResponse& response)
{
    QByteArray path = request.getPath();
    qDebug("RequestMapper: path=%s", path.data());

    if (path.startsWith("/dump")) {
        DumpController().service(request, response);
    }

    else if (path.startsWith("/template")) {
        TemplateController().service(request, response);
    }

    else if (path.startsWith("/form")) {
        FormController().service(request, response);
    }

    else if (path.startsWith("/file")) {
        FileUploadController().service(request, response);
    }

    else if (path.startsWith("/session")) {
        SessionController().service(request, response);
    }

    else if (path.startsWith("/framebuffer")) {
        FramebufferController().service(request, response);
    }

    else if (path.startsWith("/scripts/")) {
        Static::scriptController->service(request, response);
    }

    else if (path.startsWith("/bridge")) {
        Static::bridgeController->service(request, response);
    }

//Only available on Linux system
#if defined (CURSOR_CONTROLLER)
    else if (path.startsWith("/cursor")) {
        Static::cursorController->service(request, response);
    }
#endif

    // All other pathes are mapped to the static file controller
    else {
        Static::staticFileController->service(request, response);
    }
}

//Dispatch different type of Socket request to different controller
void RequestMapper::service(SocketRequest& request, SocketResponse& response)
{
    if (request.hasError())
        return;

    //We only have the bridge controller as SocketHandler at the moment
    Static::bridgeController->service(request, response);
}
