#ifndef UDPSOCKETSERVER_H
#define UDPSOCKETSERVER_H

#include <QSettings>
#include <QRunnable>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QHostAddress>
#include <QUdpSocket>
#include "socketrequesthandler.h"

#define SERVICE_INTERVAL    20                  //milliseconds
#define MULTICAST_GROUP     "225.0.0.37"

/**
  Listens for incoming UDP data from mobile devices and communicate in XML format
*/

class UdpSocketServer : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(UdpSocketServer)
public:

    /** Constructor */
    UdpSocketServer(QSettings* settings, SocketRequestHandler* requestHandler, QObject* parent = 0);

    /** Destructor */
    virtual ~UdpSocketServer();

private:

    /** Configuration settings for the server */
    QSettings* settings;

    //Flags
    QMutex      mutex;
    int         mPort;
    bool        bStop;
    bool        bRunning;

public:

    void start();
    void terminate();
    void msleep(unsigned long msecs);
    bool isRunning();
    bool isStopping();

    void sendMessage( QMap<QByteArray, QByteArray> params, QByteArray peerAddress );

private:

    /** Main loop of the thread */
    void run();

    /** Request handler for the server */
    SocketRequestHandler* requestHandler;

    /** The socket object **/
    QUdpSocket* pSocket;

};

#endif // UDPSOCKETSERVER_H
