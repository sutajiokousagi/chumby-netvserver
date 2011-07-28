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
echo "Stopping previous AP"
if ps ax | grep -v grep | grep hostapd > /dev/null
then
	killall hostapd
fi
if ps ax | grep -v grep | grep dnsmasq > /dev/null
then
	killall dnsmasq
fi

# Bring up network interface
ifconfig $(ls -1 /sys/class/net/ | grep wlan | head -1) up

# Start NetworkManager
/etc/init.d/NetworkManager restart