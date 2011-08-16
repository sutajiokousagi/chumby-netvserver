#!/bin/sh

#
# This script is to be called on new DHCP event by dnsmasq
# Currently it doesn't do anything
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
#SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
#SCRIPTPATH=`dirname $SCRIPT`

