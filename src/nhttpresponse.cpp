#include "nhttpresponse.h"
#include "qhttpserverresponse.hpp"

NHttpResponse::NHttpResponse(qhttp::server::QHttpResponse *response, QObject *parent)
    : QObject(parent)
{
    this->response = response;
    response->addHeader("Access-Control-Allow-Origin", "*");
}

NHttpResponse::~NHttpResponse() {
    QHTTP_LINE_LOG
}

void NHttpResponse::setStatusCode(qhttp::TStatusCode code)
{
    response->setStatusCode(code);
}

void NHttpResponse::setVersion(const QString& versionString)
{
    response->setVersion(versionString);
}

void NHttpResponse::addHeader(const QByteArray& field, const QByteArray& value)
{
    response->addHeader(field, value);
}

qhttp::THeaderHash& NHttpResponse::headers(void)
{
    return response->headers();
}

void NHttpResponse::write(const QByteArray &data)
{
    response->write(data);
}

void NHttpResponse::write(const QString &data)
{
    response->write(data.toUtf8());
}

void NHttpResponse::end(const QByteArray &data)
{
    response->end(data);
}

void NHttpResponse::standardResponse(const QString &command, int result, QString data)
{
    /*
    write(QString("<xml>\n"
                  "    <status>%1</status>\n"
                  "    <cmd>%2</cmd>\n"
                  "    <data>\n"
                  "        <value>%3</value>\n"
                  "    </data>\n"
                  "</xml>\n").arg(result).arg(command).arg(data));
    */
    // Note: the client appears to parse XML with split(), so whitespace matters.
    write(QString("<xml>"
                  "<status>%1</status>"
                  "<cmd>%2</cmd>"
                  "<data>"
                  "<value>%3</value>"
                  "</data>"
                  "</xml>\n").arg(result).arg(command).arg(data));
}

/** returns the parent QHttpConnection object. */
qhttp::server::QHttpConnection* NHttpResponse::connection() const
{
    return response->connection();
}
