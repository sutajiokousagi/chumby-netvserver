#include "startup.h"
#include <unistd.h>
#include <stdio.h>
#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QStringList>

#include <stdio.h>
#include <stdlib.h>
#include <fcgiapp.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define FCGI_SOCKET "/tmp/bridge.socket"
#define THREAD_COUNT 20

#define CONTENT_TYPE_STRING "Content-Type: text/plain\r\n\r\n"
#define NO_PARAM_STRING "<status>2</status><data><value>please provide some parameter(s)</value></data>"
#define TEST_STRING "<status>1</status><cmd>HELLOWORLD</cmd><data><value>123456</value></data>"

/*
 * This function should be moved into BridgeController later
 */
int handle_bridge_uri(FCGX_Request *request)
{
    char *raw_uri = FCGX_GetParam("REQUEST_URI", request->envp);

    //No parameters
    if (strlen(raw_uri) <= strlen("/bridge/")) {
        FCGX_FPrintF(request->out, CONTENT_TYPE_STRING);
        FCGX_FPrintF(request->out, NO_PARAM_STRING);
        return 0;
    }

    char *uri = FCGX_GetParam("REQUEST_URI", request->envp) + strlen("/bridge/");

    qDebug("handle_bridge_uri: %s", uri);

    //QByteArray test = "<status>1</status><cmd>HELLOWORLD</cmd><data><value>123456</value></data>";
    //FCGX_PutStr(test.constData(), test.size(), request->out);
    FCGX_FPrintF(request->out, CONTENT_TYPE_STRING);
    int numWritten = FCGX_PutStr(TEST_STRING, strlen(TEST_STRING), request->out);

    qDebug("written: %d bytes", numWritten);

    return 0;
}


/*
 * Initial parsing of the request path & redirect to actual handler function
 */
static int
handle_request(FCGX_Request *request)
{
    int ret = -1;
    char *uri = FCGX_GetParam("REQUEST_URI", request->envp);

    if (!strncmp(uri, "/bridge", strlen("/bridge"))) {
        ret = handle_bridge_uri(request);
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
        //static pthread_mutex_t counts_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        handle_request(&request);
        FCGX_Finish_r(&request);
    }


    return NULL;
}


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
 * This needs no comments :)
 */
int main(int argc, char *argv[])
{
    QCoreApplication instance(argc, argv);
    instance.setApplicationName(APPNAME);
    instance.setOrganizationName(ORGANISATION);

    if (isRunning()) {
        qDebug("Another NeTVServer is already running");
        return 1;
    }

    QStringList argsList = instance.arguments();
    QString argsString = argsList.join(ARGS_SPLIT_TOKEN);

    // If the args list contains "-d", then daemonize
    int temp = 0;
    if (argsList.contains("-d"))
        temp = daemon(0, 0);

    // Print out console arguments (if any)
    if (argsList.count() > 0) {
        qDebug("Starting new NeTVServer with console args: ");
        qDebug("%s", qPrintable(argsString));
    }

    Startup startup;
    bool toBeReturn = startup.receiveArgs(argsString);
    if (toBeReturn == true)
        return 0;

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

    //This will not return
    request_thread((void *)&listen_socket);

    //-----------------------------------------------------------

    //This is normally unreachable
    return instance.exec();
}
