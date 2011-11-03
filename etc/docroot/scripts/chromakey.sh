#!/bin/sh

# Turn on
if [ "$1" == "on" ]; then
	if fpga_ctl x i 2>&1; then
		echo "on"
	else
		echo "error"
	fi
	exit 0
fi

# Turn off
if [ "$1" == "off" ]; then
	if fpga_ctl x o 2>&1; then
		echo "off"
	else
		echo "error"
	fi
	exit 0
fi

