#ifndef CURSORCONTROLLER_H
#define CURSORCONTROLLER_H

#include "httprequesthandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

class CursorController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(CursorController);
public:

    /** Constructor */
    CursorController(QSettings* settings, QObject* parent = 0);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:

    int                     uinp_fd;        // Input device file descriptor
    struct uinput_user_dev  uinp;           // uInput device structure
    struct input_event      event;          // Input device structure

    int     resX;
    int     resY;

    // Shortcuts
    void    send_mouse_click_left(int x = -1, int y = -1);
    void    send_mouse_click_right(int x = -1, int y = -1);
    void    send_mouse_click_middle(int x = -1, int y = -1);

    // Raw sending functions
    int     setup_uinput_device(int maxX, int maxY);
    void    send_mouse_move_abs(int x, int y);
    void    send_mouse_move_relative(int x, int y);
    void    send_mouse_down(int button);
    void    send_mouse_up(int button);
    void    send_key_down(int keyCode);
    void    send_key_up(int keyCode);

};

#endif // CURSORCONTROLLER_H
