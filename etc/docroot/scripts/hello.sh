#!/bin/sh
#
# Returning identification information of this hardware
#

# Give demo GUID if no guidgen.sh
if [ ! -e /usr/bin/guidgen.sh ]; then
	GUID=A620123B-1F0E-B7CB-0E11-921ADB7BE22A
else	
	GUID=$(/usr/bin/guidgen.sh)
fi

# To be replaced by a actual script
DCID=''
if [ -z $DCID ]; then
	DCID='a long string of 1024 bytes'
fi

# To be replaced by a dcid script
hwver=''
if [ -z $hwver ]; then
	hwver='10.1'
fi

# To be replaced by a dcid script
fwver=''
if [ -z $fwver ]; then
	fwver='1.0'
fi

# No flash plugin on NeTV
flashplugin='0'

# Flash version
flashver='10.62.0'

# Network status
network_status=$(network_status.sh)
internet='false'
hasLinkTrue=$(echo $network_status | grep 'link="true"')
if [ ${#hasLinkTrue} -gt 10 ]; then
	internet='true'
fi

# MAC address
INTIF=wlan0
MAC=$(ifconfig | grep ${INTIF} | tr -s ' ' | cut -d ' ' -f5)

# IP address
IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')

#
# Print them out
#
echo "<guid>$GUID</guid>"
echo "<dcid>$DCID</dcid>"
echo "<hwver>$hwver</hwver>"
echo "<fwver>$fwver</fwver>"
echo "<flashplugin>$flashplugin</flashplugin>"
echo "<flashver>$flashver</flashver>"
echo "<internet>$internet</internet>"
echo "<mac>$MAC</mac>"
echo "<ip>$IP</ip>"
echo "$network_status"