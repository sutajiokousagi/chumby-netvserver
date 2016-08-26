#include <QDateTime>
#include <QLocale>
#include <QDir>
#include <QMimeDatabase>

#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "netvserverapplication.h"
#include "nhttprequest.h"
#include "nhttpresponse.h"

static int defaultCommandHandler(const QString & cmd, NeTVServerApplication & app,
                                 const NHttpRequest & request, NHttpResponse & response)
{
    Q_UNUSED(app);
    Q_UNUSED(cmd);
    Q_UNUSED(request);
    response.standardResponse(cmd, NETV_STATUS_UNIMPLEMENTED, "Check for valid 'cmd' variable");

    return 0;
}

namespace {
    ///////////////////////////////////////////////////////////////////////////////
    using namespace qhttp::server;

    /** connection class for gathering incoming chunks of data from HTTP client.
     * @warning please note that the incoming request instance is the parent of
     * this QObject instance. Thus this class will be deleted automatically.
     * */
    class ClientHandler : public QObject
    {
    public:
        explicit ClientHandler(NeTVServerApplication * newApp, QHttpRequest* req, QHttpResponse* res)
            : QObject(req /* as parent*/), app(newApp) {

            // automatically collect http body(data)
            req->collectData();

            // when all the incoming data are gathered, send some response to client.
            req->onEnd([this, req, res]() {
                res->setStatusCode(qhttp::ESTATUS_OK);

                NHttpRequest request(req, this);
                NHttpResponse response(res, this);

                if (request.url().path() == "/bridge") {
                    const QString cmdString = request.parameter("cmd").toUpper();
                    qInfo(QString("Bridge: %1").arg(cmdString).toUtf8());

                    handleBridgeCallbackType cb = app->callback(cmdString);

                    if (!cb(cmdString, *app, request, response))
                        res->end();
                    return;
                }

                // Convert the URL part to a local part, relative to docRoot.
                QFileInfo targetInfo(QFileInfo(app->staticDocRoot().canonicalPath() + QDir::separator() + request.url().path()).canonicalFilePath());
                if (!targetInfo.canonicalFilePath().startsWith(app->staticDocRoot().canonicalPath(), Qt::CaseInsensitive)) {
                    response.setStatusCode(qhttp::ESTATUS_FORBIDDEN);
                    response.write(QString("Invalid path: %1").arg(targetInfo.canonicalFilePath()));
                    res->end();
                    return;
                }

                if (targetInfo.isDir()) {
                    targetInfo = QFileInfo(targetInfo.canonicalFilePath() + QDir::separator() + "index.html");
                }

                if (!targetInfo.exists()) {
                    response.setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                    response.write(QString("File not found"));
                    res->end();
                    return;
                }

                QFile targetFile(targetInfo.canonicalFilePath());
                if (!targetFile.open(QIODevice::ReadOnly)) {
                    response.setStatusCode(qhttp::ESTATUS_FORBIDDEN);
                    response.write(QString("Unable to open file: %1").arg(targetFile.errorString()));
                    res->end();
                    return;
                }

                QMimeDatabase db;

                res->addHeader("Content-Type", db.mimeTypeForFile(targetInfo).name().toUtf8());

                qInfo(QString("Sending: %1 - %2").arg(targetInfo.canonicalFilePath()).arg(db.mimeTypeForFile(targetInfo).name()).toUtf8());

                while (!targetFile.atEnd()) {
                    QByteArray data = targetFile.read(2048);
                    response.write(data);
                }

                targetFile.close();

                res->end();
            });
        }

        virtual ~ClientHandler() {
            qInfo("Destroying ClientHandler");
        }

    protected:
        NeTVServerApplication *app;
    };
}

///////////////////////////////////////////////////////////////////////////////
/// \brief NeTVServerApplication::NeTVServerApplication
/// \param argc
/// \param argv
///

NeTVServerApplication::NeTVServerApplication(int & argc, char **argv)
    : QCoreApplication(argc, argv)
{
    QString portOrUnixSocket("10022"); // default: TCP port 10022
    if (argc > 1)
        portOrUnixSocket = argv[1];

//    bridge = new BridgeController(NULL, this);

    server = new qhttp::server::QHttpServer(this);
    server->listen(portOrUnixSocket, [&](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res) {
        new ClientHandler(this, req, res);
    });

    if (!server->isListening()) {
        qFatal(QString("can not listen on %1!").arg(qPrintable(portOrUnixSocket)).toUtf8());
        quit();
    }
    qInfo(QString("Listening on port %1").arg(portOrUnixSocket).toUtf8());
}

handleBridgeCallbackType NeTVServerApplication::callback(const QString &cmd)
{
    handleBridgeCallbackType cb = callbacks.value(cmd);
    if (!cb) {
        cb = callbacks.value("");
        if (!cb)
            cb = defaultCommandHandler;
    }
    return cb;
}

int NeTVServerApplication::registerBridgeFunction(const QString &cmd, handleBridgeCallbackType callback)
{
    callbacks.insert(cmd.toUpper(), callback);
    return 0;
}

const QDir & NeTVServerApplication::setStaticDocRoot(const QString & newRoot)
{
    docRoot.setPath(newRoot);
    return docRoot;
}

const QDir & NeTVServerApplication::staticDocRoot(void)
{
    return docRoot;
}

const QVariant NeTVServerApplication::setting(const QString &key,
                                               QVariant defaultValue)
{
    return settings.value(key, defaultValue);
}

void NeTVServerApplication::setSetting(const QString &key, const QVariant &value)
{
    settings.setValue(key, value);
}
