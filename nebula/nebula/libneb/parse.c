/*
 * parse.c
 *
 * This file is meant to be included as a library -
 * it should not make any references to external files which
 * are not in the library
 */
#include "alena.h"

#include "parse.h"
#include "options.h"
#include "config.h"

static void xml_insert(element_t *,struct tags *);

#define PDATA_BUF 1024

/*
 * parse_file
 *
 * Load a file and parse it into the the relevant database
 *
 * Args:
 *    char *pointer to the file to load
 *
 * Returns:
 *      0 on success
 *      Non-zero on error
 */
int
parse_file(const char *filename,struct tags *parsers){
        element_t *el;

        el = alena_parsefile(filename,WHITESPACE_IGNORE);

        if (el == NULL){
                printf("File '%s' not valid\n",filename);
                return -1;
        }

        xml_insert(el,parsers);

        return 0;
}

/*
 * xml_insert
 *
 * Insert data from any XML tree into main data
 * Note this allows you to set all sorts of random things ;-)
 */
static void
xml_insert(element_t *alldata,struct tags *parsers){
        struct tags *parser;
        element_t *el;

        parser = parsers;
        for ( ; parser->tag != NULL ; parser++ ){
                el = get_element_depth(parser->tag,alldata,1);
                if (el){
                        parser->parsefunc(el);
                }
        }
}

