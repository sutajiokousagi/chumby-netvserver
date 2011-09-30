#!/bin/sh
#
# This script will try its best to guess the
# current city/country/timezone offset
# by using various 'free' web services
# and copy the correct timezone data file to /psp/localtime
#
#

no_internet='<value>no internet</value>'
timeurl='http://j.maxmind.com/app/geoip.js'
timeurl2='http://www.earthtools.org/timezone/'			#followed by <latitude>/<longitude>'

# Some simple checking for Internet
if [ ! -z "$(pidof hostapd)" ]; then
	echo ${no_internet}
	exit
fi
if [ -z "$(pidof NetworkManager)" ]; then
	echo ${no_internet}
	exit
fi

# Guess local timezone
if [ ! -e /psp/localtime ];
then
	geoip=$(curl $timeurl)
	
	country_code=$(echo ${geoip##*country_code})
	country_code=$(echo $country_code| cut -d"'" -f 2)
	country_name=$(echo ${geoip##*country_name})
	country_name=$(echo $country_name| cut -d"'" -f 2)
	country_name_no_space=$(echo "$country_name" | sed 's/ \+/_/g')
	region_name=$(echo ${geoip##*region_name})
	region_name=$(echo $region_name| cut -d"'" -f 2)
	region_name_no_space=$(echo "$region_name" | sed 's/ \+/_/g')
	city=$(echo ${geoip##*city})
	city=$(echo $city| cut -d"'" -f 2)
	city_no_space=$(echo "$city" | sed 's/ \+/_/g')
	postal_code=$(echo ${geoip##*postal_code})
	postal_code=$(echo $postal_code| cut -d"'" -f 2)
	latitude=$(echo ${geoip##*latitude})
	latitude=$(echo $latitude| cut -d"'" -f 2)
	longitude=$(echo ${geoip##*longitude})
	longitude=$(echo $longitude| cut -d"'" -f 2)
	
	if [ "${country_code}" == "US" ]; then
		country_name="America"
	fi
	#echo $country_code
	#echo $country_name_no_space
	#echo $region_name_no_space
	#echo $city_no_space
	#echo $postal_code
	
	# extract the timezone file in /usr/share/zoneinfo
	alltimezones=$(find /usr/share/zoneinfo/ -type f)
	city_file=''
	region_file=''
	country_file=''
	if [ ! -z "${city_no_space}" ]; then
		city_file=$(echo "${alltimezones}" | grep "${city_no_space}" | tail -1)
	fi
	if [ ! -z "${region_name_no_space}" ]; then
		region_file=$(echo "${alltimezones}" | grep "${region_name_no_space}" | tail -1)
	fi
	if [ ! -z "${country_name_no_space}" ]; then
		country_file=$(echo "${alltimezones}" | grep "${country_name_no_space}" | tail -1)
	fi
	
	# select the best timezone file
	timezonefile=$city_file
	if [ -z "${timezonefile}" ]; then
		timezonefile=$region_file
	fi
	
	# use a different service to get offset
	# See http://www.earthtools.org/webservices.htm#timezone
	if [ -z "${timezonefile}" ];
	then
		longlatinfo=$(curl "${timeurl2}/${latitude}/${longitude}")
		offset_prefix="<offset>"
		offset=$(echo ${longlatinfo##*$offset_prefix})
		offset=$(echo $offset| cut -d"<" -f 1)
		if [ ${#offset} -gt 0 -a ${offset:0:1} != "-" ]; then
			offset="+${offset}";
		fi
		offset_file=''
		if [ ! -z "${offset}" ]; then
			offset_file=$(echo "${alltimezones}" | grep "GMT${offset}" | tail -1)
		fi
		if [ ! -z "${offset_file}" ]; then
			timezonefile=$offset_file;
		fi
				
		dst_prefix="<dst>"
		dst=$(echo ${longlatinfo##*${dst_prefix}})
		dst=$(echo $dst| cut -d"<" -f 1)
		#dst==Unknown, False, True
		
		#echo $dst
		#echo $offset
		#echo $offset_file
		#echo $timezonefile
	fi

	# last resort, use country timezone
	if [ -z "${timezonefile}" ]; then
		timezonefile=$country_file;
	fi
	
	# Set timezone (non-volatile)
	#echo $timezonefile
	cp -f $timezonefile /psp/localtime
	sync
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
