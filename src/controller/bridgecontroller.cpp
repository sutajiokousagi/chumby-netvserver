#include "bridgecontroller.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QCryptographicHash>
#include <QXmlStreamWriter>
#include <QTimer>

#include <QUrlQuery>

#define CONTENT_TYPE_PLAIN     "Content-Type: text/plain\r\n\r\n"
#define ERROR_400               "HTTP/1.1 400 bad request\r\nConnection: close\r\n\r\n"
#define ERROR_403               "HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n"
#define ERROR_NO_PARAM          "<status>2</status><data><value>please provide some parameter(s)</value></data>"


BridgeController::BridgeController(QSettings* settings, QObject* parent) :
    QObject(parent)
{

    if (settings) {
        //Settings from NeTVServer.ini
        docroot = settings->value("path",".").toString();
        paramsFile = settings->value("paramsFile",".").toString();
        networkConfigFile = settings->value("networkConfigFile",".").toString();
    }

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

void BridgeController::replyToHttpClient(QHttpResponse *response, int success, const QByteArray & cmdString)
{
    //Reply to HTTP client
    if (success)
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>OK</value></data></xml>");
    else
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>No browser running</value></data></xml>");
}

void BridgeController::service(QHttpRequest *request, QHttpResponse *response)
{
    //Documentation
    //http://wiki.chumby.com/index.php/NeTV_web_services

    const auto & headers = request->headers();
    const QByteArray & body = request->collectedData();

    QHash<QString, QString> parameters;
    QUrlQuery query(request->url());

    // Copy over the querystring values
    QList<QPair<QString, QString>>items = query.queryItems();
    QPair<QString, QString> pair;
    foreach (pair, items)
        parameters[pair.first] = pair.second;

    // Copy over the POST values

    QByteArray authorizedCaller;

    /*
    //Allow Authorized-Caller to be passed through HTTP Header or regular POST parameters, or through XML style passing
    QByteArray authorizedCaller = headers[STRING_AUTHORIZED_CALLER].toUpper();
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
    */

    QByteArray cmdString = parameters[STRING_COMMAND].toUpper().toUtf8();
    QByteArray dataString = parameters[STRING_DATA].toUpper().toUtf8();

    //Some commands requires XML escape. This would be an option.
    QByteArray xmlEscapeString = parameters[STRING_XML_ESCAPE].toUpper().toUtf8();
    bool xmlEscape = (xmlEscapeString == "1"
                   || xmlEscapeString == "TRUE"
                   || xmlEscapeString == "YES"
                   || xmlEscapeString == "ON") ? true : false;

    //-----------------------------------------------------------

    /*
    if (cmdString == "MOTOR")
    {
        QStringList newArgsList = dataString.split(' ');
        QStringList newArgs;
        newArgs.append("mot_ctl");
        for (int i=0; i<newArgsList.length(); i++)
            newArgs << newArgsList.at(i);

        //Convert QStringList to char** array
        char **output = new char*[newArgs.size() + 1];
        for (int i = 0; i < newArgs.size(); i++)
        {
            output[i] = new char[strlen(newArgs.at(i).toStdString().c_str())+1];
            memcpy(output[i], newArgs.at(i).toStdString().c_str(), strlen(newArgs.at(i).toStdString().c_str())+1);
        }
        output[newArgs.size()] = ((char)NULL);

        //Execute C motor command
        int result = main_motor(newArgs.size(), output);

        //Delete char** array
        int i = 0;
        while (output[i]) {
            delete output[i];
            i++;
        }
        delete output;

        if (result == 0)        response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>OK</value></data></xml>", true);
        else                    response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>invalid arguements</value></data></xml>", true);
    }
    */
    //-----------------------------------------------------------

    if (cmdString == "GETURL")
    {
        QString urlString = parameters["url"];
        QString postString = parameters["post"];
        QStringList argsList;

        if (urlString.length() > 0)
            argsList.append(QString(urlString));

        if (postString.length() > 0)
            argsList.append(QString(postString));

        if (argsList.length() < 1)
        {
            response->write(QByteArray("<xml><status>")
                            + BRIDGE_RETURN_STATUS_ERROR
                            + "</status><cmd>"
                            + cmdString
                            + "</cmd><data><value>no arguments received</value></data></xml>");
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/geturl.sh", argsList, xmlEscape);
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
            buffer = QByteArray();
        }
    }

    else if (cmdString == "GETJPG" || cmdString == "GETJPEG")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString), xmlEscape);
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
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
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>Invalid arguments</value></data></xml>");
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList, xmlEscape);
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
            buffer = QByteArray();
        }
    }

    else if (cmdString == "ENABLESSH" || cmdString == "SSH")
    {
        if (dataString.length() < 1)
            dataString = "start-chumby";
        QByteArray buffer = this->Execute("/etc/init.d/sshd", QStringList(QString(dataString)), xmlEscape);

        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
        buffer = QByteArray();
    }

    else if (cmdString == "SETDOCROOT")
    {
        QByteArray current_docroot = SetStaticDocroot(dataString).toLatin1();
        if (xmlEscape)
            current_docroot = XMLEscape(current_docroot);
        if (current_docroot.length() < 1)
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>path not found</value></data></xml>");
        else
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + current_docroot.trimmed() + "</value></data></xml>");
    }

    else if (cmdString == "GETDOCROOT")
    {
        QByteArray current_docroot = GetStaticDocroot().toLatin1();
        if (xmlEscape)
            current_docroot = XMLEscape(current_docroot);
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + current_docroot.trimmed() + "</value></data></xml>");
    }

    //-----------

    else if (cmdString == "REMOTECONTROL" || cmdString == "REMOTE" || cmdString == "KEY" || cmdString == "KEYBOARD")
    {
        //Forward to all TCP clients
        //int numClient = Static::tcpSocketServer->broadcast(rawXmlString, "all");

        replyToHttpClient(response, 1, cmdString);
    }

    else if (cmdString == "SETURL" || cmdString == "MULTITAB" || cmdString == "TAB" || cmdString == "KEEPALIVE" || cmdString == "JAVASCRIPT" || cmdString == "JS"
             || cmdString == "UPDATEREADY" || cmdString == "UPDATEDONE" || cmdString == "UPGRADEDONE")
    {
        //Forward to NeTVBrowser
        //int numClient = Static::tcpSocketServer->broadcast(rawXmlString, "netvbrowser");

        replyToHttpClient(response, 1, cmdString);
    }

    //-----------

    else if (cmdString == "ANDROID" || cmdString == "IOS")
    {
        //Allow user to use both normal POST style API as well as XML style passing
        QByteArray eventName = parameters["eventname"].trimmed().toUtf8();
        QByteArray eventData = parameters["eventdata"].trimmed().toUtf8();
        if (eventName.length() <= 0 || eventData.length() <= 0) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>invalid arguements</value></data></xml>");
            return;
        }

        //Convert to a JavaScript command & forward to NeTVBrowser
        QByteArray javaScriptString = "fAndroidEvents(\"" + eventName + "\",\"" + eventData + "\");";
        //int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><value>") + javaScriptString + "</value></xml>", "netvbrowser");

        replyToHttpClient(response, 1, cmdString);
    }

    //-----------

    else if (cmdString == "CONTROLPANEL")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>");
            return;
        }

        //This is simply passed directly to NeTVBrowser
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString), xmlEscape);

        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
        buffer = QByteArray();
    }

    else if (cmdString == "TICKEREVENT")
    {
        //All these variables should be URI encoded (%) before passing to JavaScript
        QByteArray title, message, image, type, level;
        title = parameters["title"].toUtf8().toPercentEncoding();
        message = parameters["message"].toUtf8().toPercentEncoding();
        image = parameters["image"].toUtf8().toPercentEncoding();
        type = parameters["type"].toUtf8().toPercentEncoding();
        level = parameters["level"].toUtf8().toPercentEncoding();

        //Convert to a JavaScript command & forward to NeTVBrowser
        QByteArray javaScriptString = "fTickerEvents(\"" + message + "\",\"" + title + "\",\"" + image + "\",\"" + type + "\",\"" + level + "\");";
        //int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>JavaScript</cmd><value>") + javaScriptString + "</value></xml>", "netvbrowser");

        replyToHttpClient(response, 1, cmdString);
    }

    //-----------

    else if (cmdString == "SETTIME")
    {
        // <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
        // <timezone> is standard timezone ID string formated as "Asia/Singapore"
        // see http://developer.android.com/reference/java/util/TimeZone.html#getID()
        QString time = parameters["time"];
        QString timezone = parameters["timezone"];

        QStringList argsList;
        argsList.append(time);
        argsList.append(timezone);
        QByteArray buffer = this->Execute(docroot + "/scripts/set_time.sh", argsList, xmlEscape);

        replyToHttpClient(response, 1, cmdString);
    }

    else if (cmdString == "SETNETWORK")
    {
        QHash<QString,QString> temp_params;
        temp_params.insert("allocation", parameters["wifi_allocation"]);
        temp_params.insert("ssid", parameters["wifi_ssid"]);
        temp_params.insert("auth", parameters["wifi_authentication"]);
        temp_params.insert("encryption", parameters["wifi_encryption"]);
        temp_params.insert("key", parameters["wifi_password"]);
        temp_params.insert("encoding", parameters["wifi_encoding"]);
        temp_params.insert("type", parameters["type"]);
        temp_params.insert("test", parameters["test"]);

        //Write configuration to /psp/network_config or /psp/network_config_test
        bool isTest = parameters["test"].length() > 1;
        bool fileOK = SetNetworkConfig(temp_params);
        temp_params.clear();

        //Report the content of network_config
        QByteArray buffer;
        if (isTest)                 buffer = this->GetFileContents(networkConfigFile + "_test");
        else                        buffer = this->GetFileContents(networkConfigFile);
        if (xmlEscape)              buffer = XMLEscape(buffer);

        //Reply to HTTP client
        replyToHttpClient(response, fileOK, cmdString);

        //Allow time for HTTP response to complete before we bring down the network
        if (!isTest)
            StopAPwithDelay();
    }

    //-----------

    else if (cmdString == "STOPAP")
    {
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>");
        StopAPwithDelay();
    }
    else if (cmdString == "STARTAP")
    {
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>");
        StartAPwithDelay();
    }
    else if (cmdString == "STARTAPFACTORY")
    {
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>");
        StartAPFactorywithDelay();
    }
    else if (cmdString == "RESTART" || cmdString == "REBOOT")
    {
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd></xml>");
        RebootwithDelay();
    }

    //-----------

    else if (cmdString == "GETALLPARAMS")
    {
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" +  GetAllParameters() + "</data></xml>");
    }

    else if (cmdString == "GETPARAM")
    {
        QByteArray value = GetParameter(dataString);
        if (xmlEscape)
            value = XMLEscape(value);
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + value + "</value></data></xml>");
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
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>no parameters received</value></data></xml>");
        else
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" + buffer + "</data></xml>");
    }

    else if (cmdString == "SETPARAM")
    {
        replyToHttpClient(response, 0, cmdString);
#if 0
        if (request.getParameterMap().count() > 0)
        {
            QMapIterator<QByteArray, QByteArray> i(request.getParameterMap());
            while (i.hasNext())
            {
                i.next();
                if (i.key() != "data")
                    SetParameter( i.key(), i.value() );
            }
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + QByteArray().setNum(request.getParameterMap().count()) + "</value></data></xml>", true);
        }
        else
        {
            response.write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>no parameters received</value></data></xml>", true);
        }
#endif
    }

    //-----------

    else if (cmdString == "GETFILECONTENTS")
    {
        QByteArray filedata = GetFileContents(dataString);
        if (xmlEscape)
            filedata = XMLEscape(filedata);

        replyToHttpClient(response, (filedata != "file not found" && filedata != "no permission"), cmdString);
    }

    /*
    else if (cmdString == "SETFILECONTENTS")
    {
        //<path>full path to a file</path><content>........</content>
        QByteArray path = dataXmlString.mid(6, dataXmlString.indexOf("</path>") - 6);
        QByteArray content = dataXmlString.mid(dataXmlString.indexOf("<content>") + 9, dataXmlString.length()-dataXmlString.indexOf("<content>")-9);
        if (!SetFileContents(path,content))
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>0</value></data></xml>");
        else
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + QByteArray().setNum(GetFileSize(path)) + "</value></data></xml>");
    }
    */

    else if (cmdString == "FILEEXISTS")
    {
        QByteArray existStr = "false";
        if (FileExists(dataString))
            existStr = "true";
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + existStr + "</value></data></xml>");
    }

    else if (cmdString == "GETFILESIZE")
    {
        QByteArray sizeStr = QByteArray().setNum(GetFileSize(dataString));
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + sizeStr + "</value></data></xml>");
    }

    else if (cmdString == "UNLINKFILE")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>");
            return;
        }

        bool success = UnlinkFile(dataString);
        if (!success)
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><value>false</value></data></xml>");
        else
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>true</value></data></xml>");
    }

    else if (cmdString == "MD5FILE")
    {
        if (!FileExists(dataString)) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>file not found</message><path>" + dataString + "</path></data></xml>");
            return;
        }
        QByteArray md5sum = GetFileMD5(dataString);
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><path>" + dataString + "</path><md5sum>" + md5sum + "</md5sum></data></xml>");
    }

    //-----------

    /*
    else if (cmdString == "DOWNLOADFILE")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>");
            return;
        }

        DumpStaticFile(dataString, response);
    }
    */

    /*
    else if (cmdString == "UPLOADFILE" || cmdString == "FILEUPLOAD" || cmdString == "UPLOAD")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>");
            return;
        }

        QString pathString = QString(parameters["path"]);

        //Clear existing file (if any)
        bool cleanFile = true;
        if (FileExists(pathString))
            cleanFile = UnlinkFile(pathString);
        if (!cleanFile) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>unable to clean existing file</message><path>" + pathString.toLatin1() + "</path></data></xml>");
            return;
        }

        QTemporaryFile* file = request.getUploadedFile("filedata");
        if (!file) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>uploaded file data not found</message><path>" + pathString.toLatin1() + "</path></data></xml>");
            return;
        }

        //Write the buffer to non-volatile disk
        bool success = file->copy( pathString );

        if (!success) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>error writing to disk (read-only partition?)</message><path>" + pathString.toLatin1() + "</path></data></xml>");
            return;
        }

        quint64 filesize = GetFileSize(pathString);
        if (filesize <= 0) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data><message>invalid filesize</message><path>" + pathString.toLatin1() + "</path></data></xml>");
            return;
        }
        QByteArray sizeStr = QByteArray().setNum(filesize);
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><path>" + pathString.toLatin1() + "</path><filesize>" + sizeStr + "</filesize></data></xml>");
    }
    */

    //-----------

    else if (cmdString == "NECOMMAND")
    {
        if (!IsAuthorizedCaller(authorizedCaller)) {
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNAUTHORIZED + "</status><cmd>" + cmdString + "</cmd><data><value>Unauthorized</value></data></xml>");
            return;
        }

        QList<QByteArray> newArgsList = dataString.split(' ');
        QString command = QString(newArgsList.at(0));

        QStringList newArgs;
        newArgsList.removeAt(0);
        for (int i=0; i<newArgsList.length(); i++)
            newArgs << newArgsList.at(i);
        QByteArray buffer = this->Execute(command, newArgs, xmlEscape);

        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
        buffer = QByteArray();
    }

    //A generic command to execute name-alike scripts
    else if (FileExists(docroot + "/scripts/" + cmdString.toLower() + ".sh"))
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/" + cmdString.toLower() + ".sh", QStringList(dataString), xmlEscape);

        if (buffer.length() > 1)
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data>" + buffer.trimmed() + "</data></xml>");
        else
            response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><cmd>" + cmdString + "</cmd><data>" + buffer.trimmed() + "</data></xml>");
        buffer = QByteArray();
    }

    //-----------

    else
    {
        response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><cmd>" + cmdString + "</cmd><data><value>Check for valid 'cmd' variable</value></data></xml>");
    }
}

#if 0
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

    //from NeTVServer mostly
    if (cmdString == "TEXTINPUT")
    {
        QByteArray id = request.getParameter("id");
        if (id == "")       qDebug("%s: TextInput event from NeTVBrowser (de-focus)",  TAG);
        else                qDebug("%s: TextInput event from NeTVBrowser (id: %s, value: %s)",  TAG, id.constData(), dataString.constData());

        //TCP won't work because all requests get disconnected as soon as they finish
        /*
        int numClient = Static::tcpSocketServer->broadcast(request.getRawData(), "android");
        if (numClient > 0)  qDebug("%s: forwarded to Android",  TAG);
        else                qDebug("%s: no Android connected",  TAG);
        */

        //Send multicast to whoever subscribe
        QMap<QByteArray, QByteArray> parameters;
        parameters.insert("cmd", "TextInput");
        parameters.insert("id", id);
        parameters.insert("value", dataString);
        Static::udpSocketServer->sendMessage(parameters, "multicast");
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
#endif

//-----------------------------------------------------------------------------------------------------------
// Interaction with StaticFileController
//-----------------------------------------------------------------------------------------------------------

QString BridgeController::SetStaticDocroot(QString newPath)
{
    return this->Execute(docroot + "/scripts/setdocroot.sh", QStringList(newPath), false);
}

QString BridgeController::GetStaticDocroot()
{
    return this->Execute(docroot + "/scripts/getdocroot.sh", false);
}


//-----------------------------------------------------------------------------------------------------------
// High level helper functions
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::IsAuthorizedCaller(QByteArray headerValue)
{
    Q_UNUSED(headerValue);
    return true;

    /*
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
    if (headerValue == hashResult1 || headerValue == hashResult2 || headerValue == hashResult3)
        return true;
    return false;
    */
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
    //slot_StartAP();
}

void BridgeController::StartAPFactorywithDelay(int msec /* = 500 */)
{
    //slot_StartAP_Factory();
}

void BridgeController::StopAPwithDelay(int msec /* = 500 */)
{
    //slot_StopAP();
}

void BridgeController::RebootwithDelay(int msec /* = 500 */)
{
    //slot_Reboot();
}


//----------------------------------------------------------------------------------------------------
// Non-volatile parameters
//----------------------------------------------------------------------------------------------------

QByteArray BridgeController::GetParameter(QString name)
{
    //Refresh param
    LoadParameters();

    return parameters->value(name, "").toByteArray();
}

QByteArray BridgeController::GetAllParameters()
{
    //Refresh param
    LoadParameters();

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
    LoadParameters();
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


//-----------------------------------------------------------------------------------------------------------
// Process Utilities
//-----------------------------------------------------------------------------------------------------------

QByteArray BridgeController::Execute(const QString &fullPath, bool xmlEscape /* = false */)
{
    return this->Execute(fullPath, QStringList(), xmlEscape);
}

QByteArray BridgeController::Execute(const QString &fullPath, QStringList args, bool xmlEscape /* = false */)
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

    if (xmlEscape)
        return XMLEscape(buffer);
    return buffer;
}


//-----------------------------------------------------------------------------------------------------------
// File Utilities
//-----------------------------------------------------------------------------------------------------------

void BridgeController::Sync(void)
{
    this->Execute("/bin/sync");
}

bool BridgeController::DirExists(const QString &fullPath)
{
    QDir dir(fullPath);
    return dir.exists(fullPath);
}

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
    return ((file.permissions() & QFile::ExeOwner) || (file.permissions() & QFile::ExeOther) || (file.permissions() & QFile::ExeUser));
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

    //Make sure the file is written to disk
    Sync();
    return true;
}

bool BridgeController::SetFileExecutable(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())                         //doesn't exist
        return false;
    if (FileExecutable(fullPath))
        return true;
    //return true;
    return file.setPermissions(file.permissions() | QFile::ExeUser | QFile::ExeOther | QFile::ExeOwner);
}

bool BridgeController::UnlinkFile(const QString &fullPath)
{
    QFile file(fullPath);
    if (!file.exists())                         //doesn't exist
        return true;
    return file.remove();
}

QByteArray BridgeController::GetFileMD5(const QString &fullPath)
{
    QByteArray output = Execute(QString("/usr/bin/md5sum"), QStringList(fullPath));
    return output.split(' ')[0];
}


//-----------------------------------------------------------------------------------------------------------
// Mount/Unmount utility
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::MountRW()
{
    Execute("/bin/mount", QStringList() << "-o" << "remount,rw" << "/");
    return true;
}

bool BridgeController::MountRO()
{
    QByteArray output = Execute("/bin/mount", QStringList() << "-o" << "remount,ro" << "/");
    if (output.contains("error"))
        return false;
    return true;
}


//-----------------------------------------------------------------------------------------------------------
// Other Utilities
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::IsHexString(QString testString)
{
    static QString hexCharacters = "0123456789ABCDEF";
    bool isHex = true;
    for (int i=0; i<testString.length(); i++)
    {
        if ( hexCharacters.contains(testString.at(i), Qt::CaseInsensitive) )
            continue;
        isHex = false;
        break;
    }
    return isHex;
}

QString BridgeController::XMLEscape(QString inputString)
{
    return inputString.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");     //.replace("'", "&apos;");
}

QString BridgeController::XMLUnescape(QString inputString)
{
    return inputString.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
}

QByteArray BridgeController::XMLEscape(QByteArray inputString)
{
    return inputString.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");     //.replace("'", "&apos;");
}

QByteArray BridgeController::XMLUnescape(QByteArray inputString)
{
    return inputString.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
}


//----------------------------------------------------------------------------------------------------
// Copied from StaticFileController
//-----------------------------------------------------------------------------------------------------

/*
void BridgeController::DumpStaticFile(QByteArray path, HttpResponse& response)
{
    // If the filename is a directory, append index.html.
    if (QFileInfo(path).isDir())
        path+="/index.html";

    // IF file not exists
    QFile file(path);
    if (!file.exists())
    {
        response.setStatus(404,"not found");
        response.write("404 not found",true);
        return;
    }

    // Error opening the file
    if (!file.open(QIODevice::ReadOnly))
    {
        response.setStatus(403,"forbidden");
        response.write("403 forbidden",true);
        return;
    }

    SetContentType(path,response);

    // Return the file content
    while (!file.atEnd() && !file.error())
        response.write(file.read(65536));
    file.close();
}
    */

// Static function
/*
void BridgeController::SetContentType(QString fileName, HttpResponse& response)
{
    if (fileName.endsWith(".png")) {
        response.setHeader("Content-Type", "image/png");
    }
    else if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") || fileName.endsWith(".jpe")) {
        response.setHeader("Content-Type", "image/jpeg");
    }
    else if (fileName.endsWith(".gif")) {
        response.setHeader("Content-Type", "image/gif");
    }
    else if (fileName.endsWith(".bmp")) {
        response.setHeader("Content-Type", "image/bmp");
    }
    else if (fileName.endsWith(".ico")) {
        response.setHeader("Content-Type", "image/x-icon");
    }

    else if (fileName.endsWith(".wav")) {
        response.setHeader("Content-Type", "audio/x-wav");
    }
    else if (fileName.endsWith(".mid") || fileName.endsWith(".rmi")) {
        response.setHeader("Content-Type", "audio/mid");
    }
    else if (fileName.endsWith(".mp3")) {
        response.setHeader("Content-Type", "audio/mpeg");
    }
    else if (fileName.endsWith(".avi")) {
        response.setHeader("Content-Type", "video/x-msvideo");
    }

    else if (fileName.endsWith(".txt")) {
        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    }
    else if (fileName.endsWith(".html") || fileName.endsWith(".htm")) {
        response.setHeader("Content-Type", "text/html; charset=UTF-8");
    }
    else if (fileName.endsWith(".css")) {
        response.setHeader("Content-Type", "text/css");
    }

    else if (fileName.endsWith(".js")) {
        response.setHeader("Content-Type", "application/x-javascript; charset=UTF-8");
    }
    else if (fileName.endsWith(".xml")) {
        response.setHeader("Content-Type", "application/xml");
    }
    else if (fileName.endsWith(".gz")) {
        response.setHeader("Content-Type", "application/x-gzip");
    }
    else if (fileName.endsWith(".tar")) {
        response.setHeader("Content-Type", "application/x-tar");
    }
    else if (fileName.endsWith(".tgz")) {
        response.setHeader("Content-Type", "application/x-compressed");
    }
    else if (fileName.endsWith(".zip")) {
        response.setHeader("Content-Type", "application/zip");
    }

    else
    {
        response.setHeader("Content-Type", "text/plain; charset=UTF-8");
    }
}
*/
