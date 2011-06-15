#include "bridgecontroller.h"
#include "../static.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>

#define BRIDGE_RETURN_STATUS_UNIMPLEMENTED  "0"
#define BRIDGE_RETURN_STATUS_SUCCESS        "1"
#define BRIDGE_RETURN_STATUS_ERROR          "2"

BridgeController::BridgeController(QSettings* settings, QObject* parent)
    :HttpRequestHandler(parent)
{
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

    QByteArray cmdString = request.getParameter("cmd");
    QByteArray dataXmlString = request.getParameter("data");
    QByteArray dataString = "";

    //Strip the XML tag if it is a simple value
    bool singleValueArg = dataXmlString.startsWith("<value>") && dataXmlString.endsWith("</value>");
    if (singleValueArg)
        dataString = dataXmlString.mid(7, dataXmlString.length() - 15);

    //-----------

    if (cmdString == "GetXML")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/wget.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
        buffer = QByteArray();
    }

    else if (cmdString == "GetBase64")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/wget.sh", QStringList(dataString));
        buffer = buffer.toBase64();
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
        buffer = QByteArray();
    }

    else if (cmdString == "HasFlashPlugin")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>false</value></data>");
    }

    else if (cmdString == "PlayWidget")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>");
    }

    else if (cmdString == "PlaySWF")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>");
    }

    else if (cmdString == "SetWidgetSize")
    {
        //Forward to widget rendering engine
        int numClient = Static::tcpSocketServer->broadcast(QByteArray("<xml><cmd>") + cmdString + "</cmd><data>" + dataXmlString + "</data></xml>");

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>");
    }

    else if (cmdString == "SetBox")
    {
        QByteArray buffer = this->Execute("setbox", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
        buffer = QByteArray();
    }

    //-----------

    else if (cmdString == "GetAllParams")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>GetAllParams</value></data>");
    }

    else if (cmdString == "GetParam")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>GetParam</value></data>");
    }

    else if (cmdString == "SetParam")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>SetParam</value></data>");
    }

    //-----------

    else if (cmdString == "GetFileContents")
    {
        QByteArray filedata = GetFileContents(dataString);
        if (filedata == "file not found" || filedata == "no permission")
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>" + filedata + "</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + filedata + "</value></data>");
    }

    else if (cmdString == "SetFileContents")
    {
        //<path>full path to a file</path><content>........</content>
        QByteArray path = dataXmlString.mid(6, dataXmlString.indexOf("</path>") - 6);
        QByteArray content = dataXmlString.mid(dataXmlString.indexOf("<content>") + 9, dataXmlString.length()-dataXmlString.indexOf("<content>")-9);
        if (!SetFileContents(path,content))
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>0</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + QByteArray().setNum(GetFileSize(path)) + "</value></data>");
    }

    else if (cmdString == "FileExists")
    {
        QByteArray existStr = "false";
        if (FileExists(dataString))
            existStr = "true";
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + existStr + "</value></data>");
    }

    else if (cmdString == "GetFileSize")
    {
        QByteArray sizeStr = QByteArray().setNum(GetFileSize(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + sizeStr + "</value></data>");
    }

    else if (cmdString == "UnlinkFile")
    {
        bool success = UnlinkFile(dataString);
        if (!success)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>false</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>true</value></data>");
    }

    //-----------

    else
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_UNIMPLEMENTED + "</status><data><value>" + cmdString + "</value></data>");
    }
}


void BridgeController::service(SocketRequest& request, SocketResponse& response)
{
    QByteArray cmdString = request.getCommand();
    QByteArray dataString = request.getParameter("data");


    //This is just dumb socket right now
    //socket->write("<xml><cmd>play_widget</cmd><data>http://s3movies.chumby.com/cdn/xmlmovie/F3A86472-05F6-11DF-A1F8-001B24E044BE</data></xml>\n\n");
}


//-----------------------------------------------------------------------------------------------------------
// Process Utilities
//-----------------------------------------------------------------------------------------------------------

QByteArray BridgeController::Execute(const QString &fullPath, QStringList args)
{
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
// File Utilities
//-----------------------------------------------------------------------------------------------------------

bool BridgeController::FileExists(const QString &fullPath)
{
    QFile file(fullPath);
    return file.exists();
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
