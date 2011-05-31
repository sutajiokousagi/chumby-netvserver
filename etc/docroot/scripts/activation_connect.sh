#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

#DEBUG=1
HOSTNAME=NeTV
INTIF=wlan1

###########################################################
# Start network
###########################################################

# Stop AP mode
${SCRIPTPATH}/stop_ap.sh

# Make sure hostname is 'NeTV' (for mDNS)
hostname ${HOSTNAME}

# Start connecting to 'home' network
if [ $?DEBUG ];
then
	fbwrite --pos=0,9 --color=0,160,255 "Connecting..."
fi

# Modified version of start_network script DOES NOT stop eth0
mount -o remount,rw /
/usr/chumby/scripts/start_network_mod
mount -o remount,ro /

# Use this for actual product
#/usr/chumby/scripts/start_network

# Extract IP address of wlan interface
WIFIUP=$(/sbin/ifconfig ${INTIF} | grep 'UP')
if [ ${#WIFIUP} -lt 4 ];
then
	echo "start.sh: Failed to bring up ${INTIF}"
	
	if [ $?DEBUG ];
	then
		fbwrite --pos=0,9 --color=255,0,0 "             FAILED"
		fbwrite --pos=0,10 --color=255,0,0 "Failed to bring up ${INTIF}"
		fbwrite --pos=0,11 --color=255,255,255 "Reverting to Access Point mode"
	fi
	${SCRIPTPATH}/set_data.sh "Failed to bring up ${INTIF}"
	${SCRIPTPATH}/set_data.sh flash_status 2.2
	${SCRIPTPATH}/set_data.sh internal_status "ERROR_1"

	# Revert to AP mode	
        ${SCRIPTPATH}/stop_ap.sh
        sleep 3
        ${SCRIPTPATH}/start_ap.sh
        exit
fi
              
IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')
if [ "x${IP}" == "x" ];
then
	echo "start.sh: Failed to obtain IP address ${INTIF}"
	
	if [ $?DEBUG ];
	then
		fbwrite --pos=0,9 --color=255,0,0 "             FAILED"
		fbwrite --pos=0,10 --color=255,0,0 "Failed to obtain IP address ${INTIF}"
		fbwrite --pos=0,11 --color=255,255,255 "Reverting to Access Point mode"
	fi	
	${SCRIPTPATH}/set_data.sh last_error "Failed to obtain IP address ${INTIF}"
	${SCRIPTPATH}/set_data.sh flash_status 2.2
	${SCRIPTPATH}/set_data.sh internal_status "ERROR_2"

	# Revert to AP mode	
        ${SCRIPTPATH}/stop_ap.sh
        sleep 3
        ${SCRIPTPATH}/start_ap.sh
        exit
fi

# Change status interface
${SCRIPTPATH}/set_data.sh ip_address ${IP}
${SCRIPTPATH}/set_data.sh flash_status 2.5
${SCRIPTPATH}/set_data.sh internal_status "networked"
${SCRIPTPATH}/set_data.sh last_error " "

# Publish services on mDNS
mDNSResponder
sleep 1
(mDNSPublish $(hostname) _http._tcp. 80) &
sleep 1
(mDNSPublish $(hostname) _ssh._tcp. 22) &
sleep 1 

# Check mDNS again
SERVICE=mDNSResponder
 
if ps ax | grep -v grep | grep $SERVICE > /dev/null
then 
	echo "mDNS: OK"
else
	killall mDNSResponder
	killall mDNSPublish
	sleep 2
	mDNSResponder 
	sleep 2
	(mDNSPublish $(hostname) _http._tcp 80) &
	sleep 2
	(mDNSPublish $(hostname) _ssh._tcp 22) &
fi

# Internal status
${SCRIPTPATH}/set_data.sh internal_status "activating"

################################################################

# We should be contacting Chumby server for activation right here
# Right now I'm just skipping it - Torin
sleep 5 

################################################################

# Success status
${SCRIPTPATH}/set_data.sh internal_status "activated"
${SCRIPTPATH}/set_data.sh flash_status 2.6


