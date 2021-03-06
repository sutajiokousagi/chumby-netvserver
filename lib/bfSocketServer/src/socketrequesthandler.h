#ifndef SocketRequestHandler_H
#define SocketRequestHandler_H

#include "socketrequest.h"
#include "socketresponse.h"

class SocketRequestHandler
{

public:

    /** Constructor */
    SocketRequestHandler();

    /** Destructor */
    virtual ~SocketRequestHandler();

    virtual void service(SocketRequest& request, SocketResponse& response);

};

#endif // SocketRequestHandler_H
