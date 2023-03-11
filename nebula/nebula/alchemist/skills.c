#ifndef AL_ITEM_SKILLS_H
#include "skills.h"
#endif

#include "tree.h"

static GtkWidget * widget_skill_genre;
static GtkWidget * widget_skill_entry;
static GtkWidget * widget_genre_load;
static GtkWidget * widget_show_bonus_only;

/* Table for skill bonus */
typedef enum {
        SKILL_NAME,
        SKILL_BONUS
} skill_store_number;

struct store_info skill_store_info[] = {
        { SKILL_NAME,  G_TYPE_STRING, "", 0, 0 ,0},
        { SKILL_BONUS, G_TYPE_INT   , "", 0, 0 ,0},
        { -1, 0, NULL, 0, 0,0 }
};

struct view_info skill_view_info[] = {
        { "Name",  SKILL_NAME,  "text",0, 0,0 },
        { "Bonus", SKILL_BONUS, "text",0, 0,0 },
        { NULL, 0, NULL, 0 , 0, 0 }
};

static GtkTreeStore * skillstore;
static GtkWidget *    skillview;

void
al_create_skills (GtkNotebook * notebook)
{
    GtkWidget * up_table;
    GtkWidget * main_table;
    GtkLabel  * label;

    /* Main table */
	main_table = gtk_table_new (1, 2, FALSE);
	gtk_widget_show (main_table);

    /* Upper table */
	up_table = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (up_table);

    gtk_table_attach (GTK_TABLE (main_table), up_table, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    /* Combo box for genre file */
    widget_skill_genre = gtk_combo_new ();
	gtk_widget_show (widget_skill_genre);
    gtk_table_attach (GTK_TABLE (up_table), widget_skill_genre, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    {
        /* Genre file test. TODO : load available genre files */
        GList * genre_list = NULL;
        genre_list = g_list_append (genre_list, (gpointer) "RMSS");
        genre_list = g_list_append (genre_list, (gpointer) "Shadow World");
        gtk_combo_set_popdown_strings (GTK_COMBO (widget_skill_genre), genre_list);
        g_list_free(genre_list);
    }

    widget_skill_entry = GTK_COMBO (widget_skill_genre)->entry;
    gtk_widget_show (widget_skill_genre);
    gtk_entry_set_text (GTK_ENTRY (widget_skill_entry), _("Genre file"));

    widget_genre_load = (GtkWidget *) gui_new_button_in_table(_("Load genre"), up_table, 1, 2, 0, 1);
    
    /* Show only bonus */
    widget_show_bonus_only = (GtkWidget *) gui_new_checkbutton_in_table(_("Show only skills with bonus"),up_table,0,1,1,2);
    
    /* Skill bonus tree view */
    skillstore = make_tree_store(skill_store_info);
    skillview = tree_create_view(GTK_TREE_MODEL(skillstore),skill_view_info);
    g_object_unref(G_OBJECT(skillstore));
    
    gtk_widget_show (skillview);
    gtk_table_attach (GTK_TABLE (main_table), skillview, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);


    /* Adding to notebook */
    label = GTK_LABEL (gtk_label_new ("Skills"));
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                              GTK_WIDGET   (main_table),
                              GTK_WIDGET   (label));

}
