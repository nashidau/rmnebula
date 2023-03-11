.SUFFIXES: .html .xml

XALAN=xalan -xsl ../spelllist.xsl 


default: sandlaw.xml ../spelllist.xsl
	$(XALAN) -in sandlaw.xml -out sandlaw.html
	$(XALAN) -in sandlaw.xml -out sandlaw10.html -param knownlevel 10
	$(XALAN) -in sandlaw.xml -out sandlawcondensed.html -param condensed 1
	$(XALAN) -in mind_visions.xml -out mind_visions.html -param \
	                                                        knownlevel 5 \
                                                        -param condensed 1
BENT=alterdreams dream_reality dream_traveller dream_walls


.xsl.html :
	xalan -xsl ../spelllist.xsl -out $@ -in $<
