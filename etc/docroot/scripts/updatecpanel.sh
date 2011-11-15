#!/bin/sh

# This is in home folder of root account in NeTV, always writable
# Returns (echo) the path to caller so that we can change the path just by changing this script
DOCROOT=/media/storage
GITPATH=http://git.chumby.com.sg/git/chumby-sg/netv-controlpanel.git

echo "<docroot>${DOCROOT}/docroot</docroot>"
echo "<gitoutput>"

# If repo already exist, check if it is valid repo by a hard reset
if [ -e "${DOCROOT}/docroot" ];
then
	cd ${DOCROOT}/docroot
	if ! git reset --hard HEAD;
	then
		rm -rf "${DOCROOT}/docroot"
	fi
fi

# Clone the repo if it doesn't exist
if [ ! -e "${DOCROOT}/docroot" ];
then
	cd ${DOCROOT}
	if git clone ${GITPATH} docroot;
	then
		# create symlink for /tmp
		if [ ! -e ${DOCROOT}/docroot/tmp ]; then
			mkdir ${DOCROOT}/docroot/tmp
		fi
		ln -sf /tmp ${DOCROOT}/docroot/tmp/netvserver
		exit 0;
	fi
	if [ ! -e "${DOCROOT}/docroot" ]; then
		rm -rf "${DOCROOT}/docroot"
	fi
	echo "Error updating cpanel git repo"
	echo "</gitoutput>"
	exit 1;
fi

# Exists, just do a pull
if git pull; then
	# create for /tmp
	if [ ! -e ${DOCROOT}/docroot/tmp ]; then
		mkdir ${DOCROOT}/docroot/tmp
	fi
	ln -sf /tmp ${DOCROOT}/docroot/tmp/netvserver
	echo "</gitoutput>"
	exit 0;
fi

# Failed halfway, rollback the changes
rm -rf "${DOCROOT}/docroot"
echo "Error updating cpanel git repo"
echo "</gitoutput>"
exit 1;
