# Makefile for i18n
include ../mk/debian.inc

all: find-gettext potfile compile-po

find-gettext:
	cd .. && grep '_(' `find * -type f -name "*[ch]"` | cut -d: -f1 | sort -u > po/POTFILES

potfile:
	cd .. && xgettext -a -i -o po/nebula.pot -f po/POTFILES

compile-po:
	@for i in *.po; do \
		echo -n "Compiling '$$i' ... "; \
		msgfmt -c --statistics $$i \
			-o `echo $$i | sed 's/\.po/\.mo/'`; \
	done

install:
	@for i in *.mo; do \
		D=${localedir}/`echo $$i | sed 's/\.mo//'`/LC_MESSAGES; \
		mkdir -p $$D; \
		echo -n "Installing '$$i' in '$$D' ... "; \
		install -m 0644 -o root -g root $$i $$D/nebula.mo; \
		echo "done."; \
	done
	
clean:
	rm -f *.mo nebula.pot POTFILES
