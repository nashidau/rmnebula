#ifndef AL_LISTITEMS_H
#define AL_LISTITEMS_H

#include <gtk/gtk.h>
#include <gnome.h>
#include "alena_types.h"

#ifndef AL_ITEMS_H
#include "items.h"
#endif

struct al_item * al_get_selected_item();

void al_create_listitems(GtkHPaned * paned);
void al_list_add_one_item(element_t *all);
void al_item_list_empty(void);
void al_item_list_commit(void);

#endif

