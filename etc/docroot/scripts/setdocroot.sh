#!/bin/sh
#
# $1 is the new docroot directory
# This script will relink /www/netvserver to a new location
# and also relink life-support stuffs back to original docroot
# Returns 0 if everything is ok
# Returns 1 if new path doesn't exist
# Returns 2 if new path is already the current symlink, do nothing
#

origin=/usr/share/netvserver/docroot
SYMLINK=/www/netvserver
NEW_PATH=$1

#---------------------------------------------------------------
#---------------------------------------------------------------

# Desired local path does not exist
if [ ! -e ${NEW_PATH} ]; then
	echo "Error: ${NEW_PATH} does not exist. Do nothing";
	exit 1;
fi

# Check that we already have this symlink in place
# So that we don't need to relink the life support stuff
CURRENT_LOCAL_PATH=$(readlink -fn ${SYMLINK})
if [ "$(stat -c "%d:%i" ${CURRENT_LOCAL_PATH})" == "$(stat -c "%d:%i" ${NEW_PATH})" ]; then
	#echo "Warning: Symlink already exists and not changed. Do nothing"
	echo "${NEW_PATH}"
	exit 2;
fi


#---------------------------------------------------------------
#---------------------------------------------------------------

# Remove current symlink if it exists
mount -o remount,rw /
if [ -e ${SYMLINK} ]; then
	rm ${SYMLINK};
fi

# Create a new symlink pointing to new path
ln -s ${NEW_PATH} ${SYMLINK};


#---------------------------------------------------------------
#---------------------------------------------------------------
# Symlink has just changed, relink life support stuffs

mount -o remount,rw /

if [ -e "$SYMLINK/scripts" ]; then
	rm "$SYMLINK/scripts"
fi
if [ -e "$origin/scripts" ]; then
	ln -s "$origin/scripts" "$SYMLINK/scripts"
fi

if [ -e "$SYMLINK/html_config" ]; then
	rm "$SYMLINK/html_config"
fi
if [ -e "$origin/html_config" ]; then
	ln -s "$origin/html_config" "$SYMLINK/html_config"
fi

if [ -e "$SYMLINK/html_motor" ]; then
	rm "$SYMLINK/html_motor"
fi
if [ -e "$origin/html_motor" ]; then
	ln -s "$origin/html_motor" "$SYMLINK/html_motor"
fi

if [ -e "$SYMLINK/html_remote" ]; then
	rm "$SYMLINK/html_remote"
fi
if [ -e "$origin/html_remote" ]; then
	ln -s "$origin/html_remote" "$SYMLINK/html_remote"
fi

if [ -e "$SYMLINK/html_test" ]; then
	rm "$SYMLINK/html_test"
fi
if [ -e "$origin/html_test" ]; then
	ln -s "$origin/html_test" "$SYMLINK/html_test"
fi

if [ -e "$SYMLINK/html_update" ]; then
	rm "$SYMLINK/html_update"
fi
if [ -e "$origin/html_update" ]; then
	ln -s "$origin/html_update" "$SYMLINK/html_update"
fi

if [ -e "$SYMLINK/html_web" ]; then
	rm "$SYMLINK/html_web"
fi
if [ -e "$origin/html_web" ]; then
	ln -s "$origin/html_web" "$SYMLINK/html_web"
fi

if [ -e "$SYMLINK/scripts" ]; then
	rm "$SYMLINK/scripts"
fi
if [ -e "$origin/scripts" ]; then
	ln -s "$origin/scripts" "$SYMLINK/scripts"
fi

if [ -e "$SYMLINK/tests" ]; then
	rm "$SYMLINK/tests"
fi
if [ -e "$origin/tests" ]; then
	ln -s "$origin/tests" "$SYMLINK/tests"
fi

if [ -e "$SYMLINK/favicon.ico" ]; then
	rm "$SYMLINK/favicon.ico"
fi
if [ -e "$origin/favicon.ico" ]; then
	ln -s "$origin/favicon.ico" "$SYMLINK/favicon.ico"
fi

if [ -e "$SYMLINK/webapp_icon.png" ]; then
	rm "$SYMLINK/webapp_icon.png"
fi
if [ -e "$origin/webapp_icon.png" ]; then
	ln -s "$origin/webapp_icon.png" "$SYMLINK/webapp_icon.png"
fi

sync
mount -o remount,ro /

#---------------------------------------------------------------
#---------------------------------------------------------------

echo "${NEW_PATH}";
exit 0;