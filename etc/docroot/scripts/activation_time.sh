#!/bin/sh

#
# $FORM_time is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
# $FORM_timezone is standard timezone ID string formated as "Asia/Singapore"
#

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Process GET/POST variables and export to environment
eval `${SCRIPTPATH}/proccgi.sh $*`

INTIF=wlan1

time=$FORM_time
timezone=$FORM_timezone
flash_status=$FORM_flash_status
last_error=''

# Set time & timezone
echo -e "Setting: $time\r\n"
date -s $time
/usr/chumby/scripts/time_zone.sh $timezone

# Number of seconds since epoch
last_epoch=$(date +%s)

# IP address of wlan1
ip_address=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')

# Last error
if [ -e ${SCRIPTPATH}/flash_error ];
then
        last_error=$(cat ${SCRIPTPATH/flash_error})
fi
        
# Yes, we did print to XML :)
cat > ${SCRIPTPATH}/flash_data.xml << EOFEOF
<data>
	<flash_status>$flash_status</flash_status>
        <last_epoch>$last_epoch</last_epoch>
        <ip_address>$ip_address</ip_address>
        <last_error>$last_error</last_error>
	<ssid> </ssid>
        <wifi_capabilities> </wifi_capabilities>
        <wifi_authentication> </wifi_authentication>
        <wifi_encryption> </wifi_encryption>
        <wifi_password> </wifi_password>
        <wifi_encoding> </wifi_encoding>
        <chumby_username> </chumby_username>
        <chumby_password> </chumby_password>
        <chumby_device_name> </chumby_device_name>
</data>
EOFEOF
                        

echo -e "ScriptOK\r\n"
