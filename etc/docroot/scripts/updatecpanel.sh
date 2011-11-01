#!/bin/sh

# This is in home folder of root account in NeTV, always writable
# Returns (echo) the path to caller so that we can change the path just by changing this script
DOCROOT=/media/storage/root
GITPATH=http://git.chumby.com.sg/git/chumby-sg/netv-controlpanel.git

echo "${DOCROOT}/docroot"

# Clone the repo if it doesn't exist
if [ ! -e "${DOCROOT}/docroot" ];
then
	cd ${DOCROOT}
	if git clone ${GITPATH} docroot; then
		exit 0;
	fi
	echo "Error updating cpanel git repo"
	rm -rf "${DOCROOT}/docroot"
	exit 1;
fi

# Pull latest changes, discard any local changes made
cd "${DOCROOT}/docroot"
git reset --hard HEAD
if git pull; then
	exit 0;
fi

# Failed halfway, rollback the changes
git reset --hard HEAD
echo "Error updating cpanel git repo"
exit 1;
