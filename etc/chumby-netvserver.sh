#!/bin/sh


case "$1" in
	start)
		# [Temp]
		$(ifconfig lo up)

		# start in the background so we don't hog the console
		NeTVServer > /dev/null 2>&1 &
		;;

	stop)
		killall NeTVServer > /dev/null 2>&1 &
		;;

	restart)
		$0 stop
		$0 start
		;;

	*)

		echo "Usage: $0 {start|stop|restart}"
		;;
esac
