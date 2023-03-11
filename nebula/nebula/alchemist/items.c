#include <gtk/gtk.h>
#include <gnome.h>

#ifndef AL_ITEMS_H
#include "items.h"
#endif

extern GList * al_allocated_items;

void
al_set_item_text_field(char ** field,const char * name)
{
	int size = strlen(name)+1;
	*field = (char*) malloc(size*sizeof(char));
	strcpy(*field,name);
}

struct al_item *
al_allocate_item(void)
{
	/* Allocate one item object */
	struct al_item * an_item;
	an_item = (struct al_item*) malloc(sizeof(struct al_item));
	if(!an_item)
	{
		printf("Fatal Error : no more memory for allocation an item\n");
		exit(1);
	}
	memset(an_item,0,sizeof(struct al_item));
	
	al_allocated_items = g_list_prepend(al_allocated_items,an_item);
	return an_item;
}

void
al_free_item(struct al_item * an_item)
{
	/* Free all spells */
	GList *elems;
	
	/* Item structure */
	for (elems = an_item->spell_list; elems != NULL; elems = elems->next)
	{
        al_free_spelllist(elems->data);
		free(elems->data);
	}
	g_list_free(an_item->spell_list);
	an_item->spell_list = NULL;
	
	#define AL_FREE_TEXT_FIELD(POINTER) { if(POINTER){free(POINTER); POINTER=NULL;} }
	AL_FREE_TEXT_FIELD(an_item->name);
	AL_FREE_TEXT_FIELD(an_item->description);
	AL_FREE_TEXT_FIELD(an_item->weight);
	AL_FREE_TEXT_FIELD(an_item->size);
	AL_FREE_TEXT_FIELD(an_item->value);
	AL_FREE_TEXT_FIELD(an_item->misc_name);
	AL_FREE_TEXT_FIELD(an_item->int_alignment);
	AL_FREE_TEXT_FIELD(an_item->int_purpose);
	AL_FREE_TEXT_FIELD(an_item->int_level);

	an_item->widget_pointer = NULL;
	#undef AL_FREE_TEXT_FIELD
}
