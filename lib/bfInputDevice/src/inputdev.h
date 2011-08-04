#ifndef InputDevice_H
#define InputDevice_H

#include "httprequesthandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define INPUT_DEV_PATH "/dev/input/event1"

class InputDevice : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputDevice)
public:

    /** Constructor */
    InputDevice(QObject* parent = 0);

    /** Desctructor */
    virtual ~InputDevice();

public:

    //Setup
    void    setRelativeMode();
    void    setAbsoluteMode();
    bool    isRelativeMode();
    bool    isAbsoluteMode();

    // Shortcuts
    void    send_mouse_click_left(int x = -1, int y = -1);
    void    send_mouse_click_right(int x = -1, int y = -1);
    void    send_mouse_click_middle(int x = -1, int y = -1);

    // Raw sending functions
    void    send_mouse_move_abs(int x, int y);
    void    send_mouse_move_relative(int x, int y);
    void    send_mouse_wheel_relative(bool isUp);
    void    send_mouse_down(int button = -1);
    void    send_mouse_up(int button = -1);
    void    send_key_down(int keyCode);
    void    send_key_up(int keyCode);

private:

    int                     uinp_fd;        // Input device file descriptor
    struct uinput_user_dev  uinp;           // uInput device structure
    struct input_event      event;          // Input device structure

    const char * device_path;
    bool    isRelative;
    int     resX;
    int     resY;

    int     setup_uinput_device(int maxX, int maxY, bool isRelative, const char * device = NULL);


private slots:

    void slot_sendKey(int keyCode, bool isPressed, bool autoRepeat);
    void slot_sendKey(QByteArray keyName, bool isPressed, bool autoRepeat);
};

#endif // InputDevice_H
