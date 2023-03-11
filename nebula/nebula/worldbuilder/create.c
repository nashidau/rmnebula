#include <libgnomeui-2.0/gnome.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "libneb.h"
#include "worldbuilder.h"
#include "regions.h"
#include "realms.h"
#include "general.h"

#include "create.h"

#define WB_BASE_GENRE "base/worldbuilder.xml"

/*
 * Genre Create overview
 *
 * Each of the creation functions returns a structure.
 * The structure is declared as:
 * struct create {
 *      GtkWidget *page;
 * }
 *
 * Obviously they can return a larger structure if they wish, so
 * long as the first field is the druid.  The rest is used as 
 * data for the other related callbacks.
 *
 * There are five functions:
 *      a creation function
 *      a callback for when next is clicked
 *              returns TRUE if you _can't_ continue
 *      a callback when finish is clicked
 *      a callback to update (when someone moves onto the page)
 *      a callback when cancel is clicked
 *
 * All the functions except create are optional, depending on usage how sane
 * it is to not have others.
 *
 * Note if cancel is clicked all structs are free'd automatically.
 * 
 * FIXME: This should be extracted and moved into libneb.
 *	This will make usuable in nebula, stardust and other applications
 */

struct create {
        GtkWidget *page;
};

static gboolean create_finish_cb(GnomeDruidPage *,GtkWidget *,gpointer);
static gboolean create_cancel_cb(GnomeDruidPage *, gpointer data);

static struct create *create_first(void);

/*
 * Genre
 * 
 *Basic genre selection - has name and prefix for genre 
 */
struct create_genre {
        GtkWidget *page;
        GtkWidget *name;
};
static struct create *genre_create(void);
static gboolean genre_next(GnomeDruidPage *,GtkWidget *,gpointer);
static void genre_finalise(struct create *);

/*
 * Regions
 *
 * Functions for creating regions, note a lot is very similar to
 * realms
 */
extern struct view_info regionviewinfo[];
extern struct store_info regioninfo[];
extern GtkTreeStore *regionstore;
extern GtkItemFactoryEntry regionmenu[];
struct create_region {
        GtkWidget *page;
        GtkWidget *view;
        GtkWidget *add_child;
        GtkWidget *add_sibling;
        GtkWidget *delete;
};
static struct popupinfo *popupinfo;
static struct treepopupinfo popup_info[] =  {
        { REGION_NAME, POPUP_ENTRY, "Name:", 0,0},
        { REGION_DESCRIPTION, POPUP_TEXTVIEW,"Description", 0,0},
        { -1, 0, NULL, 0 , 0 },
};
static struct create *region_create(void);


/* Realms */
static struct rcmenu_info *realmminfo;
/* Realm funtions */
static struct create *realm_create(void);

/*
 * skills
 *
 */
static struct create *skill_create(void);

/* Finialisation and finish page */
static struct create *last_create(void);


typedef struct create *(*createfn)(void);

struct functions {
        struct create *(*create)(void);
        gboolean (*next)(GnomeDruidPage *,GtkWidget *,gpointer);
        void (*finish)(struct create *);
        void (*prepare)(GnomeDruidPage *,GtkWidget *,gpointer);
        void (*cancel)(struct create *);
        struct create *data;
};

struct functions functions[] = {
        { create_first, NULL, NULL, NULL, NULL, NULL },
        { genre_create, genre_next, genre_finalise, NULL, NULL, NULL },
        { region_create, NULL, NULL, NULL, NULL, NULL },
        { realm_create, NULL, NULL, NULL, NULL, NULL },
        { skill_create, NULL, NULL, NULL, NULL, NULL },
        { last_create, NULL, NULL, NULL, NULL, NULL },
};

extern GtkWidget *window; /* Main window */
extern GdkPixbuf *icon;
extern GdkPixbuf *logo;

GtkWidget *druid;
GtkWidget *druidwindow;

struct fileinfo *
wb_create(void){
        struct fileinfo *finfo;
        unsigned int i;

        druid = gnome_druid_new_with_window(_("Create Genre"),
                        GTK_WINDOW(window),TRUE,&druidwindow);

        /* Create the pages */
        for (i = 0 ; i < G_N_ELEMENTS(functions) ; i ++){
                assert(functions[i].create != NULL);
                functions[i].data = functions[i].create();

                assert(functions[i].data != NULL);
                assert(functions[i].data->page != NULL);
                
                gnome_druid_append_page(GNOME_DRUID(druid),
                                GNOME_DRUID_PAGE(functions[i].data->page));
                if (functions[i].next != NULL){
                        g_signal_connect(G_OBJECT(functions[i].data->page),
                                        "next",
                                        (GtkSignalFunc)functions[i].next,
                                        functions[i].data);
                }
                if (functions[i].prepare != NULL){
                        g_signal_connect(G_OBJECT(functions[i].data->page),
                                        "prepare",
                                        (GtkSignalFunc)functions[i].prepare,
                                        functions[i].data);
                }
		g_signal_connect(G_OBJECT(functions[i].data->page),"cancel",
			(GtkSignalFunc)create_cancel_cb,NULL);
        }

        gtk_widget_show_all(druid);

        finfo = g_malloc0(sizeof(struct fileinfo));
        finfo->state = FILE_MODIFIED;
        finfo->name = NULL;

        return finfo;
}

static gboolean
create_finish_cb(GnomeDruidPage *page, GtkWidget *widget, gpointer data){
        unsigned int i;

        for (i = 0 ; i < G_N_ELEMENTS(functions) ; i ++){
                if (functions[i].finish == NULL)
                        continue;

                functions[i].finish(functions[i].data);
        }

        gtk_widget_destroy(druid);

        return TRUE;
}

static gboolean
create_cancel_cb(GnomeDruidPage *page, gpointer data){
	unsigned int i;

	for (i = 0 ; i < G_N_ELEMENTS(functions) ; i ++){
		if (functions[i].cancel != NULL)
			functions[i].cancel(functions[i].data);

		g_free(functions[i].data);
	}

	gtk_widget_destroy(GTK_WIDGET(druid));

	/* We have no file now */
	file_set_none();
	
	/* Signal handled */
	return TRUE;
}



















static struct create *
create_first(void){
        struct create *create;

        create = g_malloc0(sizeof(struct create));

        create->page = gnome_druid_page_edge_new_with_vals(GNOME_EDGE_START,
                                FALSE /* Anti Aliased */,
                                _("Create Genre"),
                                _("This is where you create a genre"),
                                icon,logo,NULL);

        return create;
}

/*
 * Genre functions
 *
 * For creating the Genre name and prefix 
 */
static struct create *
genre_create(void){
        struct create_genre *create;
        const char *paths;
        char *file = NULL,**strs;
        struct stat st;
        int i;

        /* Load the basic template  */
        /* FIXME: This code is lifted straight from nebula/src/nebfile.c
         *      This should be generalised (slightly improved) */
        paths = config_get_string_default(FILE_OPTION "datapath",CONFIG_PATH);

        strs = g_strsplit(paths,":",-1);
        for (i = 0 ; strs[i] != NULL ; i ++){
                file = g_strconcat(strs[i],"/",WB_BASE_GENRE,NULL);
                if (stat(file,&st) == -1 ||
                                S_ISREG(st.st_mode) == FALSE){
                        g_free(file);
                        file = NULL;
                        continue;
                }
                break;
        }
        g_strfreev(strs);

        if (file == NULL){
                gui_warning_msg("<b>Could not load Base Genre File</b>\n\n"
                                "Check your data path\n");
                return NULL;
        }

        /* Load the base file */ 
        parse_file(file,parsers);
        g_free(file);

        
        /* Okay - now lets start making those pages */
        create = g_malloc0(sizeof(struct create_genre));
        create->page = gnome_druid_page_standard_new_with_vals(
                                _("Genre Choices"),icon,logo);
        create->name = gtk_entry_new();
        gnome_druid_page_standard_append_item(
                                GNOME_DRUID_PAGE_STANDARD(create->page),
                                _("Genre Name:"),create->name,NULL);


        
        return (struct create *)create;
}

static gboolean
genre_next(GnomeDruidPage *page,GtkWidget *button,gpointer userdata){
        struct create_genre *data;
        const char *str;
        
        data = userdata;

        str = gtk_entry_get_text(GTK_ENTRY(data->name));
        assert(str != NULL);

        if (strlen(str) == 0){
		/* Too short */
		/* FIXME: This message bites */
                gui_message(_("You must name your genre\n"));
                return TRUE;
        }
                
        
        return FALSE;
}

static void
genre_finalise(struct create *data){
	struct create_genre *create;
	const char *str;
	
	create = (struct create_genre *)data;

	str = gtk_entry_get_text(GTK_ENTRY(create->name));
	assert(str != NULL);
	assert(strlen(str) > 0);

	wb_set_genre_name(str);	
}

	


/*
 * Create Region and realms 
 */
void region_create_button_cb(GtkButton *button,gpointer data);
static struct rcmenu_info *regminfo;
static struct create *
region_create(void){
        struct create_region *create;
        GtkWidget *box;
        GtkWidget *scrollwin;
        GtkBox *dbox;
        
        
        create = g_malloc0(sizeof(struct create_region));
        create->page = gnome_druid_page_standard_new_with_vals(
                                _("Create Regions"),NULL,NULL);
        dbox = GTK_BOX(GNOME_DRUID_PAGE_STANDARD(create->page)->vbox); 

       
        gtk_box_pack_start(dbox,gtk_label_new(_("Insert your realms")),
                                FALSE,0,0);
        
        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_box_pack_start(dbox,scrollwin,TRUE,TRUE,2);

        /* Create a view for the region */
        create->view = tree_create_view(GTK_TREE_MODEL(regionstore),
                        regionviewinfo);
        gtk_container_add(GTK_CONTAINER(scrollwin),create->view);

        
        box = gtk_hbutton_box_new();
        gtk_box_pack_start(dbox,box,FALSE,TRUE,2);

        /* Now the buttons (FIXME: Should add a Edit button) */
        create->add_child = gtk_button_new_with_label(_("Add Child"));
        create->add_sibling = gtk_button_new_with_label(_("Add Sibling"));
        create->delete = gtk_button_new_from_stock(GTK_STOCK_DELETE);
        gtk_box_pack_start(GTK_BOX(box),create->add_child,TRUE,FALSE,0); 
        gtk_box_pack_start(GTK_BOX(box),create->add_sibling,TRUE,FALSE,0); 
        gtk_box_pack_start(GTK_BOX(box),create->delete,TRUE,FALSE,0); 
        g_signal_connect(G_OBJECT(create->add_child),"clicked",
                        (GtkSignalFunc)region_create_button_cb,
                        GINT_TO_POINTER(RCMENU_NEW_CHILD));
        g_signal_connect(G_OBJECT(create->add_sibling),"clicked",
                        (GtkSignalFunc)region_create_button_cb,
                        GINT_TO_POINTER(RCMENU_NEW_SIBLING));
        g_signal_connect(G_OBJECT(create->delete),"clicked",
                        (GtkSignalFunc)region_create_button_cb,
                        GINT_TO_POINTER(RCMENU_DELETE));



        /* Create a popup window */
        popupinfo = tree_create_popup(popup_info,druidwindow);
        popupinfo->store = regionstore;
        
        regminfo = g_malloc0(sizeof(struct rcmenu_info));
        regminfo->store = regionstore;
        regminfo->view = GTK_TREE_VIEW(create->view);
        regminfo->storeinfo = regioninfo;
        regminfo->item = _("region");
        regminfo->name_field = 0 /* REGION_NAME */;
        regminfo->popupinfo = popupinfo;
        regminfo->maxdepth = -1;
        regminfo->set_defaults = region_new;
        tree_popup_add(create->view,regionmenu,4 /* FIXME */,regminfo);

        return (struct create *)create;
}

/*
 * Provides a button wrapper for the right click menu items 
 */
void
region_create_button_cb(GtkButton *button,gpointer data){
        gint value;
	struct rcmenu_info *minfo;

	value = GPOINTER_TO_INT(data);
	if (value < 0){
		value = -value;
		minfo = realmminfo;
	} else 
		minfo = regminfo;

	
        if (value == RCMENU_DELETE){
                rcmenu_delete_callback(minfo,value,GTK_WIDGET(button));
        } else if (value == RCMENU_NEW_SIBLING || value == RCMENU_NEW_CHILD){
                rcmenu_new_callback(minfo,value,GTK_WIDGET(button));
        } else {
                printf("Unknwn value (%d) from button press\n",value);
        }
}

void
region_finalise(struct create *data){
        g_free(regminfo);
}


/*
 * realm stuff
 *
 * mostly a big clag from region stuff.  This should be fixed if I could be
 * bothered working out what to factorise
 */
extern GtkWidget *realmview;
extern GtkTreeStore *realmstore;
extern struct view_info realmviewinfo[];
extern struct store_info realminfo[];
extern GtkItemFactoryEntry realmmenu[];
static struct treepopupinfo realmpopupinfo[] = {
        { REALM_NAME, POPUP_ENTRY, "Name:", 0,0},
        { REALM_DESCRIPTION, POPUP_TEXTVIEW,"Description", 0,0},
        { -1, 0, NULL, 0 , 0 },
};


static struct create *
realm_create(void){
        struct create_region *create;
        GtkWidget *box;
        GtkWidget *scrollwin;
        GtkWidget *label;
        GtkBox *dbox;
        
        create = g_malloc0(sizeof(struct create_region));
        create->page = gnome_druid_page_standard_new_with_vals(
                        _("Create Realms"),NULL,NULL);

        dbox = GTK_BOX(GNOME_DRUID_PAGE_STANDARD(create->page)->vbox); 

        /* label */
        label = gtk_label_new(_("Insert your realms"));
        gtk_box_pack_start(dbox,label,FALSE,TRUE,2);
        
        /* Add a scrolling window */
        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_box_pack_start(dbox,scrollwin,TRUE,TRUE,2);
        
        /* Create a view for the region */
        create->view = tree_create_view(GTK_TREE_MODEL(realmstore),
                        realmviewinfo);
        gtk_container_add(GTK_CONTAINER(scrollwin),create->view);

        /* Now the action buttons */
        box = gtk_hbutton_box_new();
        gtk_box_pack_start(dbox,box,FALSE,TRUE,2);

        create->add_child = gtk_button_new_with_label(_("Add Child"));
        create->add_sibling = gtk_button_new_with_label(_("Add Sibling"));
        create->delete = gtk_button_new_from_stock(GTK_STOCK_DELETE);
        gtk_box_pack_start(GTK_BOX(box),create->add_child,TRUE,FALSE,0); 
        gtk_box_pack_start(GTK_BOX(box),create->add_sibling,TRUE,FALSE,0); 
        gtk_box_pack_start(GTK_BOX(box),create->delete,TRUE,FALSE,0); 
        g_signal_connect(G_OBJECT(create->add_child),"clicked",
                        (GtkSignalFunc)region_create_button_cb,
                        GINT_TO_POINTER(-  RCMENU_NEW_CHILD));
        g_signal_connect(G_OBJECT(create->add_sibling),"clicked",
                        (GtkSignalFunc)region_create_button_cb,
                        GINT_TO_POINTER(- RCMENU_NEW_SIBLING));
        g_signal_connect(G_OBJECT(create->delete),"clicked",
                        (GtkSignalFunc)region_create_button_cb,
                        GINT_TO_POINTER(- RCMENU_DELETE));



        /* Create a popup window */
	/* FIXME: reusing the region popupinfo is bad mmkay */
        popupinfo = tree_create_popup(realmpopupinfo,druidwindow);
        popupinfo->store = realmstore;
        
        realmminfo = g_malloc0(sizeof(struct rcmenu_info));
        realmminfo->store = realmstore;
        realmminfo->view = GTK_TREE_VIEW(create->view);
        realmminfo->storeinfo = realminfo;
        realmminfo->item = _("realm");
        realmminfo->name_field = 0 /* REGION_NAME */;
        realmminfo->popupinfo = popupinfo;
        realmminfo->maxdepth = -1;
        realmminfo->set_defaults = realms_new;
        tree_popup_add(create->view,realmmenu,4 /* FIXME */,realmminfo);

        return (struct create *)create;
}

/***************************************************************
 * Skills                                                      *
 ***************************************************************/
static struct create *
skill_create(void){
	struct create *create;

	create = g_malloc0(sizeof(struct create));

	create->page = gnome_druid_page_edge_new_with_vals(GNOME_EDGE_FINISH,
                        FALSE /* Anti Aliased */,
                        _("Select Skills"),
                        _("Select which skills are in your campaign"),
                        icon,logo,NULL);
	

	return create;
}
	


/*
 * Creates the last page in the Druid 
 */
static struct create *
last_create(void){
        struct create *create;

        create = g_malloc0(sizeof(struct create));
        
        create->page = gnome_druid_page_edge_new_with_vals(GNOME_EDGE_FINISH,
                        FALSE /* Anti Aliased */,
                        _("Finish Genre"),
                        _("This is the last step in creating a Genre"),
                        icon,logo,NULL);

        g_signal_connect(G_OBJECT(create->page), "finish",
                        (GtkSignalFunc)create_finish_cb,NULL);

        
        return create;
}
