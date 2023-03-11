#include <gnome.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <glib.h>
#include <sys/types.h>
#ifdef unix
#include <dirent.h>
#endif

/* Nebula files */
#include "tree.h"
#include "alena.h"
#include "parse.h"
#include "gui.h"
#include "file.h"

#define STARDUST_NAME "Stardust"
#define STARDUST_VERSION "0.1.0"

#define QSPELLLISTDTD "<!DOCTYPE spelllist SYSTEM " \
                        "\"http://www.nash.nu/Rolemaster/"  \
                        "XML/spelllist/spelllist.dtd\">\n"
#define SPELLLISTDTD " "

enum {
        SPELL_ELEMENT_NAME, /* slot 0 reserved for parser */
        SPELL_NAME,
        SPELL_LEVEL,
        SPELL_AREAOFEFFECT,
        SPELL_DURATION,
        SPELL_RANGE,
        SPELL_TYPE,
        SPELL_NOPP,
        SPELL_INSTANTANEOUS,
        SPELL_DESCRIPTION,
        SPELL_TAGS,
        SPELL_ANNOTATION,
        SPELL_ANNOTATION_TAGS,
        SPELL_EDIT,
};

static void star_create_window(void);
struct fileinfo *star_open_spelllist(const char *name);
int star_close_spelllist(void);
void star_save_spelllist(struct fileinfo *finfo);
