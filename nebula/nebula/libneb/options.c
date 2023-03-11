#include <gconf/gconf-client.h>

#include "libneb.h"

static void options_create_controls(GtkWidget *notebook);
struct optags *get_tab(char *path,GtkWidget *notebook);
static void add_toggle(GtkWidget *box,struct option_info *option);
static void add_entry(GtkWidget *box,struct option_info *option);
static void button_toggle(gpointer data,GtkWidget *widget);
static void button_toggled_gconf(struct option_info *option);
static void string_changed_gconf(struct option_info *option);
static void entry_changed(gpointer data,GtkWidget *widget);
void save_options(GtkWidget *widget);
void close_options(GtkWidget *widget);
struct optags {
        char *path;
        char *tab;
        char *title;
        GtkWidget *box;
};

struct optags tags[] = {
        { FILE_OPTION,    "File",    "File and Path Options",0 },
        { CREATION_OPTION,"Creation","Character Creation Options",0 },
        { STAT_OPTION,    "Stats",   "Stat Options",0 },
        { SKILL_OPTION,    "Skills", "Skill Options",0 },
        { XSL_OPTION,     "XSL",     "XSL Options",0 },
        { NULL,NULL,NULL,NULL},
};

static struct option_info *option_info;

void
options_init(struct option_info *options){
        option_info = options;
}


void
options_creation(void){
        GtkWidget *optionwindow;
        GtkWidget *okbutton;
        GtkWidget *closebutton;
        GtkWidget *book;

        /* Create a window with a close button */
        optionwindow = gtk_dialog_new_with_buttons("Nebula Preferences",
                        GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,
                        NULL);
        gtk_signal_connect_object(GTK_OBJECT(optionwindow), "delete_event",
                                (GtkSignalFunc)close_options, optionwindow);
        closebutton = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	okbutton = gtk_button_new_from_stock(GTK_STOCK_OK);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(optionwindow)->action_area),
                                closebutton,TRUE,TRUE,2);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(optionwindow)->action_area),
                                okbutton,TRUE,TRUE,2);
        gtk_signal_connect_object(GTK_OBJECT(closebutton), "clicked",
                                (GtkSignalFunc)close_options, optionwindow);
	gtk_signal_connect_object(GTK_OBJECT(okbutton), "clicked",
	    			(GtkSignalFunc)save_options, optionwindow);

        book = gtk_notebook_new();
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(optionwindow)->vbox),
                                book,TRUE,TRUE,2);

        options_create_controls(book);

        gtk_widget_show_all(optionwindow);


}

static void
options_create_controls(GtkWidget *notebook){
        struct option_info *option;
        struct optags *tab;

        for (option = option_info ; option->path != NULL ; option ++){
                tab = get_tab(option->path,notebook);
                if (tab == NULL){
                        printf("Couldn't find tab for %s\n",option->path);
                        continue;
                }
                switch (option->type){
                case G_TYPE_BOOLEAN:
                        add_toggle(tab->box,option);
			button_toggled_gconf(option);
                        break;
                case G_TYPE_STRING:
                        add_entry(tab->box,option);
			string_changed_gconf(option);
                        break;
                case G_TYPE_INT:
                        break;
                default:
                        printf("Unknown type: %s\n",g_type_name(option->type));
                }
        }

}

struct optags *
get_tab(char *path,GtkWidget *notebook){
        GtkWidget *frame;
        struct optags *tag;

        for (tag = tags ; tag->title != NULL ; tag ++){
                if (strncmp(path,tag->path,strlen(tag->path)) == 0 &&
                                tag->box != NULL){
                        return tag;
                }
        }
        for (tag = tags; tag->title != NULL ; tag ++){
                if (strncmp(path,tag->path,strlen(tag->path)) == 0){
                        frame = gtk_frame_new(tag->title);
                        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,
                                        gtk_label_new(tag->tab));
                        tag->box = gtk_vbox_new(FALSE,2);
                        gtk_container_add(GTK_CONTAINER(frame),tag->box);
                        return tag;
                }
        }

        fprintf(stderr,"Unknown tag %s\n",path);

        return NULL;
}

static void
add_toggle(GtkWidget *box,struct option_info *option){
        gboolean flag;

        option->button = gtk_check_button_new_with_label(option->desc);
        gtk_box_pack_start(GTK_BOX(box),option->button,TRUE,TRUE,0);
        set_tip(option->button,option->longdesc);

        flag = config_get_bool(option->path);
        if (flag == TRUE){
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(option->button),
                                flag);
        }
}

static void
add_entry(GtkWidget *box,struct option_info *option){
        GtkWidget *label;
        GtkWidget *row;
        const char *string;

        row = new_row(box);
        label = gtk_label_new(option->desc);
        gtk_box_pack_start(GTK_BOX(row),label,FALSE,TRUE,0);
        option->button = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(row),option->button,TRUE,TRUE,0);
        set_tip(option->button,option->longdesc);

        string = config_get_string(option->path);
        if (string == NULL)
                string = "";
        gtk_entry_set_text(GTK_ENTRY(option->button),string);
}


/*
 * Normal call backs
 */
static void
button_toggle(gpointer data,GtkWidget *widget){
        struct option_info *option;
        gboolean state;

        option = (struct option_info *)data;

        state = GTK_TOGGLE_BUTTON(widget)->active;

        config_set_bool(option->path,state);
}
static void
entry_changed(gpointer data,GtkWidget *widget){
        struct option_info *option;
        const char *string;

        option = (struct option_info *)data;

        string = gtk_entry_get_text(GTK_ENTRY(widget));

        config_set_string(option->path,string);
}



/*
 * GConf Call backs
 */


static void
button_toggled_gconf(struct option_info *option){
        gboolean state;

        state = config_get_bool(option->path);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(option->button),state);

}
static void
string_changed_gconf(struct option_info *option){
        const char *string;

        string = config_get_string(option->path);

        gtk_entry_set_text(GTK_ENTRY(option->button),string);
}

void save_options(GtkWidget *widget)
{
  struct option_info *option;

  for (option = option_info ; option->path != NULL ; option ++){
    switch (option->type){
      case G_TYPE_BOOLEAN:
	button_toggle(option, option->button);
	break;
      case G_TYPE_STRING:
	entry_changed(option, option->button);
	break;
      default:
	break;
    }
  }

  close_options(widget);
}

void close_options(GtkWidget *widget)
{
  struct optags *tag;

  for (tag = tags ; tag->path != NULL ; tag ++)
    tag->box=NULL;
  gtk_widget_destroy(widget);
}
