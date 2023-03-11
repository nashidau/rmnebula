#ifndef AL_ITEM_SPELLS_H
#define AL_ITEM_SPELLS_H

#include <gtk/gtk.h>
#include <gnome.h>
#include "alena_types.h"

#ifndef AL_ITEMS_H
#include "items.h"
#endif

struct al_item_spell * al_get_selected_spell();

void al_create_spells         (GtkNotebook * notebook);
void al_item_spells           (struct al_item * an_item, element_t * el);
void al_set_spells_description(struct al_item * an_item);
void al_get_spells_description(struct al_item * an_item);
void al_free_spelllist        (struct al_item_spell * a_spell);

#endif
