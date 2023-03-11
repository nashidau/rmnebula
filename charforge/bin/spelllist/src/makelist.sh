#!/bin/sh

SPELLLISTROOT=..

EXTRACTKNOWNSPELLS=$SPELLLISTROOT/data/extractspells.xsl
EXTRACTXSL=$SPELLLISTROOT/data/extractlist.xsl
CONVERTXSL=$SPELLLISTROOT/data/spelllisttops.xsl
TMPFILE=/tmp/spellbook.xml
CONVERTTMP=/tmp/extract.xsl
KNOWNTMP=/tmp/knownspells.xml

cd ../examples

# Extract the characters known spells
xsltproc $EXTRACTKNOWNSPELLS Evant.xml > $KNOWNTMP

xsltproc $EXTRACTXSL $KNOWNTMP  > $TMPFILE

cpp -P -o $CONVERTTMP $CONVERTXSL
xsltproc $CONVERTTMP $TMPFILE > spells.ps


#rm $TMPFILE $CONVERTTMP $KNOWNTMP
