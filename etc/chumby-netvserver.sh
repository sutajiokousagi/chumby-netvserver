#!/bin/sh


case "$1" in
	start)
		# hide the ugly initialization sequence
		setbox 0 0 1 1

		# prefix chroma key
		fpga_ctl w 0xc 2
		fpga_ctl w 0xd 0
		fpga_ctl w 0xe 7
		fpga_ctl w 0xf 0

		# Start in the background so we don't hog the console
		# This app uses QtGui hence requires -qws option,
		# but does not render anything to the framebuffer
		NeTVServer -qws -nomouse 2>&1 2> /dev/null&
		;;

	stop)
		killall NeTVServer 2> /dev/null
		;;

	*)

		echo "Usage: $0 {start|stop}"
		;;
esac
