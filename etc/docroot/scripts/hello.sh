#!/bin/sh
#
# Returning identification information of this hardware
#

#Testing GUID
#echo "<guid>A620123B-1F0E-B7CB-0E11-921ADB7BE22A</guid>"
#echo "<guid>4FCD4051-9867-006A-9DC1-443BD3477E8E</guid>"
GUID=$(guidgen.sh)

echo "<guid>${GUID}</guid>"
echo "<dcid>a long string of 1024 bytes</dcid>"
echo "<hwver>10.1</hwver>"
echo "<fwver>1.0</fwver>"
echo "<flashplugin>0</flashplugin>"
