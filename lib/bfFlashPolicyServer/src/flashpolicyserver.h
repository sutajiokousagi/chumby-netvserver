#ifndef FLASHLISTENER_H
#define FLASHLISTENER_H

#include <QTcpServer>
#include <QSettings>

/**
  Listens for incoming TCP connections.
  Refer to http://www.adobe.com/devnet/flashplayer/articles/socket_policy_files.html for detailed specs
*/

#define FLASH_POLICY_SERVER_PORT	843
#define	FLASH_POLICY_REQUEST_STRING     "<policy-file-request/>"
#define	FLASH_POLICY_DEFAULT            "<cross-domain-policy><site-control permitted-cross-domain-policies=\"master-only\"/><allow-access-from to-ports=\"*\" domain=\"*\"/></cross-domain-policy>"

class FlashPolicyServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(FlashPolicyServer)
public:

    /** Constructor */
    FlashPolicyServer(QSettings* settings, QObject* parent = 0);

    /** Destructor */
    virtual ~FlashPolicyServer();

protected:

    /** Serves new incoming connection requests */
    void incomingConnection(int socketDescriptor);

private:

    /** Configuration settings for the HTTP server */
    QSettings* settings;

};

#endif // FLASHLISTENER_H
