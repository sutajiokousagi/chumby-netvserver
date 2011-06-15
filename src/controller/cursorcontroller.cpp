/**
  @file
  @author Stefan Frings
  @version $Id: CursorController.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "cursorcontroller.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QApplication>
#include <QDesktopWidget>

#if defined (Q_WS_QWS)
    #define INPUT_DEV_PATH "/dev/input/by-id/soc-noserial-event-ts"
#else
    #define INPUT_DEV_PATH "/dev/uinput"
#endif

CursorController::CursorController(QSettings* /* settings */, QObject* parent)
    :HttpRequestHandler(parent)
{
    this->resX = QApplication::desktop()->screenGeometry().width();
    this->resY = QApplication::desktop()->screenGeometry().height();

    setup_uinput_device(this->resX, this->resY);
}

void CursorController::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "text/html");

    //Example full request "/cursor?x=0.123&y=0.456&c=d"
    QByteArray xStr = request.getParameter("x");
    QByteArray yStr = request.getParameter("y");
    QByteArray cStr = request.getParameter("c");

    if (xStr == "" || yStr == "")
    {
        response.write("Invalid arguments.<br> Acceptable format: /cursor?x=0.123&y=0.456&c=d", true);
        return;
    }

    bool okx = false, oky = false;
    float xx = xStr.toFloat(&okx);
    float yy = yStr.toFloat(&oky);
    if (!okx || !oky)
    {
        response.write("Invalid arguments.<br> Acceptable format: /cursor?x=0.123&y=0.456&c=d", true);
        return;
    }

    //Event type (up, down, move)
    if (cStr == "")
        cStr = "m";

    int x = (int)(xx*this->resX);
    int y = (int)(yy*this->resY);

    if (cStr == "m")
    {
        send_mouse_move_abs(x,y);
    }
    else if (cStr == "d")
    {
        qDebug("CursorController: mouse down (%d, %d)", x,y);
        response.write("Mouse down: ", false);
        QByteArray temp;
        response.write( temp.setNum(x) );
        response.write(",");
        response.write( temp.setNum(y) );
        response.write("", true);
        send_mouse_down(BTN_LEFT);
    }
    else if (cStr == "u")
    {
        qDebug("CursorController: mouse up (%d, %d)", x,y);
        response.write("Mouse up: ", false);
        QByteArray temp;
        response.write( temp.setNum(x) );
        response.write(",");
        response.write( temp.setNum(y) );
        response.write("", true);
        send_mouse_up(BTN_LEFT);
    }
}

//-------------------------------------------------------------------------------------
// Shortcuts
//-------------------------------------------------------------------------------------

void CursorController::send_mouse_click_left(int x /*= -1*/, int y /*= -1*/)
{
    if (x != -1 && y != -1)
        send_mouse_move_abs(x,y);
    send_mouse_down(BTN_LEFT);
    send_mouse_up(BTN_LEFT);
}

void CursorController::send_mouse_click_right(int x /*= -1*/, int y /*= -1*/)
{
    if (x != -1 && y != -1)
        send_mouse_move_abs(x,y);
    send_mouse_down(BTN_RIGHT);
    send_mouse_up(BTN_RIGHT);
}

void CursorController::send_mouse_click_middle(int x /*= -1*/, int y /*= -1*/)
{
    if (x != -1 && y != -1)
        send_mouse_move_abs(x,y);
    send_mouse_down(BTN_MIDDLE);
    send_mouse_up(BTN_MIDDLE);
}

//-------------------------------------------------------------------------------------
// Raw sending functions
//-------------------------------------------------------------------------------------

int CursorController::setup_uinput_device(int maxX, int maxY)
{
    // Temporary variable
    int i=0;

    // Open the input device
    uinp_fd = open(INPUT_DEV_PATH, O_WRONLY | O_NDELAY);
    if (uinp_fd == NULL)
    {
        qDebug("CursorController: unable to open %s", INPUT_DEV_PATH);
        return -1;
    }

#if !defined (Q_WS_QWS)

    memset(&uinp,0,sizeof(uinp));
    strncpy(uinp.name, "NeTV UDP Input Device", UINPUT_MAX_NAME_SIZE);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_USB;
    uinp.id.vendor  = 0x1;
    uinp.id.product = 0x1;

    uinp.absmin[ABS_X]=0;
    uinp.absmax[ABS_X]=maxX;
    uinp.absfuzz[ABS_X]=0;
    uinp.absflat[ABS_X ]=0;
    uinp.absmin[ABS_Y]=0;
    uinp.absmax[ABS_Y]=maxY;
    uinp.absfuzz[ABS_Y]=0;
    uinp.absflat[ABS_Y]=0;

    // Setup the uinput device
    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinp_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(uinp_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_X);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_Y);

    for (i=0; i < 256; i++)
        ioctl(uinp_fd, UI_SET_KEYBIT, i);

    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_FORWARD);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_BACK);

    // Create input device into input sub-system
    write(uinp_fd, &uinp, sizeof(uinp));

    if ( ioctl(uinp_fd, UI_DEV_CREATE) )
    {
        qDebug("Unable to create UINPUT device.");
        return -1;
    }
#endif

    return 1;
}

void CursorController::send_mouse_move_abs(int x, int y)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_ABS;
    event.code = ABS_X;
    event.value = x;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_ABS;
    event.code = ABS_Y;
    event.value = y;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void CursorController::send_mouse_move_relative(int x, int y)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_REL;
    event.code = REL_X;
    event.value = x;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_REL;
    event.code = REL_Y;
    event.value = y;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void CursorController::send_mouse_down(int button)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = button;        //BTN_LEFT, BTN_RIGHT
    event.value = 1;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void CursorController::send_mouse_up(int button)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = button;        //BTN_LEFT, BTN_RIGHT
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void CursorController::send_key_down(int keyCode)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = keyCode;
    event.value = 1;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void CursorController::send_key_up(int keyCode)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = keyCode;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}
