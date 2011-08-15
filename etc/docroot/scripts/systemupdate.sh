#!/bin/sh
#

#
# This script is deprecated.
# All upgrading operation is now controlled by NeTVBrowser directly
#

#Get the list of upgradable packages
if [ "$1" == "list-upgradable" ]; then
	upgradeOuput=$(opkg list-upgradable)
	echo "<outputlog>${upgradeOuput}</outputlog>"
	exit
	# Example output:
	# chumby-netvserver - 1.0-r76.9 - 1.0-r77.9
	# angstrom-version - v20110703-r18.9 - v20110703-r19.9
	# chumby-netvbrowser - 1.0-r36.9 - 1.0-r37.9
	# netv-controlpanel - 1.0-r30.9 - 1.0-r31.9
fi

#Get the size of all packages in /var/lib/opkg/tmp
if [ "$1" == "list-size" ]; then
	FILES=/var/lib/opkg/tmp/*.ipk
	for filename in $FILES
	do
		filesizebyte=$(stat -c %s $filename)
		echo "<file><filename>${filename}</filename><filesize>${filesizebyte}</filesize></file>"
	done
	exit
fi


# The downloading should have been done before this is triggered
mount -o remount,rw /
upgradeOuput=$(opkg --cache /var/lib/opkg/tmp upgrade)
mount -o remount,ro /

# Software version (build number)
fwver=$(chumby_version -f)
if [ -z "${fwver}" ]; then
	fwver='1.0'
fi

# Output to web service
echo "<fwver>${fwver}</fwver>"
#echo "<outputlog>${upgradeOuput}</outputlog>"
