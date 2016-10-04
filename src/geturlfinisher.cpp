#include "geturlfinisher.h"
#include "netvserverapplication.h"

GetUrlFinisher::GetUrlFinisher(const QString &cmd, NHttpResponse *response, QObject *parent)
    : cmd(cmd), QObject(parent)
{
    this->response = response;
}

void GetUrlFinisher::responseFinished(QNetworkReply *reply)
{
    response->standardResponse(cmd, NETV_STATUS_SUCCESS, reply->readAll());
    response->end();
    reply->deleteLater();
}

void GetUrlFinisher::responseFailed(QNetworkReply *reply)
{
    response->standardResponse(cmd, NETV_STATUS_ERROR, reply->readAll());
    response->end();
    reply->deleteLater();
}
