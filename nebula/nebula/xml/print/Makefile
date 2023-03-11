# Makfile for the talk

TARGET=dhrun.html

$(TARGET): dhrun.xml html.xsl
	xalan -IN dhrun.xml -XSL html.xsl -OUT $(TARGET)

clean:
	rm -f $(TARGET)
