
#ifndef SocketRequest_H
#define SocketRequest_H

#include <QMap>
#include <QByteArray>

/**
  This is the read-only version of SocketResponse
*/

class SocketRequest
{
    Q_DISABLE_COPY(SocketRequest)
public:

    /**
      Constructor.
      @param socket used to write the response
    */
    SocketRequest(QByteArray data, QByteArray address);
    ~SocketRequest();

    bool hasError();

    QByteArray getCommand();
    QByteArray getAddress();

    QMap<QByteArray,QByteArray>& getParameters();
    QByteArray getParameter(QByteArray paramName);
    int getParametersCount();

private:

    bool containsError;

    QByteArray commandText;
    QByteArray address;

    QMap<QByteArray,QByteArray> parameters;

    void ParseMessageXML(const char* data);
};

#endif // SocketRequest_H
