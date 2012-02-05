#!/bin/bash

# The folder /media/storage is always writable
# Returns the successful absolute path to docroot to caller

PSP_HOMEPAGE=/psp/homepage
DOCROOT=/www/netvserver
GIT_DOCROOT_PATH=/media/storage/docroot


# Function to switch symlink path to new path
do_set_symlink()
{
	SYMLINK=$1
	NEW_PATH=$2
	
	# Desired local path does not exist
	if [ ! -e ${NEW_PATH} ]; then
		echo "Error: Local path ${NEW_PATH} does not exist. Do nothing"
		return 1;
	fi
	
	# Check that we already have this symlink in place
	CURRENT_LOCAL_PATH=$(readlink -fn ${SYMLINK})
	if [ "$(stat -c "%d:%i" ${CURRENT_LOCAL_PATH})" == "$(stat -c "%d:%i" ${NEW_PATH})" ]; then
		echo "Warning: Symlink already exists and not changed. Do nothing"
		echo "${SYMLINK} -> ${CURRENT_LOCAL_PATH}"
		return 0;
	fi

	# Remove current symlink if it exists
	mount -o remount,rw /
	if [ -e ${SYMLINK} ]; then
		rm ${SYMLINK};
	fi

	# Create a new symlink pointing to new path
	ln -s ${NEW_PATH} ${SYMLINK};
	mount -o remount,ro /
	
	echo "${DOCROOT} -> ${NEW_PATH}"
	return 0;
}


	
# if /psp/homepage doesn't exist
#	# if /media/storage/docroot exists, set symlink to it
#	# else print out warning message, do nothing
if [ ! -e ${PSP_HOMEPAGE} ]; then
	if [ -e ${GIT_DOCROOT_PATH} ]; then
		do_set_symlink ${DOCROOT} ${GIT_DOCROOT_PATH}
		exit 0;
	fi
	
	echo "Warning: ${PSP_HOMEPAGE} does not exist. Do nothing"
	exit 0;
fi

# Check empty
if [ ! -s "${PSP_HOMEPAGE}" ]; then
	echo "Warning: Content of ${PSP_HOMEPAGE} is empty"
fi

PSP_HOMEPAGE_CONTENT=$(cat ${PSP_HOMEPAGE})
echo ${PSP_HOMEPAGE_CONTENT};

# Content too short
if [ ${#PSP_HOMEPAGE_CONTENT} -lt 5 ]; then
	echo "Warning: Content of ${PSP_HOMEPAGE} is too short. Do nothing"
	exit 0;
fi

#
# Check for valid remote HTTP(S) path
#

# Check if is remote HTTP path type
if [[ ${PSP_HOMEPAGE_CONTENT} == http://* || ${PSP_HOMEPAGE_CONTENT} == https://* ]]; then
	echo "Setting homepage as remote HTTP path: ${PSP_HOMEPAGE_CONTENT}"
	NeTVBrowser -qws -nomouse SetUrl ${PSP_HOMEPAGE_CONTENT}
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
# From here on, it must be local path
#

do_set_symlink ${DOCROOT} ${PSP_HOMEPAGE_CONTENT}
