#include "framebuffercontroller.h"
#include <QtGui>
#include <QApplication>
#include <QPixmap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

FramebufferController::FramebufferController(QSettings* settings, QObject* parent) : HttpRequestHandler(parent)
{
    width=settings->value("width","400").toInt();
    height=settings->value("height","240").toInt();
    quality=settings->value("quality","70").toInt();
    format=settings->value("format","JPG").toByteArray();
    smooth=settings->value("smooth","false").toString() == "true" ? true : false;

    printf("FramebufferController:: %d %d %d %s", width, height, quality, format.constData());
}

void FramebufferController::service(HttpRequest& request, HttpResponse& response)
{
    //Get the desired size
    int w = request.getParameter("w").toInt();
    int h = request.getParameter("h").toInt();
    if (width >= 10 && height >= 10) {
        this->width = w;
        this->height = h;
    }
    QSize scaledSize = QSize(400,240);

    //Get the quality
    int q = request.getParameter("q").toInt();
    if (q >= -1 && q <= 100)
        this->quality = q;

    //Get the format
    QByteArray f = request.getParameter("f");
    f = f.toUpper();
    if (f == "JPG" || f == "JPEG" || f == "BMP" || f == "PNG")
        this->format = f;

    //Get the smooth setting
    QByteArray s = request.getParameter("s");
    s = s.toUpper();
    if (s != "")
        this->smooth = (s == "1" || s == "TRUE" || s == "T" || s == "YES") ? true : false;

    //One embedded linux, it only works if we use QImage!?!?

//Grab the framebuffer
#if defined (Q_WS_QWS)
    QImage image = QPixmap::grabWindow(QApplication::desktop()->winId()).toImage();
#else
    QPixmap image = QPixmap::grabWindow(QApplication::desktop()->winId());
#endif
    image = image.scaled(scaledSize, Qt::IgnoreAspectRatio, this->smooth ? Qt::SmoothTransformation : Qt::FastTransformation);

    //Convert it to a QByteArray
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, this->format.constData(), this->quality);

    //Write directly to HTTP response
    response.setHeader("Content-Type", QByteArray("image/") + this->format.toLower());
    response.write(bytes, true);
}
