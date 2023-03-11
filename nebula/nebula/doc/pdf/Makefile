# This file is generated automatically doing a "make config" in the top level
# directory.
include ../../mk/nebconf.inc

all:user_guide.pdf

user_guide.pdf: ../user_guide.sgml
	rm -f *.pdf
	${JADE} -c ${CATALOG} -d../user_guide.dsl#print -t tex -o user_guide.tex ../user_guide.sgml
	# Twice to generate links
	pdfjadetex user_guide.tex
	pdfjadetex user_guide.tex
	rm -f *.log
	rm -f *.aux
	rm -f *.out
install:
	install -o root -g root -m 0644 user_guide.pdf ${docdir}
clean:
	rm -f *.pdf *.tex *.log *.aux *.out
