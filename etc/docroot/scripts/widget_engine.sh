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
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		echo "Already running"
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
	fi
	exit;
fi

# Start
if [ "$cmd" == "startminimize" ]; then
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		echo "Already running"
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 2
	fi
	setplayer c 0 0 1 1
	exit;
fi

# Minimize
if [ "$cmd" == "minimize" -o "$cmd" == "hide" ]; then
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		kill -s STOP $(pidof $process_name)
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 2
	fi
	setplayer c 0 0 1 1
	exit;
fi

# Maximize/Fullscreen
if [ "$cmd" == "maximize" -o "$cmd" == "fullscreen" -o "$cmd" == "show" ]; then
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		kill -s CONT $(pidof $process_name)
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 2
	fi
	setplayer c 0 0 1280 720
	exit;
fi

# Quit/Stop
if [ "$cmd" == "quit" -o "$cmd" == "exit" -o "$cmd" == "terminate" -o "$cmd" == "stop" ]; then
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		echo "stopping..."
		kill -9 $(pidof $process_name)
	else
		echo "Already stopped"
	fi
	exit;
fi

# Restart
if [ "$cmd" == "restart" ]; then
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		echo "stopping previous instance..."
		kill -9 $(pidof $process_name)
		sleep 2
	fi
	${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
	exit;
fi

# SetBox
if [ "$cmd" == "setbox" ]; then
	if ps ax | grep -v grep | grep $process_name > /dev/null
	then
		echo ""
	else
		${process_name} -i ${widget_engine_swf} 2> /dev/null > /dev/null &
		sleep 2
	fi
	setplayer c $2 $3 $4 $5
	exit;
fi
