#!/bin/sh


case "$1" in
	start)
		# hide the ugly initialization sequence
		setbox 0 0 1 1

		# prefix chroma key
		fpga_ctl w 0xc 2
		fpga_ctl w 0xd 240
		fpga_ctl w 0xe 0
		fpga_ctl w 0xf 240
		
		# if no network config, start in AP mode
		# IP address of the interface
		IPADDRESS=$(ifconfig ${wlan0} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')

		# Extract IP address of wlan0
		if [ ! -e /psp/network_config -o ${#IPADDRESS} -lt 4 ];
		then
			echo "Starting in Access Point mode"
			/usr/share/netvserver/docroot/scripts/start_ap.sh
		fi

		# start in the background so we don't hog the console
		# This app uses QtGui hence requires -qws option,
		# but does not render anything to the framebuffer
		NeTVServer -qws -nomouse 2>&1 2> /dev/null&

		# fullscreen
		setbox 0 0 1279 719

		;;

	stop)
		killall NeTVServer 2> /dev/null
		;;

	*)

		echo "Usage: $0 {start|stop}"
		;;
esac
