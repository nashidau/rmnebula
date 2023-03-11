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
NEBULA=../libneb
NEBULA_OBJECTS=$(NEBULA)/libneb.a
        
CFLAGS+=-I$(ALENA) -I../include

ALC_OBJECTS=alchemist.o listitems.o general.o intelligence.o \
	items.o spells.o logwindow.o itemparsing.o  guitools.o \
	skills.o


OBJECTS=$(ALC_OBJECTS) $(ALENA_OBJECTS) $(NEBULA_OBJECTS)

# Important headers - if any of these change we should recompile
HEADERS=alchemist.h listitems.h general.h

all: alchemist

alchemist : $(HEADERS) $(OBJECTS)
	$(CC) -O0 -o $@ $(OBJECTS) $(LDFLAGS)        

tags :
	ctags *.[ch] ../src/*.[ch] ../alena/*.[ch] 

install: alchemist
	@echo "Installing alchemist binary as ${bindir}/alchemist"
	mkdir -p ${bindir}
	install -o root -g root -m 0755 alchemist ${bindir}/alchemist

uninstall:
	rm -f ${bindir}/alchemist

clean :
	rm -f $(ALC_OBJECTS) $(ALENA_OBJECTS) alchemist core
	rm -f *~

../libneb/libneb.a:
	(cd ../libneb && make)
        
