#include "nm_ap_interface.h"

OrgFreedesktopNetworkManagerAPInterface::OrgFreedesktopNetworkManagerAPInterface(QObject *parent)
    : QDBusAbstractInterface(OrgFreedesktopNetworkManagerAPInterface::staticServiceName(),
                            OrgFreedesktopNetworkManagerAPInterface::staticPathName(),
                            OrgFreedesktopNetworkManagerAPInterface::staticInterfaceName(),
                            QDBusConnection::systemBus(),
                            parent)
{

}

OrgFreedesktopNetworkManagerAPInterface::~OrgFreedesktopNetworkManagerAPInterface()
{

}

