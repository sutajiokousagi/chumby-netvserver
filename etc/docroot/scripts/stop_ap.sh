#!/bin/sh

ifconfig wlan1 down

#killall httpd
killall hostapd
killall dnsmasq
killall wpa_supplicant
killall wpa_cli
killall udhcpc
killall mDNSPublish
killall mDNSResponder

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

if [ -e ${SCRIPTPATH}/dhcp_current_client ];
then
	rm ${SCRIPTPATH}/dhcp_current_client
fi
