#!/bin/sh
#
# $1 is the new docroot directory
#

symlink=/www/netvserver

if [ ! -e $symlink ]; then
	echo "";
	exit 0;
fi

echo $(readlink -f $symlink)