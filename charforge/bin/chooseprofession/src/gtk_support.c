#include <ctype.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libxml/parser.h>

#include "main.h"






void
error_dialog(const char *msg,const char *desc){
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(MainWindow), 
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
			"%s\n%s",msg,desc);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);	

}


/*
 * xmltree helper functions
 */
xmlChar *
node_get_text(xmlNode *node){
	char *str;

	if (node != NULL && node->type != XML_TEXT_NODE){
		node = node->children;
	}
	
	if (node == NULL || node->content == NULL)
		return NULL;

	str = g_strdup(node->content);
	g_strstrip(str);
	return str;

}

xmlChar *
node_get_attribute(xmlNode *node, const xmlChar *name){
	xmlAttr *attr;

	if (node == NULL)
		return NULL;

	for (attr = node->properties; attr != NULL ; attr = attr->next){
		if (xmlStrcmp(attr->name,name) != 0)
			continue;
		return g_strdup(attr->children->content);
	}

	return NULL;
}


/* 
 * text buffer helper functions
 *
 * Should be in a separate lilbrary or something
 */
void
text_buffer_insert(GtkTextBuffer *buffer, const char *style, xmlChar *str){
	GtkTextIter end;

	gtk_text_buffer_get_end_iter(buffer,&end);

	gtk_text_buffer_insert_with_tags_by_name(buffer,&end,str,-1,
			style,NULL);
}

void
text_buffer_clear(GtkTextBuffer *buffer){
	GtkTextIter start,end;

	gtk_text_buffer_get_start_iter(buffer,&start);
        gtk_text_buffer_get_end_iter(buffer,&end);

	gtk_text_buffer_delete(buffer,&start,&end);
	
}



void
str_clean(char *str){
	int inspace,len;
	char *p = NULL;

	inspace = 0;
	while (*str != 0){
		if (*str == '\n'){
			inspace = 1;
			p = str + 1;
		} 
		if (p && inspace && !isspace(*str)){
			len = strlen(str);
			memmove(p,str,len);
			p[len] = 0;
			inspace = 0;
		}
		str ++;
	}	
			

	
}
