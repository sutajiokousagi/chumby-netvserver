#include "nm_interface.h"

OrgFreedesktopNetworkManagerInterface::OrgFreedesktopNetworkManagerInterface(QObject *parent)
    : QDBusAbstractInterface(OrgFreedesktopNetworkManagerInterface::staticServiceName(),
                            OrgFreedesktopNetworkManagerInterface::staticPathName(),
                            OrgFreedesktopNetworkManagerInterface::staticInterfaceName(),
                            QDBusConnection::systemBus(),
                            parent)
{

}

OrgFreedesktopNetworkManagerInterface::~OrgFreedesktopNetworkManagerInterface()
{

}

