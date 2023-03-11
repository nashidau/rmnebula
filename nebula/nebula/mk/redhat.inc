#### Settings for RedHat (and similar, as Mandrake)
# This settings are not tested, please report if anything is wrong.

# GTK
PKGCONFIG=pkg-config
CONFIG=$(PKGCONFIG) libgnomeui-2.0 libgnomeprint-2.0 libgnomeprintui-2.0
LDFLAGS:=`$(CONFIG) --libs` ${LDFLAGS}
CFLAGS:=`$(CONFIG) --cflags` ${CFLAGS}

# Installation
PREFIX=/usr
bindir=${PREFIX}/bin
libdir=${PREFIX}/lib
sharedir=${PREFIX}/share/nebula
localedir=${sharedir}/locale
datadir=${sharedir}/data
docdir=${PREFIX}/share/doc/nebula
mandir=${PREFIX}/share/man

# Documentation
JADE=jade
CATALOG=/etc/sgml/catalog
PDFJADETEX=pdfjadetex

