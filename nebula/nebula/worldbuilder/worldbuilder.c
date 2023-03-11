/*
 * WorldBuilder
 *
 * Genre File editor, and creator
 */
#include <gtk/gtk.h>
#include <stdlib.h>
#include <gnome.h>
#include <assert.h>
#include "libneb.h"
#include "config.h"
#include "worldbuilder.h"
#include "general.h"
#include "realms.h"
#include "skills.h"
#include "regions.h"
#include "create.h"

#ifndef PIXMAPS_PATH
#error No PIXMAPS_PATH defined
#endif
#ifndef WB_LOGO 
#error No WB_LOGO defined
#endif

/* Functions */
static void wb_create_window(void);
static void wb_about(void);
static struct fileinfo * wb_open(const char *file);
static int wb_close(void);
static void wb_save(struct fileinfo *finfo);
static void wb_refresh(void);

/* temp */
void wb_general_init(GtkNotebook *notebook);
void wb_realms_init(GtkNotebook *notebook);

struct tags parsers[] = {
        { "rmssgenre", genre_load },
        { "skills",    skill_add_xml },
        { "regions",   regions_load },
        { "realms",    realms_load },
        { NULL,NULL}
};



/* Variables */
GtkWidget *window;

GdkPixbuf *icon;
GdkPixbuf *logo;


static GtkItemFactoryEntry menu_items[] = {
        FILE_MENU_ITEM,
        /* Edit Menu */
        {       "/_Edit",       NULL,
                NULL,      0,
                "<Branch>",0
        },
                {       "/Edit/_Preferences",     NULL,
                        (GtkItemFactoryCallback)options_creation,     0,
                        "<StockItem>",GTK_STOCK_PREFERENCES
        },
                { "/Edit/_Refresh",NULL,  (GtkItemFactoryCallback)wb_refresh, 
                        0,  "<StockItem>"       ,GTK_STOCK_REFRESH },
        { "/_Help",                NULL,         NULL,     0,  "<Branch>",0},
        {
                "/Help/_About",          NULL,
                (GtkItemFactoryCallback)wb_about,     0,
                "<StockItem>"        ,GNOME_STOCK_ABOUT
        },
};


struct option_info options[] = {
        {
                XSL_OPTION "translator",
                "XSLT Processor",
                "Path to the XSLT processor\n"
                        "/usr/bin/xalan",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "worldbuilder/postscript",
                "Postscript XSLT",
                "XSLT to convert to postscript",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "worldbuilder/html",
                "HTML XSLT",
                "XSLT to convert to Hyper-Text Markup Language",
                G_TYPE_STRING, 0,
        },
        {        NULL,NULL,NULL,G_TYPE_NONE,NULL },

};

struct fileops fileops;


int
main(int argc,char **argv){

        fileops.docname = g_strdup("genre");
        fileops.printgconfkey = g_strdup(XSL_OPTION "worldbuilder/postscript");
        fileops.defaultprintkey = g_strdup("/home/nash/work/rolemaster/XML/"
                        "genre/genretops.xsl");
        fileops.create = wb_create;
        fileops.open = wb_open;
        fileops.save = wb_save;
        fileops.close = wb_close;
        fileops.refresh = wb_refresh;
        fileops.getfilename = wb_get_genre_name;




        libneb_init("World Builder","0.1",argc,argv,&fileops,options);

        libneb_icons_init(WB_LOGO,&logo,
                        WB_ICON,&icon,NULL);

        wb_create_window();

        if (icon != NULL){
                gtk_window_set_icon(GTK_WINDOW(window),icon);
        }

        if (argc > 1){
                 file_preload(argv[1]);
        }

        gtk_main();

        exit(EXIT_SUCCESS);
}


static void
wb_create_window(void){
        GtkAccelGroup *accel_group;
        GtkNotebook *notebook;
        GtkWidget *widget;
        GtkWidget *mainbox;
        int n;
        GtkItemFactory *item_factory;


        /* Make a window */
        window = gnome_app_new("World Builder","0.1");
        gtk_window_set_default_size(GTK_WINDOW(window),450,470);
        gtk_window_set_title(GTK_WINDOW(window),"World Builder");

        /* Make a menu */
        accel_group = gtk_accel_group_new();
        gtk_window_add_accel_group(GTK_WINDOW(window),accel_group);
        g_object_unref(accel_group);
        item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
                        "<main>",accel_group);
        n = sizeof(menu_items) / sizeof(menu_items[0]);
        gtk_item_factory_create_items(item_factory,n,menu_items,NULL);
        widget = gtk_item_factory_get_widget(item_factory,"<main>");
        gnome_app_set_menus(GNOME_APP(window),GTK_MENU_BAR(widget));

        /* Notebook pages */
        mainbox = gtk_vbox_new(FALSE,2);
        gnome_app_set_contents(GNOME_APP(window),mainbox);
        notebook = GTK_NOTEBOOK(gtk_notebook_new());

        /* Create the tabs */
        wb_general_init(notebook);
        regions_init(notebook);
        wb_realms_init(notebook);
        wb_skills_init(notebook);
        gtk_container_add(GTK_CONTAINER(mainbox), GTK_WIDGET(notebook));

        gtk_widget_show_all(window);

        return;
}


static void
wb_about(void){
        static GtkWidget *about;
        const char *copyright = "Copyright 2002 Brett Nash";
        const char *authors[] = {
                "Brett Nash <nash@nash.nu>",
                NULL };
        const char *comments = NULL;
        const char *documentation = NULL;
        const char *translation = NULL;


        if (about == NULL){
                about = gnome_about_new(WORLDBUILDER_NAME,WORLDBUILDER_VERSION,
                                copyright, comments,
                                (const char **)authors,
                                (const char **)documentation,
                                translation, logo);
                gtk_signal_connect(GTK_OBJECT(about),"destroy",
                                    GTK_SIGNAL_FUNC(gtk_widget_destroyed),
                                    &about);
        }

        gtk_window_set_transient_for(GTK_WINDOW(about),GTK_WINDOW(window));
        gtk_widget_show_now(about);

}


/*
 * File Operations
 */

static struct fileinfo *
wb_open(const char *file){
        struct fileinfo *finfo;

        parse_file(file,parsers);

        finfo = g_malloc(sizeof(struct fileinfo));
        finfo->name = g_strdup(file);
        finfo->state = FILE_CLEAN;

        return finfo;
}

static int
wb_close(void){

        tree_listinfo_clear(listinfo);

        realms_clear();
        regions_clear();
        skills_clear();

        return 0;
}


static void
wb_save(struct fileinfo *finfo){
        element_t *el;

        assert(finfo != NULL);
        assert(finfo->name != NULL);

        el = create_element(NULL,"rmssgenre",ETYPE_ELEMENT);
        set_attribute(el,"id","worldbuilder-001");
        set_attribute(el,"version",WORLDBUILDER_VERSION);

        tree_listinfo_save(listinfo,el);

        realms_save(el);
        regions_save(el);
        skills_save(el);

        alena_save(finfo->name,el,NULL);

}

static void
wb_refresh(void){
        GList *regions,*religions;
        GList *tmp;

        /* Put in all the known regions */ 
        regions = region_get();
        skills_insert_list("Lore \xE2\x80\xA2 General\\Region Lore (By Region)",
                        regions);
        skills_insert_list("Lore \xE2\x80\xA2 General\\History (By Region/Group/Planet/Society/City)",
                        regions);

        religions = religions_get();
        skills_insert_list("Lore \xE2\x80\xA2 General\\Religion "
                        "(By Pantheon/Religion)",religions);

        /* Clean up */
        for (tmp = regions ; tmp != NULL ; tmp = tmp->next)
                g_free(tmp->data);
        g_list_free(regions);

        for (tmp = religions ; tmp != NULL ; tmp = tmp->next)
                g_free(tmp->data);
        g_list_free(religions);



}


void
genre_load(element_t *all){
        GList *elems;
        element_t *el;

        /* Lets load the basic bits first */
        elems = all->elements;
        for (elems = all->elements ; elems != NULL ; elems = elems->next){
                el = elems->data;
                tree_listinfo_insert(listinfo,el);
        }
}


