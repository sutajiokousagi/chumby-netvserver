#include <QTcpSocket>
#include <QFile>
#include <QCoreApplication>
#include "xmlsocketserver.h"

XmlSocketServer::XmlSocketServer(QSettings* settings, QObject *parent)
    : QTcpServer(parent)
{
    Q_ASSERT(settings!=0);
    this->settings=settings;

    // Start listening
    int port = settings->value("port").toInt();
    listen(QHostAddress::Any, port);
    if (!isListening())
    {
        qCritical("XmlSocketServer: Cannot bind on port %i: %s", port,qPrintable(errorString()));
    }
    else
    {
        qDebug("XmlSocketServer: Listening on port %i", port);
    }
}

XmlSocketServer::~XmlSocketServer()
{
    close();
    qDebug("XmlSocketServer: closed");
}


void XmlSocketServer::incomingConnection(int socketDescriptor)
{
#ifdef SUPERVERBOSE
    qDebug("XmlSocketServer: New connection");
#endif

    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    //This is just dumb socket right now
    socket->write("<xml><cmd>play_widget</cmd><data>http://s3movies.chumby.com/cdn/xmlmovie/F3A86472-05F6-11DF-A1F8-001B24E044BE</data></xml>\n\n");

    //Just keep the socket open
    //socket->disconnectFromHost();
}
