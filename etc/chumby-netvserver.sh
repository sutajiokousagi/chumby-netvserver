#!/bin/sh


case "$1" in
	start)
		export QWS_KEYBOARD=chumbyirkb

		# hide the ugly initialization sequence
		setbox 0 0 1 1

		# prefix chroma key
		fpga_ctl w 0xc 2
		fpga_ctl w 0xd 240
		fpga_ctl w 0xe 0
		fpga_ctl w 0xf 240
		
		# Temp fix for "Semop lock/unlock failure Identifier removed" flood
		rm -rf /tmp/qtembedded-0

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

	restart)
		$0 stop
		$0 start
		;;

	*)

		echo "Usage: $0 {start|stop|restart}"
		;;
esac
