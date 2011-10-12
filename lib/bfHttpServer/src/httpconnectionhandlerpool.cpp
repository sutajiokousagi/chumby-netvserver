#include "httpconnectionhandlerpool.h"

HttpConnectionHandlerPool::HttpConnectionHandlerPool(QSettings* settings, HttpRequestHandler* requestHandler)
    : QObject()
{
    Q_ASSERT(settings!=0);
    this->settings=settings;
    this->requestHandler = requestHandler;

    minThreads = settings->value("minThreads",1).toInt();
    maxThreads = settings->value("maxThreads",20).toInt();

    connect(&cleanupTimer, SIGNAL(timeout()), SLOT(cleanup()));
    cleanupTimer.start(settings->value("cleanupInterval",1000).toInt());
}


HttpConnectionHandlerPool::~HttpConnectionHandlerPool()
{
    foreach(HttpConnectionHandler* handler, pool)
        delete handler;
}


HttpConnectionHandler* HttpConnectionHandlerPool::getConnectionHandler()
{
    //Search for an existing free worker thread
    HttpConnectionHandler* freeHandler = NULL;
    foreach(HttpConnectionHandler* handler, pool)
    {
        if (!handler->isBusy()) {
            freeHandler=handler;
            break;
        }
    }

    //Found a idling worker thread
    if (freeHandler != NULL)
        return freeHandler;

    //Pool is already full, no free worker thread
    if (pool.count() >= this->maxThreads)
        return NULL;

    //No free handler exists in pool, create new handler on-the-fly
    //only if maxThreads has not been reached
    freeHandler = new HttpConnectionHandler(settings, requestHandler);
    pool.append(freeHandler);
    return freeHandler;
}

void HttpConnectionHandlerPool::cleanup()
{
    int idleCounter = 0;

    foreach(HttpConnectionHandler* handler, pool)
    {
        if (handler->isBusy())
            continue;

        if (++idleCounter > this->minThreads)
        {
            pool.removeOne(handler);
            //qDebug("HttpConnectionHandlerPool: Removed connection handler (%x), pool size is now %i",(unsigned int) handler,pool.size());
            connect(handler,SIGNAL(finished()),handler,SLOT(deleteLater()));
            handler->quit();
            break; // remove only one handler in each interval
        }
    }
}
