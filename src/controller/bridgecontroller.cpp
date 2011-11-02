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
    //Documentation
    //http://wiki.chumby.com/index.php/NeTV_web_services

    //Allow Authorized-Caller to be passed through HTTP Header or regular POST parameters, or through XML style passing
    QByteArray authorizedCaller = request.getHeader(STRING_AUTHORIZED_CALLER).toUpper();
    if (authorizedCaller.length() < 1)
        authorizedCaller = request.getParameter(STRING_AUTHORIZED_CALLER).toUpper();
    request.removeParameter(STRING_AUTHORIZED_CALLER);

    //Extract the command name & most commonly used 'value' parameter (if any)
    QByteArray cmdString = request.getParameter(STRING_COMMAND).toUpper();
    QByteArray dataString = request.getParameter(STRING_VALUE);
    QByteArray dataXmlString = request.getParameter(STRING_DATA);
    QByteArray rawXmlString = request.getParameterMapXML();
    request.removeParameter(STRING_COMMAND);
    request.removeParameter(STRING_VALUE);

    //This happens when we POST 'value=something' (and not 'data=<value>something</value>' in XML format)
    if (dataString.length() != 0 && dataXmlString.length() == 0)
        dataXmlString = "<value>" + dataString + "</value>";

    //Some commands requires XML escape. This would be an option.
    QByteArray xmlEscapeString = request.getParameter(STRING_XML_ESCAPE).toUpper();
    bool xmlEscape = (xmlEscapeString == "1" || xmlEscapeString == "TRUE" || xmlEscapeString == "YES" || xmlEscapeString == "ON") ? true : false;
    request.removeParameter(STRING_XML_ESCAPE);

    //-----------------------------------------------------------

    if (cmdString == "GETURL")
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
            QByteArray buffer = this->Execute(docroot + "/scripts/geturl.sh", argsList, xmlEscape);
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
            buffer = QByteArray();
        }
    }

    else if (cmdString == "GETJPG" || cmdString == "GETJPEG")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString), xmlEscape);
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "SETCHROMAKEY" || cmdString == "CHROMAKEY")
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
            QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList, xmlEscape);
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
            buffer = QByteArray();
        }
    }

    else if (cmdString == "ENABLESSH" || cmdString == "SSH")
    {
        if (dataString.length() < 1)
            dataString = "start-chumby";
        QByteArray buffer = this->Execute("/etc/init.d/sshd", QStringList(QString(dataString)), xmlEscape);

        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "SETDOCROOT")
    {
        QByteArray current_docroot = SetStaticDocroot(dataString).toLatin1();
        if (xmlEscape)
            current_docroot = XMLEscape(current_docroot);
        if (current_docroot.length() < 1)       response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>path not found</value></data></xml>", true);
        else                                    response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + current_docroot.trimmed() + "</value></data></xml>", true);
    }

    else if (cmdString == "GETDOCROOT")
    {
        QByteArray current_docroot = GetStaticDocroot().toLatin1();
        if (xmlEscape)
            current_docroot = XMLEscape(current_docroot);
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + current_docroot.trimmed() + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "REMOTECONTROL" || cmdString == "REMOTE" || cmdString == "KEY" || cmdString == "KEYBOARD")
    {
        //Forward to all TCP clients
        int numClient = Static::tcpSocketServer->broadcast(rawXmlString, "all");

        //Reply to HTTP client
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + STRING_COMMAND_FORWARDED_CLIENT + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + STRING_NO_CLIENT_RUNNING + "</value></data></xml>", true);
    }

    else if (cmdString == "SETURL" || cmdString == "MULTITAB" || cmdString == "TAB" || cmdString == "KEEPALIVE" || cmdString == "JAVASCRIPT" || cmdString == "JS"
             || cmdString == "UPDATEREADY" || cmdString == "UPDATEDONE" || cmdString == "UPGRADEDONE")
    {
        //Forward to NeTVBrowser
        int numClient = Static::tcpSocketServer->broadcast(rawXmlString, "netvbrowser");

        //Reply to HTTP client
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + STRING_COMMAND_FORWARDED_BROWSER + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + STRING_NO_BROWSER_RUNNING + "</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "ANDROID" || cmdString == "IOS")
    {
        //Allow user to use both normal POST style API as well as XML style passing
        QByteArray eventName = request.getParameter("eventname").trimmed();
        QByteArray eventData = request.getParameter("eventdata").trimmed();
        if (eventName.length() <= 0 || eventData.length() <= 0) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>invalid arguements</value></data></xml>", true);
            return;
        }

        //Convert to a JavaScript command & forward to NeTVBrowser
        QByteArray javaScriptString = "fAndroidEvents(\"" + eventName + "\",\"" + eventData + "\");";
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><value>") + javaScriptString + "</value></xml>", "netvbrowser");

        //Reply to HTTP client
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + dataString + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>no client running</value></data></xml>", true);
    }

    //-----------

    else if (cmdString == "CONTROLPANEL")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>", true);
            return;
        }

        //This is simply passed directly to NeTVBrowser
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString), xmlEscape);

        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "TICKEREVENT")
    {
        //All these variables should be URI encoded (%) before passing to JavaScript
        QByteArray title, message, image, type, level;
        title = request.getParameter("title").toPercentEncoding();
        message = request.getParameter("message").toPercentEncoding();
        image = request.getParameter("image").toPercentEncoding();
        type = request.getParameter("type").toPercentEncoding();
        level = request.getParameter("level").toPercentEncoding();

        //Convert to a JavaScript command & forward to NeTVBrowser
        QByteArray javaScriptString = "fTickerEvents(\"" + message + "\",\"" + title + "\",\"" + image + "\",\"" + type + "\",\"" + level + "\");";
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><value>") + javaScriptString + "</value></xml>", "netvbrowser");

        //Reply to HTTP client
        if (numClient > 0)          response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>OK</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>", true);
    }

    //-----------

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
        QByteArray buffer = this->Execute(docroot + "/scripts/set_time.sh", argsList, xmlEscape);

        //Reply to HTTP client
        if (time == "")    response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>time parameter is required</value></data></xml>", true);
        else               response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
    }

    else if (cmdString == "SETNETWORK")
    {
        QHash<QString,QString> temp_params;
        temp_params.insert("allocation", request.getParameter("wifi_allocation"));
        temp_params.insert("ssid", request.getParameter("wifi_ssid"));
        temp_params.insert("auth", request.getParameter("wifi_authentication"));
        temp_params.insert("encryption", request.getParameter("wifi_encryption"));
        temp_params.insert("key", request.getParameter("wifi_password"));
        temp_params.insert("encoding", request.getParameter("wifi_encoding"));
        temp_params.insert("type", request.getParameter("type"));
        temp_params.insert("test", request.getParameter("test"));

        //Write configuration to /psp/network_config or /psp/network_config_test
        bool isTest = request.getParameter("test").length() > 1;
        bool fileOK = SetNetworkConfig(temp_params);
        temp_params.clear();

        //Report the content of network_config
        QByteArray buffer;
        if (isTest)                 buffer = this->GetFileContents(networkConfigFile + "_test");
        else                        buffer = this->GetFileContents(networkConfigFile);
        if (xmlEscape)              buffer = XMLEscape(buffer);

        //Reply to HTTP client
        if (!fileOK)                response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);

        //Allow time for HTTP response to complete before we bring down the network
        if (!isTest)
            StopAPwithDelay();
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
    else if (cmdString == "RESTART" || cmdString == "REBOOT")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>", true);
        RebootwithDelay();
    }

    //-----------

    else if (cmdString == "GETALLPARAMS")
    {
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" +  GetAllParameters() + "</data></xml>", true);
    }

    else if (cmdString == "GETPARAM")
    {
        QByteArray value = GetParameter(dataString);
        if (xmlEscape)
            value = XMLEscape(value);
        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + value + "</value></data></xml>", true);
    }

    else if (cmdString == "GETPARAMS")
    {
        QList<QByteArray> params = dataString.split(';');
        QByteArray buffer;
        for (int i = 0; i < params.size(); ++i)
        {
            QByteArray key = params.at(i);
            QByteArray value = GetParameter(key);
            if (xmlEscape)
                value = XMLEscape(value);
            buffer += "<" + key + ">" + value + "</" + key + ">";
        }
        if (params.size() < 1)
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>no parameters received</value></data></xml>", true);
        else
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer + "</value></data></xml>", true);
    }

    else if (cmdString == "SETPARAM")
    {
        if (request.getParameterMap().count() > 0)
        {
            QMapIterator<QByteArray, QByteArray> i(request.getParameterMap());
            while (i.hasNext())
            {
                i.next();
                SetParameter( i.key(), i.value() );
            }
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + QByteArray().setNum(request.getParameterMap().count()) + "</value></data></xml>", true);
        }
        else
        {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>no parameters received</value></data></xml>", true);
        }
    }

    //-----------

    else if (cmdString == "GETFILECONTENTS")
    {
        QByteArray filedata = GetFileContents(dataString);
        if (xmlEscape)
            filedata = XMLEscape(filedata);

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

    else if (cmdString == "UPLOADFILE" || cmdString == "FILEUPLOAD" || cmdString == "UPLOAD")
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
        bool success = file->copy( pathString );

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
        QByteArray buffer = this->Execute(command, newArgs, xmlEscape);

        if (buffer.length() > 5)        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        else                            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>", true);
        buffer = QByteArray();
    }

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString), xmlEscape);

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
    //Extract the command name & most commonly used 'value' parameter (if any)
    QByteArray cmdString = request.getCommand().toUpper();
    QByteArray dataString = request.getParameter(STRING_VALUE).trimmed();
    QByteArray authorizedCaller = request.getParameter(STRING_AUTHORIZED_CALLER).toUpper();
    request.removeParameter(STRING_VALUE);

    //Some commands requires XML escape. This would be an option.
    QByteArray xmlEscapeString = request.getParameter(STRING_XML_ESCAPE).toUpper();
    bool xmlEscape = (xmlEscapeString == "1" || xmlEscapeString == "TRUE" || xmlEscapeString == "YES" || xmlEscapeString == "ON") ? true : false;
    request.removeParameter(STRING_XML_ESCAPE);

    //-----------------------------------------------------------
    //Mostly from Android/iOS devices

    if (cmdString == "REMOTECONTROL" || cmdString == "REMOTE" || cmdString == "KEY" || cmdString == "KEYBOARD")
    {
        //Forward to all TCP clients
        int numClient = Static::tcpSocketServer->broadcast(request.getRawData(), "all");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_COMMAND_FORWARDED_CLIENT);
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_NO_CLIENT_RUNNING);
        }
        response.write();
    }

    else if (cmdString == "SETURL" || cmdString == "MULTITAB" || cmdString == "TAB" || cmdString == "KEEPALIVE" || cmdString == "JAVASCRIPT" || cmdString == "JS"
             || cmdString == "UPDATEREADY" || cmdString == "UPDATEDONE" || cmdString == "UPGRADEDONE")
    {
        //Forward to NeTVBrowser
        int numClient = Static::tcpSocketServer->broadcast(request.getRawData(), "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_COMMAND_FORWARDED_BROWSER);
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_NO_BROWSER_RUNNING);
        }
        response.write();
    }

    else if (cmdString == "ANDROID" || cmdString == "IOS")
    {
        QByteArray eventName = request.getParameter("eventname").trimmed();
        QByteArray eventData = request.getParameter("eventdata").trimmed();     //already URI encoded
        QByteArray javaScriptString = "fAndroidEvents(\"" + eventName + "\",\"" + eventData + "\");";

        //Convert to a JavaScript command & forward to NeTVBrowser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><value>") + javaScriptString + "</value></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_COMMAND_FORWARDED_BROWSER);
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_NO_BROWSER_RUNNING);
        }
        response.write();
    }

    else if (cmdString == "TICKEREVENT")
    {
        //All these should be URI encoded
        QByteArray message = request.getParameter("message").trimmed().toPercentEncoding();
        QByteArray title = request.getParameter("title").trimmed().toPercentEncoding();
        QByteArray image = request.getParameter("image").trimmed().toPercentEncoding();
        QByteArray type = request.getParameter("type").trimmed().toPercentEncoding();
        QByteArray level = request.getParameter("level").trimmed().toPercentEncoding();
        QByteArray javaScriptString = "fTickerEvents(\"" + message + "\",\"" + title + "\",\"" + image + "\",\"" + type + "\",\"" + level + "\");";

        //Convert to a JavaScript command & forward to NeTVBrowser
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><value>") + javaScriptString + "</value></xml>", "netvbrowser");

        //Reply to socket client (Android/iOS)
        if (numClient > 0) {
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_COMMAND_FORWARDED_BROWSER);
        }else{
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setCommand(cmdString);
            response.setParameter(STRING_VALUE, STRING_NO_BROWSER_RUNNING);
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
        QByteArray buffer = this->Execute(docroot + "/scripts/set_time.sh", argsList, xmlEscape);

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
        response.write();
    }

    else if (cmdString == "SETNETWORK")
    {
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
            StopAPwithDelay();
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

    else if (cmdString == "GETPARAMS")
    {
        QList<QByteArray> params = dataString.split(';');
        for (int i = 0; i < params.size(); ++i)
        {
            QByteArray key = params.at(i);
            response.setParameter(key, GetParameter(key));
        }
        if (params.size() < 1)          response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        if (params.size() < 1)
            response.setParameter(STRING_VALUE, "no parameters received");
        response.setCommand(cmdString);
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
    else if (cmdString == "REBOOT" || cmdString == "RESTART")
    {
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.write();
        RebootwithDelay();
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
        bool isRelative = dataString.toUpper() == "RELATIVE";
        if (isRelative)         Static::cursorController->setRelativeMode();
        else                    Static::cursorController->setAbsoluteMode();

        //No response
    }
#endif

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
        QByteArray buffer = this->Execute(command, newArgs, xmlEscape);

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, buffer.trimmed());
        response.write();
    }

    //-----------

    else if (cmdString == "ENABLESSH" || cmdString == "SSH")
    {
        if (dataString.length() < 1)
            dataString = "start-chumby";
        QByteArray buffer = this->Execute("/etc/init.d/sshd", QStringList(QString(dataString)), xmlEscape);

        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_VALUE, buffer.trimmed());
        response.write();
    }

    else if (cmdString == "SETCHROMAKEY" || cmdString == "CHROMAKEY")
    {
        //Contruct arguments
        dataString = dataString.toLower();
        QStringList argList;

        if (dataString == "on" || dataString == "true" || dataString == "yes")                argList.append(QString("on"));
        else if (dataString == "off" || dataString == "false" || dataString == "no")          argList.append(QString("off"));

        //Execute the script
        if (argList.size() <= 0)
        {
            response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
            response.setParameter(STRING_VALUE, "Invalid arguments");
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList, xmlEscape);
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setParameter(STRING_VALUE, buffer.trimmed());
            buffer = QByteArray();
        }
        response.setCommand(cmdString);
        response.write();
    }

    else if (cmdString == "SETDOCROOT")
    {
        QByteArray current_docroot = SetStaticDocroot(dataString).toLatin1();
        if (xmlEscape)
            current_docroot = XMLEscape(current_docroot);

        if (current_docroot.length() < 1)       response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                                    response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        if (current_docroot.length() < 1)       response.setParameter(STRING_VALUE, "path not found");
        else                                    response.setParameter(STRING_VALUE, current_docroot.trimmed());
        response.setCommand(cmdString);
        response.write();
    }

    else if (cmdString == "GETDOCROOT")
    {
        QByteArray current_docroot = GetStaticDocroot().toLatin1();
        if (xmlEscape)
            current_docroot = XMLEscape(current_docroot);
        if (current_docroot.length() < 1)       response.setStatus(BRIDGE_RETURN_STATUS_ERROR);
        else                                    response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        if (current_docroot.length() < 1)       response.setParameter(STRING_VALUE, "path not found");
        else                                    response.setParameter(STRING_VALUE, current_docroot.trimmed());
        response.setCommand(cmdString);
        response.write();
    }

    //-----------

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString), xmlEscape);
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setCommand(cmdString);
        response.setParameter(STRING_DATA, buffer.trimmed());
        response.write();
    }

    else
    {
        response.setStatus(BRIDGE_RETURN_STATUS_UNIMPLEMENTED);
        response.setParameter(STRING_VALUE, "Check for valid 'cmd' variable");
        response.write();
    }
}


//-----------------------------------------------------------------------------------------------------------
// Interaction with StaticFileController
//-----------------------------------------------------------------------------------------------------------

QString BridgeController::SetStaticDocroot(QString newPath)
{
    return Static::staticFileController->setDocroot(newPath);
}

QString BridgeController::GetStaticDocroot()
{
    return Static::staticFileController->getDocroot();
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

void BridgeController::RebootwithDelay(int msec /* = 500 */)
{
    QTimer::singleShot(msec, this, SLOT(slot_Reboot()));
}
