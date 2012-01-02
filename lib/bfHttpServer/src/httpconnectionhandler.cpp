/**
  @file
  @author Stefan Frings
  @version $Id: httpconnectionhandler.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"
#include <QTimer>
#include <QCoreApplication>

HttpConnectionHandler::HttpConnectionHandler(QSettings* settings, HttpRequestHandler* requestHandler)
    : QThread()
{
    Q_ASSERT(settings!=0);
    Q_ASSERT(requestHandler!=0);
    this->settings=settings;
    this->requestHandler=requestHandler;
    this->socket.close();
    currentRequest = NULL;

    // execute signals in my own thread
    moveToThread(this);
    socket.moveToThread(this);
    readTimer.moveToThread(this);
    connect(&socket, SIGNAL(readyRead()), SLOT(read()));
    connect(&socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));

    readTimer.setSingleShot(true);
    this->start();
}


HttpConnectionHandler::~HttpConnectionHandler()
{

}


void HttpConnectionHandler::run()
{
    try {
        exec();
    }
    catch (...) {
        qCritical("HttpConnectionHandler (%x): an uncatched exception occured in the thread",(unsigned int) this);
    }

    // Change to the main thread, otherwise deleteLater() would not work
    moveToThread(QCoreApplication::instance()->thread());
}


void HttpConnectionHandler::handleConnection(int socketDescriptor)
{
    //qDebug("HttpConnectionHandler (%x): handle new connection",(unsigned int) this);

    //For some reason it fails here, although HttpConnectionHandlerPool already check the same condition
    //Q_ASSERT(socket.isBusy()==false);                     // if not, then the handler is already busy

    //So we just kill it if busy
    if (this->isBusy())
        socket.close();

    if (!socket.setSocketDescriptor(socketDescriptor))
    {
        qCritical("HttpConnectionHandler (%x): cannot initialize socket: %s",(unsigned int) this,qPrintable(socket.errorString()));
        return;
    }
    // Start timer for read timeout
    int readTimeout=settings->value("readTimeout",10000).toInt();
    readTimer.start(readTimeout);
    currentRequest = NULL;
}


bool HttpConnectionHandler::isBusy()
{
    return socket.isOpen();
}


void HttpConnectionHandler::readTimeout()
{
    //qDebug("HttpConnectionHandler (%x): read timeout occured",(unsigned int) this);
    socket.write("HTTP/1.1 408 request timeout\r\nConnection: close\r\n\r\n408 request timeout\r\n");
    socket.disconnectFromHost();
    delete currentRequest;
    currentRequest = NULL;
}


void HttpConnectionHandler::disconnected()
{
    //qDebug("HttpConnectionHandler (%x): disconnected",(unsigned int) this);
    socket.close();
    delete currentRequest;
    currentRequest = NULL;
    readTimer.stop();
}

void HttpConnectionHandler::read()
{
    // Create new HttpRequest object if necessary
    if (!currentRequest) {
        currentRequest=new HttpRequest(settings);
    }

    // Collect data for the request object
    while (socket.bytesAvailable() > 0
           && currentRequest->getStatus() != HttpRequest::complete
           && currentRequest->getStatus() != HttpRequest::abort)
    {
        currentRequest->readFromSocket(socket);
        if (currentRequest->getStatus()==HttpRequest::waitForBody)
        {
            // Restart timer for read timeout, otherwise it would
            // expire during large file uploads.
            int readTimeout=settings->value("readTimeout",10000).toInt();
            readTimer.start(readTimeout);
        }
    }

    // If the request is aborted, return error message and close the connection
    if (currentRequest->getStatus()==HttpRequest::abort)
    {
        QByteArray lastError = currentRequest->getLastError();
        if (lastError.size() > 0)
            socket.write("HTTP/1.1 400 bad request\r\nConnection: close\r\n\r\n" + lastError + "\r\n");
        else
            socket.write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
        socket.disconnectFromHost();
        delete currentRequest;
        currentRequest = NULL;
        return;
    }

    //NeTVServer used to segfault right after this comment
    //due to a null pointer 'currentRequest' and missing 'return' statement just above

    // If the request is complete, let the request mapper dispatch it
    if (currentRequest->getStatus()==HttpRequest::complete)
    {
        readTimer.stop();
        HttpResponse response(&socket);
        try {
            requestHandler->service(*currentRequest, response);
        }
        catch (...) {
            qCritical("HttpConnectionHandler (%x): An uncatched exception occured in the request handler",(unsigned int) this);
        }

        // If the request/reponse is long poll type
        // We can delete the request, but not the response
        if (response.isLongPoll()) {
            delete currentRequest;
            currentRequest = NULL;
            return;
        }

        // Finalize sending the response if not already done
        if (!response.hasSentLastPart()) {
            response.write(QByteArray(),true);
        }
        // Close the connection after delivering the response, if requested
        if (QString::compare(currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0) {
            socket.disconnectFromHost();
        }
        else
        {
            // Start timer for next request
            int readTimeout=settings->value("readTimeout",10000).toInt();
            readTimer.start(readTimeout);
        }
        // Prepare for next request
        delete currentRequest;
        currentRequest = NULL;
    }
}
