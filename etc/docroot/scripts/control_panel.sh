#!/bin/sh

# The Browser/Control Panel is definitely running when this command is called.

# This command should return immediately
content=$(NeTVBrowser -qws $*)
echo $content

