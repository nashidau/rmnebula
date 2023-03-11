#include <libxml/tree.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>

int
xslt_process_file(const char *ss_file,const char *xml_file, int fd){
	xmlDocPtr doc;
	xsltStylesheetPtr xslt_ss;
	int rv;
	FILE *f;

	/* Read and parse our XSL file */
	xslt_ss = xsltParseStylesheetFile((const xmlChar *)ss_file);
	/* Read and parse our XML file */
	doc = xmlParseFile(xml_file);
	/* Apply the XSL style sheet to our XML doc and write the result */
	f = fdopen(fd, "a");
	doc = xsltApplyStylesheet(xslt_ss, doc, NULL);
	rv = xsltSaveResultToFile(f, doc, xslt_ss);

	return rv;
}

