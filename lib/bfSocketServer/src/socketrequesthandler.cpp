#include "socketrequesthandler.h"

SocketRequestHandler::SocketRequestHandler(QObject* parent)
    : QObject(parent)
{
}

SocketRequestHandler::~SocketRequestHandler()
{
}

void SocketRequestHandler::service(SocketRequest& request, SocketResponse& response)
{
    qCritical("SocketRequestHandler: you need to override the dispatch() function");
    qDebug("SocketRequestHandler: request=%s",request.getCommand() );
}
