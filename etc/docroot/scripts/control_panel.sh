#!/bin/sh

# $1 is either 'true'/'false' or 'show'/'hide' or 'yes'/'no'
# The Browser/Control Panel is definitely running when this command is called.

# Hardcode the path until we get a better place
/psp/netvbrowser/NeTVBrowser -qws ShowHide $1

