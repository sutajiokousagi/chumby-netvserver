#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QStringList>
#include <QDir>
#include <QByteArray>
#include <QStringList>
#include "static.h"
#include "flashpolicyserver.h"
#include "tcpsocketserver.h"
#include "udpsocketserver.h"
#include "requestmapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcgiapp.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/** Name of this application */
#define APPNAME "NeTVServer"

#define FCGI_SOCKET "/tmp/bridge.socket"
#define THREAD_COUNT 20


//----------------------------------------------------------------------------------------------------------------
// FastCGI stuffs
//----------------------------------------------------------------------------------------------------------------

/*
 * Initial parsing of the request path & redirect to actual handler function
 */
static int
handle_request(FCGX_Request *request)
{
    int ret = -1;
    char *uri = FCGX_GetParam("REQUEST_URI", request->envp);

    if (!strncmp(uri, "/bridge", strlen("/bridge"))) {
        ret = Static::bridgeController->handle_fastcgi_request(request);
    }

    else {
        fprintf(stderr, "Unrecognized URI: %s\n", uri);
    }

    return ret;
}


/*
 * Threaded function to handle FastCGI request
 */
static void *
request_thread(void *s_ptr)
{
    int rc;
    FCGX_Request request;

    if (FCGX_InitRequest(&request, *((int *)s_ptr), 0)) {
        perror("Unable to init request");
        return NULL;
    }

    while (1) {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        handle_request(&request);
        FCGX_Finish_r(&request);
    }

    return NULL;
}



//----------------------------------------------------------------------------------------------------------------
// Qt stuffs
//----------------------------------------------------------------------------------------------------------------

/*
 * Return true if there is another instance of this program already running
 */
bool isRunning()
{
    QProcess *newProc = new QProcess();
    newProc->start("/bin/pidof", QStringList(APPNAME));
    newProc->waitForFinished();

    QByteArray buffer = newProc->readAllStandardOutput();
    newProc->close();
    delete newProc;
    newProc = NULL;

    QStringList pidList = QString(buffer.trimmed()).split(" ", QString::SkipEmptyParts);
    if (pidList.length() > 1)
        return true;

    return false;
}


/*
 * Create handlers, listeners and assign them to global pointers in Static class
 */
int initialize_modules()
{
    // Configuration file
    QString configFileName=Static::getConfigDir()+"/"+APPNAME+".ini";

    // Configure bridge controller
    QSettings* bridgeSettings=new QSettings(configFileName,QSettings::IniFormat);
    bridgeSettings->beginGroup("bridge-controller");
    Static::bridgeController=new BridgeController(bridgeSettings);

    RequestMapper *requestMapper = new RequestMapper();
    Static::requestMapper = requestMapper;

    // Configure Flash policy server
    QSettings* flashpolicySettings=new QSettings(configFileName,QSettings::IniFormat);
    flashpolicySettings->beginGroup("flash-policy-server");
    new FlashPolicyServer(flashpolicySettings);

    // Configure TCP socket server
    QSettings* tcpServerSettings=new QSettings(configFileName,QSettings::IniFormat);
    tcpServerSettings->beginGroup("tcp-socket-server");
    Static::tcpSocketServer = new TcpSocketServer(tcpServerSettings, requestMapper);
    if (!Static::tcpSocketServer->isListening()) {
        delete Static::tcpSocketServer;
        Static::tcpSocketServer = NULL;
    }

    // Configure UDP socket server
    QSettings* udpServerSettings=new QSettings(configFileName,QSettings::IniFormat);
    udpServerSettings->beginGroup("udp-socket-server");
    Static::udpSocketServer = new UdpSocketServer(udpServerSettings, requestMapper);

    // DBus monitor
#ifdef ENABLE_DBUS_STUFF
    Static::dbusMonitor = new DBusMonitor(this);
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_StateChanged(uint)), Static::bridgeController, SLOT(slot_StateChanged(uint)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_PropertiesChanged(QByteArray,QByteArray)), Static::bridgeController, SLOT(slot_PropertiesChanged(QByteArray,QByteArray)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_DeviceAdded(QByteArray)), Static::bridgeController, SLOT(slot_DeviceAdded(QByteArray)));
    QObject::connect(Static::dbusMonitor, SIGNAL(signal_DeviceRemoved(QByteArray)), Static::bridgeController, SLOT(slot_DeviceRemoved(QByteArray)));
#endif

    return 0;
}



//----------------------------------------------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication instance(argc, argv);
    instance.setApplicationName(APPNAME);
    instance.setOrganizationName("");

    if (isRunning()) {
        qDebug("Another NeTVServer is already running");
        return 1;
    }

    QStringList argsList = instance.arguments();

    // If the args list contains "-d", then daemonize
    int temp = 0;
    if (argsList.contains("-d"))
        temp = daemon(0, 0);

    // Print out console arguments (if any)
    if (argsList.count() > 0)
        qDebug("Starting new NeTVServer");

    // Initialize global pointers
    initialize_modules();

    //-----------------------------------------------------------
    // FastCGI

    int listen_socket;
    int i;
    pthread_t threads[THREAD_COUNT];

    FCGX_Init();
    listen_socket = FCGX_OpenSocket(FCGI_SOCKET, 10);
    if (listen_socket < 0) {
        perror("Unable to open listen socket");
        exit(1);
    }

    for (i=0; i<THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, request_thread, (void *)&listen_socket);
        pthread_detach(threads[i]);
    }

    //-----------------------------------------------------------

    // Qt events loop
    return instance.exec();
}
