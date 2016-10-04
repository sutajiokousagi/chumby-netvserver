#include "geturlfinisher.h"

GetUrlFinisher::GetUrlFinisher(NHttpResponse *response, QObject *parent)
    : QObject(parent)
{
    this->response = response;
}

void GetUrlFinisher::responseFinished(QNetworkReply *reply)
{
    response->end(reply->readAll());
    reply->deleteLater();
}

void GetUrlFinisher::responseFailed(QNetworkReply *reply)
{
    response->end(reply->readAll());
    reply->deleteLater();
}
