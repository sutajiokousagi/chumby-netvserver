#!/bin/sh

#
# $1 is the url + GET parameters (if any)
# $2 is the POST data in format "param1=value1&param2=value2" (or blank if using GET)
#

content=''
if [ -z "${2}" ]; then
	content=$(curl $1)
else
	content=$(curl -d "$2" $1)
fi
echo $content
