#ifndef UDPSOCKETSERVER_H
#define UDPSOCKETSERVER_H

#include <QSettings>
#include <QRunnable>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QHostAddress>
#include "socketrequesthandler.h"

#define SERVICE_INTERVAL    25                  //milliseconds
#define MULTICAST_GROUP     "225.0.0.37"
#define USE_MULTICAST       false
#define DATA_MESSAGE        "da"
#define COMMAND_MESSAGE     "cm"
#define MESSAGE_KEY_TYPE    "tp"
#define MESSAGE_KEY_ADDRESS "address"

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
    bool        bStop;
    bool        bRunning;

    //Data storage
    QList< QMap<QString, QString> > messageQueue;

public:

    void start();
    void terminate();
    void msleep(unsigned long msecs);

    void queueMessage( QMap<QString, QString> params );
    QByteArray getSerializedInfoXML( const QMap<QString, QString> params );

private:

    bool isRunning();
    bool hasToStop();
    void run();

    /** Request handler for the server */
    SocketRequestHandler* requestHandler;
};

#endif // UDPSOCKETSERVER_H
