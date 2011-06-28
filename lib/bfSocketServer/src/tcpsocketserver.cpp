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
    connect(socket, SIGNAL(readyRead()), this, SLOT(read()));

    connections.append(socket);
    connectionsType.insert(socket, "");

    //Just keep the socket open
    //socket->disconnectFromHost();
}

int TcpSocketServer::broadcast(QByteArray messageText, QByteArray type /* = "" */)
{
    int counter = 0;
    foreach(QTcpSocket *socket, connections)
    {
        if (connectionsType.value(socket) != type)
            continue;

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

    //Preprocessing to get connection type
    if (request->getCommand().toUpper() == "HELLO")
    {
        QByteArray hardwareType = request->getParameter("value");
        if (hardwareType == "")
            hardwareType = request->getParameter("type");
        connectionsType.insert(socket, hardwareType);
    }

    //Post processing by higher level classes
    SocketResponse response(socket, socket->peerAddress().toString().toLatin1(), socket->peerPort());
    this->requestHandler->service(*request, response);

    //Clean up
    buffer = QByteArray();
    delete request;
}

void TcpSocketServer::disconnected()
{
#ifdef SUPERVERBOSE
    qDebug("TcpSocketServer (%x): disconnected",(unsigned int) this);
#endif

    QTcpSocket *socket = (QTcpSocket *)QObject::sender();

    connectionsType.remove(socket);
    connections.removeOne(socket);
    socket->close();
    socket->deleteLater();
}
