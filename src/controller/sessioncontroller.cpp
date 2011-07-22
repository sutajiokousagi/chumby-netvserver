/**
  @file
  @author Stefan Frings
  @version $Id: sessioncontroller.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "sessioncontroller.h"
#include "../static.h"
#include <QVariant>
#include <QDateTime>

SessionController::SessionController()
{
}

void SessionController::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

    // Get current session, or create a new one
    HttpSession session=Static::sessionStore->getSession(request,response);
    if (!session.contains("startTime"))
    {
        response.write("<html><body>New session started. Reload this page now.</body></html>");
        session.set("startTime",QDateTime::currentDateTime());
    }
    else
    {
        QDateTime startTime=session.get("startTime").toDateTime();
        response.write("<html><body>Your session started ");
        response.write(startTime.toString().toLatin1());
        response.write("</body></html>");
    }
}
