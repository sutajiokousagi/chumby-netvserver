#ifndef BridgeController_H
#define BridgeController_H

#include "httprequesthandler.h"
#include "socketrequesthandler.h"

#define TAG         "NeTVServer"

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

signals:

    // To InputDevice module
    void signal_SendKeyEvent(QByteArray keyName, bool isPressed, bool autoRepeat);

private:

    /** Variables from NeTVServer.ini */
    QString docroot;
    QString paramsFile;
    QString networkConfigFile;
    QString accountConfigFile;

    /** Process Utilities */
    QByteArray Execute(const QString &fullPath);
    QByteArray Execute(const QString &fullPath, QStringList args);

    /** Helper */
    bool IsAuthorizedCaller(QByteArray headerValue);
    bool SetNetworkConfig(QHash<QString, QString> parameters);

    /** Parameters */
    QSettings * parameters;
    QByteArray GetParameter(QString name);
    QByteArray GetAllParameters();
    void SetParameter(QString name, QString value);
    void LoadParameters(QString * filename = NULL);
    void SaveParameters(QString * filename = NULL);

    /** File Utilities */
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

    void DumpStaticFile(QByteArray path, HttpResponse& response);
    void SetContentType(QString fileName, HttpResponse& response) const;
};

#endif // BridgeController_H
