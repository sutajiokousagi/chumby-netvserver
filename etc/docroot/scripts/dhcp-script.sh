#!/bin/sh

#
# This script is to be called on new DHCP event by dnsmasq
#

# if $1 is 'init' which is called by dnsmasq at startup, it's ignored
# if $1 is 'add' or 'old', then update the static arp table entry.
# if $1 is 'del', then delete the entry from the table

# Arguments.
# $1 is action (init, add, del, old)
# $2 is MAC
# $3 is ip address
# $4 is hostname (optional, may be unset)

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`


#fbwrite --pos=0,0 --color=0,255,0     "DHCP Event script"
#fbwrite --pos=0,1 --color=255,255,255 "Action: $1   MAC: $2"
#fbwrite --pos=0,2 --color=255,255,255 "IP: $3   Hostname: $4"

