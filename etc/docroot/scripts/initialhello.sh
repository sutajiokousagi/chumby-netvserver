#!/bin/sh
# This script is identical to hello.sh except that it will switch to Access Point mode if internet='false'
# To be called by ControlPanel only once
# Returning identification information of this hardware
#

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

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

# No flash plugin on NeTV
flashplugin='0'

# Flash version
flashver=$(chumbyflashplayer.x -v | sed '5!d')

# Network status
network_status=$(network_status.sh)
internet='connecting'
hasLinkTrue=$(echo $network_status | grep 'link="true"')
if [ ${#hasLinkTrue} -gt 15 ]; then
	internet='true'
fi

# MAC & IP address
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
MAC=$(ifconfig | grep ${INTIF} | tr -s ' ' | cut -d ' ' -f5)
IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')

# Public Internet IP address
PUBLIC_IP=$(curl -s http://whatismyip.org)

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
# Start Access Point mode if neccesary
#
if [ "$internet" == "false" ]; then

	# Perform very first WifiScan before going to AP mode
	# Wifilist is saved to /tmp/wifilist.xml
	wifilist=$(${SCRIPTPATH}/wifiscan.sh -f)

	ap_status=$(${SCRIPTPATH}/start_ap.sh)

	# if called too early, encounter "nl80211 driver failed to initialize" error
	if [ ! -z "$(echo $ap_status | awk '/failed/')" ]; then
		internet='connecting'
	fi
fi

#
# Print them out
#
echo "<guid>${GUID}</guid>"
echo "<dcid>${DCID}</dcid>"
echo "<hwver>${hwver}</hwver>"
echo "<fwver>${fwver}</fwver>"
echo "<flashplugin>${flashplugin}</flashplugin>"
echo "<flashver>${flashver}</flashver>"
echo "<internet>${internet}</internet>"
echo "<mac>${MAC}</mac>"
echo "<ip>${IP}</ip>"
echo "<public_ip>${PUBLIC_IP}</public_ip>"
echo "${network_status}"


