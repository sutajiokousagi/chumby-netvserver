#ifndef STARTUP_H
#define STARTUP_H

#include <QObject>

/** Name of this application */
#define APPNAME "NeTVServer"

/** Publisher of this application */
#define ORGANISATION " "

class Startup : public QObject
{
    Q_OBJECT

public:

    Startup(QObject* parent=0);

};

#endif // STARTUP_H
