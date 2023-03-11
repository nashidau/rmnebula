/*
 * Realms.c
 *
 * For Worldbuilder, controls the various magical realms for world builder
 */
#include <gtk/gtk.h>
#include <assert.h>

#include "libneb.h"
#include "menu.h"
#include "general.h"

#include "realms.h"

GtkWidget *realmview;
GtkTreeStore *realmstore;


struct store_info realminfo[] = {
        { REALM_TAG, G_TYPE_STRING, NULL, 0,0, SAVE_ELEMENT },
        { REALM_NAME, G_TYPE_STRING, "name", 0,0, SAVE_ALWAYS },
        { REALM_DESCRIPTION, G_TYPE_STRING, "description", 0, 0,SAVE_DEFINED},
        { -1, 0, NULL, 0, 0, 0 },
};

struct view_info realmviewinfo[] = {
        { "Realm", REALM_NAME, "text", 0,0,0 },
        { "Description", REALM_DESCRIPTION, "text", 0,0,0 },
        { NULL,-1,NULL,0,0,0 },
};

/* Popup info for realm editing */
static struct popupinfo *popupinfo;
static struct treepopupinfo popup_info[] = {
        { REALM_NAME, POPUP_ENTRY, "Name:", 0,0},
        { REALM_DESCRIPTION, POPUP_TEXTVIEW,"Description", 0,0},
        { -1, 0, NULL, 0 , 0 },
};

enum { REGION_NEW, REGION_EDIT, REGION_DELETE };

/* The popup (right click) menus */
GtkItemFactoryEntry realmmenu[] = {
        {
                N_("/_New Realm..."),NULL,
                (GtkItemFactoryCallback)rcmenu_new_callback,REGION_NEW,
                "<StockItem>", GTK_STOCK_NEW
        },
        {
                N_("/_Edit Realm..."), NULL,
                (GtkItemFactoryCallback)rcmenu_edit_callback,REGION_EDIT,
                "<Item>", NULL
        },{
                "/sep1--------------", NULL,
                0, 0,
                "<Separator>",0
        },
	{
                N_("/_Delete Realm"), NULL,
                (GtkItemFactoryCallback)rcmenu_delete_callback,REGION_DELETE,
                "<StockItem>", GTK_STOCK_DELETE
        },
};



static int realm_double_click(GtkWidget *wid,GdkEventButton *ev,gpointer data);
static GList * religions_get_internal(GList *list,GtkTreeIter *parent);

void
wb_realms_init(GtkNotebook *notebook){
	GtkWidget *realmbox;
        GtkWidget *scrollwin;
        GtkWidget *frame;
        GtkWidget *label;
        static struct rcmenu_info *minfo;

        frame = gtk_frame_new("Magical Realms");
        label = gtk_label_new("Realms");
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
        realmbox = gtk_vbox_new(FALSE,2);
        gtk_container_add(GTK_CONTAINER(frame),realmbox);


        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(realmbox),scrollwin);

        realmstore = make_tree_store(realminfo);
        realmview = tree_create_view(GTK_TREE_MODEL(realmstore),realmviewinfo);

        g_signal_connect(G_OBJECT(realmview), "button_press_event",
                                (GtkSignalFunc)realm_double_click,NULL);

        gtk_container_add(GTK_CONTAINER(scrollwin),realmview);

        /* Create a popup window */
        popupinfo = tree_create_popup(popup_info,window);
        popupinfo->store = realmstore;

        /* Create a right click menu */
        minfo = g_malloc0(sizeof(struct rcmenu_info));
        minfo->store = realmstore;
        minfo->view = GTK_TREE_VIEW(realmview);
        minfo->storeinfo = realminfo;
        minfo->item = _("realm");
        minfo->name_field = REALM_NAME;
        minfo->popupinfo = popupinfo;
        minfo->maxdepth = -1; 
        minfo->set_defaults = realms_new;
        tree_popup_add(realmview,realmmenu,G_N_ELEMENTS(realmmenu),minfo);

}


void
realms_load(element_t *realms){
        GList *elems;
        element_t *realm;
        char *recurse[] = { "realm", 0 };
        int primaries[] = { REALM_NAME,-1};

        for (elems = realms->elements ; elems != NULL ; elems = elems->next){
                realm = elems->data;
                /* add it to the tree */
                tree_insert(realmstore,realminfo,primaries,realm,recurse,NULL);
        }
}

void
realms_save(element_t *el){
        el = create_element(el,"realms",ETYPE_ELEMENT);

        tree_save(GTK_TREE_MODEL(realmstore),realminfo,el);
}
void
realms_clear(void){
        gtk_tree_store_clear(realmstore);
}



/*
 * double_click
 *
 * Handle the various click events in the tree view
 */
static int
realm_double_click(GtkWidget *widget, GdkEventButton *event, gpointer data){
        gboolean sel;

        if (event->type != GDK_2BUTTON_PRESS){
                return FALSE; /* Do normal handling */
        }

        sel = gtk_tree_selection_get_selected(gtk_tree_view_get_selection
                                        (GTK_TREE_VIEW(realmview)),
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
realms_get(void){
        return wb_get_leaves(GTK_TREE_MODEL(realmstore),REALM_NAME,
                        REALM_DESCRIPTION);
}

GList *
religions_get(void){
        GtkTreeIter root;
        gboolean valid;
        char *str;
        GList *list;

        valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(realmstore),&root);
        while (valid){
                gtk_tree_model_get(GTK_TREE_MODEL(realmstore),&root,
                                REALM_NAME,&str, -1);

                if (strcmp(str,"Channeling") == 0){
                        break;
                }
                
                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(realmstore),&root);
        }

        if (valid == FALSE){
                /* FIXME: This message should be improved */
                gui_warning_msg("No 'Channeling', hence no religions!");
                return NULL;
        }

        if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(realmstore),&root) 
                        == FALSE){
                gui_warning_msg("No children to Channeling!");
                return NULL;
        }

        list = religions_get_internal(NULL,&root);        
        list = g_list_reverse(list);

        return list;        
}

static GList *
religions_get_internal(GList *list,GtkTreeIter *parent){
        GtkTreeIter node;
        struct leaf *leaf;
        char *name,*desc;
        gboolean valid;

        assert(parent != NULL);

        valid =gtk_tree_model_iter_children(GTK_TREE_MODEL(realmstore),&node,
                        parent);

        while (valid){
                gtk_tree_model_get(GTK_TREE_MODEL(realmstore),&node,
                                REALM_NAME,&name,
                                REALM_DESCRIPTION,&desc,
                                -1);
                leaf = g_malloc(sizeof(struct leaf));
                leaf->name = name;
                leaf->description = desc;

                list = g_list_prepend(list,leaf);

                if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(realmstore),
                                        &node) == TRUE){
                        list = religions_get_internal(list,&node);
                }
                        
                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(realmstore),
                                &node);
        }


        return list; 
}

/*
 * Callback on creating a new realm
 */
void
realms_new(struct rcmenu_info *minfo,GtkTreeIter *iter){
        gtk_tree_store_set(minfo->store,iter,
                        REALM_TAG, "realm",
                        REALM_NAME, "New Realm",
                        REALM_DESCRIPTION, "Put your description here",
                        -1);
}
