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
    currentRequest=0;
    // execute signals in my own thread
    moveToThread(this);
    socket.moveToThread(this);
    readTimer.moveToThread(this);
    connect(&socket, SIGNAL(readyRead()), SLOT(read()));
    connect(&socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    readTimer.setSingleShot(true);
    qDebug("HttpConnectionHandler (%x): constructed",(unsigned int) this);
    this->start();
}


HttpConnectionHandler::~HttpConnectionHandler() {
    qDebug("HttpConnectionHandler (%x): destroyed",(unsigned int) this);
}


void HttpConnectionHandler::run() {
    qDebug("HttpConnectionHandler (%x): thread started",(unsigned int) this);
    try {
        exec();
    }
    catch (...) {
        qCritical("HttpConnectionHandler (%x): an uncatched exception occured in the thread",(unsigned int) this);
    }
    qDebug("HttpConnectionHandler (%x): thread stopped",(unsigned int) this);
    // Change to the main thread, otherwise deleteLater() would not work
    moveToThread(QCoreApplication::instance()->thread());
}


void HttpConnectionHandler::handleConnection(int socketDescriptor) {
    qDebug("HttpConnectionHandler (%x): handle new connection",(unsigned int) this);
    Q_ASSERT(socket.isOpen()==false); // if not, then the handler is already busy
    if (!socket.setSocketDescriptor(socketDescriptor)) {
        qCritical("HttpConnectionHandler (%x): cannot initialize socket: %s",(unsigned int) this,qPrintable(socket.errorString()));
        return;
    }
    // Start timer for read timeout
    int readTimeout=settings->value("readTimeout",10000).toInt();
    readTimer.start(readTimeout);
    currentRequest=0;
}


bool HttpConnectionHandler::isBusy() {
    return socket.isOpen();
}


void HttpConnectionHandler::readTimeout() {
    qDebug("HttpConnectionHandler (%x): read timeout occured",(unsigned int) this);
    socket.write("HTTP/1.1 408 request timeout\r\nConnection: close\r\n\r\n408 request timeout\r\n");
    socket.disconnectFromHost();
    delete currentRequest;
    currentRequest=0;
}


void HttpConnectionHandler::disconnected() {
    qDebug("HttpConnectionHandler (%x): disconnected",(unsigned int) this);
    socket.close();
    delete currentRequest;
    currentRequest=0;
    readTimer.stop();
}

void HttpConnectionHandler::read() {
#ifdef SUPERVERBOSE
    qDebug("HttpConnectionHandler (%x): read input",(unsigned int) this);
#endif

    // Create new HttpRequest object if necessary
    if (!currentRequest) {
        currentRequest=new HttpRequest(settings);
    }

    // Collect data for the request object
    while (socket.bytesAvailable() && currentRequest->getStatus()!=HttpRequest::complete && currentRequest->getStatus()!=HttpRequest::abort) {
        currentRequest->readFromSocket(socket);
        if (currentRequest->getStatus()==HttpRequest::waitForBody) {
            // Restart timer for read timeout, otherwise it would
            // expire during large file uploads.
            int readTimeout=settings->value("readTimeout",10000).toInt();
            readTimer.start(readTimeout);
        }
    }

    // If the request is aborted, return error message and close the connection
    if (currentRequest->getStatus()==HttpRequest::abort) {
        socket.write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
        socket.disconnectFromHost();
        delete currentRequest;
        currentRequest=0;
    }

    // If the request is complete, let the request mapper dispatch it
    if (currentRequest->getStatus()==HttpRequest::complete) {
        readTimer.stop();
        qDebug("HttpConnectionHandler (%x): received request",(unsigned int) this);
        HttpResponse response(&socket);
        try {
            requestHandler->service(*currentRequest, response);
        }
        catch (...) {
            qCritical("HttpConnectionHandler (%x): An uncatched exception occured in the request handler",(unsigned int) this);
        }

        // Finalize sending the response if not already done
        if (!response.hasSentLastPart()) {
            response.write(QByteArray(),true);
        }
        // Close the connection after delivering the response, if requested
        if (QString::compare(currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0) {
            socket.disconnectFromHost();
        }
        else {
            // Start timer for next request
            int readTimeout=settings->value("readTimeout",10000).toInt();
            readTimer.start(readTimeout);
        }
        // Prepare for next request
        delete currentRequest;
        currentRequest=0;
    }
}
