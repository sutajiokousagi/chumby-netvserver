#!/bin/sh

#
# $1 is username
# $2 is password
# $3 is device name
#

if [ $# lt 3 ]
then
	echo "Not enough parameters."
	exit
fi

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`


###########################################################
# Query server
###########################################################

GUID=$(/usr/chumby/scripts/guidgen.sh)
SERVER_ADDR=http://xml.chumby.com/xml/chumbies/?id=${GUID}
PROFILE=$(curl ${SERVER_ADDR})


