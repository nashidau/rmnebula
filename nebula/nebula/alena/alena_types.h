#ifndef ALENA_TYPES_H
#define ALENA_TYPES_H

#include <stdio.h>
#include <glib.h>

enum {
        WHITESPACE_USE,
        WHITESPACE_IGNORE,
};


typedef enum {
        ETYPE_NONE,  /* Invalid etype (used internally) */
        ETYPE_TEXT,  /* Contains plain text only (in text) */
        ETYPE_ELEMENT, /* Contains other elements (in elements) */
        ETYPE_EMPTY  /* An empty tag (<foo />) */
} etype_t;

typedef struct attribute {
        char *name;
        char *value;
} attribute_t;


typedef struct element {
        char *name;  /* The name of the tag: <foo> */
        etype_t type; /* What is in this tag (see etype_t) */

        char *text;  /* If it is TEXT tag (ETYPE_TEXT) the text is here */
        GList *elements; /* Sub elements (GList of element_t's) */

        GList *attributes; /* A list of attributes */
} element_t;




#endif /* ALENA_TYPES_H */
