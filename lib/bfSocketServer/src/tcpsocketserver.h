#ifndef XMLLISTENER_H
#define XMLLISTENER_H

#include <QList>
#include <QTcpServer>
#include <QSettings>
#include <QMap>
#include "socketrequesthandler.h"

/**
  Listens for incoming TCP connections by Flash and communicate in XML format.
*/

class TcpSocketServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(TcpSocketServer)
public:

    /** Constructor */
    TcpSocketServer(QSettings* settings, SocketRequestHandler* requestHandler, QObject* parent = 0);

    /** Destructor */
    virtual ~TcpSocketServer();

    /** Broadcast a message to all connected clients */
    int broadcast(QByteArray messageText, QByteArray type = "");

protected:

    /** Serves new incoming connection requests */
    void incomingConnection(int socketDescriptor);

private:

    /** Configuration settings for the server */
    QSettings* settings;

    /** Request handler for the server */
    SocketRequestHandler* requestHandler;

    /** Pool of socket connections */
    QList<QTcpSocket*> connections;
    QMap<QTcpSocket*, QByteArray> connectionsType;

private slots:

    /** Received from the socket when incoming data can be read */
    void read();

    /** Received from the socket connection is disconnected */
    void disconnected();
};

#endif // XMLLISTENER_H
