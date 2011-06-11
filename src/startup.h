#ifndef STARTUP_H
#define STARTUP_H

#include <qtsingleapplication.h>

/**
  The main class of the application
*/
class Startup : public QtSingleApplication
{
    Q_OBJECT

public:

    Startup(int argc, char *argv[]);

private slots:

    void receiveArgs(const QString &argsString);

protected:

    void start();
};

#endif // STARTUP_H
