/**
  @file
  @author Stefan Frings
  @version $Id: fileuploadcontroller.cpp 894 2010-10-18 20:54:32Z stefan $
*/

#include "framebuffercontroller.h"
#include <QtGui>
#include <QApplication>
#include <QPixmap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

FramebufferController::FramebufferController() {}

void FramebufferController::service(HttpRequest& /* request */, HttpResponse& response)
{
    //Resize it. Hmmm.
    //One embedded linux, it only works if we use QImage. Why!?!?
    QSize scaledSize = QSize(400,240);

//Grab the framebuffer
#if defined (Q_WS_QWS)
    QImage image = QPixmap::grabWindow(QApplication::desktop()->winId()).toImage();
#else
    QPixmap image = QPixmap::grabWindow(QApplication::desktop()->winId());
#endif
    image = image.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);    //Qt::FastTransformation or Qt::SmoothTransformation

    //Convert it to a QByteArray
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");

    //Write directly to HTTP response
    response.setHeader("Content-Type", "image/jpeg");
    response.write(bytes, true);
}
