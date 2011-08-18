#!/bin/sh

#
# Commands (all lowercase)
# start/startminimize/stop
# minimize/maximize/fullscreen
# quit/exit/terminate/restart
# setbox xx yy width height
#
# We must check if Flash Player is running/not running and start/stop it accordingly.

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

process_name='/usr/bin/chumbyflashplayer.x'
widget_engine_swf="${SCRIPTPATH}/../widget_engine.swf"

cmd=$(echo $1 | tr '[A-Z]' '[a-z]')

# Start
if [ "$cmd" == "start" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo "Already running"
		setplayer c
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
	fi
	exit;
fi

# Start
if [ "$cmd" == "startminimize" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo "Already running"
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 3
	fi
	setplayer c 0 0 1 1
	setplayer p
	exit;
fi

# Minimize
if [ "$cmd" == "minimize" -o "$cmd" == "hide" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo "Already running"
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 3
		
	fi
	setplayer c 0 0 1 1
	setplayer p
	exit;
fi

# Maximize/Fullscreen
if [ "$cmd" == "maximize" -o "$cmd" == "fullscreen" -o "$cmd" == "show" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo "Already running"
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 3
	fi
	setplayer c 0 0 1280 720
	setplayer c
	exit;
fi

# Quit/Stop
if [ "$cmd" == "quit" -o "$cmd" == "exit" -o "$cmd" == "terminate" -o "$cmd" == "stop" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo "stopping..."
		kill -9 "$(pidof $process_name)"
	else
		echo "Already stopped"
	fi
	exit;
fi

# Restart
if [ "$cmd" == "restart" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo "stopping previous instance..."
		kill -9 "$(pidof $process_name)"
		sleep 2
	fi
	${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
	exit;
fi

# SetBox
if [ "$cmd" == "setbox" ]; then
	if [ ! -z "$(pidof $process_name)" ];
	then
		echo ""
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 2
	fi
	setplayer c $2 $3 $4 $5
	exit;
fi
