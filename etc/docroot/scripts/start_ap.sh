#!/bin/sh

INTIF=wlan0

# Path of various configuration files
CONFPATH=/usr/share/netvserver/docroot/scripts/hostapd.conf
DNSCONF=/usr/share/netvserver/docroot/scripts/dnsmasq.conf

if [ $# -gt 0 ];
then
	CONFPATH=$1
fi

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

if [ ! -e ${CONFPATH} ];
then
	echo "hostapd Configuration file not found"
	return
fi

# Stop previous AP
echo "Stopping previous AP"
killall hostapd
killall dnsmasq
/etc/init.d/NetworkManager stop

# Bring down the interface
ifconfig ${INTIF} down
sleep 1

# Run hostapd, to set up the access point
cp ${CONFPATH} /tmp/hostapd.conf
hostapd -B /tmp/hostapd.conf

sleep 1

# Extract IP address of wlan0
WIFIUP=$(/sbin/ifconfig ${INTIF} | grep 'UP')
if [ ${#WIFIUP} -lt 4 ];
then
	echo "start.sh: Failed to bring up "${INTIF}
	exit
fi

# Bring up the internal wifi interface.
ifconfig ${INTIF} 192.168.100.1

IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')
if [ "x${IP}" != "x192.168.100.1" ];
then
	echo "start.sh: Failed to set IP address "${INTIF}
	exit
fi

# Run dnsmasq, which is a combination DNS relay and DHCP server.
mkdir -p /var/lib/misc
dnsmasq -F 192.168.100.100,192.168.100.130,1h -i ${INTIF} -K -C ${DNSCONF}
