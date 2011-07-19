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

# To be replaced by a script
DCID=''
if [ -z $DCID ]; then
	DCID='a long string of 1024 bytes'
fi

# To be replaced by a script
hwver=''
if [ -z $hwver ]; then
	hwver='10.1'
fi

# Software version
fwver=$(chumby_version -f)
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
if [ ${#hasLinkTrue} -gt 15 ]; then
	internet='true'
fi

# MAC address
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
MAC=$(ifconfig | grep ${INTIF} | tr -s ' ' | cut -d ' ' -f5)
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

#
# Start access point mode if necessary
#
if [ "$internet" == "false" ]; then
	${SCRIPTPATH}/start_ap.sh
fi
