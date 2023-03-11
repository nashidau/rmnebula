###
# Nebula Config
###

# This file is generated automatically doing a "make config" in the top level
# directory.
include ../mk/nebconf.inc

#########################################
# You shouldn't need to modify anything below this line.
ALENA=../alena
ALENA_OBJECTS=$(ALENA)/alena.o $(ALENA)/token.o
LIBNEB=../libneb
LIBNEB_OBJ=$(LIBNEB)/libneb.a

CFLAGS+=-I$(ALENA) -I../include

NEB_OBJECTS=nebula.o menu.o stats.o skills.o character.o create.o  \
        rr.o profession.o item.o special.o spells.o choice.o tp.o nebfile.o

OBJECTS=$(NEB_OBJECTS) $(LIBNEB_OBJ) $(ALENA_OBJECTS)

# Important headers - if any of these change we should recompile
HEADERS=nebula.h types.h credits.h ../include/nebconf.h

all: nebula

nebula : $(HEADERS) $(OBJECTS) $(ALENA_OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

tags :
	ctags *.[ch] ../alena/*.[ch] ../libneb/*.c ../include/*.h

install: nebula
	@echo "Installing nebula binary in ${bindir}/nebula"
	mkdir -p ${sharedir}/pixmaps ${docdir}/characters ${bindir}
	install -o root -g root -m 0755 nebula ${bindir}/nebula
	install -o root -g root -m 0644 ../pixmaps/*.png \
		${sharedir}/pixmaps
	install -o root -g root -m 0644 ../pixmaps/*.xpm \
		${sharedir}/pixmaps
	install -o root -g root -m 0644 ../characters/*.xml \
		${docdir}/characters

uninstall:
	@echo -n "Uninstalling nebula ... "
	rm -f ${bindir}/nebula
	rm -fr ${sharedir}/pixmaps
	rm -rf ${docdir}
	@echo "done."

clean :
	rm -f $(NEB_OBJECTS) $(ALENA_OBJECTS) nebula core *.o

distclean: clean
	rm -f tags TAGS *~ .*.bak diff.diff changes.txt

../libneb/libneb.a:
	(cd ../libneb && make)
 
