#!/bin/sh
#

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Running in Access Point mode
is_ap_mode=0
if [ ! -z "$(pidof hostapd)" ];
then
	is_ap_mode=1
fi

# Parse arguements
is_force=0
is_verbose=0
while [[ $1 == -* ]]; do
    case "$1" in
      -f|--force) is_force=1; shift;;
      -v|--verbose) is_verbose=1; shift;;
      --) shift; break;;
    esac
done

if [ $is_ap_mode -eq 1 ]; then
	if [ $is_force -ne 1 ]; then

		if [ -e /tmp/wifilist.xml ]; then
			cat /tmp/wifilist.xml
			exit 0
		else
			echo "No wifiscan cache found. Cannot perform new wifiscan in AP mode. Use -f option."
			exit 0
		fi

	else
		whatever=$(${SCRIPTPATH}/stop_ap.sh)
		sleep 4
	fi
fi

# Wireless Interface
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
whatever=$(ifconfig ${INTIF} up)
sleep 1

wifilist=$(${SCRIPTPATH}/wifiscan.pl)

#
# Revert back to Access Point mode if neccesary
#
if [ $is_ap_mode -eq 1 ]; then
	ap_status=$(${SCRIPTPATH}/start_ap.sh)
fi

#
# Save new wifilist to some where
#
echo ${wifilist} > /tmp/wifilist.xml

#
# Print them out
#
echo "${wifilist}"

