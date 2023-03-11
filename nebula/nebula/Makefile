# Global makefile for Nebula

# Directories that have makefiles:
subdirs=data libneb src doc/pdf doc/rtf doc/html doc/man \
	stardust worldbuilder po

all: config
	@for dir in ${subdirs}; do \
	(cd $$dir && $(MAKE) all ) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	@echo -e "\n--- Building finished ---\n"
	
install:
	@for dir in ${subdirs}; do \
	(cd $$dir && $(MAKE) install ) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	
uninstall:
	@for dir in ${subdirs}; do \
	(cd $$dir && $(MAKE) uninstall ) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

config: mk/nebconf.inc

mk/nebconf.inc:
	@./nebconf.sh

clean:
	@for dir in ${subdirs}; do \
	(cd $$dir && $(MAKE) clean) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	
	# Clean the nebconfured files:
	/bin/rm -f mk/nebconf.inc include/nebconf.h
	
	# This is for a *real* tree cleaning :-)
	/bin/rm -rf *~ *-stamp debian/nebula debian/nebula-data \
		debian/files debian/nebula.postinst.debhelper \
		debian/nebula.postrm.debhelper \
		debian/nebula.substvars
	
debug-makefile:
	@echo "Configuration:"
	@echo "--------------"
	@echo
	@echo "Directories to build:"
	@echo ${subdirs}
	@echo
	@echo "Installation directories:"
	@echo -e "\tBinaries:      ${bindir}"
	@echo -e "\tShared files:  ${sharedir}"
	@echo -e "\ti18n files:    ${localedir}"
	@echo -e "\tData files:    ${datadir}"
	@echo -e "\tDocumentation: ${docdir}"
	@echo
	@echo "Programs:"
	@echo -e "\tC Compiler: ${CC}"
	@echo -e "\tpkg-config: ${PKGCONFIG}"
	@echo -e "\tjade:       ${JADE}"
	@echo -e "\tpdfjadetex: ${PDFJADETEX}"
	@echo
	@echo "Variables:"
	@echo -e "\tCFLAGS = ${CFLAGS}"
	@echo -e "\tLDFLAGS = ${LDFLAGS}"
	@echo -e "\tCATALOG = ${CATALOG}"

