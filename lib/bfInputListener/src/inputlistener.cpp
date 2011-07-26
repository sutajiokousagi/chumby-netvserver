#include "inputlistener.h"

#include <QWSKeyboardHandler>
#include <QSocketNotifier>
#include <QStringList>

#include <qplatformdefs.h>

#include <errno.h>
#include <linux/kd.h>
#include <linux/input.h>

InputListener::InputListener(QObject* /* parent */, const QString &device)
    : m_fd(-1),
      m_tty_fd(-1),
      m_orig_kbmode(K_XLATE)
{
    int repeat_delay = -1;
    int repeat_rate = -1;

    m_fd = QT_OPEN(device.toLocal8Bit().constData(), O_RDWR, 0);
    if (m_fd >= 0)
    {
        if (repeat_delay > 0 && repeat_rate > 0) {
            int kbdrep[2] = { repeat_delay, repeat_rate };
            ::ioctl(m_fd, EVIOCSREP, kbdrep);
        }

        QSocketNotifier *notifier;
        notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(readKeycode()));

        qDebug("InputListener: started on %s", qPrintable(device));
    }
    else
    {
        qWarning("InputListener: Cannot open input device '%s': %s", qPrintable(device), strerror(errno));
        return;
    }
}

InputListener::~InputListener()
{
    if (m_tty_fd >= 0)
    {
        ::ioctl(m_tty_fd, KDSKBMODE, m_orig_kbmode);
        tcsetattr(m_tty_fd, TCSANOW, &m_tty_attr);
    }
    if (m_fd >= 0)
        QT_CLOSE(m_fd);
}

void InputListener::switchLed(int led, bool state)
{
    struct ::input_event led_ie;
    ::gettimeofday(&led_ie.time, 0);
    led_ie.type = EV_LED;
    led_ie.code = led;
    led_ie.value = state;

    QT_WRITE(m_fd, &led_ie, sizeof(led_ie));
}

void InputListener::readKeycode()
{
    struct ::input_event buffer[32];
    int n = 0;

    forever
    {
        n = QT_READ(m_fd, reinterpret_cast<char *>(buffer) + n, sizeof(buffer) - n);

        if (n == 0) {
            qWarning("Got EOF from the input device.");
            return;
        } else if (n < 0 && (errno != EINTR && errno != EAGAIN)) {
            qWarning("Could not read from input device: %s", strerror(errno));
            return;
        } else if (n % sizeof(buffer[0]) == 0) {
            break;
        }
        else {
            qWarning("Other");
        }
    }

    n /= sizeof(buffer[0]);

    for (int i = 0; i < n; ++i)
    {
        if (buffer[i].type != EV_KEY)
            continue;

        quint16 code = buffer[i].code;
        qint32 value = buffer[i].value;

        //QWSKeyboardHandler::KeycodeAction ka;
        //ka = m_handler->processKeycode(code, value != 0, value == 2);

        emit signal_keyInput(code, value != 0, value == 2);
    }
}
