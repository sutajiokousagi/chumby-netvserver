#include "inputdev.h"
#include <QDebug>

InputDevice::InputDevice(QObject *parent) : QObject(parent)
{
    this->resX = 1280;  //QApplication::desktop()->screenGeometry().width();
    this->resY = 720;   //QApplication::desktop()->screenGeometry().height();
    isRelative = true;

    setup_uinput_device(this->resX, this->resY, this->isRelative);
}

InputDevice::~InputDevice()
{
    if (uinp_fd != NULL)
        close(uinp_fd);
    uinp_fd = NULL;
}

/*
void InputDevice::service(HttpRequest& request, HttpResponse& response)
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
        qDebug("InputDevice: mouse down (%d, %d)", x,y);
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
        qDebug("InputDevice: mouse up (%d, %d)", x,y);
        response.write("Mouse up: ", false);
        QByteArray temp;
        response.write( temp.setNum(x) );
        response.write(",");
        response.write( temp.setNum(y) );
        response.write("", true);
        send_mouse_up(BTN_LEFT);
    }
}
*/

//-------------------------------------------------------------------------------------
// Shortcuts
//-------------------------------------------------------------------------------------

void InputDevice::send_mouse_click_left(int x /*= -1*/, int y /*= -1*/)
{
    if (x != -1 && y != -1)
        send_mouse_move_abs(x,y);
    send_mouse_down(BTN_LEFT);
    send_mouse_up(BTN_LEFT);
}

void InputDevice::send_mouse_click_right(int x /*= -1*/, int y /*= -1*/)
{
    if (x != -1 && y != -1)
        send_mouse_move_abs(x,y);
    send_mouse_down(BTN_RIGHT);
    send_mouse_up(BTN_RIGHT);
}

void InputDevice::send_mouse_click_middle(int x /*= -1*/, int y /*= -1*/)
{
    if (x != -1 && y != -1)
        send_mouse_move_abs(x,y);
    send_mouse_down(BTN_MIDDLE);
    send_mouse_up(BTN_MIDDLE);
}


//-------------------------------------------------------------------------------------
// Raw sending functions
//-------------------------------------------------------------------------------------

void InputDevice::send_mouse_move_abs(int x, int y)
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

void InputDevice::send_mouse_move_relative(int x, int y)
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

void InputDevice::send_mouse_down(int button)
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

void InputDevice::send_mouse_up(int button)
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

void InputDevice::send_key_down(int keyCode)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = (__u16)keyCode;
    event.value = 1;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void InputDevice::send_key_up(int keyCode)
{
    if (uinp_fd == NULL)
        return;

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = (__u16)keyCode;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

int InputDevice::setup_uinput_device(int maxX, int maxY, bool isRelative, const char * device /* = NULL */)
{
    //printf("Setup input device with resolution [ %d , %d ]", maxX, maxY);

    // Auto select which device to inject to
    if (device == NULL)
        device = isRelative ? INPUT_DEV_PATH : INPUT_DEV_PATH;
    this->device_path = device;

    // Open the input device
    uinp_fd = open(device, O_WRONLY | O_NDELAY);
    if (uinp_fd == NULL)
    {
        qDebug("InputDevice: unable to open %s", device);
        return -1;
    }
    qDebug("InputDevice: injecting to %s", device);

#if !defined (Q_WS_QWS)
/*
    //This will be used on actual NeTV device which has no input to inject to.
    //We will need to recompile a new kernel that has 'uinput' module for this.

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

    int i=0;
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
        //qDebug("Unable to create UINPUT device.");
        return -1;
    }
*/
#endif  //!Q_WS_QWS

    return 1;
}

void InputDevice::setRelativeMode()
{
    //Set the flag
    if (isRelative)
        return;
    isRelative = true;

    //Reinitialize the device
    if (uinp_fd != NULL)
    {
        ioctl(uinp_fd, UI_DEV_DESTROY);
        close(uinp_fd);
        uinp_fd = NULL;
    }
    setup_uinput_device(this->resX, this->resY, this->isRelative);
}

void InputDevice::setAbsoluteMode()
{
    //Set the flag
    if (!isRelative)
        return;
    isRelative = false;

    //Reinitialize the device
    if (uinp_fd != NULL)
    {
        ioctl(uinp_fd, UI_DEV_DESTROY);
        close(uinp_fd);
        uinp_fd = NULL;
    }
    setup_uinput_device(this->resX, this->resY, this->isRelative);
}

bool InputDevice::isRelativeMode()
{
    return isRelative;
}

bool InputDevice::isAbsoluteMode()
{
    return !isRelative;
}


void InputDevice::slot_sendKey(int keyCode, bool isPressed, bool autoRepeat)
{
    Q_UNUSED(autoRepeat);

    if (isPressed)      send_key_down(keyCode);
    else                send_key_up(keyCode);
}

void InputDevice::slot_sendKey(QByteArray keyName, bool isPressed, bool autoRepeat)
{
    Q_UNUSED(autoRepeat);
    int keyCode = KEY_ENTER;

    if (keyName[0] >= 'a' && keyName[0] <= 'z')             keyCode = KEY_A + (keyName[0]-'a');
    else if (keyName[0] >= '0' && keyName[0] <= '9')        keyCode = KEY_0 + (keyName[0]-'0');
    else if (keyName == "enter")                            keyCode = KEY_ENTER;
    else if (keyName == "space" || keyName == " ")          keyCode = KEY_SPACE;
    else if (keyName == "backspace")                        keyCode = KEY_BACKSPACE;
    else if (keyName == "del" || keyName == "delete")       keyCode = KEY_DELETE;
    else if (keyName == "tab")                              keyCode = KEY_TAB;
    else if (keyName == "shift")                            keyCode = KEY_LEFTSHIFT;
    else if (keyName == "caps" || keyName == "capslock")    keyCode = KEY_CAPSLOCK;
    else if (keyName == "up")                               keyCode = KEY_UP;
    else if (keyName == "down")                             keyCode = KEY_DOWN;
    else if (keyName == "left")                             keyCode = KEY_LEFT;
    else if (keyName == "right")                            keyCode = KEY_RIGHT;
    else if (keyName == "esc" || keyName == "escape")       keyCode = KEY_ESC;
    else if (keyName == "close")                            keyCode = KEY_CLOSE;
    else if (keyName == "exit")                             keyCode = KEY_EXIT;
    else if (keyName == "-")                                keyCode = KEY_MINUS;
    else if (keyName == "=")                                keyCode = KEY_EQUAL;
    else if (keyName == "{")                                keyCode = KEY_LEFTBRACE;
    else if (keyName == "}")                                keyCode = KEY_RIGHTBRACE;
    else if (keyName == ";")                                keyCode = KEY_SEMICOLON;
    else if (keyName == "'")                                keyCode = KEY_APOSTROPHE;
    else if (keyName == "`")                                keyCode = KEY_GRAVE;
    else if (keyName == "\\")                               keyCode = KEY_BACKSLASH;
    else if (keyName == ",")                                keyCode = KEY_COMMA;
    else if (keyName == ".")                                keyCode = KEY_DOT;
    else if (keyName == "/")                                keyCode = KEY_SLASH;
    else if (keyName == "$")                                keyCode = KEY_DOLLAR;
    else if (keyName == "?")                                keyCode = KEY_QUESTION;
    else if (keyName == "home")                             keyCode = KEY_HOME;
    else if (keyName == "end")                              keyCode = KEY_END;
    else if (keyName == "pageup" || keyName == "pgup")      keyCode = KEY_PAGEUP;
    else if (keyName == "pagedown" || keyName == "pgdown")  keyCode = KEY_PAGEDOWN;
    else if (keyName == "play")                             keyCode = KEY_PLAY;
    else if (keyName == "pause")                            keyCode = KEY_PAUSE;
    else if (keyName == "stop")                             keyCode = KEY_STOP;
    else if (keyName == "next")                             keyCode = KEY_NEXT;
    else if (keyName == "previous")                         keyCode = KEY_PREVIOUS;
    else if (keyName == "print")                            keyCode = KEY_PRINT;

    else if (keyName == "search")                           keyCode = KEY_SEARCH;
    else if (keyName == "cam" || keyName == "camera")       keyCode = KEY_CAMERA;
    else if (keyName == "menu")                             keyCode = KEY_MENU;
    else if (keyName == "back")                             keyCode = KEY_BACK;
    else if (keyName == "power")                            keyCode = KEY_POWER;
    else if (keyName == "address")                          keyCode = KEY_ADDRESSBOOK;

    else if (keyName == "forward")                          keyCode = KEY_FORWARD;
    else if (keyName == "volup")                            keyCode = KEY_VOLUMEUP;
    else if (keyName == "voldown")                          keyCode = KEY_VOLUMEDOWN;
    else if (keyName == "mute")                             keyCode = KEY_MUTE;
    else if (keyName == "refresh")                          keyCode = KEY_REFRESH;
    else if (keyName == "fn")                               keyCode = KEY_FN;

    else if (keyName == "red")                              keyCode = KEY_RED;
    else if (keyName == "green")                            keyCode = KEY_GREEN;
    else if (keyName == "blue")                             keyCode = KEY_BLUE;
    else if (keyName == "yellow")                           keyCode = KEY_YELLOW;
    else if (keyName == "chup")                             keyCode = KEY_CHANNELUP;
    else if (keyName == "chdown")                           keyCode = KEY_CHANNELDOWN;
    else if (keyName == "last")                             keyCode = KEY_LAST;
    else if (keyName == "ok")                               keyCode = KEY_OK;
    else if (keyName == "opt" || keyName == "option")       keyCode = KEY_OPTION;
    else if (keyName == "info")                             keyCode = KEY_INFO;
    else if (keyName == "rec" || keyName == "record")       keyCode = KEY_RECORD;
    else if (keyName == "guide" || keyName == "help")       keyCode = KEY_HELP;
    else if (keyName == "rewind" || keyName == "rwd")       keyCode = KEY_REWIND;
    else if (keyName == "fastforward" || keyName == "ffd")  keyCode = KEY_FASTFORWARD;
    else if (keyName == "subtitle" || keyName == "caption") keyCode = KEY_SUBTITLE;
    else if (keyName == "zoomin")                           keyCode = KEY_ZOOMIN;
    else if (keyName == "zoomout")                          keyCode = KEY_ZOOMOUT;
    else if (keyName == "zoom")                             keyCode = KEY_ZOOM;
    else return;

    //else if (keyName == "@" || keyName == "at")             keyCode = KEY_AT;     //missing this
    //else if (keyName == "#")                                keyCode = KEY_AT;     //missing this
    //else if (keyName == ".com")                             keyCode = KEY_AT;     //missing this
    //else if (keyName == "chumby")                           keyCode = KEY_AT;     //missing this
    //else if (keyName == "cpanel")                           keyCode = KEY_AT;     //missing this

    if (isPressed)      send_key_down(keyCode);
    else                send_key_up(keyCode);
}
