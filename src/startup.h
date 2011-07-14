#ifndef STARTUP_H
#define STARTUP_H

#include <qtsingleapplication.h>
#include <QByteArray>
#include <QStringList>

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
    QByteArray processStatelessCommand(QByteArray command, QStringList argsList = QStringList());
};

#endif // STARTUP_H
