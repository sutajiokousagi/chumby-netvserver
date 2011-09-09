#!/bin/sh
#
# Returning identification information of this hardware
# TODO: remove more network stuff, taking too long & too heavy to execute frequently
#       to have a better way to determin minimum client version required
#

# Give demo GUID if no guidgen.sh
if [ ! -e /usr/bin/guidgen.sh ]; then
	GUID=A620123B-1F0E-B7CB-0E11-921ADB7BE22A
else	
	GUID=$(/usr/bin/guidgen.sh)
fi

# DCID
DCID=$(dcid -o | grep -v "xml version=")
if [ -z "${DCID}" ]; then
	DCID='a long string of 1024 bytes'
fi

# Hardware version
hwver=$(chumby_version -h | sed "1 d")
if [ -z "${hwver}" ]; then
	hwver='10.1'
fi

# Software version
fwver=$(chumby_version -f)
if [ -z "${fwver}" ]; then
	fwver='1.0'
fi

# Flash version
flashver=$(chumbyflashplayer.x -v | sed '5!d')

# Network status
network_status=$(network_status.sh)
internet='connecting'
hasLinkTrue=$(echo $network_status | grep 'link="true"')
if [ ${#hasLinkTrue} -gt 10 ]; then
	internet='true'
fi

# MAC & IP address
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
MAC=$(ifconfig | grep ${INTIF} | tr -s ' ' | cut -d ' ' -f5)
IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')

# Public Internet IP address
if [ -z "${IP}" -o "${IP}" == "192.168.1.100" -o -z "$(pidof NetworkManager)" ];
then
	PUBLIC_IP=$IP
else
	PUBLIC_IP=$(curl -s http://whatismyip.org)
	if [ ${#PUBLIC_IP} -gt 20 ]; then
		PUBLIC_IP=$IP
	fi
fi

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

#
# Print them out
#
echo "<guid>${GUID}</guid>"
echo "<dcid>${DCID}</dcid>"
echo "<hwver>${hwver}</hwver>"
echo "<fwver>${fwver}</fwver>"
echo "<minAndroid>0.4.91</minAndroid>"
echo "<minIOS>0.0.0</minIOS>"
echo "<flashver>${flashver}</flashver>"
echo "<internet>${internet}</internet>"
echo "<mac>${MAC}</mac>"
echo "<ip>${IP}</ip>"
echo "<public_ip>${PUBLIC_IP}</public_ip>"
echo "${network_status}"
