#ifndef STARTUP_H
#define STARTUP_H

#include <QObject>
#include <QByteArray>
#include <QStringList>

/** Name of this application */
#define APPNAME "NeTVServer"

/** Publisher of this application */
#define ORGANISATION "Chumby Industries"

/** Short description of this application */
#define DESCRIPTION "Customized web server based on Qt"

/** The special string used to split & join arguements */
#define ARGS_SPLIT_TOKEN    "|~|"

/**
  The main class of the application
*/
class Startup : public QObject
{
    Q_OBJECT

public:

    Startup(QObject* parent=0);

public slots:

    void receiveArgs(const QString &argsString);

protected:

    void start();
    QByteArray processStatelessCommand(QByteArray command, QStringList argsList = QStringList());
};

#endif // STARTUP_H
