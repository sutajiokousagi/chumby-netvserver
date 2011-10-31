#ifndef BridgeController_H
#define BridgeController_H

#include "httprequesthandler.h"
#include "socketrequesthandler.h"

#define TAG                                 "NeTVServer"
#define BRIDGE_RETURN_STATUS_UNIMPLEMENTED  "0"
#define BRIDGE_RETURN_STATUS_SUCCESS        "1"
#define BRIDGE_RETURN_STATUS_ERROR          "2"
#define BRIDGE_RETURN_STATUS_UNAUTHORIZED   "3"

#define ARGS_SPLIT_TOKEN    "|~|"

#define STRING_AUTHORIZED_CALLER            "Authorized-Caller"
#define STRING_COMMAND                      "cmd"
#define STRING_DATA                         "data"
#define STRING_VALUE                        "value"
#define STRING_XML_ESCAPE                   "xmlescape"
#define STRING_COMMAND_FORWARDED_BROWSER    "Command forwarded to NeTVBrowser"
#define STRING_NO_BROWSER_RUNNING           "No browser is running"
#define STRING_COMMAND_FORWARDED_CLIENT     "Command forwarded to TCP clients"
#define STRING_NO_CLIENT_RUNNING            "No client is running"


class BridgeController : public QObject, public HttpRequestHandler, public SocketRequestHandler
{
    Q_OBJECT
    //Q_DISABLE_COPY(BridgeController);
public:

    /** Constructor */
    BridgeController(QSettings* settings, QObject* parent = 0);

    ~BridgeController();

    /** Receive & response to HTTP requests */
    void service(HttpRequest& request, HttpResponse& response);

    /** Receive & response to socket requests */
    void service(SocketRequest& request, SocketResponse& response);

public slots:

    // From dbusmonitor.h
    void slot_StateChanged(uint state);
    void slot_PropertiesChanged(QByteArray prop_name, QByteArray prop_value);
    void slot_DeviceAdded(QByteArray objPath);
    void slot_DeviceRemoved(QByteArray objPath);

    // For delayed AccessPoint mode
    void slot_StopAP();
    void slot_StartAP();
    void slot_StartAP_Factory();
    void slot_Reboot();

signals:

    // To InputDevice module
    void signal_SendKeyEvent(QByteArray keyName, bool isPressed, bool autoRepeat);

private:

    /** Variables from NeTVServer.ini */
    QString docroot;                //This docroot is for executing scripts, should not be changed
    QString paramsFile;
    QString networkConfigFile;

    /** Interact with StaticFileController */
    QString SetStaticDocroot(QString newPath);
    QString GetStaticDocroot();

    /** High level helper functions */
    bool IsAuthorizedCaller(QByteArray headerValue);
    bool SetNetworkConfig(QHash<QString, QString> parameters);
    void StartAPwithDelay(int msec = 500);
    void StartAPFactorywithDelay(int msec = 500);
    void StopAPwithDelay(int msec = 500);
    void RebootwithDelay(int msec = 500);

    /** Parameters */
    QSettings * parameters;
    QByteArray GetParameter(QString name);
    QByteArray GetAllParameters();
    void SetParameter(QString name, QString value);
    void LoadParameters(QString * filename = NULL);
    void SaveParameters(QString * filename = NULL);

    /** Process Utilities */
    QByteArray Execute(const QString &fullPath, bool xmlEscape = false);
    QByteArray Execute(const QString &fullPath, QStringList args, bool xmlEscape = false);

    /** File Utilities */
    void Sync(void);
    bool FileExists(const QString &fullPath);
    bool FileExecutable(const QString &fullPath);
    qint64  GetFileSize(const QString &fullPath);
    QByteArray GetFileContents(const QString &fullPath);
    bool SetFileContents(const QString &fullPath, QByteArray data);
    bool SetFileExecutable(const QString &fullPath);
    bool UnlinkFile(const QString &fullPath);
    QByteArray GetFileMD5(const QString &fullPath);

    /** Mount/Unmount Utilities */
    bool MountRW();
    bool MountRO();

    /** Other Utilities */
    bool IsHexString(QString testString);
    QString XMLEscape(QString inputString);
    QString XMLUnescape(QString inputString);
    QByteArray XMLEscape(QByteArray inputString);
    QByteArray XMLUnescape(QByteArray inputString);

    /** HTTP response */
    void DumpStaticFile(QByteArray path, HttpResponse& response);

public:
    static void SetContentType(QString fileName, HttpResponse& response);
};

#endif // BridgeController_H
