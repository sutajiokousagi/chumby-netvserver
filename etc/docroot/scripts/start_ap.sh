#!/bin/sh

#
#

INTIF=wlan1

CONFPATH=/mnt/storage/ap_NeTV/hostapd.conf
DNSCONF=/mnt/storage/ap_NeTV/dnsmasq.conf

if [ $# -gt 0 ];
then
	CONFPATH=$1
fi

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

echo ${CONFPATH}

if [ ! -e ${CONFPATH} ];
then
	echo "Configuration file not found"
	return
fi

# Stop previous AP

WIFISTOP=$(/sbin/ifconfig | grep ${INTIF})
if [ ${#WIFISTOP} -gt 3 ];
then
	echo "Stopping previous AP"
	${SCRIPTPATH}/stop.sh
fi

# Bring down the interface
ifconfig ${INTIF} down

# Run hostapd, to set up the access point
cp ${CONFPATH} /tmp/hostapd.conf
hostapd -B /tmp/hostapd.conf

# Run dnsmasq, which is a combination DNS relay and DHCP server.
mkdir -p /var/lib/misc
dnsmasq -i ${INTIF} -F 10.0.0.100,10.0.0.250,3600 -K -C ${DNSCONF}

# Start web server (we didn't kill it)
#httpd -h /www

# Extract IP address of wlan0
WIFIUP=$(/sbin/ifconfig ${INTIF} | grep 'UP')
if [ ${#WIFIUP} -lt 4 ];
then
	echo "start.sh: Failed to bring up "${INTIF}
	exit
fi

# Bring up the internal wifi interface.
ifconfig ${INTIF} 10.0.0.1

IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')
if [ "x${IP}" != "x10.0.0.1" ];
then
	echo "start.sh: Failed to set IP address "${INTIF}
	exit
fi
