#ifndef AL_ITEM_INTELLIGENCE_H
#define AL_ITEM_INTELLIGENCE_H

#include <gtk/gtk.h>
#include <gnome.h>
#include "alena_types.h"

#ifndef AL_ITEMS_H
#include "items.h"
#endif

void al_create_intelligence         (GtkNotebook * notebook);
void al_item_intelligence           (struct al_item * an_item, element_t * el);
void al_set_intelligence_description(struct al_item * an_item);
void al_get_intelligence_description(struct al_item * an_item);
#endif
