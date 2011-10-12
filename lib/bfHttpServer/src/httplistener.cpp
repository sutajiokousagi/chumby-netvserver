#include "httplistener.h"
#include "httpconnectionhandler.h"
#include "httpconnectionhandlerpool.h"
#include <QCoreApplication>

HttpListener::HttpListener(QSettings* settings, HttpRequestHandler* requestHandler, QObject *parent)
    : QTcpServer(parent), pool(settings,requestHandler)
{
    Q_ASSERT(settings!=0);
    this->settings = settings;

    // Start listening
    int port = settings->value("port").toInt();
    listen(QHostAddress::Any, port);

    // Socket is setup ok
    if (!isListening())
        qCritical("HttpListener: Cannot bind on port %i: %s", port, qPrintable(errorString()));
}

HttpListener::~HttpListener()
{
    close();
}

void HttpListener::incomingConnection(int socketDescriptor)
{
    HttpConnectionHandler* freeHandler = pool.getConnectionHandler();

    // Server is too busy, reject the connection
    if (freeHandler == NULL)
    {
        qDebug("HttpListener: Too many connections");
        QTcpSocket* socket=new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        socket->write("HTTP/1.1 503 too many connections\r\nConnection: close\r\n\r\nToo many connections\r\n");
        socket->disconnectFromHost();
        return;
    }

    // Let the handler process the new connection.
    // The descriptor is passed via signal/slot because the handler lives in another
    // thread and cannot open the socket when called by another thread.
    connect(this,SIGNAL(handleConnection(int)),freeHandler,SLOT(handleConnection(int)));
    emit handleConnection(socketDescriptor);
    disconnect(this,SIGNAL(handleConnection(int)),freeHandler,SLOT(handleConnection(int)));
    return;
}
