#!/bin/sh

# This script tries to guess the system we're in, and make some links to
# configuration files suited to it. It's the poor man's autoconf :-)

SYSTEM=generic

case `uname` in
Linux)
	if [ -e /etc/debian_version ]
	then
		SYSTEM=debian
	fi

	if [ -e /etc/redhat-release ]
	then
		SYSTEM=redhat
	fi
	;;
Darwin)
	SYSTEM=darwin
	;;
*)
	;;
esac

echo "Configuring for system: $SYSTEM"

echo "Creating mk/nebconf.inc ..."
ln -fs $SYSTEM.inc mk/nebconf.inc

# Extract data of chosen nebconf.inc file:
PREFIX=`egrep ^PREFIX= mk/nebconf.inc | sed 's,^PREFIX=\(.*\)$,\1,'`
SHAREDIR=$PREFIX/`egrep ^sharedir= mk/nebconf.inc \
	| sed -e "s,\$.PREFIX./,," -e 's,^sharedir=\(.*\)$,\1,'`
DATADIR="$SHAREDIR/data"

echo "Creating include/nebconf.h ..."
cat <<EOF > include/nebconf.h

/* Automatically generated, do not change manually */

#define	CONFIG_PATH		"$DATADIR"
#define PIXMAPS_PATH	"$SHAREDIR/pixmaps"

EOF

exit 0

