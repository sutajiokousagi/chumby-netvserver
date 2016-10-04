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
DECLARE_NETV_HANDLER(handleTickerEvent);

DECLARE_NETV_HANDLER(handleGetLocalWidgets);
DECLARE_NETV_HANDLER(handleGetLocalWidgetConfig);
DECLARE_NETV_HANDLER(handleGetLocalFileContents);
DECLARE_NETV_HANDLER(handleGetChannelInfo);

#endif // NETVHANDLERS_H
