#!/bin/sh

# Absolute path to this script. /home/user/bin/foo.sh
SCRIPT=$(readlink -f $0)
# Absolute path this script is in. /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Process GET/POST variables and export to environment
eval `${SCRIPTPATH}/proccgi.sh $*`

INTIF=wlan1

ssid=$FORM_ssid
authentication=$FORM_wifi_authentication
wifi_encryption=$FORM_wifi_encryption
wifi_password=$FORM_wifi_password
wifi_encoding=$FORM_wifi_encoding
wifi_capabilities=$FORM_wifi_capabilities
chumby_username=$FORM_chumby_username
chumby_password=$FORM_chumby_password
chumby_device_name=$FORM_chumby_device_name
flash_status=$FORM_flash_status
internal_status=''
last_error=''

# Number of seconds since epoch
last_epoch=$(date +%s)

# IP address of wlan1
ip_address=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')

# Last flash_status
last_flash_status=$(cat ${SCRIPTPATH}/flash_data.xml | grep -e '<flash_status>' | sed -e "s/<flash_status>//g" | sed -e "s/<\/flash_status>//g" | sed -e 's/^[ \t]*//')

# We print to XML
cat > ${SCRIPTPATH}/flash_data.xml << EOFEOF
<data>
        <ssid>$ssid</ssid>
        <wifi_capabilities>$wifi_capabilities</wifi_capabilities>
        <wifi_authentication>$authentication</wifi_authentication>
        <wifi_encryption>$wifi_encryption</wifi_encryption>
        <wifi_password>$wifi_password</wifi_password>
        <wifi_encoding>$wifi_encoding</wifi_encoding>
        <chumby_username>$chumby_username</chumby_username>
        <chumby_password>$chumby_password</chumby_password>
        <chumby_device_name>$chumby_device_name</chumby_device_name>
        <flash_status>$flash_status</flash_status>
        <last_epoch>$last_epoch</last_epoch>
        <ip_address>$ip_address</ip_address>
        <last_error>$last_error</last_error>
        <internal_status>$internal_status</internal_status>
</data>
EOFEOF

# Execute configuration scripts
if [ "$last_flash_status" != "2.3" -o "$flash_status" != "2.4" ];
then
	echo -e "ScriptOK\r\n"
	exit
fi

echo "Configuring network..."
fbwrite --pos=0,0 --color=255,255,255 "Configuring network..."
internal_status="configuring"

###########################################################
# Make /psp/network_config file
###########################################################

SSID=$FORM_ssid
ENCRYPTION=$FORM_wifi_encryption
AUTH=$FORM_wifi_authentication
ENCODING=$FORM_wifi_encoding
KEY=$FORM_wifi_password
HWADDR=$(ifconfig ${INTIF} | grep 'HWaddr' | awk '{print $5}')

cat > /tmp/network_config << 0123456789
<configuration type="wlan" allocation="dhcp" auth="${AUTH}" encryption="${ENCRYPTION}" ssid="${SSID}" key="${KEY}" hwaddr="${HWADDR}" encoding="${ENCODING}" />
0123456789

cp -f /tmp/network_config /psp/

###########################################################
# Return status to HTTP request
###########################################################

# Use another process to configure Wifi as we need to
# return right now to then pending HTTP request
${SCRIPTPATH}/activation_connect.sh &


# We print to XML
cat > ${SCRIPTPATH}/flash_data.xml << EOFEOF
<data>
        <ssid>$ssid</ssid>
        <wifi_capabilities>$wifi_capabilities</wifi_capabilities>
        <wifi_authentication>$authentication</wifi_authentication>
        <wifi_encryption>$wifi_encryption</wifi_encryption>
        <wifi_password>$wifi_password</wifi_password>
        <wifi_encoding>$wifi_encoding</wifi_encoding>
        <chumby_username>$chumby_username</chumby_username>
        <chumby_password>$chumby_password</chumby_password>
        <chumby_device_name>$chumby_device_name</chumby_device_name>
        <flash_status>$flash_status</flash_status>
        <last_epoch>$last_epoch</last_epoch>
        <ip_address>$ip_address</ip_address>
        <last_error>$last_error</last_error>
        <internal_status>$internal_status</internal_status>
</data>
EOFEOF
                                                                                                                
echo -e "ScriptOK\r\n"

