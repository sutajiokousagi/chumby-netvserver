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

    // Start listening
    start();
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

    bStop = false;
    bRunning = true;
    mutex.unlock();

    //Create the socket
    int port = settings->value("port").toInt();

    QUdpSocket sock;
    bool isBound = sock.bind(port);
    if (!isBound) {
        qDebug() << "UdpSocketServer failed to bind to port " << port;
        return;
    }
    qDebug("UdpSocketServer: Listening on port %i", port);

    //Get own IP addresses
    QList <QHostAddress> list = QHostInfo::fromName(QHostInfo::localHostName()).addresses();

    while(!bStop)
    {
        msleep(SERVICE_INTERVAL);

        while(sock.hasPendingDatagrams())
        {
            int bytesAvail = sock.pendingDatagramSize();
            char *buf = (char*) malloc(bytesAvail);
            memset(buf, 0, sizeof(bytesAvail));

            QHostAddress peerAddress;
            int byteRead = sock.readDatagram(buf, bytesAvail, &peerAddress);
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
                SocketRequest *request = new SocketRequest(QByteArray(buf, byteRead), peerAddress.toString().toLatin1(), port);
                if (!request->hasError() && this->requestHandler != NULL)
                {
                    SocketResponse response(&sock, peerAddress.toString().toLatin1() , port);
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

void UdpSocketServer::queueMessage( QMap<QString, QString> params )
{
    messageQueue.push_back(params);
}

QByteArray UdpSocketServer::getSerializedInfoXML( const QMap<QString, QString> params )
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter *xmlfile = new QXmlStreamWriter();
    xmlfile->setDevice(&buffer);

    xmlfile->writeStartElement( "xml" );

    QMapIterator<QString, QString> i(params);
    while (i.hasNext())
    {
         i.next();
         xmlfile->writeTextElement(i.key(), i.value());
    }

    xmlfile->writeEndElement();
    delete xmlfile;

    buffer.close();
    return byteArray;
}
