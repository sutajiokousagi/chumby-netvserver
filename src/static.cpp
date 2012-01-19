#include "static.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>

QString Static::configDir = NULL;
RequestMapper* Static::requestMapper = NULL;
TcpSocketServer* Static::tcpSocketServer = NULL;
UdpSocketServer* Static::udpSocketServer = NULL;
BridgeController* Static::bridgeController = NULL;

#ifdef ENABLE_DBUS_STUFF
    DBusMonitor* Static::dbusMonitor=0;
#endif



QString Static::getConfigFileName()
{
    return QString("%1/%2.ini").arg(getConfigDir()).arg(QCoreApplication::applicationName());
}

QString Static::getConfigDir()
{
    if (!configDir.isNull())
        return configDir;

    // Search config file
    QString binDir=QCoreApplication::applicationDirPath();
    QString organization=QCoreApplication::organizationName();
    QString configFileName=QCoreApplication::applicationName()+".ini";

    QStringList searchList;
    searchList.append(QDir::rootPath()+"etc/xdg/"+organization);
    searchList.append(QDir::rootPath()+"etc/opt");
    searchList.append(QDir::rootPath()+"etc");
    searchList.append(QDir::cleanPath(binDir));
    searchList.append(QDir::cleanPath(binDir+"/../etc"));
    searchList.append(QDir::cleanPath(binDir+"/../../etc")); // for development under Windows

    foreach (QString dir, searchList)
    {
        QFile file(dir+"/"+configFileName);
        if (file.exists())
        {
            // found
            configDir=dir;
            qDebug("NeTVServer: Using config file %s",qPrintable(file.fileName()));
            return configDir;
        }
    }

    // not found
    foreach (QString dir, searchList)
        qWarning("%s/%s not found",qPrintable(dir),qPrintable(configFileName));

    qFatal("Cannot find config file %s",qPrintable(configFileName));
    return 0;
}
