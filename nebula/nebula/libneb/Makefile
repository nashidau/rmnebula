###
# Build rules for libnebula
###

# This file is generated automatically doing a "make config" in the top level
# directory.
include ../mk/nebconf.inc

#########################################
# You shouldn't need to modify anything below this line.
ALENA=../alena
ALENA_OBJECTS=$(ALENA)/alena.o $(ALENA)/token.o

CFLAGS+=-I../include -I$(ALENA)

OBJECTS=parse.o file.o gui.o dice.o options.o unix.o \
        tips.o print.o tree.o config.o libneb.o xml.o xslt.o

# Important headers - if any of these change we should recompile
HEADERS=../include/config.h ../include/libneb.h

all: libneb.a

libneb.a : $(HEADERS) $(OBJECTS) $(ALENA_OBJECTS)
	ar rv libneb.a $(OBJECTS)
	ranlib libneb.a

.c.o : $(HEADERS) $<

tags :
	ctags *.[ch] ../alena/*.[ch]

install:

uninstall:

#install: all
#	@echo "Installing nebula library in ${libdir}/nebula"
#	install -o root -g root -m 0644 libneb.a ${libdir}

#uninstall:
#	@echo -n "Uninstalling nebula library ... "
#	rm -f ${libdir}/libneb.a
#	@echo "done."

clean :
	rm -f $(OBJECTS) $(ALENA_OBJECTS) core *.a

distclean: clean
	rm -f tags TAGS *~ .*.bak diff.diff changes.txt
        
