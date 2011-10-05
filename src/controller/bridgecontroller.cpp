#include "bridgecontroller.h"
#include "../static.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUrl>
#include <QXmlStreamWriter>
#include <QTimer>


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

    QByteArray cmdString = request.getParameter(STRING_COMMAND).toUpper();
    QByteArray dataString = request.getParameter(STRING_VALUE);
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

    //Allow Authorized-Caller to be passed through HTTP Header or regular POST parameters, or through XML style passing
    QByteArray authorizedCaller = request.getHeader(STRING_AUTHORIZED_CALLER).toUpper();
    if (authorizedCaller.length() < 1)
        authorizedCaller = request.getParameter(STRING_AUTHORIZED_CALLER).toUpper();
    if (authorizedCaller.length() < 1 && xmlparameters.contains(STRING_AUTHORIZED_CALLER))
        authorizedCaller = xmlparameters.value(STRING_AUTHORIZED_CALLER).toUpper();

    //Allow XML style parameters passing
    if (cmdString.length() < 1 && xmlparameters.contains(STRING_COMMAND))
        cmdString = xmlparameters.value(STRING_COMMAND).toUpper();
    if (dataString.length() < 1 && xmlparameters.contains(STRING_VALUE))
        dataString = xmlparameters.value(STRING_VALUE);

    //-----------------------------------------------------------

    if (cmdString == "SETURL")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        //Send it straight to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to browser</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser found</value></data></xml>", true);
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

    else if (cmdString == "ENABLESSH")
    {
        if (dataString.length() < 1)
            dataString = "start-chumby";
        QByteArray buffer = this->Execute("/etc/init.d/sshd", QStringList(QString(dataString)));

        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
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
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to widget rendering engine</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No widget rendering engine found</value></data></xml>", true);
    }

    else if (cmdString == "SETWIDGETSIZE")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "widget_engine");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to widget rendering engine</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No widget rendering engine found</value></data></xml>", true);
    }

    else if (cmdString == "REMOTECONTROL")
    {
        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>", "all");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>Command forwarded to browser</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>", true);
    }

    else if (cmdString == "KEY")
    {
        //Forward to all clients
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataString + "</data></xml>", "all");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No client running</value></data></xml>", true);
    }

    else if (cmdString == "ANDROID" || cmdString == "IOS")
    {
        QByteArray eventName, eventData;

        //Allow user to use both normal POST style API as well as XML style passing
        if (xmlparameters.size() <= 0)
        {
            eventName = request.getParameter("eventname").trimmed();
            eventData = request.getParameter("eventdata").trimmed();
        }
        else
        {
            eventName = xmlparameters.value("eventname", "").trimmed();
            eventData = xmlparameters.value("eventdata", "").trimmed();
        }
        QByteArray javaScriptString = "fAndroidEvents(\"" + eventName + "\",\"" + eventData + "\");";

        //Forward to browser only
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><data><value>") + javaScriptString + "</value></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No client running</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "CONTROLPANEL")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        //This is simply passed directly to NeTVBrowser
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString));
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "TICKEREVENT")
    {
        //All these should be URI encoded before passing to JavaScript
        //Allow user to use both normal POST style API as well as XML style passing
        QByteArray title, message, image, type, level;
        if (xmlparameters.size() > 0)
        {
            title = QUrl::toPercentEncoding(QString(xmlparameters.value("title", "")), "", "/'\"");
            message = QUrl::toPercentEncoding(QString(xmlparameters.value("message", "")), "", "/'\"");
            image = QUrl::toPercentEncoding(QString(xmlparameters.value("image", "")), "", "/'\"");
            type = QUrl::toPercentEncoding(QString(xmlparameters.value("type", "")), "", "/'\"");
            level = QUrl::toPercentEncoding(QString(xmlparameters.value("level", "")), "", "/'\"");
        }
        else
        {
            title = QUrl::toPercentEncoding(QString(request.getParameter("title")), "", "/'\"");
            message = QUrl::toPercentEncoding(QString(request.getParameter("message")), "", "/'\"");
            image = QUrl::toPercentEncoding(QString(request.getParameter("image")), "", "/'\"");
            type = QUrl::toPercentEncoding(QString(request.getParameter("type")), "", "/'\"");
            level = QUrl::toPercentEncoding(QString(request.getParameter("level")), "", "/'\"");
        }
        QByteArray javaScriptString = "fTickerEvents(\"" + message + "\",\"" + title + "\",\"" + image + "\",\"" + type + "\",\"" + level + "\");";

        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><data><value>") + javaScriptString + "</value></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>OK</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>", true);
    }

    else if (cmdString == "SETIFRAME" || cmdString == "MULTITAB")
    {
        QByteArray param, options, tab;
        if (xmlparameters.size() > 0)
        {
            param = xmlparameters.value("param", "");
            options = xmlparameters.value("options", "");
            tab = xmlparameters.value("tab", "");
        }
        else
        {
            param = request.getParameter("param");
            options = request.getParameter("options");
            tab = request.getParameter("tab");
        }

        //Forward to browser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>" + cmdString + "</cmd><data><param>") + param + "</param><options>" + options + "</options><tab>" + tab + "</tab></data></xml>", "netvbrowser");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>OK</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "SETTIME")
    {
        // <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
        // <timezone> is standard timezone ID string formated as "Asia/Singapore"
        // see http://developer.android.com/reference/java/util/TimeZone.html#getID()
        QString time, timezone;
        if (xmlparameters.size() > 0)
        {
            time = xmlparameters.value("time", "");
            timezone = xmlparameters.value("timezone", "");
        }
        else
        {
            time = request.getParameter("time");
            timezone = request.getParameter("timezone");
        }

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
        bool isTest = false;
        if (xmlparameters.size() > 0)
        {
            params.insert("allocation", xmlparameters.value("wifi_allocation",""));
            params.insert("ssid", xmlparameters.value("wifi_ssid",""));
            params.insert("auth", xmlparameters.value("wifi_authentication",""));
            params.insert("encryption", xmlparameters.value("wifi_encryption",""));
            params.insert("key", xmlparameters.value("wifi_password",""));
            params.insert("encoding", xmlparameters.value("wifi_encoding",""));
            params.insert("type", xmlparameters.value("type",""));
            params.insert("test", xmlparameters.value("test",""));
            isTest = xmlparameters.value("test","").length() > 1;
        }
        else
        {
            params.insert("allocation", request.getParameter("wifi_allocation"));
            params.insert("ssid", request.getParameter("wifi_ssid"));
            params.insert("auth", request.getParameter("wifi_authentication"));
            params.insert("encryption", request.getParameter("wifi_encryption"));
            params.insert("key", request.getParameter("wifi_password"));
            params.insert("encoding", request.getParameter("wifi_encoding"));
            params.insert("type", request.getParameter("type"));
            params.insert("test", request.getParameter("test"));
            isTest = request.getParameter("test").length() > 1;
        }
        bool fileOK = SetNetworkConfig(params);
        params.clear();

        //Report the content of network_config
        QByteArray buffer;
        if (isTest)                 buffer = this->GetFileContents(networkConfigFile + "_test");
        else                        buffer = this->GetFileContents(networkConfigFile);

        //Reply to JavaScriptCore/ControlPanel
        if (!fileOK)                response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);

        //Allow time for HTTP response to complete before we bring down the network
        if (!isTest)
            StopAPwithDelay(500);
    }

    else if (cmdString == "SETACCOUNT")
    {
        //Do NOT use this command. Not safe to store username/password in /psp

        QString activated = request.getParameter("activated");
        QString username = request.getParameter("chumby_username");
        QString password = request.getParameter("chumby_password");
        QString device_name = request.getParameter("chumby_device_name");

        if (activated == "")        activated = "false";
        if (device_name == "")      device_name = "NeTV";

        QString account_config = QString("<configuration username=\"%1\" password=\"%2\" device_name=\"%3\" />").arg(username).arg(password).arg(device_name);
        bool fileOK = SetFileContents(accountConfigFile, account_config.toLatin1());

        //We should ask the JSCore to do something here.
        //QByteArray buffer = this->Execute(docroot + "/scripts/stop_ap.sh");
        QByteArray buffer;
        if (!fileOK)                buffer = this->GetFileContents(accountConfigFile);
        else                        buffer = activated.toLatin1();

        //Reply to JavaScriptCore/ControlPanel
        if (!fileOK)                response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "STOPAP")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>", true);
        StopAPwithDelay();
    }
    else if (cmdString == "STARTAP")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>", true);
        StartAPwithDelay();
    }
    else if (cmdString == "STARTAPFACTORY")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>", true);
        StartAPFactorywithDelay();
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
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        bool success = UnlinkFile(dataString);
        if (!success)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>false</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>true</value></data></xml>", true);
    }

    else if (cmdString == "MD5FILE")
    {
        if (!FileExists(dataString)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>file not found</message><path>" + dataString + "</path></data></xml>", true);
            return;
        }
        QByteArray md5sum = GetFileMD5(dataString);
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><path>" + dataString + "</path><md5sum>" + md5sum + "</md5sum></data></xml>", true);
    }

    //-----------

    else if (cmdString == "DOWNLOADFILE")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        DumpStaticFile(dataString, response);
    }

    else if (cmdString == "UPLOADFILE")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        QString pathString = QString(request.getParameter("path"));

        //Clear existing file (if any)
        bool cleanFile = true;
        if (FileExists(pathString))
            cleanFile = UnlinkFile(pathString);
        if (!cleanFile) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>unable to clean existing file</message><path>" + pathString.toLatin1() + "</path></data></xml>", true);
            return;
        }

        QTemporaryFile* file = request.getUploadedFile("filedata");
        if (!file) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>uploaded file data not found</message><path>" + pathString.toLatin1() + "</path></data></xml>", true);
            return;
        }

        //Write the buffer to non-volatile disk
        //MountRW();
        bool success = file->copy( pathString );
        //MountRO();

        if (!success) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>error writing to disk (read-only partition?)</message><path>" + pathString.toLatin1() + "</path></data></xml>", true);
            return;
        }

        quint64 filesize = GetFileSize(pathString);
        if (filesize <= 0) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>invalid filesize</message><path>" + pathString.toLatin1() + "</path></data></xml>", true);
            return;
        }
        QByteArray sizeStr = QByteArray().setNum(filesize);
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><path>" + pathString.toLatin1() + "</path><filesize>" + sizeStr + "</filesize></data></xml>", true);
    }

    //-----------

    else if (cmdString == "NECOMMAND")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        QList<QByteArray> newArgsList = dataString.split(' ');
        QString command = QString(newArgsList.at(0));

        QStringList newArgs;
        newArgsList.removeAt(0);
        for (int i=0; i<newArgsList.length(); i++)
            newArgs << newArgsList.at(i);
        QByteArray buffer = this->Execute(command, newArgs);

        if (buffer.length() > 5)        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "REBOOT")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        QByteArray buffer = this->Execute("/sbin/reboot");
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString));
        if (buffer.length() > 5)        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" + buffer.trimmed() + "</data></xml>", true);
        else                            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data>" + buffer.trimmed() + "</data></xml>", true);
        buffer = QByteArray();
    }

    //-----------

    else
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><cmd>" + cmdString + "</cmd><data><value>Check for valid 'cmd' variable</value></data></xml>", true);
    }
}


void BridgeController::service(SocketRequest& request, SocketResponse& response)
{
    QByteArray cmdString = request.getCommand().toUpper();
    QByteArray dataString = request.getParameter(STRING_VALUE).trimmed();
    QByteArray authorizedCaller = request.getParameter(STRING_AUTHORIZED_CALLER).toUpper();

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

    else if (cmdString == "ANDROID" || cmdString == "IOS")
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

    else if (cmdString == "SETIFRAME" || cmdString == "MULTITAB")
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

    else if (cmdString == "TICKEREVENT")
    {
        //All these should be URI encoded
        QByteArray message = QUrl::toPercentEncoding(QString(request.getParameter("message").trimmed()), "", "/'\"");
        QByteArray title = QUrl::toPercentEncoding(QString(request.getParameter("title").trimmed()), "", "/'\"");
        QByteArray image = QUrl::toPercentEncoding(QString(request.getParameter("image").trimmed()), "", "/'\"");
        QByteArray type = QUrl::toPercentEncoding(QString(request.getParameter("type").trimmed()), "", "/'\"");
        QByteArray level = QUrl::toPercentEncoding(QString(request.getParameter("level").trimmed()), "", "/'\"");
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

    else if (cmdString == "SETTIME")
    {
        // <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
        // <timezone> is standard timezone ID string formated as "Asia/Singapore"
        // see http://developer.android.com/reference/java/util/TimeZone.html#getID()
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
        params.insert("allocation", request.getParameter("wifi_allocation"));
        params.insert("ssid", request.getParameter("wifi_ssid"));
        params.insert("auth", request.getParameter("wifi_authentication"));
        params.insert("encryption", request.getParameter("wifi_encryption"));
        params.insert("key", request.getParameter("wifi_password"));
        params.insert("encoding", request.getParameter("wifi_encoding"));
        params.insert("type", request.getParameter("type"));
        params.insert("test", request.getParameter("test"));
        bool isTest = request.getParameter("test").length() > 1;
        bool fileOK = SetNetworkConfig(params);
        params.clear();

        qDebug("NeTVServer: Receive network config for '%s'", request.getParameter("wifi_ssid").constData());

        //Report the content of network_config
        QByteArray buffer;
        if (!fileOK)                qDebug("%s, Error writing network config file", TAG);
        else                        qDebug("%s, Writing network config file OK", TAG);

        if (isTest)                 buffer = this->GetFileContents(networkConfigFile + "_test");
        else                        buffer = this->GetFileContents(networkConfigFile);

        response.setStatus(fileOK ? BRIDGE_RETURN_STATUS_SUCCESS : BRIDGE_RETURN_STATUS_ERROR);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
        response.write();

        //Allow time for socket response to complete before we bring down the network
        if (!isTest)
            StopAPwithDelay(500);
    }

    else if (cmdString == "SETACCOUNT")
    {
        //Do NOT use this command. Not safe to store username/password in /psp
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
        if (!fileOK)                qDebug("%s, Error writing account config file", TAG);
        else                        qDebug("%s, Writing account config file OK", TAG);

        if (!fileOK)                buffer = this->GetFileContents(accountConfigFile);
        else                        buffer = activated.toLatin1();

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

    else if (cmdString == "GETFILECONTENTS")
    {
        QByteArray filedata = GetFileContents(dataString);
        if (filedata == "file not found" || filedata == "no permission")        response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                                                                    response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, filedata);
        response.setParameter("path", dataString);
        response.write();
    }

    else if (cmdString == "SETFILECONTENTS")
    {
        //<path>full path to a file</path><content>........</content>
        QByteArray path = request.getParameter("path");
        QByteArray content = request.getParameter("content");
        if (!SetFileContents(path,content))        response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                                       response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, QByteArray().setNum(GetFileSize(path)));
        response.setParameter("path", path);
        response.write();
    }

    else if (cmdString == "FILEEXISTS")
    {
        QByteArray existStr = "false";
        if (FileExists(dataString))
            existStr = "true";
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, existStr);
        response.setParameter("path", dataString);
        response.write();
    }

    else if (cmdString == "GETFILESIZE")
    {
        QByteArray sizeStr = QByteArray().setNum(GetFileSize(dataString));
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, sizeStr);
        response.setParameter("path", dataString);
        response.write();
    }

    else if (cmdString == "UNLINKFILE")
    {
        bool success = UnlinkFile(dataString);
        if (!success)                       response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                                response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, QByteArray(success ? "true" : "false"));
        response.setParameter("path", dataString);
        response.write();
    }

    else if (cmdString == "MD5FILE")
    {
        QByteArray md5sum = GetFileMD5(dataString);
        if (!FileExists(dataString))        response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                                response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, md5sum);
        response.setParameter("path", dataString);
        response.write();
    }

    //-----------

    else if (cmdString == "STOPAP")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.write();
        StopAPwithDelay();
    }
    else if (cmdString == "STARTAP")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.write();
        StartAPwithDelay();
    }
    else if (cmdString == "STARTAPFACTORY")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.write();
        StartAPFactorywithDelay();
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

    else if (cmdString == "NECOMMAND")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.setStatus(BRIDGE_RETURN_STATUS_UNAUTHORIZED);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, "Unauthorized");
            response.write();
            return;
        }

        QList<QByteArray> newArgsList = dataString.split(' ');
        QString command = QString(newArgsList.at(0));

        QStringList newArgs;
        newArgsList.removeAt(0);
        for (int i=0; i<newArgsList.length(); i++)
            newArgs << newArgsList.at(i);
        QByteArray buffer = this->Execute(command, newArgs);

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, buffer.trimmed());
        response.write();
    }

    else if (cmdString == "ENABLESSH")
    {
        if (dataString.length() < 1)
            dataString = "start-chumby";
        QByteArray buffer = this->Execute("/etc/init.d/sshd", QStringList(QString(dataString)));

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, buffer.trimmed());
        response.write();
    }

    else if (cmdString == "REBOOT")
    {
        QByteArray buffer = this->Execute("reboot");

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, buffer.trimmed());
        response.write();
    }

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
        response.setParameter(STRING_VALUE, "Check for valid 'cmd' variable");
        response.write();
    }
}




//-----------------------------------------------------------------------------------------------------------
// High level helper functions
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::IsAuthorizedCaller(QByteArray headerValue)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 second1 = (now - 1000) / 1000;
    qint64 second2 = now / 1000;
    qint64 second3 = (now + 1000) / 1000;

    QByteArray hashResult1 = QCryptographicHash::hash(QByteArray().setNum(second1),QCryptographicHash::Md5).toHex().toUpper();
    QByteArray hashResult2 = QCryptographicHash::hash(QByteArray().setNum(second2),QCryptographicHash::Md5).toHex().toUpper();
    QByteArray hashResult3 = QCryptographicHash::hash(QByteArray().setNum(second3),QCryptographicHash::Md5).toHex().toUpper();

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
    //We first need to HTML-unescape the parameters in case of HTTP version (also in test version)
    QString ssid = XMLUnescape( parameters.value("ssid") );
    QString key = XMLUnescape( parameters.value("key") );

    QString type = parameters.value("type");
    QString allocation = parameters.value("allocation");
    QString auth = parameters.value("auth");
    QString encryption = parameters.value("encryption");
    QString encoding = parameters.value("encoding");
    bool isTest = parameters.value("test").length() > 1;

    if (type == "")         type = "wlan";
    if (auth == "")         auth = "OPEN";
    if (encryption == "")   encryption = "NONE";
    if (encoding == "")     encoding = "hex";
    if (allocation == "")   allocation = "dhcp";

    auth = auth.toUpper();
    if (key == "" || auth == "" || auth == "OPEN")
    {
        encryption = "NONE";
        auth = "OPEN";
        encoding = "";
    }
    else if (auth.contains("WEP"))
    {
        auth = "WEPAUTO";
        encryption = "WEP";

        bool isHex = IsHexString(key);
        if (isHex && (key.length()==10 || key.length()==26))   encoding = "hex";        //this is a good condition
        else if (key.length()==5 || key.length()==13)          encoding = "hex";        //NetworkManager will convert each character to ascii-code in hex directly
        else                                                   encoding = "ascii";      //NetworkManager will md5 this & cut first 26 characters
    }
    else
    {
        encryption = "AES";             //will also work for TKIP
        auth = "WPA2PSK";               //will also work for WPA
        encoding = "";
    }

    //----------------

    QString filename = networkConfigFile;
    if (isTest)
        filename += "_test";

    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug("%s, cannot write network config file %s", TAG, qPrintable(filename) );
        return false;
    }

    //Note that QXmlStreamWriter doesn't escape apostrophe (')
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.writeStartElement("configuration");
    if (type.length() > 0)              xmlWriter.writeAttribute("type", type);
    if (allocation.length() > 0)        xmlWriter.writeAttribute("allocation", allocation);
    if (ssid.length() > 0)              xmlWriter.writeAttribute("ssid", ssid);
    if (auth.length() > 0)              xmlWriter.writeAttribute("auth", auth);
    if (encryption.length() > 0)        xmlWriter.writeAttribute("encryption", encryption);
    if (key.length() > 0)               xmlWriter.writeAttribute("key", key);
    if (encoding.length() > 0)          xmlWriter.writeAttribute("encoding", encoding);
    xmlWriter.writeEndElement();
    file.close();
    if (file.error())
    {
        qDebug("%s, cannot write network config file %s", TAG, qPrintable(filename) );
        return false;
    }

    //Make sure the file is written to disk
    Sync();
    return true;
}

void BridgeController::StartAPwithDelay(int msec /* = 500 */)
{
    QTimer::singleShot(msec, this, SLOT(slot_StartAP()));
}

void BridgeController::StartAPFactorywithDelay(int msec /* = 500 */)
{
    QTimer::singleShot(msec, this, SLOT(slot_StartAP_Factory()));
}

void BridgeController::StopAPwithDelay(int msec /* = 500 */)
{
    QTimer::singleShot(msec, this, SLOT(slot_StopAP()));
}
