#ifndef LIBNEB_H
#define LIBNEB_H

/*
 * These are the files which are application independant
 */
#include <gtk/gtk.h>
#include <gnome.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>



#include "tree.h"
#include "alena.h"
#include "parse.h"
#include "gui.h"
#include "file.h"
#include "tips.h"
#include "options.h"
#include "config.h"
#include "print.h"
#include "unix.h"
#include "xml.h"

/* Assumes the main program window is available */
extern GtkWidget *window;

/* Initialises the libraries with one call, and
 *      does various GTK startup stuff
 * Args:
 *      file
 */
int libneb_init(char *name,char *version,int argc,char **argv,
                struct fileops *fileops,struct option_info *options);

/*
 * Loads a list of icons.
 *
 * Arguments are name (char *), icon save point (GdkPixbuf **)
 * Terminate witha a NULL name
 */
int
libneb_icons_init(char *name,...);

#endif /* LIBNEB_H */
