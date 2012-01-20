#!/bin/sh
#
# $1 is the new docroot directory
# This script will relink /www/netvserver to a new location
# and also relink life-support stuffs back to original docroot
#

origin=/usr/share/netvserver/docroot
symlink=/www/netvserver

if [ ! -e $1 ]; then
	echo "path not found";
	exit 0;
fi

mount -o remount,rw /

# The docroot
if [ -e "$symlink" ]; then
	rm $symlink
fi
ln -s $1 $symlink

# Lift support stuffs
if [ -e "$symlink/scripts" ]; then
	rm "$symlink/scripts"
fi
if [ -e "$origin/scripts" ]; then
	ln -s "$origin/scripts" "$symlink/scripts"
fi

if [ -e "$symlink/html_config" ]; then
	rm "$symlink/html_config"
fi
if [ -e "$origin/html_config" ]; then
	ln -s "$origin/html_config" "$symlink/html_config"
fi

if [ -e "$symlink/html_motor" ]; then
	rm "$symlink/html_motor"
fi
if [ -e "$origin/html_motor" ]; then
	ln -s "$origin/html_motor" "$symlink/html_motor"
fi

if [ -e "$symlink/html_remote" ]; then
	rm "$symlink/html_remote"
fi
if [ -e "$origin/html_remote" ]; then
	ln -s "$origin/html_remote" "$symlink/html_remote"
fi

if [ -e "$symlink/html_test" ]; then
	rm "$symlink/html_test"
fi
if [ -e "$origin/html_test" ]; then
	ln -s "$origin/html_test" "$symlink/html_test"
fi

if [ -e "$symlink/html_update" ]; then
	rm "$symlink/html_update"
fi
if [ -e "$origin/html_update" ]; then
	ln -s "$origin/html_update" "$symlink/html_update"
fi

if [ -e "$symlink/html_web" ]; then
	rm "$symlink/html_web"
fi
if [ -e "$origin/html_web" ]; then
	ln -s "$origin/html_web" "$symlink/html_web"
fi

if [ -e "$symlink/scripts" ]; then
	rm "$symlink/scripts"
fi
if [ -e "$origin/tests" ]; then
	ln -s "$origin/tests" "$symlink/tests"
fi

if [ -e "$symlink/favicon.ico" ]; then
	rm "$symlink/favicon.ico"
fi
if [ -e "$origin/favicon.ico" ]; then
	ln -s "$origin/favicon.ico" "$symlink/favicon.ico"
fi

if [ -e "$symlink/webapp_icon.png" ]; then
	rm "$symlink/webapp_icon.png"
fi
if [ -e "$origin/webapp_icon.png" ]; then
	ln -s "$origin/webapp_icon.png" "$symlink/webapp_icon.png"
fi

sync
mount -o remount,ro /

echo "$1";