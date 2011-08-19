#!/bin/sh

#
# $1 is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
# $2 is standard timezone ID string formated as "Asia/Singapore"
# see http://developer.android.com/reference/java/util/TimeZone.html#getID()
#

time=$1
timezone=$2

# Set timezone (non-volatile)
if [ -e /usr/share/zoneinfo/$2 ];
then
	cp /usr/share/zoneinfo/$2 /psp/localtime
fi

# Set time (volatile)
datetime=$(date -s $time)

#Print output
echo "<value>${datetime}</value>"