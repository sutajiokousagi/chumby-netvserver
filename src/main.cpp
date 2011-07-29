#include "startup.h"
#include <unistd.h>
#include <QCoreApplication>
#include "stdio.h"

int main(int argc, char *argv[])
{
    QCoreApplication instance(argc, argv);
    instance.setApplicationName(APPNAME);
    instance.setOrganizationName(ORGANISATION);

    QStringList argsList = instance.arguments();
    QString argsString = argsList.join(ARGS_SPLIT_TOKEN);

    // If the args list contains "-d", then daemonize
    if (argsList.contains("-d"))
        daemon(0, 0);

    printf("Starting new NeTVServer with args: ");
    printf("%s", argsString.toLatin1().constData());

    Startup startup;
    startup.receiveArgs(argsString);

    return instance.exec();
}
