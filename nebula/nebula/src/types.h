#ifndef TYPES_H
#define TYPES_H

/* FIXME: This should be in a config file or part of the class desription
 */
enum { EXTRA_BASE = 4 };

typedef enum {
        BUY_DP,
        BUY_RANKS,
        BUY_CANCEL
} buymode_t;

struct realm_subtype {
        char *name;
        char *description;
};

/* Printing modes */
typedef enum {
        PRINT_NORMAL, /* Anything not hidden */
        PRINT_ALL,    /* Everything */
        PRINT_MINIMAL /* Anything with more then 1 skill rank  Unimplemented */
} print_t;

typedef enum {
        NOTFOUND,
        FOUND
} found_t;

#endif /* TYPES_H */
