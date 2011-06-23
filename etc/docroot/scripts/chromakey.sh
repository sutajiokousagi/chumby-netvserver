#!/bin/sh

# Turn on
if [ "$1" == "on" ]; then
	fpga_ctl w 0xc 2
	echo "on"
	exit 0
fi

# Turn off
if [ "$1" == "off" ]; then
	fpga_ctl w 0xc 0
	echo "off"
	exit 0
fi

# Setting color $1 $2 $3
fpga_ctl w 0xc 2
fpga_ctl w 0xd $1
fpga_ctl w 0xe $2
fpga_ctl w 0xf $3
