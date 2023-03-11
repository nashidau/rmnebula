#ifndef AL_GENERAL_H
#define AL_GENERAL_H

#include <gtk/gtk.h>
#include <gnome.h>

#ifndef AL_ITEMS_H
#include "items.h"
#endif

void al_create_general         (GtkNotebook *notebook);
void al_set_general_description(struct al_item * an_item);
void al_get_general_description(struct al_item * an_item);


#endif

