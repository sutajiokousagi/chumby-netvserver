#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

if [ -e ${SCRIPTPATH}/dhcp_current_client ];
then
	rm ${SCRIPTPATH}/dhcp_current_client
fi

# Stop the AP
killall hostapd
killall dnsmasq

# Reset network interface
ifconfig wlan0 up

# Start NetworkManager
/etc/init.d/NetworkManager restart

