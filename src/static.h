#ifndef STATIC_H
#define STATIC_H

#include <QString>
#include "tcpsocketserver.h"
#include "udpsocketserver.h"
#include "requestmapper.h"
#include "controller/bridgecontroller.h"

#ifdef ENABLE_DBUS_STUFF
    #include "dbusmonitor.h"
#endif


/**
  This class contains static resources (global pointers) that are used by the application.
*/

class Static
{
public:

    /**
      Search the main config file and return its full path.
      On the first call, the INI file gets searched. If not found,
      the application aborts with an error message.
      <p>
      The filename is the applications name plus the ending ".ini". It is searched
      in the following directories:

      - Same directory as the applications executable file
      - In ../etc relative to the applications executable file
      - In ../../etc relative to the applications executable file
      - In /etc/xdg/{organisation name} on the root drive
      - In /etc/opt on the root drive
      - In /etc on the root drive

     */
    static QString getConfigFileName();

    /**
      Gets the directory where the main config file is located.
      On the first call, the INI file gets searched. If not found,
      the application aborts with an error message.
      @see getConfigFileName()
     */
    static QString getConfigDir();

    /** TCP/UDP request mapper */
    static RequestMapper* requestMapper;

    /** TCP Socket server for Flash player */
    static TcpSocketServer* tcpSocketServer;

    /** UDP Socket server for mobile device */
    static UdpSocketServer* udpSocketServer;

    /** Controller for hardware bridge */
    static BridgeController* bridgeController;

    /** DBus monitor */
#ifdef ENABLE_DBUS_STUFF
    static DBusMonitor* dbusMonitor;
#endif

private:

    /** Directory of the main config file */
    static QString configDir;

};

#endif // STATIC_H
