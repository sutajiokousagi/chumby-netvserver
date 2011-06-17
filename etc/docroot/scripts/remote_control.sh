#!/bin/sh

#
# $1 is the button name
# Infrared remote: up/down/left/right/center/cpanel/widget/hidden1/hidden2
# Android/iOS remote: tbd
#

cmd=$(echo $1 | tr '[A-Z]' '[a-z]')

echo $cmd

# Forward the button name to JavaScript Control Panel
#/usr/bin/NeTVBrowser -qws JavaScript "mCPanel.fButtonPress('$cmd')"
