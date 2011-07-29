#include "bridgecontroller.h"
#include "../static.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
//#include <QApplication>
//#include <QDesktopWidget>
#include <QDateTime>

#define BRIDGE_RETURN_STATUS_UNIMPLEMENTED  "0"
#define BRIDGE_RETURN_STATUS_SUCCESS        "1"
#define BRIDGE_RETURN_STATUS_ERROR          "2"

#define BRIDGE_NETWORK_CONFIG               "/psp/network_config"
#define BRIDGE_ACCOUNT_CONFIG               "/psp/chumby_account"

#define ARGS_SPLIT_TOKEN    "|~|"

BridgeController::BridgeController(QSettings* settings, QObject* parent) : QObject(parent), HttpRequestHandler(), SocketRequestHandler()
{
    longPollResponses.clear();

    encoding=settings->value("encoding","UTF-8").toString();
    docroot=settings->value("path",".").toString();

    // Convert relative path to absolute, based on the directory of the config file.
#ifdef Q_OS_WIN32
    if (QDir::isRelativePath(docroot) && settings->format()!=QSettings::NativeFormat)
#else
    if (QDir::isRelativePath(docroot))
#endif
    {
        QFileInfo configFile(settings->fileName());
        docroot=QFileInfo(configFile.absolutePath(),docroot).absoluteFilePath();
    }
    qDebug("BridgeController: docroot=%s, encoding=%s",qPrintable(docroot),qPrintable(encoding));
}


void BridgeController::service(HttpRequest& request, HttpResponse& response)
{
    //Protocol documentation
    //https://internal.chumby.com/wiki/index.php/JavaScript/HTML_-_Hardware_Bridge_protocol

    QByteArray cmdString = request.getParameter("cmd").toUpper();
    QByteArray dataXmlString = request.getParameter("data");
    QByteArray dataString = "";

    //Strip the XML tag if it is a simple value
    bool singleValueArg = dataXmlString.startsWith("<value>") && dataXmlString.endsWith("</value>");
    if (singleValueArg)
        dataString = dataXmlString.mid(7, dataXmlString.length() - 15).trimmed();

    //-----------

    /* Deprecated. Use generic command at the bottom
    if (cmdString == "HELLO")
    {
        //Returning GUID, DCID, HWver, SWver, etc.Z
        QByteArray buffer = this->Execute(docroot + "/scripts/hello.sh", QStringList(dataString));
        if (buffer.length() > 5)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data>" + buffer.trimmed() + "</data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data>" + buffer.trimmed() + "</data>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "INITIALHELLO")
    {
        //This is identical to 'Hello' command except that it will switch to Acces Point mode if necessary
        //To be called only once by JavaScriptCore

        //Returning GUID, DCID, HWver, SWver, etc.
        QByteArray buffer = this->Execute(docroot + "/scripts/initial_hello.sh", QStringList(dataString));
        if (buffer.length() > 5)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data>" + buffer.trimmed() + "</data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data>" + buffer.trimmed() + "</data>", true);
        buffer = QByteArray();
    }
    */

    if (cmdString == "SETURL")
    {
        //Send it straight to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to browser</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No browser found</value></data>", true);
    }

    else if (cmdString == "GETURL")
    {
        QByteArray urlString = request.getParameter("url");
        QByteArray postString = request.getParameter("post");
        QStringList argsList;
        if (urlString.length() > 0)     argsList.append(QString(urlString));
        if (postString.length() > 0)    argsList.append(QString(postString));
        if (argsList.length() < 1)
        {
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>no arguments received</value></data>", true);
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/geturl.sh", argsList);
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
            buffer = QByteArray();
        }
    }

    else if (cmdString == "GETJPG" || cmdString == "GETJPEG")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
    }

    //-----------

    else if (cmdString == "HASFLASHPLUGIN")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>false</value></data>", true);
    }

    else if (cmdString == "PLAYWIDGET")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>", true);
    }

    //-----------

    /* Deprecated. Use generic command at the bottom
    else if (cmdString == "SETBOX")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/setbox.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
        buffer = QByteArray();
    }
    */

    else if (cmdString == "SETCHROMAKEY")
    {
        //Contruct arguments
        dataString = dataString.toLower();
        QStringList argList;

        if (dataString == "on" || dataString == "true" || dataString == "yes")                argList.append(QString("on"));
        else if (dataString == "off" || dataString == "false" || dataString == "no")          argList.append(QString("off"));
        else if (dataString.count(",") == 2 && dataString.length() <= 11 && dataString.length() >= 5)
        {
            //Do 8-bit to 6-bit conversion
            QList<QByteArray> rgb = dataString.split(',');
            int r = rgb[0].toInt();
            int g = rgb[1].toInt();
            int b = rgb[2].toInt();
            argList.append(QString("%1").arg(r));
            argList.append(QString("%1").arg(g));
            argList.append(QString("%1").arg(b));
        }

        //Execute the script
        if (argList.size() <= 0)
        {
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>Invalid arguments</value></data>", true);
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList);
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
            buffer = QByteArray();
        }
    }

    //-----------

    else if (cmdString == "WIDGETENGINE")
    {
        //Forward simple commands to widget rendering engine
        if (!dataString.contains(" "))
            Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "widget_engine");

        QByteArray buffer = this->Execute(docroot + "/scripts/widget_engine.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
        buffer = QByteArray();
    }


    else if (cmdString == "PLAYSWF")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>", true);
    }

    else if (cmdString == "SETWIDGETSIZE")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Set the window size
        this->Execute(docroot + "/scripts/setbox.sh", QStringList(dataString));

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>", true);
    }

    else if (cmdString == "REMOTECONTROL")
    {
        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "all");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to browser</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No browser running</value></data>", true);

        /*
        QByteArray buffer = this->Execute(docroot + "/scripts/remote_control.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
        buffer = QByteArray();
        */
    }

    //-----------

    else if (cmdString == "CONTROLPANEL")
    {
        //Send it straight to browser
        //int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "KEY")
    {
        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "all");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to browser</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No browser running</value></data>", true);
    }

    //-----------

    else if (cmdString == "SETTIME")
    {
        // <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
        // <timezone> is standard timezone ID string formated as "Asia/Singapore"
        QString time = request.getParameter("time");
        QString timezone = request.getParameter("timezone");

        QStringList argsList;
        argsList.append(time);
        argsList.append(timezone);
        QByteArray buffer = this->Execute(docroot + "/scripts/set_time.sh", argsList);

        //Reply to JavaScriptCore/ControlPanel
        if (time == "")    response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>time parameter is required</value></data>", true);
        else               response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
    }

    else if (cmdString == "SETNETWORK")
    {
        QHash<QString,QString> params;
        params.insert("type", request.getParameter("type"));
        params.insert("allocation", request.getParameter("wifi_allocation"));
        params.insert("ssid", request.getParameter("wifi_ssid"));
        params.insert("auth", request.getParameter("wifi_authentication"));
        params.insert("encryption", request.getParameter("wifi_encryption"));
        params.insert("key", request.getParameter("wifi_password"));
        params.insert("encoding", request.getParameter("wifi_encoding"));
        bool fileOK = SetNetworkConfig(params);
        params.clear();

        QByteArray buffer;
        if (!fileOK)        buffer = this->GetFileContents(BRIDGE_NETWORK_CONFIG);
        else                buffer = this->Execute(docroot + "/scripts/stop_ap.sh");

        //Reply to JavaScriptCore/ControlPanel
        if (!fileOK)        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
        else                response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
    }

    else if (cmdString == "SETACCOUNT")
    {
        QString activated = request.getParameter("activated");
        QString username = request.getParameter("chumby_username");
        QString password = request.getParameter("chumby_password");
        QString device_name = request.getParameter("chumby_device_name");

        if (activated == "")    activated = "false";
        if (device_name == "")  device_name = "NeTV";

        QString account_config = QString("<configuration username=\"%1\" password=\"%2\" device_name=\"%3\" />").arg(username).arg(password).arg(device_name);
        bool fileOK = SetFileContents(BRIDGE_ACCOUNT_CONFIG, account_config.toLatin1());

        //We should ask the JSCore to do something here.
        //QByteArray buffer = this->Execute(docroot + "/scripts/stop_ap.sh");
        QByteArray buffer;
        if (!fileOK)        buffer = this->GetFileContents(BRIDGE_ACCOUNT_CONFIG);
        else                activated.toLatin1();

        //Reply to JavaScriptCore/ControlPanel
        if (!fileOK)        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
        else                response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer.trimmed() + "</value></data>", true);
    }

    //-----------

    else if (cmdString == "GETALLPARAMS")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>GetAllParams</value></data>", true);
    }

    else if (cmdString == "GETPARAM")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>GetParam</value></data>", true);
    }

    else if (cmdString == "SETPARAM")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>SetParam</value></data>", true);
    }

    //-----------

    else if (cmdString == "GETFILECONTENTS")
    {
        QByteArray filedata = GetFileContents(dataString);
        if (filedata == "file not found" || filedata == "no permission")
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>" + filedata + "</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + filedata + "</value></data>", true);
    }

    else if (cmdString == "SETFILECONTENTS")
    {
        //<path>full path to a file</path><content>........</content>
        QByteArray path = dataXmlString.mid(6, dataXmlString.indexOf("</path>") - 6);
        QByteArray content = dataXmlString.mid(dataXmlString.indexOf("<content>") + 9, dataXmlString.length()-dataXmlString.indexOf("<content>")-9);
        if (!SetFileContents(path,content))
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>0</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + QByteArray().setNum(GetFileSize(path)) + "</value></data>", true);
    }

    else if (cmdString == "FILEEXISTS")
    {
        QByteArray existStr = "false";
        if (FileExists(dataString))
            existStr = "true";
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + existStr + "</value></data>", true);
    }

    else if (cmdString == "GETFILESIZE")
    {
        QByteArray sizeStr = QByteArray().setNum(GetFileSize(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + sizeStr + "</value></data>", true);
    }

    else if (cmdString == "UNLINKFILE")
    {
        bool success = UnlinkFile(dataString);
        if (!success)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>false</value></data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>true</value></data>", true);
    }

    //-----------

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString));
        if (buffer.length() > 5)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data>" + buffer.trimmed() + "</data>", true);
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data>" + buffer.trimmed() + "</data>", true);
        buffer = QByteArray();
    }

    //-----------

    else
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>" + cmdString + "</value></data>", true);
    }
}


void BridgeController::service(SocketRequest& request, SocketResponse& response)
{
    QByteArray cmdString = request.getCommand().toUpper();
    QByteArray dataString = request.getParameter("value").trimmed();

    /* Deprecated. Use generic command at the bottom
    if (cmdString == "HELLO")
    {
        //The type of connection is already handled by the TcpSocketServer, but we do post-processing here
        //QByteArray hardwareType = request.getParameter("value");

        //Returning GUID, DCID, HWver, SWver, etc.
        QByteArray buffer = this->Execute(docroot + "/scripts/hello.sh", QStringList(dataString));
        response.setCommand(cmdString);
        response.setParameter("data", buffer.trimmed());
        response.write();
    }

    else if (cmdString == "INITIALHELLO")
    {
        //This is identical to 'Hello' command except that it will switch to Acces Point mode if necessary
        //To be called only once by JavaScriptCore

        //Returning GUID, DCID, HWver, SWver, etc.
        QByteArray buffer = this->Execute(docroot + "/scripts/initial_hello.sh", QStringList(dataString));
        response.setCommand(cmdString);
        response.setParameter("data", buffer.trimmed());
        response.write();
    }
    */

    if (cmdString == "SETURL")
    {
        //Send it straight to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setParameter("value", "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "No browser running");
        }
        response.write();
    }

    //-----------
    //Mostly from Android/iOS devices

    /* Deprecated. Use generic command at the bottom
    else if (cmdString == "WIDGETENGINE")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/widget_engine.sh", QStringList(dataString));
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setParameter("value", buffer.trimmed());
        response.write();
        buffer = QByteArray();
    }

    else if (cmdString == "CONTROLPANEL")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString));

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setParameter("value", buffer.trimmed());
        response.write();
        buffer = QByteArray();
    }
    */

    else if (cmdString == "REMOTECONTROL")
    {
        //Forward to all clients
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setParameter("value", "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "No browser running");
        }
        response.write();
    }

    else if (cmdString == "KEY")
    {
        //Forward to all clients
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "all");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setParameter("value", "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "No browser running");
        }
        response.write();
    }

    else if (cmdString == "GETSCREENSHOTPATH")
    {
        QByteArray tmpString = QByteArray("http://" +  request.getLocalAddress() + "/framebuffer");
        if (tmpString != "")
        {
            response.setCommand(cmdString);
            response.setParameter("value", tmpString);
            response.write();
        }
    }

    else if (cmdString == "SETSCREENSHOTRES")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setParameter("value", "Screenshot resolution changed");
        response.write();
    }

    else if (cmdString == "SETTIME")
    {
        // <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
        // <timezone> is standard timezone ID string formated as "Asia/Singapore"
        QString time = request.getParameter("time");
        QString timezone = request.getParameter("timezone");

        QStringList argsList;
        argsList.append(time);
        argsList.append(timezone);
        QByteArray buffer = this->Execute(docroot + "/scripts/set_time.sh", argsList);

        response.setCommand(cmdString);
        response.setParameter("data", buffer.trimmed());
        response.write();
    }

    else if (cmdString == "SETNETWORK")
    {
        /*
        qDebug() << "    wifi_password = " << request.getParameter("wifi_password");
        qDebug() << "    wifi_authentication = " << request.getParameter("wifi_authentication");
        qDebug() << "    wifi_encryption = " << request.getParameter("wifi_encryption");
        qDebug() << "    wifi_allocation = " << request.getParameter("wifi_allocation");
        qDebug() << "    wifi_encoding = " << request.getParameter("wifi_encoding");
        qDebug() << "    type = " << request.getParameter("type");
        */

        QHash<QString,QString> params;
        params.insert("type", request.getParameter("type"));
        params.insert("allocation", request.getParameter("wifi_allocation"));
        params.insert("ssid", request.getParameter("wifi_ssid"));
        params.insert("auth", request.getParameter("wifi_authentication"));
        params.insert("encryption", request.getParameter("wifi_encryption"));
        params.insert("key", request.getParameter("wifi_password"));
        params.insert("encoding", request.getParameter("wifi_encoding"));
        bool fileOK = SetNetworkConfig(params);
        params.clear();

        qDebug("NeTVServer: Receive network config for '%s'", request.getParameter("wifi_ssid").constData());

        QByteArray buffer;
        if (!fileOK)        qDebug("Error writing network config file");
        else                qDebug("Writing network config file OK");
        if (!fileOK)        buffer = this->GetFileContents(BRIDGE_NETWORK_CONFIG);
        else                buffer = this->Execute(docroot + "/scripts/stop_ap.sh");
        response.setCommand(cmdString);
        response.setParameter("data", buffer.trimmed());
        response.write();
    }

    else if (cmdString == "SETACCOUNT")
    {
        fprintf(stderr,"Receiving SetAccount command\n");

        QString activated = request.getParameter("activated");
        QString username = request.getParameter("chumby_username");
        QString password = request.getParameter("chumby_password");
        QString device_name = request.getParameter("chumby_device_name");

        if (activated == "")    activated = "false";
        if (device_name == "")  device_name = "NeTV";

        QString account_config = QString("<configuration username=\"%1\" password=\"%2\" device_name=\"%3\" />").arg(username).arg(password).arg(device_name);
        bool fileOK = SetFileContents(BRIDGE_ACCOUNT_CONFIG, account_config.toLatin1());

        //We should ask the JSCore to do something here.
        //QByteArray buffer = this->Execute(docroot + "/scripts/stop_ap.sh");
        QByteArray buffer;
        if (!fileOK)        fprintf(stderr,"Error writing account config file\n");
        else                fprintf(stderr,"Writing account config file OK\n");
        if (!fileOK)        buffer = this->GetFileContents(BRIDGE_ACCOUNT_CONFIG);
        else                activated.toLatin1();

        response.setCommand(cmdString);
        response.setParameter("data", buffer.trimmed());
        response.write();
    }

    //-----------

#if defined (CURSOR_CONTROLLER)
    else if (cmdString == "CURDOWN")
    {
        bool isXOK = true;
        bool isYOK = true;
        float xf = request.getParameter("x").toFloat(&isXOK);
        float yf = request.getParameter("y").toFloat(&isYOK);
        if (isXOK && isYOK)
        {
            QRect screenRect = QApplication::desktop()->screenGeometry();
            int xi = screenRect.width() * xf;
            int yi = screenRect.height() * yf;
            Static::cursorController->send_mouse_move_abs(xi,yi);
            Static::cursorController->send_mouse_down(BTN_LEFT);

            //No response
        }
        else
        {
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "Error in XY value");
            response.write();
        }
    }

    else if (cmdString == "CURMOVE")
    {
        bool isXOK = true;
        bool isYOK = true;
        float xf = request.getParameter("x").toFloat(&isXOK);
        float yf = request.getParameter("y").toFloat(&isYOK);
        if (isXOK && isYOK)
        {
            QRect screenRect = QApplication::desktop()->screenGeometry();
            int xi = screenRect.width() * xf;
            int yi = screenRect.height() * yf;
            Static::cursorController->send_mouse_move_abs(xi,yi);

            //No response
        }
        else
        {
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "Error in XY value");
            response.write();
        }
    }

    else if (cmdString == "CURUP")
    {
        bool isXOK = true;
        bool isYOK = true;
        float xf = request.getParameter("x").toFloat(&isXOK);
        float yf = request.getParameter("y").toFloat(&isYOK);
        if (isXOK && isYOK)
        {
            QRect screenRect = QApplication::desktop()->screenGeometry();
            int xi = screenRect.width() * xf;
            int yi = screenRect.height() * yf;
            Static::cursorController->send_mouse_move_abs(xi,yi);
            Static::cursorController->send_mouse_up(BTN_LEFT);

            //No response
        }
        else
        {
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "Error in XY value");
            response.write();
        }
    }

    else if (cmdString == "CURSORMODE")
    {
        bool isRelative = request.getParameter("value").toUpper() == "RELATIVE";
        if (isRelative)         Static::cursorController->setRelativeMode();
        else                    Static::cursorController->setAbsoluteMode();

        //No response
    }
#endif

    //-----------
    //Mostly from Flash widget_engine.swf

    else if (cmdString == "JAVASCRIPT")
    {
        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(request.getRawData(), "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setParameter("value", "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter("value", "No browser running");
        }
        response.write();
    }

    //-----------

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString));
        response.setCommand(cmdString);
        response.setParameter("data", buffer.trimmed());
        response.write();
    }

    //-----------

    else
    {
        response.setStatus(BRIDGE_RETURN_STATUS_UNIMPLEMENTED);
        response.setParameter("value", "Unimplemented command");
        response.write();
    }
}


//-----------------------------------------------------------------------------------------------------------
// Process Utilities
//-----------------------------------------------------------------------------------------------------------

QByteArray BridgeController::Execute(const QString &fullPath)
{
    return this->Execute(fullPath, QStringList());
}

QByteArray BridgeController::Execute(const QString &fullPath, QStringList args)
{
    if (!SetFileExecutable(fullPath))
        return QByteArray("no permission to execute");

    QProcess *newProc = new QProcess();
    newProc->start(fullPath, args);
    newProc->waitForFinished();

    // Print the output to HTML response & Clean up
    QByteArray buffer = newProc->readAllStandardOutput();
    newProc->close();
    delete newProc;
    newProc = NULL;

    return buffer;
}

//-----------------------------------------------------------------------------------------------------------
// Helper
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::SetNetworkConfig(QHash<QString, QString> parameters)
{
    QString type = parameters.value("type");
    QString allocation = parameters.value("allocation");
    QString ssid = parameters.value("ssid");
    QString auth = parameters.value("auth");
    QString encryption = parameters.value("encryption");
    QString key = parameters.value("key");
    QString encoding = parameters.value("encoding");

    //Debug
    /*
    qDebug() << "Received network config: ";
    QHash<QString, QString>::iterator i;
    for  (i = parameters.begin (); i != parameters.end (); i++)
        qDebug() << i.key() << " = " << i.value();
    */

    if (type == "")         type = "wlan";
    if (auth == "")         auth = "OPEN";
    if (encryption == "")   encryption = "NONE";
    if (encoding == "")     encoding = "hex";
    if (allocation == "")   allocation = "dhcp";

    //No password given or open network
    if (key == "" || encryption == "NONE")
    {
        encoding = "";
        auth = "OPEN";
    }
    else if (encryption == "WEP")
    {
        auth = "WEPAUTO";

        bool isHex = false;
        qulonglong temp = key.toULongLong ( &isHex, 16);
        temp = 0;

        if (isHex && (key.length()==10 || key.length()==26))    encoding = "hex";
        else                                                    encoding = "ascii";
    }
    //WPA
    else
    {
        encoding = "";
    }

    QString network_config = QString("<configuration type=\"%1\" allocation=\"%2\" ssid=\"%3\" auth=\"%4\" encryption=\"%5\" key=\"%6\" encoding=\"%7\" />")
                                                     .arg(type).arg(allocation).arg(ssid).arg(auth).arg(encryption).arg(key).arg(encoding);
    return SetFileContents(BRIDGE_NETWORK_CONFIG, network_config.toLatin1());
}

//-----------------------------------------------------------------------------------------------------------
// File Utilities
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::FileExists(const QString &fullPath)
{
    QFile file(fullPath);
    return file.exists();
}

bool BridgeController::FileExecutable(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())
        return false;
    return ((file.permissions() & QFile::ExeOwner) || (file.permissions() & QFile::ExeOther));
}

qint64 BridgeController::GetFileSize(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())
        return -1;
    return file.size();
}

QByteArray BridgeController::GetFileContents(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())                         //doesn't exist
        return "file not found";
    if (!file.open(QIODevice::ReadOnly))        //permission error?
        return "no permission";
    QByteArray buffer = file.readAll();
    file.close();
    return buffer;
}

bool BridgeController::SetFileContents(const QString &fullPath, QByteArray data)
{
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly))       //permission error?
        return false;
    qint64 actual = file.write(data, data.length());
    if (actual != data.length()) {
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool BridgeController::UnlinkFile(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())                         //doesn't exist
        return true;
    return file.remove();
}

bool BridgeController::SetFileExecutable(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())                         //doesn't exist
        return false;
    if (FileExecutable(fullPath))
        return true;
    return file.setPermissions(file.permissions() | QFile::ExeUser | QFile::ExeOther);
}

//----------------------------------------------------------------------------------------------------
// Public slots
//----------------------------------------------------------------------------------------------------

// From dbusmonitor.h
void BridgeController::slot_StateChanged(uint state)
{
    //Forward it to browser
    qDebug() << "BridgeController: [NMStateChanged] " << state;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMStateChanged</cmd><data><value>") + QString().number(state).toLatin1() + "</value></data></xml>", "netvbrowser");
}

void BridgeController::slot_PropertiesChanged(QByteArray /* prop_name */, QByteArray /* prop_value */)
{
    //Forward it to browser
    //qDebug() << "BridgeController: [NMPropertiesChanged] " << state;
    //Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMPropertiesChanged</cmd><data><value>" + objPath + "</value></data></xml>", "netvbrowser");
}
void BridgeController::slot_DeviceAdded(QByteArray objPath)
{
    //Forward it to browser
    qDebug() << "BridgeController: [NMDeviceAdded] " << objPath;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMDeviceAdded</cmd><data><value>") + objPath + "</value></data></xml>", "netvbrowser");
}

void BridgeController::slot_DeviceRemoved(QByteArray objPath)
{
    //Forward it to browser
    qDebug() << "BridgeController: [NMDeviceRemoved] " << objPath;
    Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>NMDeviceRemoved</cmd><data><value>") + objPath + "</value></data></xml>", "netvbrowser");
}
