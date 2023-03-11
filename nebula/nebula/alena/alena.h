#ifndef ALENA_H
#define ALENA_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "alena_types.h"

element_t *alena_parsefile(const char *file,int whitespace);
element_t *alena_parse(FILE *fp,int whitespace);
void alena_save(const char *filename,element_t *el,const char *headers);

element_t *get_element(const char *name,element_t *el);
element_t *get_element_depth(const char *name,element_t *el,int depth);
char *get_text(const char *element,element_t *el);
int get_int(const char *element,element_t *el);

element_t *create_element(element_t *parent,const char *name,etype_t type);
element_t *create_text_element(element_t *parent,const char *name,const char *text);
element_t *create_numeric_element(element_t *parent,const char *name,int value);

int set_attribute(element_t *el,const char *attribute,const char *value);

char *point_to_utf8(unsigned int point, char *dest);


#endif /* ALENA_H */
