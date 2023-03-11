/*
 * stardust
 *
 * A spelllist editor for rolemaster.  Reads XML and writes spelllists
 * compliant with the XML format from:
 *      http://www.nash.nu/Rolemaster/XML/spelllist/
 *
 * Copyiright  Brett Nash <nash@nash.nu> 2002
 */
#include <assert.h>

#include "stardust.h"

/* Main configuration file */
#include "../include/config.h"
#include "../include/tree.h"

#include "tips.h"
#include "options.h"
#include "config.h"
#include "print.h"

GtkWidget *window;
struct popupinfo *popupinfo;
GtkTreeStore *groupstore;
GtkWidget *groupview;

void star_edit_spellitem(GtkCellRendererText *cell,const gchar *path_string,
            const gchar *new_text,gpointer data);
static int load_description(GtkTreeStore *,GtkTreeIter *,int,element_t *el);


struct store_info spell_store_info[] = {
  { SPELL_ELEMENT_NAME, G_TYPE_STRING, "", 0,0, SAVE_ELEMENT ,0,0},
  { SPELL_NAME,   G_TYPE_STRING, "name", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_LEVEL,G_TYPE_INT, "level", 0, 0,SAVE_ALWAYS ,0,0},
  { SPELL_AREAOFEFFECT, G_TYPE_STRING, "areaofeffect", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_DURATION, G_TYPE_STRING, "duration", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_RANGE, G_TYPE_STRING, "range", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_TYPE, G_TYPE_STRING, "type", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_NOPP,G_TYPE_BOOLEAN, "nopp", 0, 0, SAVE_DEFINED ,0,0},
  { SPELL_INSTANTANEOUS,G_TYPE_BOOLEAN, "instantaneous", 0, 0, SAVE_DEFINED ,0,0},
  { SPELL_DESCRIPTION, G_TYPE_STRING, "description", 0, 0, SAVE_ALWAYS, 
          load_description,0/* FIXME: Need a save function */},
  { SPELL_TAGS, G_TYPE_POINTER, "tags", 0, 0, SAVE_NEVER, 0,0 },
  { SPELL_ANNOTATION,  G_TYPE_STRING, "annotation", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_ANNOTATION_TAGS,  G_TYPE_STRING, "annotationtags", 0, 0, SAVE_ALWAYS ,0,0},
  { SPELL_EDIT, G_TYPE_BOOLEAN, NULL, 0,0,SAVE_NEVER ,0,0},
  { -1, 0, NULL, 0, 0, 0,0,0 },
};

struct view_info spell_view_info[] = {
        { "Level",SPELL_LEVEL,"text",0,0,0},
        { "Name",SPELL_NAME,"text",0,SPELL_EDIT,
                        (GtkSignalFunc)star_edit_spellitem},
        { "AoE", SPELL_AREAOFEFFECT,"text",0,SPELL_EDIT,
                        (GtkSignalFunc)star_edit_spellitem},
        { "Duration",SPELL_DURATION,"text",0,SPELL_EDIT,
                        (GtkSignalFunc)star_edit_spellitem},
        { "Range",SPELL_RANGE,"text",0,SPELL_EDIT,
                        (GtkSignalFunc)star_edit_spellitem},
        { "Type",SPELL_TYPE,"text",0,SPELL_EDIT,
                        (GtkSignalFunc)star_edit_spellitem},
        { "PP",SPELL_NOPP,"toggle",0,SPELL_EDIT,0},
        { "*",SPELL_INSTANTANEOUS,"toggle",0,SPELL_EDIT,0},
        { 0,0,0,0,0,0},
};

struct listinfo listinfo[] = {
        {"name", "Spell List:", XML_COMPULSARY, 0 },
        {"listtype", "Type:",   XML_COMPULSARY, 0 },
        {"id",    "List ID:",   XML_OPTIONAL,   0 },
        {"author","Authors:",   XML_MULTIPLE,   0 },
        {"description","Description", XML_LONG, 0 },
        {NULL,NULL,-1,0 },
};

char *popuptypes[] = {
        "E","F","Fm","U","Us","I","E","DE","BE", NULL };

struct treepopupinfo popup_info[] = {
        { SPELL_NAME, POPUP_ENTRY, "Name:", 0, 0 },
        { SPELL_LEVEL, POPUP_DIGIT, "Level", 0, 0 },
        { SPELL_AREAOFEFFECT, POPUP_ENTRY, "Area of effect:", 0, 0 },
        { SPELL_DURATION, POPUP_ENTRY, "Duration:", 0, 0 },
        { SPELL_RANGE, POPUP_ENTRY, "Range:", 0, 0 },
        { SPELL_TYPE, POPUP_COMBO, "Type:", popuptypes, 0 },
        { SPELL_NOPP,POPUP_TOGGLE, "Spell Costs 0 PP", 0, 0 },
        { SPELL_INSTANTANEOUS,POPUP_TOGGLE, "Spell is instantaneous", 0, 0 },
        { SPELL_DESCRIPTION, POPUP_TEXTVIEW, "Description", 0, 0 },
        { SPELL_ANNOTATION,  POPUP_TEXTVIEW, "Annotations", 0, 0 },
        { -1, 0, NULL, 0, 0 }, /* End the list */
};


static void insert_spelllist(element_t *el);
static void insert_spellbook(element_t *el);
static void insert_spellgroup(element_t *el);
static struct fileinfo *star_file_new(void);
static void star_about(void);
static int double_click(GtkWidget *widget,GdkEventButton *event,gpointer data);
static int select_list(GtkTreeSelection *widget,gpointer data);
static const char *get_spelllist_name(struct fileinfo *finfo);


GdkPixbuf *icon;
GdkPixbuf *logo;

struct tags star_parsers[] = {
        { "spellbook", insert_spellbook },
        { "spellgroup BROKEN", insert_spellgroup },
        { "spelllist BROKEN", insert_spelllist },
        { NULL,NULL}
};



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
        { "/_Help",                NULL,         NULL,     0,  "<Branch>",0},
        {
                "/Help/_About",          NULL,
                (GtkItemFactoryCallback)star_about,     0,
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
                XSL_OPTION "stardust/postscript",
                "Postscript XSLT",
                "XSLT to convert to postscript",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "stardust/html",
                "HTML XSLT",
                "XSLT to convert to Hyper-Text Markup Language",
                G_TYPE_STRING, 0,
        },
        {        NULL,NULL,NULL,G_TYPE_NONE,NULL },

};


void init_pixmaps(void);
GtkWidget *spellview;
void star_delete(GtkWidget *widget,GdkEvent *event,gpointer data);

static struct fileops fileops;

int
main(int argc,char **argv){

        gtk_set_locale();
        gnome_program_init(STARDUST_NAME,STARDUST_VERSION,
                        LIBGNOMEUI_MODULE,argc,argv,NULL);

        fileops.docname = g_strdup("spell list");
        fileops.printgconfkey = g_strdup(XSL_OPTION "stardust/postscript");
        fileops.defaultprintkey = g_strdup("/home/nash/work/rolemaster/XML/"
                        "spelllist/spellllisttops.xsl");
        fileops.create = star_file_new;
        fileops.open = star_open_spelllist;
        fileops.save = star_save_spelllist;
        fileops.close = star_close_spelllist;
        fileops.refresh = NULL;
        fileops.getfilename = get_spelllist_name;

        file_set_fileops(&fileops);

        star_create_window();
        init_pixmaps();

        init_tips();
        config_init();
        options_init(options);

        if (argc > 1){
                 file_preload(argv[1]);
        }

        gtk_main();

        exit(EXIT_SUCCESS);
}

void
init_pixmaps(void){
        GError *err = NULL;

        icon = gdk_pixbuf_new_from_file(STARDUST_ICON,&err);
        if (icon == NULL)
                fprintf(stderr,"Warning: No app icon\n");
        else
                gtk_window_set_icon(GTK_WINDOW(window),icon);

        if (err != NULL){
                fprintf(stderr, "Unable to read file: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

        err = NULL;
        logo = gdk_pixbuf_new_from_file(STARDUST_LOGO,&err);

        if (err != NULL){
                fprintf(stderr, "Unable to read file: %s\n", err->message);
                g_error_free(err);
                logo = NULL;
        }
}

static void
star_create_window(void){
        GtkWidget *mainbox;
        GtkWidget *scrollwin,*swin2;
        GtkWidget *widget;
        GtkWidget *info;
        GtkWidget *paned;
        GtkWidget *frame2,*frame1;
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        GtkTreeSelection *sel;
        GtkAccelGroup *accel_group;
        GtkItemFactory *item_factory;
        int n;
        struct choice *choice;

        window = gnome_app_new(STARDUST_NAME,STARDUST_NAME);
        gtk_window_set_default_size(GTK_WINDOW(window),600,470);
        gtk_window_set_title(GTK_WINDOW(window),STARDUST_NAME);


        /* Add the menu */
        accel_group = gtk_accel_group_new();
        gtk_window_add_accel_group(GTK_WINDOW(window),accel_group);
        g_object_unref(accel_group);
        item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
                        "<main>",accel_group);
        n = sizeof(menu_items) / sizeof(menu_items[0]);
        gtk_item_factory_create_items(item_factory,n,menu_items,NULL);
        widget = gtk_item_factory_get_widget(item_factory,"<main>");
        gnome_app_set_menus(GNOME_APP(window),GTK_MENU_BAR(widget));

        /* The toolbar should go here */

        /* Make a panel */
        paned = gtk_hpaned_new();
        frame1 = gtk_frame_new(NULL);
        frame2 = gtk_frame_new(NULL);
        mainbox = gtk_vbox_new(FALSE,2);
        gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);
        gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
        gtk_paned_pack1(GTK_PANED(paned), frame1, FALSE, FALSE);
        gtk_paned_pack2(GTK_PANED(paned), frame2, TRUE, FALSE);
        gtk_container_add(GTK_CONTAINER(frame2),mainbox);
        gnome_app_set_contents(GNOME_APP(window),paned);

        /* Add the left panel */
        swin2 = gtk_scrolled_window_new(NULL,NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin2),
                         GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
        groupstore = gtk_tree_store_new(2,GTK_TYPE_STRING,G_TYPE_POINTER);
        groupview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(groupstore)); 
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Group",
                                                   renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(groupview), column);
        gtk_container_add(GTK_CONTAINER(frame1),swin2);
        gtk_container_add(GTK_CONTAINER(swin2),groupview);
        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(groupview));
        g_signal_connect(G_OBJECT(sel),"changed",
                        (GtkSignalFunc)select_list,NULL);


        
        /* Add the spell list summary */
        info = tree_make_listinfo(listinfo);
        gtk_box_pack_start(GTK_BOX(mainbox),info,FALSE,0,0);


        /* Add the spell list box */
        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_box_pack_start(GTK_BOX(mainbox),scrollwin,TRUE,TRUE,0);

        choice = g_malloc0(sizeof(struct choice));
        choice->min = -1;
        choice->max = -1;
        choice->count = 0;
        choice->store = NULL;

        spellview =tree_create_view_data(NULL,spell_view_info,choice);
        g_signal_connect(G_OBJECT(spellview), "button_press_event",
                        (GtkSignalFunc)double_click,NULL);

        /* Create a popup window */
        popupinfo = tree_create_popup(popup_info,window);
        popupinfo->store = NULL;

        gtk_container_add(GTK_CONTAINER(scrollwin),spellview);

        /* Attach an exit to destroy window */
        g_signal_connect(G_OBJECT(window),"delete_event",
                        G_CALLBACK(star_delete),NULL);

        gtk_widget_show_all(window);


}

/*
 * Star file new
 *
 * Creates a new file by loading
 * $(BASE)/stardust.xml
 */
static struct fileinfo *
star_file_new(void){
        struct fileinfo *finfo;

        parse_file("../data/base/stardust.xml",star_parsers);

        finfo = g_malloc(sizeof(struct fileinfo));
        finfo->name = NULL;
        finfo->state = FILE_CLEAN;

        return finfo;
}

/*
 * Callback for file open menu item
 */
struct fileinfo *
star_open_spelllist(const char *name){
        struct fileinfo *finfo;

        parse_file(name,star_parsers);

        finfo = g_malloc(sizeof(struct fileinfo));
        finfo->name = g_strdup(name);
        finfo->state = FILE_CLEAN;

        return finfo;
}
int
star_close_spelllist(void){
        /* FIXME */
        printf("not implemented\n");
/*        gtk_tree_store_clear(spellstore);

        tree_listinfo_clear(listinfo);
*/
        return 0;
}
void
star_save_spelllist(struct fileinfo *finfo){
        /* FIXME */
        printf("not implemented\n");
  /*
        element_t *root;

        root = create_element(NULL,"spelllist",ETYPE_ELEMENT);

        tree_listinfo_save(listinfo,root);

        tree_save(GTK_TREE_MODEL(spellstore),spell_store_info,root);
        alena_save(finfo->name,root,SPELLLISTDTD);
        */
}

static GtkTreeIter cgroup;


void
insert_spelllist(element_t *all){
        int spellprimaries[] = { SPELL_NAME , -1 };
        GList *elems;
        element_t *el;
        const char *name = "Unknown";
        GtkTreeIter iter;
        GtkTreeStore *store;

        elems = all->elements;
        for (elems = all->elements ; elems != NULL ; elems = elems->next){
                el = elems->data;
                if (strcmp(el->name,"name") == 0){
                        name = el->text;
                        break;
                } 
        }

        store = make_tree_store(spell_store_info);
        for (elems = all->elements ; elems != NULL ; elems = elems->next){
                el = elems->data;
                if (strcmp(el->name,"spell") == 0){
                        tree_insert(store,spell_store_info,
                                        spellprimaries,el,NULL,NULL);
                } else {
                        //tree_listinfo_insert(listinfo,el);
                        //insert_other(el);
                }
        }

        tree_set_flag(store,NULL,SPELL_EDIT,TRUE);
        
        
        gtk_tree_store_append(groupstore,&iter,&cgroup);
        gtk_tree_store_set(groupstore,&iter,0,name,1,store,-1);
        
}

static void 
insert_spellbook(element_t *root){
        GList *list;
        static GtkTreeStore *storetmp;

        if (storetmp == NULL)
                storetmp = gtk_tree_store_new(2,GTK_TYPE_STRING,G_TYPE_POINTER);
        gtk_tree_view_set_model(GTK_TREE_VIEW(groupview),
                        GTK_TREE_MODEL(storetmp));
        
        for (list = root->elements ; list != NULL ; list = list->next){
                insert_spellgroup(list->data);
        }

        gtk_tree_view_set_model(GTK_TREE_VIEW(groupview),
                        GTK_TREE_MODEL(groupstore));
        

}
static void 
insert_spellgroup(element_t *el){
        const char *realm = "unknown",*type = "unknown",*prof = NULL ;
        char *name;
        GList *list;
        attribute_t *attr;

        for (list = el->attributes ; list != NULL ; list = list->next){
                attr = list->data;
                if (strcmp(attr->name,"realm") == 0){
                        realm = attr->value;
                } else if (strcmp(attr->name,"type") == 0){
                        type = attr->value;
                } else if (strcmp(attr->name,"ref") == 0){
                        prof = attr->value;
                }
        }
        if (prof != NULL)
                name = g_strdup_printf("%s base",prof);
        else 
                name = g_strdup_printf("%s %s",type,realm);
        gtk_tree_store_append(groupstore,&cgroup,NULL);
        gtk_tree_store_set(groupstore,&cgroup,0,name,1,NULL,-1);

        for (list = el->elements ; list != NULL ; list = list->next){
                insert_spelllist(list->data);
        }
        
}


void
star_about(void){
        static GtkWidget *about;
        const char *copyright = "2002,2003 Brett Nash";
        const char *authors[] = {
                "Brett Nash <nash@nash.nu>",
                NULL };
        const char *comments = NULL;
        const char *documentation = NULL;
        const char *translation = NULL;


        if (about == NULL){
                about = gnome_about_new(STARDUST_NAME, STARDUST_VERSION,
                                copyright, comments,
                                (const char **)authors,
                                (const char **)documentation,
                                translation, logo);
                gtk_signal_connect (GTK_OBJECT (about), "destroy",
                                    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
                                    &about);
        }

        gtk_window_set_transient_for(GTK_WINDOW(about),GTK_WINDOW(window));
        gtk_widget_show_now(about);
        gtk_widget_grab_focus(about);
}

void
star_edit_spellitem(GtkCellRendererText *cell,const gchar *path_string,
            const gchar * new_text, gpointer data){
        GtkTreeIter iter;
        GtkTreeModel *model;
        int col;

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(spellview));
        gtk_tree_model_get_iter_from_string(model,&iter,path_string);

        col = (int)g_object_get_data(G_OBJECT(cell),"column");

        gtk_tree_store_set(GTK_TREE_STORE(model),&iter,col,strdup(new_text),-1);

        file_modify();
}

static int
double_click(GtkWidget *widget, GdkEventButton *event, gpointer data){
        gboolean sel;
        GtkTreeModel *model;

        if (event->type != GDK_2BUTTON_PRESS){
                return FALSE; /* Do normal handling */
        }

        sel = gtk_tree_selection_get_selected(gtk_tree_view_get_selection
                                        (GTK_TREE_VIEW(spellview)),
                                        &model, &(popupinfo->iter));
        if (sel == FALSE){
                /* Nothing selected - ignore it */
                return FALSE;
        }

        popupinfo->editable = TRUE;
        popupinfo->store = GTK_TREE_STORE(model);
        tree_set_popup(popupinfo);
        gtk_widget_show_all(popupinfo->window);

        /* We have handled it */
        return TRUE;
}



void
star_delete(GtkWidget *widget,GdkEvent *event,gpointer data){
        /* FIXME: Should prompt to save any modified files etc.. */
        gtk_exit(0);
}


static const char *
get_spelllist_name(struct fileinfo *finfo){
        const char *name = NULL;
        struct listinfo *linfo;

        assert(finfo != NULL);

        for (linfo = listinfo ; linfo->xmltag != NULL ; linfo ++){
                if (strcmp("name",linfo->xmltag) == 0){
                        name = gtk_entry_get_text(GTK_ENTRY(linfo->widget));
                        if (name != NULL && strlen(name) > 0)
                                return name;
                }
        }

        name = strrchr(finfo->name,'/');
        if (name != NULL && strlen(name) > 1){
                name ++;
                return name;
        }

        return "Un-named";
}

static int
select_list(GtkTreeSelection *selection, gpointer data){
        gboolean sel;
        GtkTreeIter iter;
        GtkTreeStore *store;
        
        sel = gtk_tree_selection_get_selected(selection, NULL, &iter);

        if (sel == FALSE){
                /* Nothing selected - ignore it */
                return FALSE;
        }

        gtk_tree_model_get(GTK_TREE_MODEL(groupstore),&iter,1,&store,-1);
        gtk_tree_view_set_model(GTK_TREE_VIEW(spellview),GTK_TREE_MODEL(store));
        
        /* Pass it on so the selection is set */
        return FALSE;
}


/* 
 * New loading functions for descriptions
 *
 * Used to handle references in a semi-useful manner
 */

/*
 * load_description
 *
 * Used to load a spell descrioption -
 *      If there is a <ref> tag, the contents are italicised
 * These should become a link enventually
 *
 * returns
 *      0 on success
 *      -1 on error
 */
static int
load_description(GtkTreeStore *store,GtkTreeIter *iter,int col,element_t *root){
        char *str,*tmp;
        element_t *el;
        GList *list;
        struct taginfo *taginfo;
        GList *tags = NULL;
        

        if (root == NULL){
                return -1;
        }
        
        if (root->elements == NULL){
                gtk_tree_store_set(store,iter,col,g_strdup(root->text),-1);
                return 0;
        }

        if (root->text == NULL){
                return 0;
        } 
        str = g_strdup(root->text);
        for (list = root->elements ; list != NULL ; list = list->next){
                el = list->data;
                if (el->name != NULL && strcmp(el->name,"ref") == 0){
                        taginfo = calloc(1,sizeof(struct taginfo));
                        /* FIXME: This should be characters not bytes */
                        taginfo->start = strlen(str);
                        taginfo->end = strlen(str) + strlen(el->text);
                        taginfo->tag = gtk_text_tag_new(NULL);
                        g_object_set(G_OBJECT(taginfo->tag),
                                        "scale",2,NULL);
                        tags = g_list_append(tags,taginfo);
                        tmp = g_strconcat(str,el->text,NULL);
                } else {
                        tmp = g_strconcat(str,el->text,NULL);
                }
                g_free(str);
                str = tmp;
        }
        gtk_tree_store_set(store,iter,col,str,-1);
        gtk_tree_store_set(store,iter,col+1,tags,-1);

        return 0;
}

