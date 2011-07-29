#!/bin/sh


case "$1" in
	start)		
		# start in the background so we don't hog the console
		NeTVServer 2>&1 2> /dev/null&
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
