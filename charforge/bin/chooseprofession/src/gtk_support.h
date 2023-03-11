void error_dialog(const char *msg,const char *desc);

xmlChar *node_get_text(xmlNode *node);
xmlChar *node_get_attribute(xmlNode *node, const xmlChar *name);

void text_buffer_insert(GtkTextBuffer *, const char *style, xmlChar *str);
void text_buffer_clear(GtkTextBuffer *buffer);


void str_clean(char *str);
