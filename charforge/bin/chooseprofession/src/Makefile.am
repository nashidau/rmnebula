## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = chooseprofession

chooseprofession_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	profession.c profession.h \
	gtk_support.c gtk_support.h \
	comms.c comms.h 

chooseprofession_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

