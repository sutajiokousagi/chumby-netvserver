#include <QFile>
#include <QCoreApplication>
#include <QUdpSocket>
#include <QStringList>
#include <QThreadPool>
#include <QHostInfo>
#include <QBuffer>
#include <QXmlStreamWriter>
#include "static.h"
#include "udpsocketserver.h"

UdpSocketServer::UdpSocketServer(QSettings* settings, SocketRequestHandler* requestHandler, QObject *parent)
    : QThread(parent)
{
    Q_ASSERT(settings!=0);
    this->settings = settings;
    this->requestHandler = requestHandler;

    bStop = false;
    bRunning = false;
    mPort = settings->value("port").toInt();

    //Create the socket
    pSocket = new QUdpSocket(this);
    bool isBound = pSocket->bind(mPort);

    //Start listening if everything is ok
    if (isBound)
    {
        qDebug("UdpSocketServer: Listening on port %i", mPort);
        start();
        return;
    }

    //Clean up when it failed
    qDebug("UdpSocketServer: failed to bind to port %i", mPort);
    delete pSocket;
    pSocket = NULL;
}

UdpSocketServer::~UdpSocketServer()
{
    terminate();

    while(isRunning())
        msleep(40);

    qDebug("UdpSocketServer: closed");
}


//----------------------------------------------------------------------------------
// Start/Stop thread
//----------------------------------------------------------------------------------

void UdpSocketServer::start()
{
    if (pSocket != NULL)
        QThread::start(QThread::LowestPriority);
}

void UdpSocketServer::terminate()
{
    if (mutex.tryLock())
    {
        bStop = true;
        mutex.unlock();
    }
}

void UdpSocketServer::msleep(unsigned long msecs)
{
    QThread::msleep(msecs);
}

bool UdpSocketServer::isRunning()
{
    bool isRunning = true;
    if (mutex.tryLock())
    {
        isRunning = bRunning;
        mutex.unlock();
    }
    return isRunning;
}

bool UdpSocketServer::isStopping()
{
    bool isStopping = false;
    if (mutex.tryLock())
    {
        isStopping = bStop;
        mutex.unlock();
    }
    return isStopping;
}

//----------------------------------------------------------------------------------
// Main loop
//----------------------------------------------------------------------------------

void UdpSocketServer::run()
{
    if (!mutex.tryLock())
        return;
    if (bRunning)
    {
        //Some how the thread is already started, we quit here
        mutex.unlock();
        return;
    }

    //No socket to run on
    if (pSocket == NULL)
        return;

    bStop = false;
    bRunning = true;
    mutex.unlock();

    //Get own IP addresses
    QList <QHostAddress> list = QHostInfo::fromName(QHostInfo::localHostName()).addresses();

    while(!bStop)
    {
        msleep(SERVICE_INTERVAL);

        while(pSocket->hasPendingDatagrams())
        {
            int bytesAvail = pSocket->pendingDatagramSize();
            char *buf = (char*) malloc(bytesAvail);
            memset(buf, 0, sizeof(bytesAvail));

            QHostAddress peerAddress;
            int byteRead = pSocket->readDatagram(buf, bytesAvail, &peerAddress);
            if (byteRead <= 0) {
                free(buf);
                continue;
            }

            //This interface is mainly used for external mobile device only
            //So it doesn't accept loopback connection
            bool isLoopback = false;
            for (int i = 0; i < list.size(); i++)
                if (peerAddress.toString() == list.at(i).toString())
                    isLoopback = true;

            if (!isLoopback)
            {
                SocketRequest *request = new SocketRequest(QByteArray(buf, byteRead), peerAddress.toString().toLatin1(), this->mPort);
                if (!request->hasError() && this->requestHandler != NULL)
                {
                    SocketResponse response(this->pSocket, peerAddress.toString().toLatin1() , this->mPort);
                    this->requestHandler->service(*request, response);
                }
                delete request;
            }

            free(buf);
        }
    }

    //We don't need mutex here
    bStop = false;
    bRunning = false;
}

//----------------------------------------------------------------------------------
// Sending
//----------------------------------------------------------------------------------

void UdpSocketServer::sendMessage( QMap<QByteArray, QByteArray> params, QByteArray peerAddress )
{
    if (peerAddress.length() < 7 || peerAddress.toUpper() == "MULTICAST")
        peerAddress = MULTICAST_GROUP;

    SocketResponse response(pSocket, peerAddress, this->mPort);
    response.setQMap(params);
    response.write();
}
