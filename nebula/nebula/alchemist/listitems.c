#include <gtk/gtk.h>
#include <gnome.h>
#include <string.h>
#include "alena_types.h"

#ifndef AL_ITEM_INTELLIGENCE_H
#include "intelligence.h"
#endif

#ifndef AL_GENERAL_H
#include "general.h"
#endif

#ifndef AL_LOGWINDOW_H
#include "logwindow.h"
#endif

#ifndef AL_ITEMS_H
#include "listitems.h"
#endif

#ifndef AL_ITEM_SPELLS_H
#include "spells.h"
#endif

#ifndef AL_ITEMPARSING_H
#include "itemparsing.h"
#endif

void al_select_list_item(GtkList *list, GtkWidget *widget, gpointer user_data);

static GtkList		  * item_list_widget	= NULL;
static GList		  * item_list			= NULL;
static struct al_item * selected_item		= NULL;

GList		* al_allocated_items = NULL;

struct al_item *
al_get_selected_item()
{
    return selected_item;
}

void
al_set_all_widgets_from_item(struct al_item * an_item)
{
	if(an_item==NULL)
		return;
	
	al_set_general_description(an_item);
	al_set_intelligence_description(an_item);
	al_set_spells_description(an_item);
}

void
al_create_listitems (GtkHPaned * paned)
{
	/* Create the list interface */
	GtkScrolledWindow *scrolledwindow;
	GtkViewport *viewport;

	scrolledwindow = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
	gtk_paned_pack1 (GTK_PANED (paned), GTK_WIDGET (scrolledwindow), FALSE, TRUE);

	viewport = GTK_VIEWPORT (gtk_viewport_new (NULL, NULL));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (viewport));

	item_list_widget = GTK_LIST (gtk_list_new ());
	gtk_container_add (GTK_CONTAINER (viewport), GTK_WIDGET (item_list_widget));
	
	g_signal_connect(G_OBJECT(item_list_widget), "select-child",
				(GtkSignalFunc)al_select_list_item,NULL);

}


void
al_free_all_items(void)
{
	/* Free all items allocated by al_allocate_item() */
	GList *elems;
	
	/* Item structure */
	for (elems = al_allocated_items; elems != NULL; elems = elems->next)
	{
		al_free_item(elems->data);
		free(elems->data);
	}
	g_list_free(al_allocated_items);
	al_allocated_items = NULL;
	
	selected_item = NULL;
	al_set_all_widgets_from_item(NULL);
}

void
al_item_list_empty(void)
{
	/* Empty the list of items and the widget list */
	gtk_list_clear_items(item_list_widget,0,-1);

	g_list_free(item_list);
	item_list = NULL;
	
	al_free_all_items();
}

void
al_item_list_commit(void)
{
	/* Add the list of item to the widget list */
	gtk_list_append_items(item_list_widget,item_list);
}

void
al_item_misc_name (char ** misc_name, element_t * el)
{
	if (strcmp(el->name, "name")==0)
	{
		if(!*misc_name)
		{
			al_set_item_text_field(misc_name,el->text);
		}
	}
}

#define MAKE_AL_GOT_TYPE_FUNCTION(NAME,FLAG) \
	void al_got_type_##NAME(struct al_item * an_item, element_t * el) \
	{ \
		an_item->type |= FLAG; \
	}

MAKE_AL_GOT_TYPE_FUNCTION(container,AL_ITEM_CONTAINER);
MAKE_AL_GOT_TYPE_FUNCTION(gem,      AL_ITEM_GEM);
MAKE_AL_GOT_TYPE_FUNCTION(poison,   AL_ITEM_POISON);
MAKE_AL_GOT_TYPE_FUNCTION(herb,     AL_ITEM_HERB);
MAKE_AL_GOT_TYPE_FUNCTION(armor,    AL_ITEM_ARMOR);
MAKE_AL_GOT_TYPE_FUNCTION(weapon,   AL_ITEM_WEAPON);
MAKE_AL_GOT_TYPE_FUNCTION(shield,   AL_ITEM_SHIELD);

void
al_got_type_misc(struct al_item * an_item, element_t * el)
{
	an_item->type |= AL_ITEM_MISC;
	if (el->elements->data)
	{
		// There is a name
		// Check first element
		al_item_misc_name(&an_item->misc_name,el->elements->data);
	}
}

void
al_got_item_type (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
											{ "container", AL_PARSING_UNIQUE, al_got_type_container , 0},
											{ "misc",      AL_PARSING_UNIQUE, al_got_type_misc, 0 },
											{ "gem",       AL_PARSING_UNIQUE, al_got_type_gem, 0 },
											{ "poison",    AL_PARSING_UNIQUE, al_got_type_poison, 0 },
											{ "herb",      AL_PARSING_UNIQUE, al_got_type_herb , 0},
											{ "armor",     AL_PARSING_UNIQUE, al_got_type_armor , 0},
											{ "weapon",    AL_PARSING_UNIQUE, al_got_type_weapon , 0},
											{ "shield",    AL_PARSING_UNIQUE, al_got_type_shield , 0},
											{ NULL, 0, 0, 0  }
										};

	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
}




void
al_got_name(struct al_item * an_item, element_t * el)
{
	GtkWidget *item_label;
	/* Add a widget list */
	item_label = gtk_list_item_new_with_label(el->text);
	gtk_widget_show(item_label);
	item_list = g_list_prepend(item_list,item_label);
			
	/* Set item data */
	an_item->widget_pointer = item_label;
	/* Sets name */
	al_set_item_text_field(&an_item->name,el->text);
}

#define MAKE_AL_GOT_FUNCTION(NAME) \
	void al_got_##NAME(struct al_item * an_item, element_t * el) \
	{ \
		al_set_item_text_field(&an_item->NAME,el->text); \
	}

MAKE_AL_GOT_FUNCTION(description);
MAKE_AL_GOT_FUNCTION(weight);
MAKE_AL_GOT_FUNCTION(size);
MAKE_AL_GOT_FUNCTION(value);

void
al_list_add_one_item (element_t * one_item)
{
	/* Load one item from the XML file */
	static struct al_xml_parsing one_item_parsing[] = {
                { "name", 		  AL_PARSING_UNIQUE,  al_got_name, 0  },
                { "description",  AL_PARSING_UNIQUE,  al_got_description, 0 },
                { "weight",       AL_PARSING_UNIQUE,  al_got_weight, 0 },
                { "size",         AL_PARSING_UNIQUE,  al_got_size, 0 },
                { "value",        AL_PARSING_UNIQUE,  al_got_value, 0 },
                { "type",         AL_PARSING_SEVERAL, al_got_item_type, 0 },
                { "intelligence", AL_PARSING_UNIQUE,  al_item_intelligence, 0 },
                { "spells",       AL_PARSING_UNIQUE,  al_item_spells, 0 },
                { NULL, 0, 0, 0 }
        };

	/* Allocate an item */
	struct al_item * an_item = al_allocate_item();

	/* Parsing */
	al_item_general_parsing(an_item,one_item,one_item_parsing);

	if(!an_item->name)
		al_set_item_text_field(&an_item->name,"No name");
	if(!an_item->description)
		al_set_item_text_field(&an_item->description,"No description");
	if(!an_item->weight)
		al_set_item_text_field(&an_item->weight,"");
	if(!an_item->size)
		al_set_item_text_field(&an_item->size,"");
	if(!an_item->value)
		al_set_item_text_field(&an_item->value,"");
	if(!an_item->misc_name)
		al_set_item_text_field(&an_item->misc_name,"");

	if(!an_item->int_alignment)
		al_set_item_text_field(&an_item->int_alignment,"");
	if(!an_item->int_purpose)
		al_set_item_text_field(&an_item->int_purpose,"");
	if(!an_item->int_level)
		al_set_item_text_field(&an_item->int_level,"");

}


void
al_select_list_item(GtkList *list, GtkWidget *widget, gpointer user_data)
{
	/* Called when an item is selected in the object list */
	GList *elems;

	selected_item = NULL;
	/* Item structure */
	for (elems = al_allocated_items; elems != NULL; elems = elems->next)
	{
		struct al_item * an_item = elems->data;
		if(an_item->widget_pointer == widget)
		{
			selected_item = an_item;
			//printf("Selected %s\n",an_item->name);
		}
	}
	
	if(selected_item == NULL)
	{
		al_logwindow_add_text("Selected item not found !\n");
	}
	al_set_all_widgets_from_item(selected_item);
}

