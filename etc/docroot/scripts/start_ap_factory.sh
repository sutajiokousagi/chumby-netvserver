#!/bin/sh

INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
INTIF=$(echo ${INTIF//mon./})

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
	exit
fi

# MAC address (last 5 characters)
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
INTIF=$(echo ${INTIF//mon./})
MAC=$(ifconfig ${INTIF} | grep ${INTIF} | tr -s ' ' | cut -d ' ' -f5)
SSID=$(echo "${MAC}" | cut -c 12-17)

# Stop previous AP
echo "Stopping previous AP"
if [ ! -z "$(pidof hostapd)" ];
then
	killall hostapd
fi
if [ ! -z "$(pidof dnsmasq)" ];
then
	killall dnsmasq
fi
if [ ! -z "$(pidof NetworkManager)" ];
then
	/etc/init.d/NetworkManager stop
	sleep 1
fi
if [ ! -z "$(pidof NetworkManager)" ];
then
	killall NetworkManager
fi

# Bring down the interface
ifconfig ${INTIF} down
sleep 1

# Random channel
RANDOM=$(date '+%s')
CHANNEL=$(echo $[($RANDOM % 10) + 1])

# Run hostapd, to set up the access point
cp -f ${CONFPATH} /tmp/hostapd.conf
echo "interface=${INTIF}" >> /tmp/hostapd.conf
echo "ssid=NeTV${SSID}" >> /tmp/hostapd.conf
echo "channel=${CHANNEL}" >> /tmp/hostapd.conf
hostapd -B /tmp/hostapd.conf

sleep 1

# Extract IP address of wlan0
WIFIUP=$(/sbin/ifconfig ${INTIF} | grep 'UP')
if [ ${#WIFIUP} -lt 4 ];
then
	echo "start.sh: failed to bring up "${INTIF}
	exit
fi

# Bring up the internal wifi interface.
ifconfig ${INTIF} 192.168.100.1

IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')
if [ "x${IP}" != "x192.168.100.1" ];
then
	echo "start.sh: failed to set IP address on "${INTIF}
	exit
fi

# Run dnsmasq, which is a combination DNS relay and DHCP server.
mkdir -p /var/lib/misc
if [ -z "$(pidof dnsmasq)" ];
then
	dnsmasq --leasefile-ro -z -F 192.168.100.100,192.168.100.130,1h -i ${INTIF} -K -C ${DNSCONF}
fi
