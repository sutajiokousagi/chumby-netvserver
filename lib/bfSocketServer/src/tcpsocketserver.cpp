#include <QTcpSocket>
#include <QFile>
#include <QCoreApplication>
#include "static.h"
#include "tcpsocketserver.h"

TcpSocketServer::TcpSocketServer(QSettings* settings, SocketRequestHandler* requestHandler, QObject *parent)
    : QTcpServer(parent)
{
    Q_ASSERT(settings!=0);
    this->settings = settings;
    this->requestHandler = requestHandler;

    // Start listening
    int port = settings->value("port").toInt();
    listen(QHostAddress::Any, port);
    if (!isListening())
    {
        qCritical("TcpSocketServer: Cannot bind on port %i: %s", port,qPrintable(errorString()));
    }
    else
    {
        qDebug("TcpSocketServer: Listening on port %i", port);
    }
}

TcpSocketServer::~TcpSocketServer()
{
    close();
    qDebug("TcpSocketServer: closed");
}


void TcpSocketServer::incomingConnection(int socketDescriptor)
{
#ifdef SUPERVERBOSE
    qDebug("TcpSocketServer: New connection");
#endif

    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(read()));

    connections.append(socket);

    //Just keep the socket open
    //socket->disconnectFromHost();
}

int TcpSocketServer::broadcast(QByteArray messageText)
{
    int counter = 0;
    foreach(QTcpSocket *socket, connections)
    {
        socket->write(messageText);
        counter++;
    }
    return counter;
}

void TcpSocketServer::read()
{
#ifdef SUPERVERBOSE
    qDebug("TcpSocketServer (%x): read input",(unsigned int) this);
#endif

    QTcpSocket *socket = (QTcpSocket *)QObject::sender();

    QByteArray buffer;
    while (socket->bytesAvailable() > 0)
        buffer.append( socket->read(socket->bytesAvailable()) );

    SocketRequest *request = new SocketRequest(buffer, socket->peerAddress().toString().toLatin1(), socket->peerPort());
    if (request->hasError() || this->requestHandler == NULL)
    {
        buffer = QByteArray();
        delete request;
        return;
    }

    SocketResponse response(socket, socket->peerAddress().toString().toLatin1(), socket->peerPort());
    this->requestHandler->service(*request, response);
    buffer = QByteArray();
    delete request;
}

void TcpSocketServer::disconnected()
{
#ifdef SUPERVERBOSE
    qDebug("TcpSocketServer (%x): disconnected",(unsigned int) this);
#endif

    QTcpSocket *socket = (QTcpSocket *)QObject::sender();

    connections.removeOne(socket);
    socket->close();
    socket->deleteLater();
}
