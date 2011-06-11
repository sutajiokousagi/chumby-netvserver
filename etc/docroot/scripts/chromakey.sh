#!/bin/sh

# Turn on
if [ "x$1" -eq "xon" ]; then
	fpga_ctl w 0xc 2
	echo "on"
	exit;
fi

# Turn off
if [ "x$1" -eq "xoff" ]; then
	fpga_ctl w 0xc 0
	echo "off"
	exit;
fi

# Setting color $1 $2 $3
fpga_ctl w 0xc 2
fpga_ctl w 0xd $1
fpga_ctl w 0xe $2
fpga_ctl w 0xf $3
