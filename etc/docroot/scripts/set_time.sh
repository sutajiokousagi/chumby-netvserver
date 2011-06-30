#!/bin/sh

#
# $1 is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
# $2 is standard timezone ID string formated as "Asia/Singapore"
#

time=$1
timezone=$2

# Set time (this will print out the time)
date -s $time
date
echo $1
echo $2

# Set timezone
#/usr/chumby/scripts/time_zone.sh $timezone		#not built yet