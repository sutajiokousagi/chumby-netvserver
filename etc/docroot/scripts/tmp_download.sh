#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Construct cache filename from the full Url
newfilename=$(echo $1 | sed -e "s/\//\|\|/g")
newpath="${SCRIPTPATH}/../tmp/netvserver/${newfilename}.jpg"

# Download only if it doesn't exist in cache
if [ ! -e $newpath ]; then
    wget $1 -q -O $newpath
fi;

# Return just the filename to the hardware bridge, to be used in a <img> tag
echo "./tmp/netvserver/${newfilename}.jpg"
