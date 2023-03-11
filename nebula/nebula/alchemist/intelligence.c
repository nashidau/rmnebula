#ifndef AL_ITEM_INTELLIGENCE_H
#include "intelligence.h"
#endif

#ifndef AL_ITEMPARSING_H
#include "itemparsing.h"
#endif

#ifndef GUITOOLS_H
#include "guitools.h"
#endif

static GList * level_list = NULL;
static GtkWidget *int_alignment; /* is this mispelt on purpose? (nash)  corrected (sg) */
static GtkWidget *int_purpose;
static GtkWidget *int_level;
static GtkWidget *int_level_entry;
static GtkWidget *intelligent_button;

static void is_int_toggle(void);
/* These are the buttons above (except intelligence_button) */
GtkWidget *boxitems[7] = {0};

void
al_create_intelligence (GtkNotebook * notebook)
{
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *label4;
	GtkWidget *label5;
	GtkWidget *label6;
	
	table = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table);

	intelligent_button = gui_new_checkbutton_in_table(_("Intelligent Object"),table,0,1,0,1);

	label4 = gui_new_label_in_table(_("Alignement"),table,0,1,1,2);
	label5 = gui_new_label_in_table(_("Purpose"),   table,0,1,2,3);
	label6 = gui_new_label_in_table(_("Level"),     table,0,1,3,4);

	int_alignment = gui_new_entry_in_table(table,1,2,1,2);
    int_purpose   = gui_new_entry_in_table(table,1,2,2,3);

    int_level = gtk_combo_new ();
	gtk_widget_show (int_level);
    gtk_table_attach (GTK_TABLE (table), int_level, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

	level_list = g_list_append (level_list, (gpointer) _("Empathic"));
	level_list = g_list_append (level_list, (gpointer) _("Low Intelligence"));
	level_list = g_list_append (level_list, (gpointer) _("Medium Intelligence"));
	level_list = g_list_append (level_list, (gpointer) _("High Intelligence"));
	level_list = g_list_append (level_list, (gpointer) _("Very High Intelligence"));
	level_list = g_list_append (level_list, (gpointer) _("Artifact"));
	level_list = g_list_append (level_list, (gpointer) _("Legendary Artifact"));

	gtk_combo_set_popdown_strings (GTK_COMBO (int_level), level_list);
	g_list_free(level_list);

    int_level_entry = GTK_COMBO (int_level)->entry;
    gtk_widget_show (int_level_entry);
    gtk_entry_set_text (GTK_ENTRY (int_level_entry), _("Empathic"));

    label = gtk_label_new ("Intelligence");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  GTK_WIDGET (table), GTK_WIDGET (label));

	{
		int i;
        /* Set up activation on 'is_intelligent' */
        g_signal_connect(GTK_OBJECT(intelligent_button),"toggled",
                        G_CALLBACK(is_int_toggle),NULL);
        boxitems[0] = int_alignment;
        boxitems[1] = int_purpose;
        boxitems[2] = int_level;
        boxitems[3] = label4;
        boxitems[4] = label5;
        boxitems[5] = label6;
        for (i = 0 ; boxitems[i] != 0 ; i ++)
                gtk_widget_set_sensitive(boxitems[i],FALSE);
	}
}


#define MAKE_AL_GOT_INT_FUNCTION(NAME) \
	void al_got_int_##NAME(struct al_item * an_item, element_t * el) \
	{ \
		al_set_item_text_field(&an_item->int_##NAME,el->text); \
	}

MAKE_AL_GOT_INT_FUNCTION(alignment);
MAKE_AL_GOT_INT_FUNCTION(purpose);
MAKE_AL_GOT_INT_FUNCTION(level);

void
al_item_intelligence(struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
	        { "alignment", AL_PARSING_UNIQUE, al_got_int_alignment,0 },
	        { "purpose",   AL_PARSING_UNIQUE, al_got_int_purpose,0 },
		{ "level",     AL_PARSING_UNIQUE, al_got_int_level,0 },
                { NULL,0,0,0 }
        };

	an_item->intelligent = 1;
	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
}

void
al_set_intelligence_description(struct al_item * an_item)
{
	/* Sets the widgets to values found it the item */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(intelligent_button), an_item->intelligent);
	gtk_entry_set_text(GTK_ENTRY(int_alignment),    an_item->int_alignment);
	gtk_entry_set_text(GTK_ENTRY(int_purpose),       an_item->int_purpose);
	gtk_entry_set_text(GTK_ENTRY(int_level_entry),   an_item->int_level);
}

void
al_get_intelligence_description(struct al_item * an_item)
{
	/* Gets the structure to values found in the widgets */
}




static void
is_int_toggle(void){
        gboolean active;
        int i;

        active = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON(intelligent_button));

        for (i = 0 ; boxitems[i] != 0 ; i ++)
                gtk_widget_set_sensitive(boxitems[i],active);
}

