#include "nebula.h"
#include "character.h"

static found_t get_char(const char *name,GtkTreeIter *result);

void characteristic_add_xml(element_t *data);
void characteristic_add_item(char *prefix,element_t *data);
static void save_characteristic_list(element_t *parent,GtkTreeIter *fogle);

static found_t get_char_subsection(const char *name,GtkTreeIter *start);
/* FIXME: Not used */
void characteristic_add_callback(gpointer,guint action,GtkWidget *);
//static void characteristic_delete_callback(gpointer,guint action,GtkWidget *);
static void characteristic_create_new_cb(struct rcmenu_info *,GtkTreeIter *);


static struct treepopupinfo popup_info[] = {
        { CHAR_NAME, POPUP_ENTRY, "Name: ", 0,0 },
/*        { CHAR_FULLNAME,POPUP_ENTRY, "Fullname: ", 0,0 },*/
        { CHAR_VALUE, POPUP_TEXTVIEW, "Value:", 0, 0 },
        { -1, 0, NULL, 0, 0 }, /* End the list */
};


static GtkItemFactoryEntry charmenu[] = {
        {
                N_("/_New Characteristic..."),NULL,
                (GtkItemFactoryCallback)rcmenu_new_callback, 0,
                "<StockItem>", GTK_STOCK_NEW
        },
        {
                N_("/_Edit Characteristic..."), NULL,
                (GtkItemFactoryCallback)rcmenu_edit_callback, 1,
                "<Item>", NULL
        },
        {
                N_("/sep1"), NULL,
                0, 0,
                "<Separator>",0
        },
	{
                N_("/_Delete Characteristic"), NULL,
                (GtkItemFactoryCallback)rcmenu_delete_callback, 2,
                "<StockItem>", GTK_STOCK_DELETE
        },
};

/*
 * chracteristic_init
 *
 * Creates characterisitic window and store.
 */
void
characteristic_init(GtkWidget *box){
        GtkTreeSelection *select;
        GtkWidget *scrollwin;
        struct rcmenu_info *minfo;
        struct popupinfo *popupinfo;

        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(box),scrollwin);

        charstore = make_tree_store(char_store_info);
        charview = tree_create_view(GTK_TREE_MODEL(charstore),char_view_info);
        g_object_unref(G_OBJECT(charstore));

        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(charview));
        gtk_tree_selection_set_mode(select,GTK_SELECTION_SINGLE);

        gtk_container_add(GTK_CONTAINER(scrollwin),charview);

        /* Create an edit popup item */
        popupinfo = tree_create_popup(popup_info,window);
        popupinfo->store = charstore;

        /* Create a right click menu */
        minfo = g_malloc0(sizeof(struct rcmenu_info));
        minfo->store = charstore;
        minfo->view = GTK_TREE_VIEW(charview);
        minfo->storeinfo = char_store_info;
        minfo->item = _("characteristic");
        minfo->name_field = CHAR_NAME;
        minfo->popupinfo = popupinfo;
        minfo->maxdepth = -1; /* Unlimited children */
        minfo->set_defaults = characteristic_create_new_cb;
        tree_popup_add(charview,charmenu,G_N_ELEMENTS(charmenu),minfo);
}

/*
 * characteristic_set
 *
 * Add a chracteristic to the tree.
 * If value == NULL element is a tree root.
 * Format is <name>/<node>
 *
 * Note: Should value be duplicated?
 */
void
characteristic_set(const char *name,const char *value){
        GtkTreeIter iter;
        GtkTreeIter *parent = NULL;
        char **strs,**part;
        gboolean valid;


        strs = g_strsplit(name,"\\",-1);
        part = strs;

        valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(charstore), &iter);

        while (valid == TRUE && *part != NULL){
                if (strlen(*part) == 0){
                        part ++;
                        continue;
                }
                if (get_char_subsection(*part,&iter) == FOUND){
                        part ++;
                        if (*part == NULL)
                                break;
                        g_free(parent);
                        parent = gtk_tree_iter_copy(&iter);
                        valid = gtk_tree_model_iter_children(
                                        GTK_TREE_MODEL(charstore),&iter,parent);
                } else {
                        valid = FALSE;
                }
        }

        while (*part != NULL){
                gtk_tree_store_append(charstore,&iter,parent);
                gtk_tree_store_set(charstore, &iter,
                                CHAR_NAME, *part,
                                CHAR_VALUE, NULL,
                                CHAR_EDITABLE, TRUE,
                                -1);

                g_free(parent);
                parent = gtk_tree_iter_copy(&iter);
                part ++;
        }

        gtk_tree_store_set(charstore, &iter,
                        CHAR_VALUE, g_strdup(value),
                        -1);
        g_free(parent);
        g_strfreev(strs);

}

char *
characteristic_get(const char *name){
        GtkTreeIter iter;
        char *str;

        if (get_char(name,&iter) == NOTFOUND)
                return NULL;

        gtk_tree_model_get(GTK_TREE_MODEL(charstore),&iter,
                        CHAR_VALUE,&str,-1);
        return str;
}

/*
 *
 */
found_t
get_char(const char *name,GtkTreeIter *result){
        char **strs;
        char **part;
        GtkTreeIter childtmp;
        found_t found = NOTFOUND;

        if (name == NULL)
                return NOTFOUND;


        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(charstore),
                                result) == FALSE){
                // Nothing in the tree
                return NOTFOUND;
        }

        strs = g_strsplit(name,"\\",-1);

        part = strs;
        while (*part != NULL){
                if (strlen(*part) == 0){
                        part ++;
                        continue;
                }
                if (get_char_subsection(*part,result) == NOTFOUND)
                        break;

                /* Found that bit :-) */
                part ++;
                if (*part == NULL){
                        found = FOUND;
                        break;
                }

                if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(charstore),
                                        result) == FALSE)
                        break;

                gtk_tree_model_iter_children(GTK_TREE_MODEL(charstore),
                                &childtmp,result);
                *result = childtmp;
        }

        g_strfreev(strs);
        return found;
}

static found_t
get_char_subsection(const char *name,GtkTreeIter *start){
        char *storename;

        if (start == NULL || name == NULL)
                return NOTFOUND;

        do {
                gtk_tree_model_get(GTK_TREE_MODEL(charstore),start,
                                CHAR_NAME,&storename, -1);
                if (strcasecmp(name,storename) == 0){
                        return FOUND; /* Found it */
                }
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(charstore),start));

        return NOTFOUND;

}


/*
 * characteristics_save
 *
 */
void
characteristics_save(element_t *parent){
        element_t *charroot;
        GtkTreeIter start;
        gboolean flag;

        flag = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(charstore),&start);
        if (flag == FALSE){
                /* Empty tree - can't save */
                printf("can't save\n");
                return;
        }

        charroot = create_element(parent,"data",ETYPE_ELEMENT);


        save_characteristic_list(charroot,&start);
}

static void
save_characteristic_list(element_t *parent,GtkTreeIter *start){
        element_t *el;
        char *name,*val;
        GtkTreeIter children;
        gboolean flag = TRUE;

        while (flag){
                gtk_tree_model_get(GTK_TREE_MODEL(charstore),start,
                                CHAR_NAME,&name,
                                CHAR_VALUE,&val,
                                -1);
                if (gtk_tree_model_iter_children(GTK_TREE_MODEL(charstore),
                                        &children,start)){
                        el = create_element(parent,name,ETYPE_ELEMENT);
                        save_characteristic_list(el,&children);
                } else {
                        create_text_element(parent,name,val);
                }


                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(charstore),
                                start);
        }
}

void
characteristics_clear(void){
        gtk_tree_store_clear(charstore);
}


void characteristic_add_xml(element_t *data){
        GList *elements;
        element_t *el;

        elements = data->elements;
        while (elements != NULL){
                el = elements->data;
                characteristic_add_item(NULL,el);

                elements = g_list_next(elements);
        }
}

void
characteristic_add_item(char *prefix,element_t *data){
        char *tmp;
        GList *elements;
        element_t *el;

        if (prefix != NULL){
                tmp = g_strjoin("\\",prefix,data->name,NULL);
        } else {
                tmp = g_strdup(data->name);
        }

        if (data->type == ETYPE_TEXT || data->type == ETYPE_EMPTY){
                characteristic_set(tmp,data->text);
        }

        elements = data->elements;
        while (elements != NULL){
                el = elements->data;
                characteristic_add_item(tmp,el);

                elements = g_list_next(elements);
        }
        free(tmp);
}

void
characteristic_update(GtkCellRendererText * cell,
            const gchar * path_string,
            const gchar * new_text, gpointer data){
        GtkTreeIter iter;

        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(charstore),
                        &iter,path_string);


        gtk_tree_store_set(charstore,&iter,
                        CHAR_VALUE,strdup(new_text),-1);

        file_modify();
}


/*
 * characteristic_add_callback
 *
 * Someone has clcked 'add' on the popup menu...
 * Now to deal with it...
 *
 * FIXME: Implement me...
 */
void
characteristic_add_callback(gpointer callback_data, guint action,
                GtkWidget * widget){
        GtkTreeIter iter;
        char *name,*value;
        gboolean sel;

        sel = gtk_tree_selection_get_selected(gtk_tree_view_get_selection
					(GTK_TREE_VIEW(charview)),
					NULL, &iter);

        if (sel == FALSE){
                /* Nothing selected */
                return;
        }

        gtk_tree_model_get(GTK_TREE_MODEL(charstore),&iter,CHAR_NAME,&name,
                                           CHAR_VALUE,&value,-1);

        printf("selected is %s=%s\n",name,value);
}

/*
 * characteristic_delete_callback
 *
 * Someone has clicked 'delete' on the popup menu
 * Delete the selected item - if there is one.
 *
 * FIXME: Should popup a warning?  'Are you sure'
 * FIXME: Should really be careful about deleting whole trees...
 */
#if 0
static void
characteristic_delete_callback(gpointer callback_data, guint action,
                GtkWidget * widget){
        GtkTreeIter iter;
        gboolean sel;
        char *name,*msg;

        if (file_modify() == FALSE){
                /* FIXME: Should print a message */
                return;
        }
        sel = gtk_tree_selection_get_selected(gtk_tree_view_get_selection
					(GTK_TREE_VIEW(charview)),
                                        NULL, &iter);
        if (sel == FALSE){
                /* Nothing selected - ignore it */
                return;
        }
        gtk_tree_model_get(GTK_TREE_MODEL(charstore),&iter,
                        CHAR_NAME,&name,-1);
        msg = g_strdup_printf("delete the characteristic %s",name);
        if (gui_confirm_dialog(msg) == TRUE){
                gtk_tree_store_remove(charstore,&iter);
                /* FIXME: I should use something like this:
                tree_delete_item(charstore,skill_store_info,&iter);*/
        }
        g_free(msg);

}
#endif

static void
characteristic_create_new_cb(struct rcmenu_info *minfo,GtkTreeIter *iter){
        
        gtk_tree_store_set(minfo->store,iter,
                        CHAR_NAME, "Characteristic",
                        CHAR_FULLNAME, "Characteristic",    
                        CHAR_VALUE, "",
                        CHAR_EDITABLE, TRUE,
                        -1);
}

/* increase_level
 *
 * Updates the level number to actual+argument, and also updates the
 * experience points to the minimum needed for that level.
 */
void
increase_level(int lvl_inc){
	char *level, *new_xp;
	int lvl;
	long int xp = 0;
	
	/* First, get the current level number */
	level = characteristic_get("experience\\level");
	
	assert(level != NULL);

	lvl = strtol(level,0,0);
	free(level);
	/* Now, update it */
	lvl += lvl_inc;
	level = g_strdup_printf("%d", lvl);
	characteristic_set("experience\\level", level);

	/* Experience: this is calculated in this form:
	 * 	- if lvl <= 5, xp = 10000*lvl
	 * 	- if lvl <= 10, xp = 20000*lvl
	 * 	- if lvl <= 15, xp = 30000*lvl
	 * 	- if lvl <= 20, xp = 40000*lvl
	 * 	- if lvl > 20, xp = 50000*lvl
	 *
	 */
	if (lvl<=5)
		xp = lvl*10000;
	else if (lvl<=10)
		xp = lvl*20000;
	else if (lvl<=15)
		xp = lvl*30000;
	else if (lvl<=20)
		xp = lvl*40000;
	else 
		xp = lvl*50000;

	/* Update the experience points */
	new_xp = g_strdup_printf("%ld", xp);
	characteristic_set("experience\\experiencepoints", new_xp);
	
}	

