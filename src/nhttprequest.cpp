#include <QUrlQuery>
#include <QString>

#include "nhttprequest.h"
#include "qhttpserverrequest.hpp"

NHttpRequest::NHttpRequest(const qhttp::server::QHttpRequest * qRequest, QObject *parent)
    : QObject(parent), _url(qRequest->url()),
      _headers(qRequest->headers()), _query(_url),
      _postData(qRequest->collectedData())
{
    // Copy over the querystring values
    QList<QPair<QString, QString>>items = _query.queryItems();
    QPair<QString, QString> pair;
    foreach (pair, items)
        _parameters[pair.first] = pair.second;

    // If it's a POST operation, add in the values from the post data
    if (qRequest->method() == qhttp::EHTTP_POST) {
        // Since the POST data is, more or less, a querystring, use
        // QUrlQuery to parse it.
        QUrlQuery postQuery(_postData);
        QList<QPair<QString, QString>>items = postQuery.queryItems();
        foreach (pair, items)
            _parameters[pair.first] = pair.second;
    }
}

const QUrl NHttpRequest::url(void) const
{
    return _url;
}

const QUrlQuery NHttpRequest::urlQuery(void) const
{
    return _query;
}

const QString NHttpRequest::parameter(const QString & key) const {

    return _parameters.value(key);
}
