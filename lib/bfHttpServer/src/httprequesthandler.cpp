/**
  @file
  @author Stefan Frings
  @version $Id: httprequesthandler.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "httprequesthandler.h"

HttpRequestHandler::HttpRequestHandler(QObject* parent)
    : QObject(parent)
{}

HttpRequestHandler::~HttpRequestHandler() {}

void HttpRequestHandler::service(HttpRequest& request, HttpResponse& response)
{
    qCritical("HttpRequestHandler: you need to override the dispatch() function");
    qDebug("HttpRequestHandler: request=%s %s %s",request.getMethod().data(),request.getPath().data(),request.getVersion().data());
    response.setStatus(501,"not implemented");
    response.write("501 not implemented",true);
}
