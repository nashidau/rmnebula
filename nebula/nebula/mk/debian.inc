#### Settings for Debian

# GTK
PKGCONFIG=pkg-config
CONFIG=$(PKGCONFIG) libgnome-2.0 libgnomeui-2.0 libgnomeprint-2.2 libgnomeprintui-2.2 libxml-2.0 libxslt
LDFLAGS:=`$(CONFIG) --libs` ${LDFLAGS}
CFLAGS:=-O2 -Wall \
	`$(CONFIG) --cflags` ${CFLAGS}

# Extra configuration for libxml and libxslt
LDFLAGS+=`xml2-config --libs` `xslt-config --libs`
CFLAGS+=`xml2-config --cflags` `xslt-config --cflags`

# Uncomment this for debugging
CFLAGS+=-g

# Installation
PREFIX=/usr
bindir=${PREFIX}/bin
libdir=${PREFIX}/lib
sharedir=${PREFIX}/share/nebula
localedir=${PREFIX}/share/locale
datadir=${sharedir}/data
docdir=${PREFIX}/share/doc/nebula
mandir=${PREFIX}/share/man

# Documentation
JADE=jade
CATALOG=/etc/sgml/catalog
PDFJADETEX=pdfjadetex

