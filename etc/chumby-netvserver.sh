#!/bin/sh

case "$1" in
	start)
		# Check /psp/homepage and set docroot accordingly
		/usr/share/netvserver/docroot/scripts/psphomepage.sh >> /var/log/cron_psphomepage.log 2>&1

		# start in the background so we don't hog the console
		NeTVServer > /dev/null 2>&1 &
		;;

	stop)
		killall NeTVServer
		;;

	restart)
		$0 stop
		sleep 1
		$0 start
		;;

	*)
		echo "Usage: $0 {start|stop|restart}"
		;;
esac
