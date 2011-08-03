#!/bin/sh
#

# The downloading should have been done before this is triggered
mount -o remount,rw /
upgradeOuput=$(opkg upgrade)
mount -o remount,ro /

# Software version
fwver=$(chumby_version -f)
if [ -z "${fwver}" ]; then
	fwver='1.0'
fi

# Output to web service
echo "<fwver>${fwver}</fwver>"
echo "<log>${upgradeOuput}</log>"
