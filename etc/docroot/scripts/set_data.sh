#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

key=$1
val=$2

perl -p -0777 -e "s@<$key>.*?<\/$key>@<$key>$val<\/$key>@sg" -i ${SCRIPTPATH}/flash_data.xml

