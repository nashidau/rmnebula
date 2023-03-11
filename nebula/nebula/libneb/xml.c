#include <stdio.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "libneb.h"


xmlDocPtr
xml_open_file(char *path,char *file){
        char *filename;
        xmlDocPtr doc;

        if (path != NULL){
                filename = g_strconcat(path,"/",file,NULL);
        } else {
                filename = g_strdup(file);
        }

        /* Open and parse the file */
        doc = xmlParseFile(filename);

        if (doc == NULL ) {
                /* FIXME: this error sucks */
		fprintf(stderr,"Document not parsed successfully. \n");
                xmlFreeDoc(doc);
		return NULL;
	}

        g_free(filename);

        return doc;
}


const char *
xml_grep(char *path,char *file,char *xpath){
        xmlDocPtr doc;
        xmlNode *cur,*node;

        doc = xml_open_file(path,file);    

        if (doc == NULL)
                return NULL;

        cur = xmlDocGetRootElement(doc);

        node = xml_find_node(cur,xpath); 

        if (node == NULL)
                return NULL;
        
        return node->content;
        
}

xmlNode *
xml_find_node(xmlNode *node,const char *path){

        if (*path == '/')
                path ++;

        if (*path == 0){
                /* last one - return this srting value */
                return node;
        }

        while (node != NULL){
                if (xmlStrncmp(node->name, path, xmlStrlen(node->name)) == 0){
                        path += xmlStrlen(node->name);
                        node = node->children;
                        return xml_find_node(node,path);
                }
                node = node->next;
        }
        
        return NULL;
}
