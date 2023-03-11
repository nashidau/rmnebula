#!/bin/sh

# This so needs work.  And it will only work for nash
DEST_ROOT=nash@nash.nu:~/VIRT/Rolemaster/Nebula/
SCP=scp
USER=`whoami`

# Put your user name to sourceforge host
if [ x$USER = xnash ] ; then
        SFUSER=nash
#elif [ x$USER = xfoo ] ; then
#       SFUSER= your sourceforge user name
fi

if [ x$USER = xnash ] ; then
        scp *.shtml $DEST_ROOT
fi

# Copy it to sourceforge web site
$SCP *.shtml $SFNAME@shell.sourceforge.net:/home/groups/r/rm/rmnebula/htdocs 
