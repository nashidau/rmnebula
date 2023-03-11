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
NEBULA_OBJECTS=../libneb/libneb.a

CFLAGS+=-I$(ALENA) -I../include

WB_OBJECTS=worldbuilder.o general.o realms.o skills.o regions.o create.o 

	
OBJECTS=$(WB_OBJECTS) $(ALENA_OBJECTS) $(NEBULA_OBJECTS)

# Important headers - if any of these change we should recompile
HEADERS=worldbuilder.h 

all: worldbuilder

worldbuilder : $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)        

tags : 
	ctags *.[ch] ../libneb/*.[ch] ../include/*.h ../alena/*.[ch] 

install: worldbuilder
	@echo "Installing worldbuilder binary as ${bindir}/worldbuilder"
	mkdir -p ${bindir}
	install -o root -g root -m 0755 worldbuilder ${bindir}/worldbuilder

uninstall:
	rm -f ${bindir}/worldbuilder

clean :
	rm -f $(WB_OBJECTS) $(ALENA_OBJECTS) worldbuilder core

../libneb/libneb.a:
	(cd ../libneb && make)
        
