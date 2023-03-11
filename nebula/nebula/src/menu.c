#include "nebula.h"
#include "nebfile.h"

void level_up(void);
void level_up_times(void);

GtkWidget *the_notebook;
GtkWidget *charpage;
GtkWidget *statpage;
GtkWidget *skillpage;
GtkWidget *defensepage;
GtkWidget *itempage;
GtkWidget *specialpage;


/*
 * FIXME: THis needs to be cleaned up
 */

static GtkItemFactoryEntry menu_items[] = {
        FILE_MENU_ITEM,
        /* Edit Menu */
        {       "/_Edit",       NULL,
                NULL,      0,
                "<Branch>",0
        },
                {
                        "/_Edit/_Stats", NULL,
                        (GtkItemFactoryCallback)stat_edit,0,
                        NULL,0
                },
                /*
                {
                        "/_Edit/S_kills",NULL,
                        (GtkItemFactoryCallback)NULL,0,
                        NULL,0
                 * },*/
                {
                        "/Edit/sep1",             NULL,
                        NULL,     0,
                        "<Separator>",0
                },
                {       "/Edit/_Preferences",     NULL,
                        (GtkItemFactoryCallback)options_creation,     0,
                        "<StockItem>",GTK_STOCK_PREFERENCES
        },

        { "/_Character",           NULL,         NULL,     0,  "<Branch>"    ,0},
        { "/Character/_Level Up",  NULL,   (GtkItemFactoryCallback)level_up,     0,  "<StockItem>"    ,GTK_STOCK_GO_UP },
        { "/Character/_Level Up several times",  NULL,   (GtkItemFactoryCallback)level_up_times,     0,  "<StockItem>"    ,GTK_STOCK_GO_UP },
        { "/Character/sep",        NULL,         NULL,     0,  "<Separator>" ,0},
        { "/Character/_Stat Gain","<control>G", (GtkItemFactoryCallback)stat_gain,     0,  NULL         ,0 },
        { "/Character/_Buy Skills",NULL,   (GtkItemFactoryCallback)skill_buy_dp,     0,  NULL          ,0},
        { "/Character/_Refresh",NULL,  (GtkItemFactoryCallback)refresh_all,     0,  "<StockItem>"       ,GTK_STOCK_REFRESH },
        { "/Character/sep",        NULL,         NULL,     0,  "<Separator>" ,0},
//        { "/Character/Edit Skills",NULL,   (GtkItemFactoryCallback)edit_skills,     0,  "<ToggleItem>"   ,0 },
        { "/_Help",                NULL,         NULL,     0,  "<Branch>",0},
        { "/Help/_About",          NULL,   (GtkItemFactoryCallback)nebula_about,     0,"<StockItem>"        ,GNOME_STOCK_ABOUT},
        { "/Help/sep",             NULL,         NULL,     0, "<Separator>" ,0 },
        { "/Help/_License",        NULL,         NULL,     0, NULL     ,0 },
        { "/Help/_Using",          NULL,         NULL,     0,   NULL,0 },

	{ 
		"/_Test",       NULL,
		NULL,     0, 
		"<Branch>", 0
       	},{
		"/Test/_Create",         "<control>T",
		(GtkItemFactoryCallback)create_begin, 0,
		"<StockItem>",GNOME_STOCK_BOOK_RED
	},
};



GtkWidget *
create_menu(GtkWidget *window,GtkWidget *box){
        GtkWidget *widget;
        GtkAccelGroup *accel_group;
        GtkItemFactory *item_factory;
        int n;

        accel_group = gtk_accel_group_new();
        gtk_window_add_accel_group(GTK_WINDOW(window),accel_group);
        g_object_unref(accel_group);
        item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
                        "<main>",accel_group);
        n = sizeof(menu_items) / sizeof(menu_items[0]);
        gtk_item_factory_create_items(item_factory,n,menu_items,NULL);
        widget = gtk_item_factory_get_widget(item_factory,"<main>");
        gnome_app_set_menus(GNOME_APP(window),GTK_MENU_BAR(widget));

        //gtk_box_pack_start(GTK_BOX(box),widget,FALSE,TRUE,0);

        return widget;
}


void
create_main_page(GtkWidget *box){
        the_notebook = gtk_notebook_new();

        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(the_notebook), GTK_POS_TOP);
        gtk_container_add(GTK_CONTAINER(box), the_notebook);
        gtk_notebook_set_scrollable(GTK_NOTEBOOK(the_notebook), TRUE);

        charpage = add_page("Character","Basic Character Info");
        statpage = add_page("Stats","Stats");
        skillpage = add_page("Skills","Categorys and Skills");
        defensepage = add_page("Defense","Resistance Rolls & DB");
        /* itempage = add_page("Item","Inventory"); */
        /* specialpage = add_page("Special","Miscellaneous"); */

}

GtkWidget *
add_page(char *name,char *description){
        GtkWidget *w1;
        GtkWidget *box;
        GtkWidget *new;

        /* Create the note book page */
        new = gtk_frame_new(description);
        gtk_container_border_width(GTK_CONTAINER(new), 5);
        w1 = gtk_label_new(name);
        gtk_notebook_append_page(GTK_NOTEBOOK(the_notebook), new, w1);

        box = gtk_vbox_new(FALSE,2);
        gtk_container_add(GTK_CONTAINER(new),box);
        gtk_container_border_width(GTK_CONTAINER(box),2);

        return box;
}

void
level_up(void){
        if (file_modify() == FALSE){
                /* FIXME: Print a message */
                return;
        }

        stat_gain();
        skill_buy_dp();
	increase_level(1);
}

void
level_up_times(void){
	int times, i;
	char *filename;

	if (file_modify() == FALSE){
		return;
	}

	times = atoi(gui_ask("Times to repeat:", window));

	/* Ask for the skillset to apply */
	filename = skill_purchase_load();
	
	/* Repeat the level up thing "times" times */
	for (i=0; i<times; i++) {
		stat_gain();
		skill_purchase_skills_times(filename);
	}

	increase_level(times);

	/* Refresh all the stuff */
	skill_update();
	refresh_all();
	
}

void
refresh_all(void){
        if (file_modify() == FALSE){
                /* FIXME: Print a message */
                /* Maybe - not sure */
                return;
        }

        stat_update();
        skill_update();
        rr_update();
        update_items();
        update_special();
}

/*
static void
menu_print_cb(void){
        const char *xslt;

        xslt = config_get_string_default(XSL_OPTION "nebula/postscript",
                        "/home/nash/work/rolemaster/XML/rmsscharacter/ps.xsl");

        print_cb(xslt,nebfile_save);
}
*/
