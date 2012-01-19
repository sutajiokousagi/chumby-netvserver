#include "requestmapper.h"
#include "static.h"

//Constructor
RequestMapper::RequestMapper() : SocketRequestHandler()
{

}

//Dispatch different type of Socket request to different controller
void RequestMapper::service(SocketRequest& request, SocketResponse& response)
{
    if (request.hasError())
        return;

    //We only have the bridge controller as SocketHandler at the moment
    Static::bridgeController->service(request, response);
}
