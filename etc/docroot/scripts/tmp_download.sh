#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Create a neat folder in /tmp for us
if [ ! -e ${SCRIPTPATH}/../tmp/netvserver ]; then
	mkdir ${SCRIPTPATH}/../tmp/netvserver;
fi

# Construct absolute cache filename from the full url
newfilename=$(echo $1 | sed -e "s/\//\|\|/g")
newpath="${SCRIPTPATH}/../tmp/netvserver/${newfilename}.jpg"

# Download only if it doesn't exist in cache OR bad cache
if [ ! -e $newpath ]; then
    curl --silent --output $newpath $1
else
    filesize=$(stat -c %s $newpath)
    if [ $filesize -lt 1 ]; then
        curl --silent --output $newpath $1
    fi
fi

# Return just the filename to the hardware bridge, to be used in a <img> tag
# This has to be a relative path to docroot
echo "tmp/netvserver/${newfilename}.jpg"
