#!/bin/bash

# The folder /media/storage is always writable
# Returns the successful absolute path to docroot to caller

PSP_HOMEPAGE=/psp/homepage
GIT_DOCROOT_PATH=/media/storage/docroot
SCRIPT_PATH=/usr/share/netvserver/docroot/scripts/setdocroot.sh

echo ''
echo `date`

# Function to switch symlink path to new path
do_set_symlink()
{
	NEW_PATH=$1
	
	SETDOCROOT=$(${SCRIPT_PATH} ${NEW_PATH})
	LAST_ERROR=$?
	
	if [ "${LAST_ERROR}" -eq "1" ]; then
		echo "Error: Local path ${NEW_PATH} does not exist. Do nothing"
		return 1;
	fi
	
	if [ "${LAST_ERROR}" -eq "2" ]; then
		echo "Warning: Symlink already exists and not changed. Do nothing"
		echo "${NEW_PATH}"
		return 2;
	fi

	echo "Info: Symlink switched to new path"
	echo "${NEW_PATH}"
	return 0;
}


	
# if /psp/homepage doesn't exist
#	# if /media/storage/docroot exists, set symlink to it
#	# else print out warning message, do nothing
if [ ! -e ${PSP_HOMEPAGE} ]; then
	if [ -e ${GIT_DOCROOT_PATH} ]; then
		do_set_symlink ${GIT_DOCROOT_PATH}
		exit 0;
	fi
	
	echo "Warning: ${PSP_HOMEPAGE} does not exist. Do nothing"
	exit 0;
fi

# Check empty
if [ ! -s "${PSP_HOMEPAGE}" ]; then
	echo "Warning: Content of ${PSP_HOMEPAGE} is empty"
	exit 0;
fi

PSP_HOMEPAGE_CONTENT=$(cat ${PSP_HOMEPAGE})
echo "${PSP_HOMEPAGE} content: ${PSP_HOMEPAGE_CONTENT}"

# Content too short
if [ ${#PSP_HOMEPAGE_CONTENT} -lt 5 ]; then
	echo "Warning: Content of ${PSP_HOMEPAGE} is too short. Do nothing"
	exit 0;
fi

#
# Check for valid remote HTTP(S) path
#

# Check if is remote HTTP path type
#	# if /media/storage/docroot exists, set symlink to it
if [[ ${PSP_HOMEPAGE_CONTENT} == http://* || ${PSP_HOMEPAGE_CONTENT} == https://* ]]; then
	echo "Setting homepage as remote HTTP path: ${PSP_HOMEPAGE_CONTENT}"
	NeTVBrowser -qws -nomouse SetUrl ${PSP_HOMEPAGE_CONTENT}
	
	# Although the homepage is a remote URL, we still need to keep localhost updated
	if [ -e ${GIT_DOCROOT_PATH} ]; then
		do_set_symlink ${GIT_DOCROOT_PATH}
		exit 0;
	fi
	
	exit 0;
fi

# Check if is invalid HTTP path
if [[ ${PSP_HOMEPAGE_CONTENT} == http* ]]; then
	echo "Error: Invalid HTTP path"
	exit 0;
fi

# Not a local path or unsupported protocol
if [[ ${PSP_HOMEPAGE_CONTENT} != /* ]]; then
	echo "Error: Unsupported protocol"
	exit 0;
fi


#
# Use SetDocroot command to set new path and relink life support stuff
#
do_set_symlink ${PSP_HOMEPAGE_CONTENT}
