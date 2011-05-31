#!/bin/sh

#
# Reset flash_data.xml to initial state
# More importantly is the flash_status which controls the UI
#

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Process GET/POST variables and export to environment
eval `${SCRIPTPATH}/proccgi.sh $*`

flash_status=1
last_error=''

# Number of seconds since epoch
last_epoch=$(date +%s)

# Print to XML
cat > ${SCRIPTPATH}/flash_data.xml << EOFEOF
<data>
	<flash_status>$flash_status</flash_status>
        <last_epoch>$last_epoch</last_epoch>
        <last_error>$last_error</last_error>
</data>
EOFEOF
                        

echo -e "ScriptOK\r\n"
