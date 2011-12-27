#!/bin/sh
#
#

echo "Checking network condition..."

# Check for real Internet access
google=$(curl www.google.com 2>&1)
if [ ${#google} -gt 32 ];
then
	echo "Everything is fine."
	exit 0;
fi

# If NetworkManger is not running, we are in AP mode (unconfigured), do nothing
if [ -z "$(pidof NetworkManager)" ];
then
	echo "NetworkManager is not running, do nothing."
	exit 0;
fi

# No network_config while NOT in AP mode for whatever reason
if [ ! -e /psp/network_config ];
then
	echo "/psp/network_config not found while NOT in AccessPoint mode. Back to AccessPoint mode."
	/usr/share/netvserver/docroot/scripts/start_ap.sh
	exit 0;
fi

# MAC & IP address
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
INTIF=$(echo ${INTIF//mon./})
IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')
MAC=$(ifconfig ${INTIF} | grep ${INTIF} | tr -s ' ' | cut -d ' ' -f5)

# If IP address is valid, do nothing (router's WAN side is down, not our fault)
if [ ! -z "${IP}" ];
then
	echo "Valid IP address for ${INTIF}: ${IP}. Probably router's WAN side is down."
	echo "Do nothing."
	exit 0;
fi

# Reset network interface, hopefully NetworkManager will pick it up again
echo "Respawn ${INTIF} network interface."
ifconfig $INTIF down
sleep 1
ifconfig $INTIF up

exit 0;




# Unused codes




# Network manager state
nmstate="4"
if [ ! -z "$(pidof NetworkManager)" ];
then
	nmstate=$(dbus-send --system --print-reply --dest='org.freedesktop.NetworkManager' /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Get string:'' string:'State' | grep uint32 | awk '{print $3}')
fi
if [ "$nmstate" == "2" ]; then
	internet='connecting'
elif [ "$nmstate" == "3" ]; then
	internet='true'
elif [ "$nmstate" == "4" -o "$nmstate" == "1" ]; then
	internet='false'
fi
