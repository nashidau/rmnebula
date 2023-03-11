# This file is generated automatically doing a "make config" in the top level
# directory.
include ../mk/nebconf.inc

# Version and name of tar.gz for distribution
VERSION:=`cat $(CURDIR)/../VERSION`
TARGZ=nebula-${VERSION}.tar.gz
TARBZ2=nebula-${VERSION}.tar.bz2

all: targz tarbz2 debian

targz:
	mkdir /tmp/nebula-package.tmp
	(cd .. && make install PREFIX=/tmp/nebula-package.tmp/${PREFIX})
	(cd /tmp/nebula-package.tmp && \
		find . -type d -name CVS | xargs rm -rf && \
		tar zcf ${TARGZ} ./${PREFIX} )
	mv /tmp/nebula-package.tmp/${TARGZ} $(CURDIR)
	rm -rf /tmp/nebula-package.tmp

tarbz2:
	mkdir /tmp/nebula-package.tmp
	(cd .. && make install PREFIX=/tmp/nebula-package.tmp/${PREFIX})
	(cd /tmp/nebula-package.tmp && \
		find . -type d -name CVS | xargs rm -rf && \
		tar jcf ${TARBZ2} ./${PREFIX} )
	mv /tmp/nebula-package.tmp/${TARBZ2} $(CURDIR)
	rm -rf /tmp/nebula-package.tmp

debian:
	(cd .. && fakeroot debian/rules binary)
	mv ../../*.deb .
redhat: debian
	alien --to-rpm *.deb

clean:
	rm -f ${TARGZ} ${TARBZ2} *.deb *.rpm
	# Just in case the build didn't finish:
	rm -rf /tmp/nebula-package.tmp
