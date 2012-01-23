#!/bin/sh
#
# Returning the current WiFi signal strenght in negative dBm unit
# 2 means (NetworkManager) is not connected
# 1 means connecting
# <= 0 valid signal strength
#

STRENGTH=0;

# Interface name
INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
INTIF=$(echo ${INTIF//mon./})

# Network manager state
nmstate="4"
if [ ! -z "$(pidof NetworkManager)" ];
then
	nmstate=$(dbus-send --system --print-reply --dest='org.freedesktop.NetworkManager' /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Get string:'' string:'State' | grep uint32 | awk '{print $3}')
fi
if [ "$nmstate" == "2" ]; then
	STRENGTH=1
elif [ "$nmstate" == "1" -o "$nmstate" == "4" ]; then
	STRENGTH=2
elif [ "$nmstate" == "3" ]; then
	#query DBus to get the actual signal strength here
	STRENGTH=-70
fi

#
# Print it out
#
echo "<value>${STRENGTH}</value>"
