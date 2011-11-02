#include "staticfilecontroller.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

StaticFileController::StaticFileController(QSettings* settings) : HttpRequestHandler()
{
    docroot = settings->value("path",".").toString();

    // Convert relative path to absolute, based on the directory of the config file.
#ifdef Q_OS_WIN32
    if (QDir::isRelativePath(docroot) && settings->format()!=QSettings::NativeFormat)
#else
    if (QDir::isRelativePath(docroot))
#endif
    {
        QFileInfo configFile(settings->fileName());
        docroot = QFileInfo(configFile.absolutePath(),docroot).absoluteFilePath();
    }
    qDebug("StaticFileController: docroot=%s", qPrintable(docroot));
    docrootDynamic = docroot;
}

QString StaticFileController::setDocroot(QString newPath)
{
    QDir dir(newPath);
    if (!dir.exists(newPath))
        return "";
    if (newPath != NULL && newPath.length() > 2 && newPath.startsWith("/"))
    {
        if (newPath.endsWith("/"))      this->docrootDynamic = newPath.left(newPath.size()-1);
        else                            this->docrootDynamic = newPath;
    }
    return getDocroot();
}

QString StaticFileController::getDocroot()
{
    return this->docrootDynamic;
}

void StaticFileController::service(HttpRequest& request, HttpResponse& response)
{
    // Path will always start with "/" character
    QByteArray path = request.getPath();

    // Forbid access to files outside the docroot directory
    if (path.startsWith("/.."))
    {
        response.setStatus(403,"forbidden");
        response.write("403 forbidden",true);
        return;
    }

    // If the filename is a directory, append index.html
    if ( QFileInfo(docrootDynamic+path).isDir() || QFileInfo(docroot+path).isDir() )
    {
        //Redirect to home page
        if (!path.endsWith('/'))
        {
            response.setStatus(301,"moved permanently");
            response.setHeader("Location", request.getHeader("Referer") + path + "/");
            response.write("301 moved permanently",true);
            return;
        }

        path += "index.html";
    }

    // Try opening the file in dynamic docroot
    QFile file(docrootDynamic+path);
    if (!file.exists())
    {
        file.setFileName(docroot+path);
        if (!file.exists())
        {
            /*
            //Redirect to home page
            response.setStatus(301,"moved permanently");
            response.setHeader("Location", "http://" + request.getHeader("Host"));
            response.write("301 moved permanently",true);
            */
            response.setStatus(404,"file not found");
            response.write("Oops! 404 file not found",true);
            return;
        }
    }

    // Error opening the file
    if (!file.open(QIODevice::ReadOnly))
    {
        response.setStatus(403,"forbidden");
        response.write("403 forbidden",true);
        return;
    }

    //Set MIME type for the response
    SetContentType(path,response);

    //We don't implement Cache-Control at all
    response.setHeader("Cache-Control", "no-cache");

    // Blast out the file content
    while (!file.atEnd() && !file.error())
        response.write(file.read(65536));
    file.close();
}

// Static function
void StaticFileController::SetContentType(QString fileName, HttpResponse& response)
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
