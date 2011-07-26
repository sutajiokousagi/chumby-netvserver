#ifndef QCHUMBYIRKB_QWS_H
#define QCHUMBYIRKB_QWS_H

#include <QObject>
#include <termios.h>

QT_MODULE(Gui)


class InputListener : public QObject
{
    Q_OBJECT
public:
    InputListener(QObject *parent = NULL, const QString &device = "/dev/input/event1");
    ~InputListener();

private:
    void switchLed(int, bool);

private Q_SLOTS:
    void readKeycode();

signals:

    void signal_keyInput(quint16 keycode, bool isPress, bool autoRepeat);

private:
    int                           m_fd;
    int                           m_tty_fd;
    struct termios                m_tty_attr;
    int                           m_orig_kbmode;
};

#endif // QCHUMBYIRKB_QWS_H
