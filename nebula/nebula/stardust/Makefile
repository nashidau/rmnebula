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

STARDUST_OBJECTS=stardust.o

OBJECTS=$(STARDUST_OBJECTS) $(ALENA_OBJECTS) $(NEBULA_OBJECTS)

# Important headers - if any of these change we should recompile
HEADERS=stardust.h 

all: stardust

stardust : $(HEADERS) $(OBJECTS) ../libneb/libneb.a
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) 

tags :
	ctags *.[ch] ../src/*.[ch] ../alena/*.[ch] ../libneb/*.c ../include/*.h

install: stardust
	@echo "Installing stardust binary as ${bindir}/stardust"
	mkdir -p ${bindir}
	install -o root -g root -m 0755 stardust ${bindir}/stardust

uninstall:
	rm -f ${bindir}/stardust

clean :
	rm -f $(STARDUST_OBJECTS) stardust core
       
../libneb/libneb.a:
	(cd ../libneb && make)
