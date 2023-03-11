#include <gtk/gtk.h>
#include <assert.h>
#include "libneb.h"
#include "menu.h"
#include "general.h"
#include "regions.h"

struct store_info regioninfo[] = {
        { REGION_TAG, G_TYPE_STRING, NULL, 0,0, SAVE_ELEMENT },
        { REGION_NAME, G_TYPE_STRING, "name", 0,0, SAVE_ALWAYS },
        { REGION_DESCRIPTION, G_TYPE_STRING, "description", 0, 0,SAVE_DEFINED},
        { -1, 0, NULL, 0, 0, 0 },
};
/* used in  create.c */
struct view_info regionviewinfo[] = {
        { "Regions", REGION_NAME, "text", 0,0,0 },
        { "Description", REGION_DESCRIPTION, "text", 0,0,0 },
        { NULL,-1,NULL,0,0,0 },
};

/* Popup info for realm editing */
struct popupinfo *popupinfo;
struct treepopupinfo popup_info[] = {
        { REGION_NAME, POPUP_ENTRY, "Name:", 0,0},
        { REGION_DESCRIPTION, POPUP_TEXTVIEW,"Description", 0,0},
        { -1, 0, NULL, 0 , 0 },
};

void popup_callback(void); void popup_callback(void){ }
enum { REGION_NEW, REGION_EDIT, REGION_DELETE };

/* The popup (right click) menus */
GtkItemFactoryEntry regionmenu[] = {
        {
                N_("/_New Region..."),NULL,
                (GtkItemFactoryCallback)rcmenu_new_callback,REGION_NEW,
                "<StockItem>", GTK_STOCK_NEW
        },
        {
                N_("/_Edit Region..."), NULL,
                (GtkItemFactoryCallback)rcmenu_edit_callback,REGION_EDIT,
                "<Item>", NULL
        },{
                "/sep1--------------", NULL,
                0, 0,
                "<Separator>",0
        },
	{
                N_("/_Delete Region"), NULL,
                (GtkItemFactoryCallback)rcmenu_delete_callback,REGION_DELETE,
                "<StockItem>", GTK_STOCK_DELETE
        },
};




/* global variables */
static GtkWidget *regionview;
GtkTreeStore *regionstore; /* Used in create */
static struct rcmenu_info *minfo;

static int region_double_click(GtkWidget *,GdkEventButton *,gpointer data);

void
regions_init(GtkNotebook *notebook){
        GtkWidget *scrollwin;
        GtkWidget *frame;
        GtkWidget *label;
        GtkWidget *box;

        frame = gtk_frame_new("Major Regions");
        label = gtk_label_new("Regions");
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
        box = gtk_vbox_new(FALSE,2);
        gtk_container_add(GTK_CONTAINER(frame),box);


        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(box),scrollwin);

        regionstore = make_tree_store(regioninfo);
        regionview = tree_create_view(GTK_TREE_MODEL(regionstore),
                        regionviewinfo);

        g_signal_connect(G_OBJECT(regionview), "button_press_event",
                                (GtkSignalFunc)region_double_click,NULL);

        gtk_container_add(GTK_CONTAINER(scrollwin),regionview);

        /* Create a popup window */
        popupinfo = tree_create_popup(popup_info,window);
        popupinfo->store = regionstore;

        /* Add a right click menu */
        minfo = g_malloc0(sizeof(struct rcmenu_info));
        minfo->store = regionstore;
        minfo->view = GTK_TREE_VIEW(regionview);
        minfo->storeinfo = regioninfo;
        minfo->item = _("region");
        minfo->name_field = REGION_NAME;
        minfo->popupinfo = popupinfo;
        minfo->maxdepth = -1;
        minfo->set_defaults = (void *)region_new; /* FIXME: This is giving me
                                                        bogus type warnings */
        tree_popup_add(regionview,regionmenu,G_N_ELEMENTS(regionmenu),minfo);




}



void
regions_load(element_t *regions){
        GList *elems;
        char *recurse[] = { "region", 0 };
        int primaries[] = { REGION_NAME,-1};

        for (elems = regions->elements ; elems != NULL ; elems = elems->next){
                /* add it to the tree */
                tree_insert(regionstore,regioninfo,primaries,elems->data,
                                recurse,NULL);
        }
}

void
regions_save(element_t *el){
        el = create_element(el,"regions",ETYPE_ELEMENT);

        tree_save(GTK_TREE_MODEL(regionstore),regioninfo,el);
}
void
regions_clear(void){
        gtk_tree_store_clear(regionstore);
}



/*
 * double_click
 *
 * Handle the various click events in the tree view
 */
static int
region_double_click(GtkWidget *widget, GdkEventButton *event, gpointer data){
        gboolean sel;

        if (event->type != GDK_2BUTTON_PRESS){
                return FALSE; /* Do normal handling */
        }

        sel = gtk_tree_selection_get_selected(gtk_tree_view_get_selection
                                        (GTK_TREE_VIEW(regionview)),
                                        NULL, &(popupinfo->iter));
        if (sel == FALSE){
                /* Nothing selected - ignore it */
                return FALSE;
        }

        popupinfo->editable = TRUE;
        tree_set_popup(popupinfo);
        gtk_widget_show_all(popupinfo->window);

        return TRUE;
}


GList *
region_get(void){
        return wb_get_leaves(GTK_TREE_MODEL(regionstore),REGION_NAME,
                        REGION_DESCRIPTION);
}



void
region_new(struct rcmenu_info *minfo,GtkTreeIter *iter){
        gtk_tree_store_set(minfo->store,iter,
                        REGION_TAG, "region",
                        REGION_NAME, "New Region",
                        REGION_DESCRIPTION, "Put your description here",
                        -1);
}
