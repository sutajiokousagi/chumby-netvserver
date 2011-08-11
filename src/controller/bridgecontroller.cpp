#include "bridgecontroller.h"
#include "../static.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QCryptographicHash>

#define BRIDGE_RETURN_STATUS_UNIMPLEMENTED  "0"
#define BRIDGE_RETURN_STATUS_SUCCESS        "1"
#define BRIDGE_RETURN_STATUS_ERROR          "2"
#define BRIDGE_RETURN_STATUS_UNAUTHORIZED   "3"

#define ARGS_SPLIT_TOKEN    "|~|"

#define STRING_AUTHORIZED_CALLER            "Authorized-Caller"
#define STRING_COMMAND                      "cmd"
#define STRING_DATA                         "data"
#define STRING_VALUE                        "value"

BridgeController::BridgeController(QSettings* settings, QObject* parent) : QObject(parent), HttpRequestHandler(), SocketRequestHandler()
{
    //Settings from NeTVServer.ini
    docroot=settings->value("path",".").toString();
    paramsFile=settings->value("paramsFile",".").toString();
    networkConfigFile=settings->value("networkConfigFile",".").toString();
    accountConfigFile=settings->value("accountConfigFile",".").toString();

    //Load non-volatile parameters
    parameters = NULL;
    LoadParameters();

    qDebug("BridgeController: path=%s",qPrintable(docroot));
    qDebug("BridgeController: paramsFile=%s",qPrintable(paramsFile));
}

BridgeController::~BridgeController()
{
    SaveParameters();
    delete parameters;
    parameters = NULL;
}

void BridgeController::service(HttpRequest& request, HttpResponse& response)
{
    //Protocol documentation
    //https://internal.chumby.com/wiki/index.php/JavaScript/HTML_-_Hardware_Bridge_protocol

    QByteArray authorizedCaller = request.getHeader(STRING_AUTHORIZED_CALLER).toUpper();
    QByteArray cmdString = request.getParameter(STRING_COMMAND).toUpper();
    QByteArray dataString = request.getParameter(STRING_VALUE).toUpper();
    QByteArray dataXmlString = request.getParameter(STRING_DATA);

    //A specialize QHash for received parameters in XML format
    QHash<QByteArray,QByteArray> xmlparameters;
    QMapIterator<QByteArray,QByteArray> i(request.getParameterMap());
    while (i.hasNext())
    {
        i.next();
        if (i.key().startsWith("dataxml_"))
            xmlparameters.insert( QByteArray(i.key()).remove(0, 8), i.value() );
    }

    //-----------

    if (cmdString == "SETURL")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd></xml>", true);
            return;
        }

        //Send it straight to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to browser</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser found</value></data></xml>", true);
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
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>no arguments received</value></data></xml>", true);
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/geturl.sh", argsList);
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
            buffer = QByteArray();
        }
    }

    else if (cmdString == "GETJPG" || cmdString == "GETJPEG")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString));
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "HASFLASHPLUGIN")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>false</value></data></xml>", true);
    }

    else if (cmdString == "PLAYWIDGET")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to widget rendering engine</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No widget rendering engine found</value></data></xml>", true);
    }

    //-----------

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
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>Invalid arguments</value></data></xml>", true);
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList);
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
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
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }


    else if (cmdString == "PLAYSWF")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to widget rendering engine</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No widget rendering engine found</value></data></xml>", true);
    }

    else if (cmdString == "SETWIDGETSIZE")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Set the window size
        this->Execute(docroot + "/scripts/setbox.sh", QStringList(dataString));

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to widget rendering engine</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No widget rendering engine found</value></data></xml>", true);
    }

    else if (cmdString == "REMOTECONTROL")
    {
        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "all");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to browser</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "CONTROLPANEL")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd></xml>", true);
            return;
        }

        //This is simply passed directly to NeTVBrowser
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString));
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "TICKEREVENT")
    {
        //All these should already be URI encoded
        QByteArray message = xmlparameters.value("message", "");
        QByteArray title = xmlparameters.value("title", "");
        QByteArray image = xmlparameters.value("image", "");
        QByteArray type = xmlparameters.value("type", "");
        QByteArray level = xmlparameters.value("level", "");
        QByteArray javaScriptString = "fTickerEvents(\"" + message + "\",\"" + title + "\",\"" + image + "\",\"" + type + "\",\"" + level + "\");";

        //Forward to browser only
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><data><value>") + javaScriptString + "</value></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>OK</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>", true);
    }

    else if (cmdString == "KEY")
    {
        //Forward to all clients
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataString + "</data></xml>", "all");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No client running</value></data></xml>", true);
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
        if (time == "")    response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>time parameter is required</value></data></xml>", true);
        else               response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
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

        //Stop AP Mode & start NetworkManager
        QByteArray buffer;
        if (!fileOK)        buffer = this->GetFileContents(networkConfigFile);
        else                buffer = this->Execute(docroot + "/scripts/stop_ap.sh");

        //Reply to JavaScriptCore/ControlPanel
        if (!fileOK)        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
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
        bool fileOK = SetFileContents(accountConfigFile, account_config.toLatin1());

        //We should ask the JSCore to do something here.
        //QByteArray buffer = this->Execute(docroot + "/scripts/stop_ap.sh");
        QByteArray buffer;
        if (!fileOK)        buffer = this->GetFileContents(accountConfigFile);
        else                activated.toLatin1();

        //Reply to JavaScriptCore/ControlPanel
        if (!fileOK)        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "GETALLPARAMS")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" +  GetAllParameters() + "</data></xml>", true);
    }

    else if (cmdString == "GETPARAM")
    {
        QByteArray value = GetParameter(dataString);
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + value + "</value></data></xml>", true);
    }

    else if (cmdString == "SETPARAM")
    {
        QHashIterator<QByteArray, QByteArray> i(xmlparameters);
        while (i.hasNext())
        {
            i.next();
            SetParameter( i.key(), i.value() );
        }
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + QByteArray().setNum(xmlparameters.size()) + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "GETFILECONTENTS")
    {
        QByteArray filedata = GetFileContents(dataString);
        if (filedata == "file not found" || filedata == "no permission")
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + filedata + "</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + filedata + "</value></data></xml>", true);
    }

    else if (cmdString == "SETFILECONTENTS")
    {
        //<path>full path to a file</path><content>........</content>
        QByteArray path = dataXmlString.mid(6, dataXmlString.indexOf("</path>") - 6);
        QByteArray content = dataXmlString.mid(dataXmlString.indexOf("<content>") + 9, dataXmlString.length()-dataXmlString.indexOf("<content>")-9);
        if (!SetFileContents(path,content))
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>0</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + QByteArray().setNum(GetFileSize(path)) + "</value></data></xml>", true);
    }

    else if (cmdString == "FILEEXISTS")
    {
        QByteArray existStr = "false";
        if (FileExists(dataString))
            existStr = "true";
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + existStr + "</value></data></xml>", true);
    }

    else if (cmdString == "GETFILESIZE")
    {
        QByteArray sizeStr = QByteArray().setNum(GetFileSize(dataString));
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + sizeStr + "</value></data></xml>", true);
    }

    else if (cmdString == "UNLINKFILE")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd></xml>", true);
            return;
        }

        bool success = UnlinkFile(dataString);
        if (!success)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>false</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>true</value></data></xml>", true);
    }

    //-----------

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString));
        if (buffer.length() > 5)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" + buffer.trimmed() + "</data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data>" + buffer.trimmed() + "</data></xml>", true);
        buffer = QByteArray();
    }

    //-----------

    else
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><cmd>" + cmdString + "</cmd><data><value>" + cmdString + "</value></data></xml>", true);
    }
}


void BridgeController::service(SocketRequest& request, SocketResponse& response)
{
    QByteArray cmdString = request.getCommand().toUpper();
    QByteArray dataString = request.getParameter(STRING_VALUE).trimmed();

    if (cmdString == "SETURL")
    {
        //Send it straight to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "No browser running");
        }
        response.write();
    }

    //-----------
    //Mostly from Android/iOS devices

    else if (cmdString == "REMOTECONTROL")
    {
        //Forward to all clients
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "No browser running");
        }
        response.write();
    }

    else if (cmdString == "ANDROID")
    {
        QByteArray eventName = request.getParameter("eventname").trimmed();
        QByteArray eventData = request.getParameter("eventdata").trimmed();     //already URI encoded
        QByteArray javaScriptString = "fAndroidEvents(\"" + eventName + "\",\"" + eventData + "\");";

        //Forward to browser only
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><data><value>") + javaScriptString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "No browser running");
        }
        response.write();
    }

    else if (cmdString == "TICKEREVENT")
    {
        //All these should already be URI encoded
        QByteArray message = request.getParameter("message").trimmed();
        QByteArray title = request.getParameter("title").trimmed();
        QByteArray image = request.getParameter("image").trimmed();
        QByteArray type = request.getParameter("type").trimmed();
        QByteArray level = request.getParameter("level").trimmed();
        QByteArray javaScriptString = "fTickerEvents(\"" + message + "\",\"" + title + "\",\"" + image + "\",\"" + type + "\",\"" + level + "\");";

        //Forward to browser only
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><data><value>") + javaScriptString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "No browser running");
        }
        response.write();
    }

    else if (cmdString == "KEY")
    {
        //Forward to browser only
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "No browser running");
        }
        response.write();
    }

    else if (cmdString == "GETSCREENSHOTPATH")
    {
        QByteArray tmpString = QByteArray("http://" +  request.getLocalAddress() + "/framebuffer");
        if (tmpString != "")
        {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, tmpString);
            response.write();
        }
    }

    else if (cmdString == "SETSCREENSHOTRES")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, "Screenshot resolution changed");
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

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
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
        if (!fileOK)        buffer = this->GetFileContents(networkConfigFile);
        else                buffer = this->Execute(docroot + "/scripts/stop_ap.sh");

        response.setStatus(fileOK ? BRIDGE_RETURN_STATUS_SUCCESS : BRIDGE_RETURN_STATUS_ERROR);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
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
        bool fileOK = SetFileContents(accountConfigFile, account_config.toLatin1());

        //We should ask the JSCore to do something here.
        //QByteArray buffer = this->Execute(docroot + "/scripts/stop_ap.sh");
        QByteArray buffer;
        if (!fileOK)        fprintf(stderr,"Error writing account config file\n");
        else                fprintf(stderr,"Writing account config file OK\n");
        if (!fileOK)        buffer = this->GetFileContents(accountConfigFile);
        else                activated.toLatin1();

        response.setStatus(fileOK ? BRIDGE_RETURN_STATUS_SUCCESS : BRIDGE_RETURN_STATUS_ERROR);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
        response.write();
    }

    //-----------

    else if (cmdString == "GETALLPARAMS")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, GetAllParameters());
    }

    else if (cmdString == "GETPARAM")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, GetParameter(dataString));
        response.write();
    }

    else if (cmdString == "SETPARAM")
    {
        int count = 0;
        QMapIterator<QByteArray, QByteArray> i(request.getParameters());
        while (i.hasNext())
        {
            i.next();
            if (i.key() == "data" || i.key() == "cmd" || i.key() == "status" || i.key() == "value")
                continue;
            count++;
            SetParameter( i.key(), i.value() );
        }
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, QByteArray().setNum(count));
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
            response.setParameter(STRING_VALUE, "Error in XY value");
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
            response.setParameter(STRING_VALUE, "Error in XY value");
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
            response.setParameter(STRING_VALUE, "Error in XY value");
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
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Command forwarded to browser");
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "No browser running");
        }
        response.write();
    }

    //-----------

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString));
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
        response.write();
    }

    //-----------

    else
    {
        response.setStatus(BRIDGE_RETURN_STATUS_UNIMPLEMENTED);
        response.setParameter(STRING_VALUE, "Unimplemented command");
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

bool BridgeController::IsAuthorizedCaller(QByteArray headerValue)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 second1 = (now - 1000) / 1000;
    qint64 second2 = now / 1000;
    qint64 second3 = (now + 1000) / 1000;

    QByteArray hashResult1 = QCryptographicHash::hash(QByteArray().setNum(second1),QCryptographicHash::Md5).toHex();
    QByteArray hashResult2 = QCryptographicHash::hash(QByteArray().setNum(second2),QCryptographicHash::Md5).toHex();
    QByteArray hashResult3 = QCryptographicHash::hash(QByteArray().setNum(second3),QCryptographicHash::Md5).toHex();

    qDebug() << "Received hash: " << headerValue;
    qDebug() << "Calculated hash: " << hashResult1;
    qDebug() << "Calculated hash: " << hashResult2;
    qDebug() << "Calculated hash: " << hashResult3;

    return true;

    if (headerValue == hashResult1 || headerValue == hashResult2 || headerValue == hashResult3)
        return true;
    return false;
}

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
    return SetFileContents(networkConfigFile, network_config.toLatin1());
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


//----------------------------------------------------------------------------------------------------
// Non-volatile parameters
//----------------------------------------------------------------------------------------------------

QByteArray BridgeController::GetParameter(QString name)
{
    return parameters->value(name, "").toByteArray();
}

QByteArray BridgeController::GetAllParameters()
{
    QByteArray paramsString = "";
    QStringList allkeys = parameters->allKeys();
    for (int i = 0; i < allkeys.size(); ++i)
    {
        QByteArray name = allkeys.at(i).toLatin1();
        QByteArray value = GetParameter(name);
        paramsString.append("<" + name + ">" + value + "</" + name + ">");
    }
    return paramsString;
}

void BridgeController::SetParameter(QString name, QString value)
{
    parameters->setValue(name, value);
    SaveParameters();
}

void BridgeController::LoadParameters(QString * filename /* = NULL */)
{
    if (parameters != NULL)        parameters->sync();
    else if (filename == NULL)     parameters = new QSettings(paramsFile, QSettings::IniFormat);
    else                           parameters = new QSettings(*filename, QSettings::IniFormat);
}

void BridgeController::SaveParameters(QString * filename /* = NULL */)
{
    if (parameters != NULL)        parameters->sync();
    else if (filename == NULL)     parameters = new QSettings(paramsFile, QSettings::IniFormat);
    else                           parameters = new QSettings(*filename, QSettings::IniFormat);
}
