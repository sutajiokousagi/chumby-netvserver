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

// Static function
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
