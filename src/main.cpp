#include "startup.h"
#include <unistd.h>
#include <stdio.h>
#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QStringList>

bool isRunning()
{
    QProcess *newProc = new QProcess();
    newProc->start("/bin/pidof", QStringList(APPNAME));
    newProc->waitForFinished();

    QByteArray buffer = newProc->readAllStandardOutput();
    newProc->close();
    delete newProc;
    newProc = NULL;

    QStringList pidList = QString(buffer.trimmed()).split(" ", QString::SkipEmptyParts);
    if (pidList.length() > 1)
        return true;

    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication instance(argc, argv);
    instance.setApplicationName(APPNAME);
    instance.setOrganizationName(ORGANISATION);

    if (isRunning()) {
        qDebug("Another NeTVServer is already running");
        return 1;
    }

    QStringList argsList = instance.arguments();
    QString argsString = argsList.join(ARGS_SPLIT_TOKEN);

    // If the args list contains "-d", then daemonize
    int temp = 0;
    if (argsList.contains("-d"))
        temp = daemon(0, 0);

    // Print out console arguments (if any)
    if (argsList.count() > 0) {
        qDebug("Starting new NeTVServer with console args: ");
        qDebug("%s", qPrintable(argsString));
    }

    Startup startup;
    bool toBeReturn = startup.receiveArgs(argsString);
    if (toBeReturn == true)
        return 0;

    return instance.exec();
}
