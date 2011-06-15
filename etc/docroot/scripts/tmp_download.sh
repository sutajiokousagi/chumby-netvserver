#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

newfilename=$(echo $1 | sed -e "s/\//\|\|/g")

newpath="${SCRIPTPATH}/../tmp/${newfilename}.jpg"
wget $1 -q -O $newpath

echo "${newfilename}.jpg"
