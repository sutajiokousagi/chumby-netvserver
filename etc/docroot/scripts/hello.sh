#!/bin/sh
#
# Returning identification information of this hardware
#

# Give demo GUID if no guidgen.sh
GUID=$(guidgen.sh)
if [ -n $GUID ]; then
	GUID=A620123B-1F0E-B7CB-0E11-921ADB7BE22A
fi

# To be replaced by a actual script
DCID=''
if [ -n $DCID ]; then
	DCID='a long string of 1024 bytes'
fi

# To be replaced by a dcid script
hwver=''
if [ -n $hwver ]; then
	hwver='10.1'
fi

# To be replaced by a dcid script
fwver=''
if [ -n $fwver ]; then
	fwver='1.0'
fi

# No flash plugin on NeTV
flashplugin='0'

# Network status
network_status=$(network_status.sh)
internet='false'
hasLinkTrue=$(echo network_status | grep 'link="true"')
if [ -n $hasLinkTrue ]; then
	internet='true'
fi

#
# Print them out
#
echo "<guid>$GUID</guid>"
echo "<dcid>$DCID</dcid>"
echo "<hwver>$hwver</hwver>"
echo "<fwver>$fwver</fwver>"
echo "<flashplugin>$flashplugin</flashplugin>"
echo "<internet>$internet</internet>"
echo "$network_status"