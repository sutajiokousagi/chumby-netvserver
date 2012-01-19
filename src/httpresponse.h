#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QMap>
#include <QString>
#include <fcgiapp.h>

class HttpResponse
{
    Q_DISABLE_COPY(HttpResponse)
public:

    /**
      Constructor.
      @param socket used to write the response
    */
    HttpResponse(FCGX_Request *request);

    /**
      Set a HTTP response header
      @param name name of the header
      @param value value of the header
    */
    void setHeader(QByteArray name, QByteArray value);

    /**
      Set a HTTP response header
      @param name name of the header
      @param value value of the header
    */
    void setHeader(QByteArray name, int value);

    /**
      Set status code and description. The default is 200,OK.
    */
    void setStatus(int statusCode, QByteArray description=QByteArray());

    /**
      Write body data to the socket.
      <p>
      If the response contains only a single chunk (indicated by lastPart=true),
      the response is transferred in traditional mode with a Content-Length
      header, which is automatically added if not already set before.
      <p>
      Otherwise, each part is transferred in chunked mode.
      @param data Data bytes of the body
      @param lastPart Indicator, if this is the last part of the response.
    */
    void write(QByteArray data, bool lastPart=false);

private:

    /** FastCGI request for writing output */
    FCGX_Request* request;

    /** Request headers */
    QMap<QByteArray,QByteArray> responseHeaders;

    /** Indicator whether headers have been sent */
    bool sentHeaders;

    /** Indicator whether the body has been sent completely */
    bool sentLastPart;

    /** Write raw data to the socket (via FastCGI API) */
    void writeToSocket(QByteArray data);

    /**
      Write the response HTTP status and headers to the socket.
      Calling this method is optional, because writeBody() calls
      it automatically when required.
    */
    void writeHeaders();

};

#endif // HTTPRESPONSE_H
