#!/bin/sh

#
# $1 is the button name
# Infrared remote: up/down/left/right/center/cpanel/widget/hidden1/hidden2
# Android/iOS remote: tbd
#

cmd=$(echo $1 | tr '[A-Z]' '[a-z]')

# Start
if [ "$cmd" == "reset" ];
then
	# Hide the flash player
	setplayer 0 0 1 1 
	
	# Reset the browser
	start_netvbrowser.sh
	exit;
fi

# Forward the button name to JavaScript Control Panel
/usr/bin/NeTVBrowser -qws -nomouse JavaScript "fButtonPress('$cmd')"
