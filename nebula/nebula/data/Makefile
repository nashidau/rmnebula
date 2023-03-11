# See ../mk/<your system>.inc for configuration

include ../mk/nebconf.inc

all:

install:
	@echo "Installing data files in ${datadir}"
	#@test -d data || (echo "No data files present." && exit 1)
	mkdir -p ${datadir}
	cp -r . ${datadir}
	rm -f ${datadir}/Makefile
	
uninstall:
	@echo "Uninstalling data files from ${datadir}"
	rm -rf ${datadir}

clean:

