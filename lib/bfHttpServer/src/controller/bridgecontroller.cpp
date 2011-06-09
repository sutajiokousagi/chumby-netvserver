/**
  @file
  @author Stefan Frings
  @version $Id: BridgeController.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "bridgecontroller.h"
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

    //Strip the XML tag if it is a simple value
    bool singleValueArg = dataXmlString.startsWith("<value>") && dataXmlString.endsWith("</value>");
    if (singleValueArg)
        dataXmlString = dataXmlString.mid(7, dataXmlString.length() - 15);

    //-----------

    if (cmdString == "GetXML")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/xmlbridge.sh", QStringList(dataXmlString));
        response.write(buffer);
    }

    else if (cmdString == "HasFlashPlugin")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>false</value></data>");
    }

    else if (cmdString == "PlayWidget")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>....TBD....</value></data>");
    }

    else if (cmdString == "PlaySWF")
    {
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>....TBD....</value></data>");
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
        QByteArray filedata = GetFileContents(dataXmlString);
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
        if (FileExists(dataXmlString))
            existStr = "true";
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + existStr + "</value></data>");
    }

    else if (cmdString == "GetFileSize")
    {
        QByteArray sizeStr = QByteArray().setNum(GetFileSize(dataXmlString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + sizeStr + "</value></data>");
    }

    else if (cmdString == "UnlinkFile")
    {
        bool success = UnlinkFile(dataXmlString);
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
