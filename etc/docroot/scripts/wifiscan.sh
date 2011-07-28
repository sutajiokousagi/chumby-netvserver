#!/bin/sh
#

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Running in Access Point mode
ap_mode='0'
if ps ax | grep -v grep | grep hostapd > /dev/null
then
	ap_mode='1'
	$(${SCRIPTPATH}/stop_ap.sh)
	sleep 4
fi

# Wireless Interface
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
$(ifconfig ${INTIF} up)
sleep 1

wifilist=$(${SCRIPTPATH}/wifiscan.pl)

#
# Revert back to Access Point mode if neccesary
#
if [ "$ap_mode" == "1" ]; then
	ap_status=$(${SCRIPTPATH}/start_ap.sh)
fi

#
# Print them out
#
echo "${wifilist}"

#
# Save it to some where
#
echo ${wifilist} > /tmp/wifilist.xml

