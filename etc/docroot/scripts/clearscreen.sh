#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

/usr/bin/imgtool --fb=0 ${SCRIPTPATH}/blank.jpg
/usr/bin/imgtool --fb=1 ${SCRIPTPATH}/blank.jpg

