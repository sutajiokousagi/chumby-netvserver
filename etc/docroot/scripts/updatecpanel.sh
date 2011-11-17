#!/bin/bash

# The folder /media/storage is always writable
# Returns the successful absolute path to docroot to caller

DOCROOT_PATH=/media/storage
GITPATH1=http://github.com/sutajiokousagi/netv-controlpanel.git
GITPATH2=http://git.chumby.com.sg/git/chumby-sg/netv-controlpanel.git


# Function
do_update()
{
	GITPATH=$1
	DOCROOT=$2
	
	# If repo already exist, check if it is valid repo by comparing path & perform a hard reset
	if [ -e "${DOCROOT_PATH}/${DOCROOT}" ];
	then
		# Check if the existing repo path matches the desired repo path
		# This is in case we change the order of the repo path
		repo_path=$(cat ${DOCROOT_PATH}/${DOCROOT}/.git/config | grep ${GITPATH})
		if [ -z "${repo_path}" ];
		then
			rm -rf "${DOCROOT_PATH}/${DOCROOT}"
		else
			#Hard reset
			cd ${DOCROOT_PATH}/${DOCROOT}
			if ! git reset --hard HEAD;
			then
				rm -rf "${DOCROOT_PATH}/${DOCROOT}"
			fi
		fi		
	fi

	# Clone the repo if it doesn't exist
	if [ ! -e "${DOCROOT_PATH}/${DOCROOT}" ];
	then
		cd ${DOCROOT_PATH}
		if git clone ${GITPATH} ${DOCROOT};
		then
			# create symlink for /tmp
			if [ ! -e ${DOCROOT_PATH}/${DOCROOT}/tmp ]; then
				mkdir ${DOCROOT_PATH}/${DOCROOT}/tmp
			fi
			ln -sf /tmp ${DOCROOT_PATH}/${DOCROOT}/tmp/netvserver
			return 0;
		fi

		# Network error?
		if [ ! -e "${DOCROOT_PATH}/${DOCROOT}" ]; then
			rm -rf "${DOCROOT_PATH}/${DOCROOT}"
		fi
		echo "Error updating cpanel git repo"
		return 1;
	fi

	# Exists, just do a pull
	if git pull; then
		# create for /tmp
		if [ ! -e ${DOCROOT_PATH}/${DOCROOT}/tmp ]; then
			mkdir ${DOCROOT_PATH}/${DOCROOT}/tmp
		fi
		ln -sf /tmp ${DOCROOT_PATH}/${DOCROOT}/tmp/netvserver
		return 0;
	fi

	# Failed due to network error?
	#rm -rf "${DOCROOT_PATH}/${DOCROOT}"

	echo "Error updating cpanel git repo"
	return 1;
}



OUTPUT=$(do_update ${GITPATH1} "docroot")
if [ "$?" -eq "0" ]; then
	echo "<docroot>${DOCROOT_PATH}/docroot</docroot>"
	echo "<gitoutput>${OUTPUT}</gitoutput>"
	exit 0;
fi

OUTPUT=$(do_update ${GITPATH2} "docroot2")
if [ "$?" -eq "0" ]; then
	echo "<docroot>${DOCROOT_PATH}/docroot2</docroot>"
	echo "<gitoutput>${OUTPUT}</gitoutput>"
	exit 0;
fi

exit 1;
