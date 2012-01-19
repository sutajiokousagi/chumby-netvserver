#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "socketrequesthandler.h"

/**
  The request mapper dispatches incoming HTTP/Socket requests to controller classes
  depending on the requested path.
*/

class RequestMapper : public SocketRequestHandler
{

public:

    /**
      Constructor.
    */
    RequestMapper();

    /**
      Dispatch a request to a controller.
      @param request The received socket request
      @param response Must be used to return the response
    */
    void service(SocketRequest& request, SocketResponse& response);
};

#endif // REQUESTMAPPER_H
