#ifndef PARSE_H
#define PARSE_H

#include "alena.h"

struct tags {
        char *tag;
        void (*parsefunc)(element_t *);
};

int parse_file(const char *file,struct tags *parsers);

#endif /* PARSE_H */
