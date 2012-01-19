#include "httpresponse.h"

HttpResponse::HttpResponse(FCGX_Request *request)
{
    this->request = request;
    sentHeaders = false;
    sentLastPart = false;
}

void HttpResponse::setHeader(QByteArray name, QByteArray value)
{
    if (sentHeaders)
        return;
    responseHeaders.insert(name,value);
}

void HttpResponse::setHeader(QByteArray name, int value)
{
    if (sentHeaders)
        return;
    responseHeaders.insert(name,QByteArray::number(value));
}

/*
 * Read this: http://www.fastcgi.com/docs/faq.html#httpstatus
 */
void HttpResponse::setStatus(int statusCode, QByteArray description)
{
    if (sentHeaders)
        return;
    responseHeaders.insert("Status", QByteArray::number(statusCode) + " " + description);
}

void HttpResponse::writeHeaders()
{
    if (sentHeaders)
        return;

    //Dump all headers field out
    QByteArray buffer;
    foreach(QByteArray name, responseHeaders.keys()) {
        buffer.append(name);
        buffer.append(": ");
        buffer.append(responseHeaders.value(name));
        buffer.append("\r\n");
    }

    //Extra line to indicate end of headers (HTTP protocol)
    buffer.append("\r\n");

    writeToSocket(buffer);
    buffer.clear();

    sentHeaders=true;
}

void HttpResponse::writeToSocket(QByteArray data)
{
    FCGX_PutStr(data.constData(), data.size(), request->out);
}

void HttpResponse::write(QByteArray data, bool lastPart)
{
    Q_ASSERT(sentLastPart==false);

    if (sentHeaders==false)
    {
        QByteArray connectionMode = responseHeaders.value("Connection");
        if (!responseHeaders.contains("Content-Length") && !responseHeaders.contains("Transfer-Encoding") && connectionMode!="close" && connectionMode!="Close")
        {
            if (!lastPart)      responseHeaders.insert("Transfer-Encoding", "chunked");
            else                responseHeaders.insert("Content-Length", QByteArray::number(data.size()));
        }
        writeHeaders();
    }

    bool chunked = responseHeaders.value("Transfer-Encoding")=="chunked" || responseHeaders.value("Transfer-Encoding")=="Chunked";
    if (chunked)
    {
        if (data.size()>0) {
            QByteArray buffer=QByteArray::number(data.size(),16);
            buffer.append("\r\n");
            writeToSocket(buffer);
            writeToSocket(data);
            writeToSocket("\r\n");
        }
    }
    else
    {
        writeToSocket(data);
    }

    if (lastPart)
    {
        if (chunked)
            writeToSocket("0\r\n\r\n");
        sentLastPart=true;
        responseHeaders.clear();
    }
}
