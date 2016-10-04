#ifndef GETURLFINISHER_H
#define GETURLFINISHER_H

#include <QObject>
#include <QNetworkReply>
#include "nhttpresponse.h"

class GetUrlFinisher : public QObject
{
    Q_OBJECT
public:
    explicit GetUrlFinisher(NHttpResponse *response, QObject *parent = NULL);

private:
    NHttpResponse *response;

signals:

public slots:
    void responseFinished(QNetworkReply *reply);
    void responseFailed(QNetworkReply *reply);
};

#endif // GETURLFINISHER_H
