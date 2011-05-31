#ifndef HTTPCONNECTIONHANDLERPOOL_H
#define HTTPCONNECTIONHANDLERPOOL_H

#include <QList>
#include <QTimer>
#include <QObject>
#include "httpconnectionhandler.h"

/**
  Pool of http connection handlers. Connection handlers are created on demand and idle handlers are
  cleaned up in regular time intervals.
  <p>
  Example for the required configuration settings:
  <code><pre>
  minThreads=1
  maxThreads=10
  cleanupInterval=1000
  </pre></code>
  The pool is empty initially and grows with the number of concurrent
  connections. A timer removes one idle connection handler at each
  interval, but a it leaves some spare handlers in memory to improve
  performance.
*/

class HttpConnectionHandlerPool : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(HttpConnectionHandlerPool)
public:

    /**
      Constructor.
      @param settings Configuration settings for the HTTP server. Must not be 0.
      @param requestHandler The handler that will process each received HTTP request.
      @warning The requestMapper gets deleted by the destructor of this pool
    */
    HttpConnectionHandlerPool(QSettings* settings, HttpRequestHandler* requestHandler);

    /** Destructor */
    virtual ~HttpConnectionHandlerPool();

    /** Get a free connection handler, or 0 if not available. */
    HttpConnectionHandler* getConnectionHandler();

private:

    /** Settings for this pool */
    QSettings* settings;

    /** Will be assigned to each Connectionhandler during their creation */
    HttpRequestHandler* requestHandler;

    /** Pool of connection handlers */
    QList<HttpConnectionHandler*> pool;

    /** Timer to clean-up unused connection handler */
    QTimer cleanupTimer;

private slots:

    /** Received from the clean-up timer.  */
    void cleanup();

};

#endif // HTTPCONNECTIONHANDLERPOOL_H
