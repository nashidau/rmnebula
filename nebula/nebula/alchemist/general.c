#include <gtk/gtk.h>
#include <gnome.h>
#include "general.h"

#ifndef GUITOOLS_H
#include "guitools.h"
#endif

static GtkWidget * general_name_entry;
static GtkWidget * general_desc_entry;
static GtkWidget * general_size_entry;
static GtkWidget * general_weight_entry;
static GtkWidget * general_value_entry;
static GtkWidget * general_misc_name;
	
#define AL_CHECK_CONTAINER	0
#define AL_CHECK_MISC		1
#define AL_CHECK_GEM		2
#define AL_CHECK_POISON		3
#define AL_CHECK_HERB		4
#define AL_CHECK_ARMOR		5
#define AL_CHECK_WEAPON		6
#define AL_CHECK_SHIELD		7
#define AL_CHECK_MAX		8

static GtkWidget * checkbutton[AL_CHECK_MAX];
static void al_toggle_misc_button (GtkToggleButton *togglebutton, gpointer user_data);

void al_create_general(GtkNotebook *notebook)
{
	GtkWidget *table;
	GtkWidget *choice_table;
	GtkWidget *general_label;

	// Layout
	table = gtk_table_new(7,2,FALSE);

	// Label
	gui_new_label_in_table(_("Name"),       table,0,1,0,1);
	gui_new_label_in_table(_("Description"),table,0,1,1,2);
	gui_new_label_in_table(_("Weight"),     table,0,1,2,3);
	gui_new_label_in_table(_("Size"),       table,0,1,3,4);
	gui_new_label_in_table(_("Value"),      table,0,1,4,5);

	// Corresping entries
	general_name_entry   = gui_new_entry_in_table(table,1,2,0,1);
	general_desc_entry   = gui_new_entry_in_table(table,1,2,1,2);
	general_weight_entry = gui_new_entry_in_table(table,1,2,2,3);
	general_size_entry   = gui_new_entry_in_table(table,1,2,3,4);
	general_value_entry  = gui_new_entry_in_table(table,1,2,4,5);

	// Types
	gui_new_label_in_table(_("Type"),table,0,1,5,6);

	choice_table = gtk_vbox_new(FALSE,0);
	gtk_table_attach (GTK_TABLE(table),choice_table,1,2,5,6,
	                  (GtkAttachOptions)(GTK_FILL),
	                  (GtkAttachOptions)(GTK_FILL),0,0);

	checkbutton[AL_CHECK_CONTAINER] = gui_new_checkbutton_in_box(_("Container"),choice_table);
	checkbutton[AL_CHECK_MISC]      = gui_new_checkbutton_in_box(_("Misc"),     choice_table);
	checkbutton[AL_CHECK_GEM]       = gui_new_checkbutton_in_box(_("Gem"),      choice_table);
	checkbutton[AL_CHECK_POISON]    = gui_new_checkbutton_in_box(_("Poison"),   choice_table);
	checkbutton[AL_CHECK_HERB]      = gui_new_checkbutton_in_box(_("Herb"),     choice_table);
	checkbutton[AL_CHECK_ARMOR]     = gui_new_checkbutton_in_box(_("Armor"),    choice_table);
	checkbutton[AL_CHECK_WEAPON]    = gui_new_checkbutton_in_box(_("Weapon"),   choice_table);
	checkbutton[AL_CHECK_SHIELD]    = gui_new_checkbutton_in_box(_("Shield"),   choice_table);

	g_signal_connect(G_OBJECT(checkbutton[AL_CHECK_MISC]),"toggled",(GtkSignalFunc)al_toggle_misc_button,NULL);

	gui_new_label_in_table(_("Misc. Name"),table,0,1,6,7);
	general_misc_name = gui_new_entry_in_table(table,1,2,6,7);
	gtk_widget_set_sensitive(general_misc_name,FALSE);

	/* Add notebook */
	general_label = gtk_label_new(_("General"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			GTK_WIDGET(table),
			GTK_WIDGET(general_label));
}

void
al_toggle_misc_button (GtkToggleButton *togglebutton, gpointer user_data)
{
	gboolean active = gtk_toggle_button_get_active(togglebutton);
	gtk_entry_set_editable  (GTK_ENTRY(general_misc_name),active);
	gtk_widget_set_sensitive(general_misc_name,           active);
}


#define AL_SET_TOGGLE(AL_ITEM_TYPE,AL_CHECK_WIDGET) \
	{ \
		gboolean active; \
		if(an_item->type & AL_ITEM_TYPE) \
			active = TRUE; \
		else \
			active = FALSE; \
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton[AL_CHECK_WIDGET]),active); \
	}


void
al_set_general_description(struct al_item * an_item)
{
	/* Sets the widgets to values found it the item */
	gtk_entry_set_text(GTK_ENTRY(general_name_entry),	an_item->name);
	gtk_entry_set_text(GTK_ENTRY(general_desc_entry),	an_item->description);
	gtk_entry_set_text(GTK_ENTRY(general_weight_entry), an_item->weight);
	gtk_entry_set_text(GTK_ENTRY(general_size_entry),	an_item->size);
	gtk_entry_set_text(GTK_ENTRY(general_value_entry),	an_item->value);
	
	AL_SET_TOGGLE(AL_ITEM_CONTAINER,AL_CHECK_CONTAINER);
	AL_SET_TOGGLE(AL_ITEM_MISC,		AL_CHECK_MISC);
	AL_SET_TOGGLE(AL_ITEM_GEM,		AL_CHECK_GEM);
	AL_SET_TOGGLE(AL_ITEM_POISON,	AL_CHECK_POISON);
	AL_SET_TOGGLE(AL_ITEM_HERB,		AL_CHECK_HERB);
	AL_SET_TOGGLE(AL_ITEM_ARMOR,	AL_CHECK_ARMOR);
	AL_SET_TOGGLE(AL_ITEM_WEAPON,	AL_CHECK_WEAPON);
	AL_SET_TOGGLE(AL_ITEM_SHIELD,	AL_CHECK_SHIELD);

	gtk_entry_set_text    (GTK_ENTRY(general_misc_name), an_item->misc_name);
	gtk_entry_set_editable(GTK_ENTRY(general_misc_name),an_item->type&AL_ITEM_MISC);
}

void
al_get_general_description(struct al_item * an_item)
{
	/* Gets the structure to values found in the widgets */
}


