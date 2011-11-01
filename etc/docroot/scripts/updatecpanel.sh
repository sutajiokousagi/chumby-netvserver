#!/bin/sh

# This is in home folder of root account in NeTV, always writable
# Returns (echo) the path to caller so that we can change the path just by changing this script
DOCROOT=/media/storage/root
GITPATH=http://git.chumby.com.sg/git/chumby-sg/netv-controlpanel.git

echo "<docroot>${DOCROOT}/docroot</docroot>"
echo "<gitoutput>"

# Clone the repo if it doesn't exist
if [ ! -e "${DOCROOT}/docroot" ];
then
	cd ${DOCROOT}
	if git clone ${GITPATH} docroot; then
		exit 0;
	fi
	if [ ! -e "${DOCROOT}/docroot" ]; then
		rm -rf "${DOCROOT}/docroot"
	fi
	echo "Error updating cpanel git repo"
	echo "</gitoutput>"
	exit 1;
fi

# Pull latest changes, discard any local changes made
cd "${DOCROOT}/docroot"
git reset --hard HEAD
if git pull; then
	echo "</gitoutput>"
	exit 0;
fi

# Failed halfway, rollback the changes
git reset --hard HEAD
echo "Error updating cpanel git repo"
echo "</gitoutput>"
exit 1;
