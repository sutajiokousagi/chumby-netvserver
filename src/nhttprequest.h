#ifndef NHTTPREQUEST_H
#define NHTTPREQUEST_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#include "qhttpserverrequest.hpp"

class NHttpRequest : public QObject
{
    Q_OBJECT
public:
    explicit NHttpRequest(const qhttp::server::QHttpRequest * request, QObject *parent = 0);
    const QString parameter(const QString & key) const;
    const QUrl url(void) const;
    const QUrlQuery urlQuery(void) const;
signals:

public slots:

private:
    QHash<QString, QString> _parameters;
    const QUrl _url;
    const QUrlQuery _query;
    const QByteArray _postData;
    const QHash<QByteArray, QByteArray> _headers;
};

#endif // NHTTPREQUEST_H
