# This file is generated automatically doing a "make config" in the top level
# directory.
include ../../mk/nebconf.inc

all: user_guide.rtf

user_guide.rtf: ../user_guide.sgml
	${JADE} -c ${CATALOG} -t rtf -d../user_guide.dsl#print -o user_guide.rtf ../user_guide.sgml

install:
	install -o root -g root -m 0644 user_guide.rtf ${docdir}

clean:
	rm -f *.rtf
