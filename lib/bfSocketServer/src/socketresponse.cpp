#include "socketresponse.h"
#include <QXmlStreamWriter>
#include <QBuffer>

SocketResponse::SocketResponse(QAbstractSocket* socket)
{
    this->socket=socket;
    this->statusText = "0";     //0:unimplemented, 1:success, 2:general error
}

SocketResponse::~SocketResponse()
{
    this->socket = NULL;
    this->statusText = "";
    this->parameters.clear();
}

void SocketResponse::setStatus(QByteArray commandName)
{
    this->statusText = commandName;
}

void SocketResponse::setStatus(int statusCode)
{
    this->statusText = QByteArray::number(statusCode);
}

void SocketResponse::setParameter(QByteArray paramName, int paramValue)
{
    setParameter(paramName, QByteArray::number(paramValue));
}

void SocketResponse::setParameter(QByteArray paramName, float paramValue)
{
    setParameter(paramName, QByteArray::number(paramValue));
}

void SocketResponse::setParameter(QByteArray paramName, QByteArray paramValue)
{
    parameters.insert(paramName, paramValue);
}

QMap<QByteArray,QByteArray>& SocketResponse::getParameters()
{
    return parameters;
}

int SocketResponse::getParametersCount()
{
    return parameters.count();
}

void SocketResponse::write()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    //Output format follows this https://internal.chumby.com/wiki/index.php/JavaScript/HTML_-_Hardware_Bridge_protocol

    QXmlStreamWriter *xmlfile = new QXmlStreamWriter();
    xmlfile->setDevice(&buffer);

    xmlfile->writeTextElement("status", statusText);

    if (parameters.count() == 1)
    {
        xmlfile->writeStartElement( "data" );
        QMapIterator<QByteArray, QByteArray> i(parameters);
        while (i.hasNext())
        {
             i.next();
             xmlfile->writeTextElement("value", i.value());
        }
        xmlfile->writeEndElement();
    }
    else if (parameters.count() > 1)
    {
        xmlfile->writeStartElement( "data" );
        QMapIterator<QByteArray, QByteArray> i(parameters);
        while (i.hasNext())
        {
             i.next();
             xmlfile->writeTextElement(i.key(), i.value());
        }
        xmlfile->writeEndElement();
    }

    delete xmlfile;

    buffer.close();
    writeToSocket(byteArray);

    byteArray.clear();
}

void SocketResponse::writeToSocket(QByteArray data)
{
    int remaining = data.size();
    char* ptr=data.data();
    while (socket->isOpen() && remaining>0)
    {
        int written = socket->write(data);
        ptr += written;
        remaining -= written;
    }
}
