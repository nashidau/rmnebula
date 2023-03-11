#### Generic settings

# GTK
PKGCONFIG=pkg-config
CONFIG=$(PKGCONFIG) libgnomeui-2.0 libgnomeprint-2.0 libgnomeprintui-2.0
LDFLAGS:=`$(CONFIG) --libs` ${LDFLAGS}
CFLAGS:=`$(CONFIG) --cflags` ${CFLAGS}

# Installation
PREFIX=/usr/local
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

