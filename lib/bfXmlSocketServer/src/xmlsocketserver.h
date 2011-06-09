#ifndef XMLLISTENER_H
#define XMLLISTENER_H

#include <QTcpServer>
#include <QSettings>

/**
  Listens for incoming TCP connections by Flash and communicate in XML format.
*/

class XmlSocketServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(XmlSocketServer)
public:

    /** Constructor */
    XmlSocketServer(QSettings* settings, QObject* parent = 0);

    /** Destructor */
    virtual ~XmlSocketServer();

protected:

    /** Serves new incoming connection requests */
    void incomingConnection(int socketDescriptor);

private:

    /** Configuration settings for the server */
    QSettings* settings;

};

#endif // XMLLISTENER_H
