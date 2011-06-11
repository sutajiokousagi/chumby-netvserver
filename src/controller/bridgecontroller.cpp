#include "bridgecontroller.h"
#include "../static.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QApplication>
#include <QDesktopWidget>

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
        dataString = dataXmlString.mid(7, dataXmlString.length() - 15).trimmed();

    //-----------

    if (cmdString == "GetXML")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/wget.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>", true);
        buffer = QByteArray();
    }

    else if (cmdString == "GetJPG" || cmdString == "GetJPEG")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>", true);
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

        //Set the window size
        this->Execute(docroot + "/scripts/setbox.sh", QStringList(dataString));

        //Reply to JavaScriptCore/ControlPanel
        if (numClient > 0)
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>Command forwarded to widget rendering engine</value></data>");
        else
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>No widget rendering engine found</value></data>");
    }

    //-----------

    else if (cmdString == "SetBox")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/setbox.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
        buffer = QByteArray();
    }

    else if (cmdString == "SetChromaKey")
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
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_ERROR + "</status><data><value>Invalid arguments</value></data>");
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList);
            response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
            buffer = QByteArray();
        }
    }

    else if (cmdString == "ControlPanel")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
        buffer = QByteArray();
    }

    else if (cmdString == "WidgetEngine")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/widget_engine.sh", QStringList(dataString));
        response.write(QByteArray("<status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><data><value>" + buffer + "</value></data>");
        buffer = QByteArray();
    }

    else if (cmdString == "RemoteControl")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/remote_control.sh", QStringList(dataString));
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
    QByteArray dataString = request.getParameter("value").trimmed();

    //-----------

#if defined (CURSOR_CONTROLLER)
    if (cmdString == "CurDown")
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

    else if (cmdString == "CurMove")
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

    else if (cmdString == "CurUp")
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

    else if (cmdString == "CursorMode")
    {
        bool isRelative = request.getParameter("value") == "relative";
        if (isRelative)         Static::cursorController->setRelativeMode();
        else                    Static::cursorController->setAbsoluteMode();
    }
#endif

    //-----------

    else if (cmdString == "ControlPanel")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/control_panel.sh", QStringList(dataString));
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setParameter("value", buffer);
        response.write();
        buffer = QByteArray();
    }

    else if (cmdString == "WidgetEngine")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/widget_engine.sh", QStringList(dataString));
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setParameter("value", buffer);
        response.write();
        buffer = QByteArray();
    }

    else if (cmdString == "RemoteControl")
    {
        QByteArray buffer = this->Execute(docroot + "/scripts/remote_control.sh", QStringList(dataString));
        response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
        response.setParameter("value", buffer);
        response.write();
        buffer = QByteArray();
    }

    else if (cmdString == "MobileApp")
    {
        if (dataString.length() > 0)
        {
            if (dataString == "SetScreenshotRes")
            {

            }
            else if (dataString == "GetScreenshotPath")
            {

            }
        }
        else
        {
            QByteArray buffer = this->Execute(docroot + "/scripts/mobile_app.sh", QStringList(dataString));
            response.setStatus(BRIDGE_RETURN_STATUS_SUCCESS);
            response.setParameter("value", buffer);
            response.write();
            buffer = QByteArray();
        }
    }

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
