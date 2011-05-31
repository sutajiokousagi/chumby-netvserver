#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

SWF_NAME=NeTV.swf
SERVER_PATH=http://www.torinnguyen.com/chumby/widgets/${SWF_NAME}
LOCAL_PATH=${SCRIPTPATH}/${SWF_NAME}

# Kill it if running
killall chumbyflashplayer.x
sleep 1

# Remove temp swf file (if any)
cd /tmp
if [ -e ${SWF_NAME} ];
then
	rm ${SWF_NAME}
fi

# Download new version
wget ${SERVER_PATH}
cd /tmp
if [ -e ${SWF_NAME} ];
then
        cp -f ${SWF_NAME} ${LOCAL_PATH}
        rm ${SWF_NAME}
fi

# Start the local version
chumbyflashplayer.x -i ${LOCAL_PATH}
