#include <QTcpSocket>
#include <QFile>
#include <QCoreApplication>
#include "flashpolicyserver.h"

FlashPolicyServer::FlashPolicyServer(QSettings* settings, QObject *parent)
    : QTcpServer(parent)
{
    Q_ASSERT(settings!=0);
    this->settings=settings;

    // Start listening
    listen(QHostAddress::Any, FLASH_POLICY_SERVER_PORT);
    if (!isListening())
    {
        qCritical("FlashPolicyServer: Cannot bind on port %i: %s", FLASH_POLICY_SERVER_PORT,qPrintable(errorString()));
    }
    else
    {
        qDebug("FlashPolicyServer: Listening on port %i", FLASH_POLICY_SERVER_PORT);
    }
}

FlashPolicyServer::~FlashPolicyServer()
{
    close();
    qDebug("FlashPolicyServer: closed");
}


void FlashPolicyServer::incomingConnection(int socketDescriptor)
{
#ifdef SUPERVERBOSE
    qDebug("FlashPolicyServer: New connection");
#endif

    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    //The client should be yelling "<policy-file-request/>\0" right here
    QByteArray newData = socket->read(25).trimmed();

    if (!newData.contains(FLASH_POLICY_REQUEST_STRING))
    {
        socket->write("HTTP/1.1 503 invalid request\r\n\r\nInvalid request\r\n");
        socket->disconnectFromHost();
        return;
    }


    QFile file( settings->value("flashPolicyFile").toString() );
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
    {
        // Spit out the allow-all default policy if we have problem with the file
        socket->write(FLASH_POLICY_DEFAULT);
    }
    else
    {
        // Spit out the flash policy file content
        while (!file.atEnd() && !file.error())
           socket->write(file.read(65536));
        file.close();
    }
    socket->disconnectFromHost();
}
