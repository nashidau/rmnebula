#ifndef AL_ITEM_SPELLS_H
#include "spells.h"
#endif

#ifndef AL_ITEMPARSING_H
#include "itemparsing.h"
#endif

#ifndef GUITOOLS_H
#include "guitools.h"
#endif

#include "tree.h"

static GtkWidget *spell_multiplier;
static GtkWidget *spell_adder;
static GtkWidget *spell_multiplier_entry;
static GtkWidget *spell_adder_entry;
static GtkWidget *spell_level_entry;
static GtkWidget *spell_cast_as_entry;
static GtkWidget *spell_charges_entry;
static GtkWidget *spell_constant_button;
static GtkWidget *spell_daily_button;
static GtkWidget *spell_charged_button;
static GtkTreeSelection *spell_select;
static struct al_item_spell * selected_spell = NULL;

static void al_spell_mul_toggle    (GtkToggleButton *togglebutton, gpointer user_data);
static void al_spell_add_toggle    (GtkToggleButton *togglebutton, gpointer user_data);
static void al_spell_charged_toggle(GtkToggleButton *togglebutton, gpointer user_data);
static void tree_selection_changed (GtkTreeSelection *selection,   gpointer data);

/* Spell store */
static GtkTreeStore * spellstore;
static GtkWidget *    spellview;

typedef enum {
        SPELL_LIST_NAME,
        SPELL_NAME,
} spell_store_number;

struct store_info spell_store_info[] = {
        { SPELL_LIST_NAME, G_TYPE_STRING, "", 0, 0 ,0},
        { SPELL_NAME,      G_TYPE_STRING, "", 0, 0 ,0},
        { -1, 0, NULL, 0, 0,0 }
};

struct view_info spell_view_info[] = {
        { "Spell list name", SPELL_LIST_NAME, "text",0, 0,0 },
        { "Spell name",      SPELL_NAME,      "text",0, 0,0 },
        { NULL,             0,          NULL  , 0,0, 0 },
};


/* -- */

void
al_create_spells (GtkNotebook * notebook)
{
    GtkWidget *vbox1;
    GtkWidget *table2;
    GtkWidget *hbox1;
    GtkWidget *table3;
	GtkWidget *add_spell;
	GtkWidget *label;
    GtkWidget *scrolledwindow;
    GtkWidget *viewport;
	
	/* General Vertical Layout */
    vbox1 = gtk_vbox_new (FALSE, 0);

	/* Top table */
    table2 = gtk_table_new (2, 3, FALSE);
    gtk_box_pack_start (GTK_BOX (vbox1), table2, FALSE, TRUE, 0);

	/* Widgets in top table */
	spell_multiplier = gui_new_checkbutton_in_table(_("Multiplier"),table2,0,1,0,1);
	spell_adder      = gui_new_checkbutton_in_table(_("Adder"),     table2,0,1,1,2);
	
    g_signal_connect(GTK_OBJECT(spell_multiplier),"toggled",G_CALLBACK(al_spell_mul_toggle),NULL);
    g_signal_connect(GTK_OBJECT(spell_adder),     "toggled",G_CALLBACK(al_spell_add_toggle),NULL);

	gui_new_label_in_table(_("times"),table2,1,2,0,1);
	gui_new_label_in_table(_("plus"), table2,1,2,1,2);

	spell_multiplier_entry = gui_new_entry_in_table(table2,2,3,0,1);
	spell_adder_entry      = gui_new_entry_in_table(table2,2,3,1,2);

	gtk_widget_set_sensitive(spell_multiplier_entry,FALSE);
	gtk_widget_set_sensitive(spell_adder_entry,FALSE);

	/* Bottom horizontal layout */
    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	/* List of spells */
    spellstore = make_tree_store (spell_store_info);
    spellview  = tree_create_view(GTK_TREE_MODEL(spellstore),spell_view_info);
    // g_object_unref(G_OBJECT(spellstore));

    /* Setup the selection handler */
    spell_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (spellview));
    gtk_tree_selection_set_mode (spell_select, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (spell_select), "changed",
                      G_CALLBACK (tree_selection_changed),
                      NULL);


    gtk_widget_show (spellview);
    gtk_box_pack_start (GTK_BOX (hbox1), spellview, TRUE, TRUE, 0);
    
	/* End list of spells */

    table3 = gtk_table_new (6, 2, FALSE);
    gtk_widget_show (table3);
    gtk_box_pack_start (GTK_BOX (hbox1), table3, TRUE, TRUE, 0);

	add_spell       = gui_new_button_in_table     (_("Add Spell..."),table3,0,1,0,1);
	spell_constant_button = gui_new_checkbutton_in_table(_("Constant"),    table3,0,1,1,2);
	spell_daily_button    = gui_new_checkbutton_in_table(_("Daily"),       table3,0,1,2,3);
	spell_charged_button  = gui_new_checkbutton_in_table(_("Charged"),     table3,0,1,3,4);
	g_signal_connect(G_OBJECT(spell_charged_button),"toggled",(GtkSignalFunc)al_spell_charged_toggle,NULL);

	gui_new_label_in_table(_("Level"),        table3,0,1,4,5);
	gui_new_label_in_table(_("Cast As Level"),table3,0,1,5,6);
	gui_new_label_in_table(_("Charges"),      table3,0,1,6,7);

	spell_level_entry   = gui_new_entry_in_table(table3,1,2,4,5);
	spell_cast_as_entry = gui_new_entry_in_table(table3,1,2,5,6);
	spell_charges_entry = gui_new_entry_in_table(table3,1,2,6,7);
	
	gtk_widget_set_sensitive(spell_charges_entry,FALSE);

	/* Add the panel */
	label = gtk_label_new ("Spells");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox1, label);
}

/*-- Loading : Multipliers --*/
void
al_item_spells_got_times (struct al_item * an_item, element_t * el)
{
	an_item->spell_mul_times = atof(el->text);
}

void
al_item_spells_multiplier (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
	        { "times", AL_PARSING_UNIQUE, al_item_spells_got_times, 0 },
                { NULL,0,0,0 }
        };

	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
	
	if(an_item->spell_mul_times!=0.f)
	{
		an_item->spell_multiplier = 1;
	}
}

/*-- Loading : Adders --*/
void
al_item_spells_got_number (struct al_item * an_item, element_t * el)
{
	an_item->spell_add_plus = atoi(el->text);
}

void
al_item_spells_adder (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
                { "number", AL_PARSING_UNIQUE, al_item_spells_got_number,0 },
                { NULL,0,0,0 }
	};

	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
	
	if(an_item->spell_add_plus!=0)
	{
		an_item->spell_adder = 1;
	}
}

/*-- Loading : Spelllists --*/
void
al_allocate_spelllist(struct al_item * an_item)
{
	/* Allocate a spell list for object effect */
	struct al_item_spell * a_spell;
	a_spell = (struct al_item_spell *) malloc(sizeof(struct al_item_spell));
	if(!a_spell)
	{
		printf("Fatal Error : no more memory for allocation a spell list\n");
		exit(1);
	}
	memset(a_spell,0,sizeof(struct al_item_spell));
		
	an_item->spell_list = g_list_prepend(an_item->spell_list,a_spell);
}

void
al_free_spelllist(struct al_item_spell * a_spell)
{
    #define AL_FREE_TEXT_FIELD(POINTER) { if(POINTER){free(POINTER); POINTER=NULL;} }
    AL_FREE_TEXT_FIELD(a_spell->spell_list_name);
    AL_FREE_TEXT_FIELD(a_spell->spell_name);
    #undef AL_FREE_TEXT_FIELD
}

void
al_item_spells_name (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    al_set_item_text_field(&(top_spell->spell_list_name),el->text);
}

void
al_item_spells_constant (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    top_spell->type |= AL_ITEM_SPELL_CONSTANT;
}

void
al_item_spells_daily (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    top_spell->type |= AL_ITEM_SPELL_DAILY;
}

void
al_item_spells_spellname (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    al_set_item_text_field(&(top_spell->spell_name),el->text);
}

void
al_item_spells_charges (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    top_spell->type |= AL_ITEM_SPELL_CHARGED;
    top_spell->charges = atoi(el->text);
}

void
al_item_spells_level (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    top_spell->level = atoi(el->text);
}


void
al_item_spells_spell (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
		{ "name",     AL_PARSING_UNIQUE, al_item_spells_spellname,0 },
		{ NULL,0,0,0 }
	};

	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
}

void
al_item_spells_castas_level (struct al_item * an_item, element_t * el)
{
    struct al_item_spell * top_spell;
    top_spell = (struct al_item_spell *) an_item->spell_list->data;
    top_spell->cast_as_level = atoi(el->text);
}

void
al_item_spells_castas (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
		{ "level",     AL_PARSING_UNIQUE, al_item_spells_castas_level,0 },
		{ NULL,0,0,0 }
	};

	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
}



void
al_item_spells_print (struct al_item * an_item, element_t * el)
{
	printf("%s\n",el->text);
}

void
al_item_spells_spelllist (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
		{ "name",     AL_PARSING_UNIQUE, al_item_spells_name,0 },
		{ "constant", AL_PARSING_UNIQUE, al_item_spells_constant,0 },
		{ "daily",    AL_PARSING_UNIQUE, al_item_spells_daily,0 },
		{ "charges",  AL_PARSING_UNIQUE, al_item_spells_charges,0 },
		{ "level",    AL_PARSING_UNIQUE, al_item_spells_level,0 },
		{ "spell",    AL_PARSING_UNIQUE, al_item_spells_spell,0 },
		{ "cast-as",  AL_PARSING_UNIQUE, al_item_spells_castas,0 },
		{ NULL,0,0,0 }
	};
	
	struct al_item_spell * a_spell;
	al_allocate_spelllist(an_item);
	
	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
}

/*-- Loading : spells on item --*/
void
al_item_spells (struct al_item * an_item, element_t * el)
{
	static struct al_xml_parsing one_item_parsing[] = {
		{ "multiplier", AL_PARSING_UNIQUE,  al_item_spells_multiplier,0},
		{ "adder",      AL_PARSING_UNIQUE,  al_item_spells_adder,0 },
		{ "spelllist",  AL_PARSING_SEVERAL, al_item_spells_spelllist,0 },
		{ NULL,0,0,0 }
	};

	/* Parsing */
	al_item_general_parsing(an_item,el,one_item_parsing);
}

/*-- Gui --*/
void
al_set_spells_description(struct al_item * an_item)
{
	/* Sets the widgets to values found in the item */
	/* Note : this (sprintf) could lead to a buffer overflow. Must set the max number somewhere */
	char text[50];
    GList *elems;
    
	sprintf(text,"%2.1f",an_item->spell_mul_times);
	gtk_entry_set_text(GTK_ENTRY(spell_multiplier_entry), text);
	sprintf(text,"%d",an_item->spell_add_plus);
	gtk_entry_set_text(GTK_ENTRY(spell_adder_entry),      text);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_multiplier), an_item->spell_multiplier);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_adder),      an_item->spell_adder);

    gtk_tree_store_clear(spellstore);
	for (elems = an_item->spell_list; elems != NULL; elems = elems->next)
	{
        GtkTreeIter iter;
		struct al_item_spell * a_spell = elems->data;
        gtk_tree_store_append(spellstore,&iter,NULL);
        gtk_tree_store_set   (spellstore,&iter,0,a_spell->spell_list_name,1,a_spell->spell_name,-1);
	}
}

void
al_get_spells_description(struct al_item * an_item)
{
}

void
al_set_one_spell_description(struct al_item_spell * a_spell)
{
    if(a_spell==NULL)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_constant_button),FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_daily_button)   ,FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_charged_button) ,FALSE);
        gtk_entry_set_text(GTK_ENTRY(spell_level_entry),  "");
        gtk_entry_set_text(GTK_ENTRY(spell_cast_as_entry),"");
        gtk_entry_set_text(GTK_ENTRY(spell_charges_entry),"");
    }
    else
    {
        /* Note : this (sprintf) could lead to a buffer overflow. Must set the max number somewhere */
        char text[50];
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_constant_button),a_spell->type&AL_ITEM_SPELL_CONSTANT);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_daily_button)   ,a_spell->type&AL_ITEM_SPELL_DAILY   );
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spell_charged_button) ,a_spell->type&AL_ITEM_SPELL_CHARGED );
        
        sprintf(text,"%i",a_spell->level);
        gtk_entry_set_text(GTK_ENTRY(spell_level_entry),  text);
        sprintf(text,"%i",a_spell->cast_as_level);
        gtk_entry_set_text(GTK_ENTRY(spell_cast_as_entry),text);
        sprintf(text,"%i",a_spell->charges);
        gtk_entry_set_text(GTK_ENTRY(spell_charges_entry),text);
    }
}

void
al_get_one_spell_description(struct al_item_spell * a_spell)
{
}


static void
al_spell_mul_toggle(GtkToggleButton *togglebutton, gpointer user_data){
        gboolean active;
        active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(spell_multiplier));
		gtk_widget_set_sensitive(spell_multiplier_entry,active);
}

static void
al_spell_add_toggle(GtkToggleButton *togglebutton, gpointer user_data){
        gboolean active;
        active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(spell_adder));
		gtk_widget_set_sensitive(spell_adder_entry,active);
}

void
al_spell_charged_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
        gboolean active;
        active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
		gtk_widget_set_sensitive(spell_charges_entry,active);
}


struct al_item_spell *
al_get_selected_spell()
{
    return selected_spell;
}


void
tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter      iter_selected;
    GtkTreeModel *   model;
    struct al_item * selected_item;

    selected_item = (struct al_item *) al_get_selected_item();
    if(!selected_item)
        return;
    
    if(gtk_tree_selection_get_selected (selection, &model, &iter_selected))
    {
        GtkTreePath * path;
        GList * elems;
        gchar * string_path;
        int index,i;
        
        path        = gtk_tree_model_get_path(model,&iter_selected);
        string_path = gtk_tree_path_to_string (path);
        index       = atoi(string_path);

        g_free(string_path);
        gtk_tree_path_free(path);
        
        i = 0;
        elems = selected_item->spell_list;
        while(elems!=NULL && i!=index)
        {
            i++;
            elems = elems->next;
        }
        
        if(elems!=NULL)
        {
            selected_spell = elems->data;
        }
        else
        {
            selected_spell = NULL;
        }
    }
    else
    {
        // Nothing more selected
        selected_spell = NULL;
    }
    
    al_set_one_spell_description(selected_spell);
}
