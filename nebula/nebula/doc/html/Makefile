# This file is generated automatically doing a "make config" in the top level
# directory.
include ../../mk/nebconf.inc

all: html_pages

book1.html:
	${JADE} -c ${CATALOG} -t xml -d../user_guide.dsl#html -o user_guide.html ../user_guide.sgml

user_guide_one_page.html:
	${JADE} -V nochunks -c ${CATALOG} -t xml -d../user_guide.dsl#html ../user_guide.sgml > user_guide_one_page.html

html_pages: user_guide_one_page.html book1.html

install: html_pages
	mkdir -p ${docdir}/user_guide
	install -o root -g root -m 0644 *.html ${docdir}/user_guide
	ln -s book1.html ${docdir}/user_guide/index.html
	mv ${docdir}/user_guide/user_guide_one_page.html ${docdir}

clean:
	rm -f *.html
	rm -f *.htm

