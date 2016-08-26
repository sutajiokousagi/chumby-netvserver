#ifndef NETVHANDLERS_H
#define NETVHANDLERS_H

#include "netvserverapplication.h"

DECLARE_NETV_HANDLER(handleChromaKey);
DECLARE_NETV_HANDLER(handleGetJpeg);
DECLARE_NETV_HANDLER(handleGetUrl);
DECLARE_NETV_HANDLER(handleDefault);
DECLARE_NETV_HANDLER(handleEnableSsh);
DECLARE_NETV_HANDLER(handleInitialHello);
DECLARE_NETV_HANDLER(handleGetParam);

#endif // NETVHANDLERS_H
