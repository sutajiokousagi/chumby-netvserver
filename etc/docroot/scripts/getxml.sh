#!/bin/sh

content=$(wget $1 -q -O - )
echo "<value>${content}</value>"
