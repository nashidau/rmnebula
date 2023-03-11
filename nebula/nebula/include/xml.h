

xmlDocPtr
xml_open_file(char *path,char *file);


const char *
xml_grep(char *path,char *file,char *xpath);

xmlNode *
xml_find_node(xmlNode *node,const char *path);
        
