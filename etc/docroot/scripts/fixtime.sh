#!/bin/sh

no_internet='<value>no internet</value>'

# Some simple checking for Internet
if [ ! -z "$(pidof hostapd)" ]; then
	echo ${no_internet}
	exit
fi
if [ -z "$(pidof NetworkManager)" ]; then
	echo ${no_internet}
	exit
fi

INTIF=$(ls -1 /sys/class/net/ | grep wlan | head -1)
IP=$(/sbin/ifconfig ${INTIF} | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1 }')
if [ -z "${IP}" -o "${IP}" == "192.168.1.100" ]; then
	echo ${no_internet}
	exit
fi

# Time server list: http://tf.nist.gov/tf-cgi/servers.cgi
timer_servers=( nist1-sj.ustiming.org
				nist1.symmetricom.com
				nist1-la.ustiming.org
				nist1.aol-ca.symmetricom.com
				nist1-lv.ustiming.org
				ntp-nist.ldsbc.edu
				utcnist.colorado.edu
				nist1-ny.ustiming.org
				nist1-nj.ustiming.org
				nist1-pa.ustiming.org
				time-a.nist.gov
				nist1.aol-va.symmetricom.com
				nist1.columbiacountyga.gov )

for t_server in "${timer_servers[@]}"
do
	# blank output if fails
	output=$(rdate $t_server)
	if [ ${#output} -gt 5 ];
	then
		if [ -z "$(echo ${output} | grep matches)" ];
		then
			echo $output
		else
			date
		fi
		exit
	fi
done
