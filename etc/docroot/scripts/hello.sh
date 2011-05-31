#!/bin/sh
#
# NeTVWebServer will execute this script in blocking manner.
# Do no perform any long waiting processes.
# Create a new process if you have to.
#

netinfo=$(ifconfig)
netinfo=$(echo ${netinfo} | tr '\n' '<br/>')

echo "<html>"
echo "	<head>"
echo "		<title>Hello script</title>"
echo "	</head>"
echo "	<body>"
echo "		Current time: <b>$(date)</b><br/>"
echo "		Kernel: <b>$(uname) $(uname -r)</b><br/>"
echo "		Network: <br/>${netinfo}<br/>"
echo "	</body>"
echo "</html>"
