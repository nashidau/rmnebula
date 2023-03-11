#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

enum {  ALENA_BUF=1024,
        MAX_SPACES = 160,  /* Used in d_spaces */
        TAB_SIZE = 8, /* How many spaces is a 'tab' in XML output */
};

void set_whitespace(int whitespace);
char *gettoken(void);
void map_file(FILE *fp);

void check_utf8(char *token);
extern int utf8;


#endif /* TOKEN_H */
