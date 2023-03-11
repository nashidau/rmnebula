#include <gtk/gtk.h>
#include <assert.h>
#include <ctype.h>

#include "libneb.h"

GtkTreeViewColumn *get_text_column(struct view_info *vinfo,
                GtkTreeModel *store,gpointer data);
GtkTreeViewColumn *get_toggle_column(struct view_info *vinfo,gpointer data);

static char *get_primary_xmlname(struct store_info *sinfo,int primary);

static void tree_refresh_level(struct store_info *sinfo,GtkTreeStore *store,
		GtkTreeIter *start);

static void node_to_xml_tree(GtkTreeModel *store,struct store_info *sinfo,
                element_t *node,GtkTreeIter *iterp);

static GType *make_type_list(struct store_info *sinfo,int *count);
static void entry_zero(GtkTreeStore *,struct store_info *,GtkTreeIter *);
static void item_toggled(GtkCellRendererToggle *cell,gchar *path_str,gpointer data);
static gint tree_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void tree_save_internal(GtkTreeModel *store,struct store_info *sinfo,
                element_t *el,GtkTreeIter *iter);
static void tree_popup_button_cb(GtkWidget *button,gpointer data);

static void tree_listinfo_save_node(struct listinfo *listinfo,element_t *el);

/*
 * make_tree_store
 *
 * Allocates a new store from the store_info provided.
 *
 * Args:
 *      sinfo describing the store
 * Returns:
 *      Newly allocated GtkTreeStore*
 */
GtkTreeStore *
make_tree_store(struct store_info *sinfo){
        GtkTreeStore *store;
        GType *types;
        int count;

        types = make_type_list(sinfo,&count);

        /* Create the tree */
        store = gtk_tree_store_newv(count,types);
        if (store == NULL)
                perror("can't make store");

        g_free(types);

        return store;
}

/*
 * make_type_list
 *
 * Helper function to create a list of type for creating a tree store
 * (could be used for other things, but nothing needs it yet :-)
 *
 * Args:
 *      struct store_info *sinfo to parse for types
 *      int *count of types
 * Returns:
 *      Gtype *array which must be freed by caller (using g_free)
 */
static GType *
make_type_list(struct store_info *sinfo,int *count){
        int i,cols = 0;
        GType *types;

        /* How many elements do we have ? */
        for (i = 0 ; sinfo[i].pos != -1 ; i ++){
                if (sinfo[i].pos > cols)
                        cols = sinfo[i].pos;
        }
        cols ++; /* Fix  ordinal -> cardinality */

        /* Allocate that */
        types =  g_malloc(cols * sizeof(GType));

        /* Preset everything with an invalid type */
        for (i = 0 ; i < cols ; i ++)
                types[i] = G_TYPE_NONE;

        /* Copy the correct types in */
        while (sinfo->pos != -1){
                types[sinfo->pos] = sinfo->datatype;
                sinfo ++;
        }

        if (count != NULL)
                *count = cols;

        return types;
}

/*
 * tree_create_view
 *
 * As tree_create_view_data with data == NULL
 *
 */
GtkWidget *
tree_create_view(GtkTreeModel *store,struct view_info *view_info){
        return tree_create_view_data(store,view_info,NULL);
}

/*
 * tree_create_view_data
 *
 * Creates a new GtkTreeView for the given store with columns
 * generated from the given view
 *
 * Args:
 *      store: Store to give store
 *      view_info: Infomation about the view
 *      data: Data to associate with callbacks
 */
GtkWidget *
tree_create_view_data(GtkTreeModel *store,struct view_info *view_info,
                        gpointer data){
        GtkWidget *view;
        GtkTreeViewColumn *column;
        struct view_info *vinfo;

        /* Create the view */
        view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

        for (vinfo = view_info; vinfo->title != NULL; vinfo ++){
                /* Get our renderer */
                if (strcmp(vinfo->displaytype,"text") == 0){
                        column = get_text_column(vinfo,store,data);
                } else if (strcmp(vinfo->displaytype,"pixmap") == 0){
                        g_warning("Pixmap columns not supported\n");
                        continue;
                } else if (strcmp(vinfo->displaytype,"toggle") == 0){
                        column = get_toggle_column(vinfo,data);
                } else {
                        printf("Unknown renderer %s\n",vinfo->displaytype);
                        return NULL;
                }

                gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
        }


        return view;
}

/*
 * get_text_column
 *
 * Creates a single text column for a view.
 *
 * Args:
 *      struct view_info *The column with the view
 *      data: Data to associate with edited callback
 * Returns:
 *      New GtkTreeViewColumn*
 */
GtkTreeViewColumn *
get_text_column(struct view_info *vinfo,GtkTreeModel *store,gpointer data){
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;
        GType type;

        renderer = gtk_cell_renderer_text_new();

        if (vinfo->editfunc == NULL){
                column = gtk_tree_view_column_new_with_attributes(
                                vinfo->title, renderer,
                                vinfo->displaytype, vinfo->col,NULL);
        } else {
                column = gtk_tree_view_column_new_with_attributes(
                                vinfo->title, renderer,
                                vinfo->displaytype, vinfo->col,
                                "editable",vinfo->editflag,
                                "visible",vinfo->editflag,
                                NULL);
                g_signal_connect(G_OBJECT(renderer), "edited",
			        G_CALLBACK(vinfo->editfunc), data);
        }

        g_object_set_data(G_OBJECT(renderer),"column",(void *)vinfo->col);
        
        return column;
        
        /* Is it for rendering ints?  If so right align it */
        /* FIXME: Should do the same for floats, doubles, longs, shorts... */
        type = gtk_tree_model_get_column_type(store,vinfo->col);
        if (type ==  G_TYPE_INT){
                /* Warning: floats must be suffixed with 'f'! */
                g_object_set(G_OBJECT(renderer),"xalign",1.0f,NULL);
        }

        /* Also put short descriptions at the top */
        g_object_set(G_OBJECT(renderer),"yalign",0.0f,NULL);

        return column;
}

/*
 * get_toggle_column
 *
 * Creates a single toggle (true/false) column for a view.
 *
 * Args:
 *      struct view_info *The column with the view
 *      data: Data to associate with toggle callback
 * Returns:
 *      New GtkTreeViewColumn*
 */
GtkTreeViewColumn *
get_toggle_column(struct view_info *vinfo,gpointer data){
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        renderer = gtk_cell_renderer_toggle_new();

        if (vinfo->editflag == 0){
                column = gtk_tree_view_column_new_with_attributes(
                                        vinfo->title, renderer,
                                        "active",vinfo->col,
                                        NULL);
        } else {
                g_signal_connect(G_OBJECT(renderer),"toggled",
                                G_CALLBACK(item_toggled),data);
                g_object_set_data(G_OBJECT(renderer),"column",(void *)vinfo->col);
                column = gtk_tree_view_column_new_with_attributes(
                                        vinfo->title, renderer,
                                        "active",vinfo->col,
                                        "activatable",vinfo->editflag,
                                        "visible",vinfo->editflag,
                                        NULL);
        }
        return column;
}

/*
 * tree_lookup
 *
 * Looks for a particular key in the store.  Primary keys are found
 * from the list supplied.  This is called recursively.
 *
 * The matching iterator must be freed with g_free
 *
 * FIXME: This should search for primaries in parallel.
 *
 * Args:
 *      Store: Tree model to search
 *      int *: List of columns to search for primary keys
 *      char *: key to search for
 *      iter: Place to start searching from, or NULL for whole tree
 * Returns:
 *      NULL if not found or error
 *      Newly allocated tree iter with matching data
 */
GtkTreeIter *
tree_lookup(GtkTreeModel *store,int *primaries,char *key,GtkTreeIter *parent){
        GtkTreeIter *iter;
        gboolean flag;
        char *str;

        if (key == NULL){
                printf("Can't lookup empty key\n");
                return NULL;
        }

        if (g_utf8_validate(key,-1,NULL) == FALSE){
                printf("couldn't validate string\n");
                return NULL;
        }
        
        iter = g_malloc(sizeof(GtkTreeIter));

        while (*primaries != -1){
                flag = gtk_tree_model_iter_children(GTK_TREE_MODEL(store),
                                iter,parent);
                while (flag){
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        *primaries,&str,-1);
                        if (str != NULL && strcmp(str,key) == 0){
                                return iter;
                        }

                        flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(store),
                                        iter);
                }
                primaries ++;
        }

        g_free(iter);

        return NULL;
}

/*
 * tree_insert
 *
 * Inserts a single row into a store
 * Will update if anything matching the primary key exists
 *
 * FIXME: This is rather unoptimised (n^3?)
 * FIXME: This is also UGLY
 */
void
tree_insert(GtkTreeStore *store,struct store_info *store_info,
                int *primaries,element_t *el,char **recurse,
                GtkTreeIter *parent){
        GtkTreeIter *iter,back;
        char *xmlname = NULL;
        char *str = NULL;
        gboolean update = TRUE;
        element_t *e2,*child;
        int val;
        int i;
        GList *elements;
        int *ptmp;
        struct store_info *sinfo = store_info;

        /* FIXME: Sanity checked please */

        for (ptmp = primaries ; *ptmp != -1 ; ptmp ++){
                xmlname = get_primary_xmlname(sinfo,*ptmp);
                if (xmlname == NULL)
                        continue;
                str = get_text(xmlname,el);
                if (str != NULL)
                        break;
        }
        if (*ptmp == -1){
                printf("could not find any primary keys\n");
                return;
        }
        if (str == NULL){
                printf("Nothing matching the key %s found\n",xmlname);
                return;
        }

        /* Cool - we have a primary key - see if it is in the store */
        iter = tree_lookup(GTK_TREE_MODEL(store),primaries,str,parent);
        if (iter == NULL){
                iter = &back;
                gtk_tree_store_append(store,iter,parent);
                entry_zero(store,sinfo,iter);
                update = FALSE;
        } else {
                back = *iter;
                iter = &back;
        }

        /* Now insert the data... */
        for ( ; sinfo->pos != -1 ; sinfo ++){
                if (sinfo->save == SAVE_ELEMENT){
                        /* Value is the loaded element */
                        gtk_tree_store_set(store,iter,
                                        sinfo->pos,el->name,-1);
                        continue;
                }
                if (sinfo->xmlname == NULL){
                        continue;
                }
                /* If there is a loading function - use it */
                if (sinfo->loadfn != NULL){
                        e2 = get_element_depth(sinfo->xmlname,el,1);
                        sinfo->loadfn(store,iter,sinfo->pos,e2);
                        continue;
                }
                switch (sinfo->datatype){
                case G_TYPE_BOOLEAN:
                        e2 = get_element_depth(sinfo->xmlname,el,1);
                        if (update == FALSE || (e2 && update))
                                gtk_tree_store_set(store,iter,
                                                sinfo->pos,e2,-1);
                        break;
                case G_TYPE_STRING:
                        /*
                        if (el->type != ETYPE_TEXT || el->elements)
                                printf("type is %d children %p\n",
                                                el->type,el->elements);
                          */                      
                        str = get_text(sinfo->xmlname,el);
                        if (str){
                                str = g_strdup(str);
                                gtk_tree_store_set(store,iter,
                                                sinfo->pos,str,-1);
                        }
                        break;
                case G_TYPE_INT:
                        str = get_text(sinfo->xmlname,el);
                        if (str != NULL){
                                val = strtol(str,NULL,0);
                                gtk_tree_store_set(store,iter,
                                                sinfo->pos,val,-1);
                        }
                        break;
                default:
                        printf("Can't handle that type \n");

                }
        }

        if (recurse != NULL){
                elements = el->elements;
                while (elements != NULL){
                        child = elements->data;
                        for (i = 0 ; recurse[i] ; i ++){
                                if (strcmp(child->name,recurse[i]) == 0){
                                        GtkTreeIter *tmp;
                                        tmp = gtk_tree_iter_copy(iter);
                                        tree_insert(store,store_info,primaries,
                                                        child, recurse,tmp);
                                        g_free(tmp);
                                        break; /* only do one */
                                }
                        }
                        elements = g_list_next(elements);
                }
        }
}

/*
 * get_primary_xmlname
 *
 * FIXME: Document
 */
static char *
get_primary_xmlname(struct store_info *sinfo,int primary){

        if (sinfo == NULL)
                return NULL;

        while (sinfo->pos != -1){
                if (sinfo->pos == primary)
                        return sinfo->xmlname;
                sinfo ++;
        }
        return NULL;
}


/*
 * tree_refresh
 *
 * Updates each cell in the tree using the supplied store info.
 *
 * FIXME: This functions arguments are backwards.
 *
 * Args:
 *      sinfo of data to search for
 *      store to update
 */
void
tree_refresh(struct store_info *sinfo,GtkTreeStore *store){
        GtkTreeIter iter;

        if (sinfo == NULL || store == NULL)
                return;

        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)==FALSE){
                /* Tree is empty - all done ;-) */
                return;
        }

        tree_refresh_level(sinfo,store,&iter);


}

/*
 * tree_refresh_level
 *
 * FIXME: Document me
 */
static void
tree_refresh_level(struct store_info *sinfo,GtkTreeStore *store,
                GtkTreeIter *start){
        GtkTreeIter child;

        do {
                tree_refresh_item(sinfo,store,start);

                if (gtk_tree_model_iter_children(GTK_TREE_MODEL(store),
                                        &child,start))
                        tree_refresh_level(sinfo,store,&child);

        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), start));


}

/*
 * refresh_tree_item
 *
 * Refreshes a single item in a tree
 * FIXME: Document me
 */
void
tree_refresh_item(struct store_info *sinfo,
                GtkTreeStore *store,GtkTreeIter *item){
        int *x;
        int total,tmp;
        GtkTreeIter *parent;

        while (sinfo->pos != -1){
                switch(sinfo->update){
                case UPDATE_NONE:
                        /* nothing to do */
                        break;
                case UPDATE_SUM:
                        x = sinfo->updatedata;
                        total = 0;
                        while (*x != -1){
                                gtk_tree_model_get(GTK_TREE_MODEL(store),
                                                item,*x,&tmp, -1);
                                total += tmp;
                                x ++;
                        }
                        gtk_tree_store_set(store,item,sinfo->pos,total,-1);
                        break;
                case UPDATE_PARENT:
                        /* Check parent/item in get parent */
                        parent = get_parent(GTK_TREE_MODEL(store),item);
                        if (parent == item)
                                total = 0;
                        else
                                gtk_tree_model_get(GTK_TREE_MODEL(store),parent,
                                                sinfo->updatedata,&total, -1);
                        gtk_tree_store_set(store,item,sinfo->pos,total,-1);
                        break;
                case UPDATE_AVG:
                        printf("Avg not implemented\n");
                        break;
                case UPDATE_FUNCTION:
                        total = ((updatefunc)sinfo->updatedata)(item);
                        gtk_tree_store_set(store,item,sinfo->pos,total,-1);
                        break;
                default:
                        printf("Unknown update %d\n",sinfo->update);
                }
                sinfo ++;
        }
}

/*
 * gets the top level parent of item
 *
 * Returns
 *      The iter if node has no parent
 *      GtkTreeIter *otherwise
 */
GtkTreeIter *
get_parent(GtkTreeModel *store,GtkTreeIter *iter){
        GtkTreePath *path;
        GtkTreeIter *parent;
        GtkTreeIter temp;
        gboolean flag;


        flag =  gtk_tree_model_iter_parent(GTK_TREE_MODEL(store),
                                &temp,iter);
        if (flag == FALSE){
                return iter;
        }

        path = gtk_tree_model_get_path(store,iter);
        while (gtk_tree_path_get_depth(path) > 1){
                gtk_tree_path_up(path);
        }
        parent = g_malloc(sizeof(GtkTreeIter));
        gtk_tree_model_get_iter(store,parent,path);
        gtk_tree_path_free(path);

        return parent;
}

/*
 * tree_save
 *
 * FIXME: Document
 *
 */
void
tree_save(GtkTreeModel *store,struct store_info *sinfo,element_t *el){
        GtkTreeIter root;

        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&root)){
                tree_save_internal(store,sinfo,el,&root);
        }
}

/*
 * tree_save_internal
 *
 * FIXME: Document
 *
 */
static void
tree_save_internal(GtkTreeModel *store,struct store_info *sinfo,
                element_t *el,GtkTreeIter *iter){
        element_t *node;
        GtkTreeIter child;
        char *tag;

        do {
                gtk_tree_model_get(store,iter,0,&tag,-1);
                if (tag == NULL){
                        printf("No tag in 0 of store\n");
                        break;
                }

                /* create a stat node */
                node = create_element(el,tag,ETYPE_ELEMENT);
                if (node == NULL){
                        g_warning("create element returned NULL\n");
                        return;
                }

                /* save this stat */
                node_to_xml_tree(store,sinfo,node,iter);

                if (gtk_tree_model_iter_children(GTK_TREE_MODEL(store),
                                        &child,iter) == TRUE){
                        tree_save_internal(store,sinfo,node,&child);
                }

        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), iter));
}

/*
 * node_to_xml_tree
 *
 * FIXME: Document
 * FIXME: Doesn't handle 'save defined'
 */
static void
node_to_xml_tree(GtkTreeModel *store,struct store_info *sinfo,
                element_t *node,GtkTreeIter *iterp){
        gboolean flag;
        gboolean child;
        gboolean hasparent;
        save_t save;
        GtkTreeIter parent;
        char *str;
        int val;

        child = gtk_tree_model_iter_has_child(store,iterp);
        hasparent = gtk_tree_model_iter_parent(store,&parent,iterp);

        /* for each row in the table */
        for ( ; sinfo->pos != -1 ; sinfo ++){

                if (sinfo->xmlname == NULL){
                        continue;
                }
                save = sinfo->save;
                /* Check that I need to save */
                if ((save == SAVE_NEVER) ||
                        (save == SAVE_ELEMENT) ||
                        (save == SAVE_PARENT && hasparent == TRUE) ||
                        (save == SAVE_CHILDREN && hasparent == FALSE) ||
                        (save == SAVE_BOTTOM && child == TRUE)){
                        /* Don't need to save */
                        continue;
                }
                if (save == SAVE_NOT_MIDDLE){
                        if (hasparent == TRUE && child == TRUE)
                                continue;
                }

                switch (sinfo->datatype){
                case G_TYPE_BOOLEAN:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iterp,
                                        sinfo->pos,&flag,
                                        -1);
                        if (flag)
                                create_element(node,sinfo->xmlname,ETYPE_EMPTY);
                        break;
                case G_TYPE_STRING:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iterp,
                                        sinfo->pos,&str,
                                        -1);
                        create_text_element(node,sinfo->xmlname,str);
                        break;
                case G_TYPE_INT:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iterp,
                                        sinfo->pos,&val,
                                        -1);
                        create_numeric_element(node,sinfo->xmlname,val);
                        break;
                default:
                        printf("Can't handle that type \n");
                        break;
                }
        }
}

/*
 * entry_zero
 *
 * Zero's out the supplied node from the supplied store info.
 * Note fields not mentioned in the store will not be modified.
 * Note that fields are not freed or cleared before deletion
 *
 * Args:
 *      Store to update
 *      struct store_info contianing store information
 *      GtkTreeIter node to delete
 *
 */
static void
entry_zero(GtkTreeStore *store,struct store_info *sinfo,GtkTreeIter *entry){
        for ( ; sinfo->pos != -1 ; sinfo ++){
                gtk_tree_store_set(store,entry,sinfo->pos,0,-1);
        }
}

/*
 * tree_set_from_list
 *
 * Set fields in a store column from a GList
 *
 * Args:
 *      Store
 *      int col to set
 *      GList list of data to set
 */
void
tree_set_from_list(GtkTreeStore *store,GtkTreeIter *parent,int col,GList *list){
        tree_set_from_list_data(store,parent,col,list,-1,0);
}
void
tree_set_from_list_data(GtkTreeStore *store,GtkTreeIter *parent,
                int col,GList *list,int field,char *data){
        GtkTreeIter iter;

        if (store == NULL){
                printf("tree_set_from_list: store is NULL\n");
        }

        while (list != NULL){
                gtk_tree_store_append(store,&iter,parent);
                if (field == -1){
                        gtk_tree_store_set(store,&iter,col,list->data,-1);
                } else {
                        gtk_tree_store_set(store,&iter,col,list->data,
                                        field,data,-1);
                }
                list = list->next;
        }
}



/*
 * tree_get_from_list
 *
 * Goes through a col and puts its data into a list
 *
 * Args:
 *      Tree model to retreive from
 *      int col to search in
 * Returns:
 *      List of data - caller must free
 */
GList *
tree_get_from_list(GtkTreeModel *store,int col){
        GList *list;
        GtkTreeIter iter;
        gboolean flag;
        char *str;

        list = NULL;
        flag = gtk_tree_model_get_iter_first(store,&iter);

        while (flag){
                gtk_tree_model_get(store,&iter,col,&str,-1);
                list = g_list_prepend(list,str);
                flag = gtk_tree_model_iter_next(store,&iter);
        }

        list = g_list_reverse(list);

        return list;
}


/*
 * tree_delete_item
 *
 * Deletes the supplied row - deletes any string data
 *
 * Args:
 *     GtkTreeStore *store,struct store_info *sinfo,GtkTreeIter *entry
 * Returns:
 *      nothing
 */
void
tree_delete_item(GtkTreeStore *store,struct store_info *sinfo,
                GtkTreeIter *entry){
        /*
        char *str;

        FIXME: need to do this recusively, then free
        for ( ; sinfo->pos != -1 ; sinfo ++){
                if (sinfo->datatype != G_TYPE_STRING){
                        gtk_tree_model_get(GTK_TREE_MODEL(store),entry,
                                        sinfo->pos,&str,-1);
                        g_free(str);

                }
                }
                */
        gtk_tree_store_remove(store,entry);
}

/*
 * tree_copy_item
 *
 * Copies from one iter to another.
 *
 * Args:
 *      GtkTreeStore *store to do copying in
 *      GtkTreeItem *src
 *      GtkTreeItem *dest or NULL if copying top level element
 */
void
tree_copy_item(GtkTreeStore *store,GtkTreeIter *src,GtkTreeIter *parent){
        GValue *vp;
        int i,max;
        GtkTreeIter dest;

        max = gtk_tree_model_get_n_columns(GTK_TREE_MODEL(store));

        gtk_tree_store_append(store,&dest,parent);

        for (i = 0 ; i < max ; i ++){
                vp = g_malloc0(sizeof(GValue));
                gtk_tree_model_get_value(GTK_TREE_MODEL(store),src,i,vp);
                gtk_tree_store_set_value(store,&dest,i,vp);
                g_free(vp);
                /* FIXME: this now leaks for strvalues
                 * - if this is on this sometimes it crashes */
                //g_value_unset(&value);
        }
}


/*
 * tree_set_flag
 *
 * Sets all the flags in a field to a particular value
 * FIXME: Document
 *
 */
void
tree_set_flag(GtkTreeStore *store,GtkTreeIter *parent,int col,gboolean value){
        GtkTreeIter iter;
        gboolean flag;

        flag = gtk_tree_model_iter_children(GTK_TREE_MODEL(store),&iter,parent);
        while (flag){
                gtk_tree_store_set(store,&iter,col,value,-1);
                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter);
        }
}
void
tree_set_flag_recursive(GtkTreeStore *store,GtkTreeIter *parent,int col,gboolean value){
        GtkTreeIter iter;
        GtkTreeIter tmp;
        gboolean flag;

        flag = gtk_tree_model_iter_children(GTK_TREE_MODEL(store),&iter,parent);
        while (flag){
                gtk_tree_store_set(store,&iter,col,value,-1);
                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter);
                if (gtk_tree_model_iter_children(GTK_TREE_MODEL(store),
                                        &tmp,&iter) == TRUE){
                        tree_set_flag_recursive(store,&iter,col,value);
                }
        }
}


/*
 * item_toggle
 *
 * Originally lifted from gtk-demo
 * FIXME: Documentation
 *
 */
static void
item_toggled(GtkCellRendererToggle *cell,gchar *path_str,gpointer data){
        struct choice *select;
        GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
        GtkTreeIter iter;
        gboolean toggle_item;
        gint column,cols;

        select = (struct choice *)data;
        if (select == NULL){
                printf("Select is NULL - can't continue\n");
                return;
        }

        column = (int)g_object_get_data(G_OBJECT(cell),"column");

        /* sanity check */
        cols = gtk_tree_model_get_n_columns(GTK_TREE_MODEL(select->store));
        assert(column < cols);
        assert(column >= 0);

        /* get toggled iter */
        gtk_tree_model_get_iter(GTK_TREE_MODEL(select->store), &iter, path);
        gtk_tree_model_get(GTK_TREE_MODEL(select->store),&iter,
                        column, &toggle_item, -1);

        if (toggle_item == TRUE){
                toggle_item = FALSE;
                select->count --;
        } else {
                toggle_item = TRUE;
                select->count ++;
        }

        if (select->button != NULL){
                if (select->count <= select->max
                                && select->count >= select->min)
                        gtk_widget_set_sensitive(select->button,TRUE);
                else
                        gtk_widget_set_sensitive(select->button,FALSE);
        }

        if (select->display != NULL)
                entry_set_digit(select->display,select->count);

        /* set new value */
        gtk_tree_store_set(select->store,&iter,column,toggle_item,-1);

        /* clean up */
        gtk_tree_path_free(path);
}

/*
 * tree_popup_add
 *
 * Adds a popup menu the given tree view.
 *
 * Args:
 *      GtkWidget *view to add popup too
 *      GtkItemFactoryEntry *menus to add
 *      int n items in ItemFactory (use G_N_ELEMENTS() to get)
 */
void
tree_popup_add(GtkWidget *view,GtkItemFactoryEntry *items,int n,gpointer data){
        GtkItemFactory *popup_factory;
        GtkWidget *menu;

        popup_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
        gtk_item_factory_create_items(popup_factory, n, items,data);

        menu = gtk_item_factory_get_widget(popup_factory,"<main>");
        /* Connect _after_ so the normal selection process works */
        g_signal_connect_after(G_OBJECT(view), "button-release-event",
	                      G_CALLBACK(tree_event_handler),
                              menu);
}

/*
 * tree_event_handler
 *
 * Event handler for tree stores to create popup menu.
 *
 * FIXME: Hard coded number '3'
 *
 */
static gint
tree_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer data){
        GdkEventButton *bevent;

        if (event->type == GDK_BUTTON_RELEASE ||
                        event->type == GDK_BUTTON_PRESS) {
                bevent = (GdkEventButton *) event;
                /* We only care about the right button */
                if (bevent->button == 3){
                        /* Pop up the menu */
                        gtk_menu_popup(GTK_MENU(data),NULL,NULL,NULL,NULL,
                                        bevent->button, bevent->time);

                        /* Our job here is done */
                        return TRUE;
                }

        }

        return FALSE; /* Event not handled */
}

/*
 * tree_create_popup
 *
 * Creates a new popup windows.  Allocates the required data, but does not
 * show the popup.  
 *
 * FIXME: Args
 */
struct popupinfo *
tree_create_popup(struct treepopupinfo *tinfo,GtkWidget *window){
        GtkWidget *wig,*dialog,*label;
        GtkWidget *frame;
        GtkWidget *button;
        struct popupinfo *popupinfo;
        GList *list;
        char **tmp;

        GtkSizeGroup *sgroup;
        GtkWidget *vbox;
        GtkWidget *hbox;

        /* Create a window */
        dialog = gtk_dialog_new_with_buttons("View",GTK_WINDOW(window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         NULL);
        /* Lets not set a default size */
        /*gtk_window_set_default_size(GTK_WINDOW(dialog),550,470);*/

        /* Create our popup info */
        popupinfo = g_malloc0(sizeof(struct popupinfo));
        popupinfo->window = dialog;
        popupinfo->tinfo = tinfo;
        popupinfo->store = NULL;

        /* This is for the labels to be the same size */
        sgroup = GTK_SIZE_GROUP(gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL));
        vbox = gtk_vbox_new(FALSE,0);

        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
                                                        GTK_WIDGET(vbox));

        button = gtk_button_new_from_stock(GTK_STOCK_OK);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
                                                        GTK_WIDGET(button));


        g_signal_connect(G_OBJECT(button),"clicked",
                        G_CALLBACK(tree_popup_button_cb),(gpointer)popupinfo);




        for ( ; tinfo->label != NULL ; tinfo ++){
                switch (tinfo->type){
                case POPUP_TOGGLE:
                        wig = gtk_check_button_new_with_label(tinfo->label);
                        gtk_box_pack_start(GTK_BOX(vbox),wig,FALSE,FALSE,0);
			break;
                case POPUP_ENTRY:
                case POPUP_DIGIT:
                        hbox = new_row(vbox);
                        label = gtk_label_new(tinfo->label);
                        gtk_misc_set_alignment(GTK_MISC(label), 1.0f, 0.5f);
                        gtk_size_group_add_widget(sgroup,label);
                        gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
                        wig = gtk_entry_new();
                        gtk_box_pack_start(GTK_BOX(hbox),wig,TRUE,TRUE,0);
                        break;

                case POPUP_TEXTVIEW:
                        frame = gtk_frame_new(tinfo->label);
                        gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);
                        wig = gtk_text_view_new();
                        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(wig),
                                        GTK_WRAP_WORD);
                        gtk_container_add(GTK_CONTAINER(frame),wig);
                        break;
                case POPUP_COMBO:
                        hbox = new_row(vbox);
                        label = gtk_label_new(tinfo->label);
                        gtk_misc_set_alignment(GTK_MISC(label), 1.0f, 0.5f);
                        gtk_size_group_add_widget(sgroup,label);
                        gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
                        tmp = tinfo->data;
                        list = NULL;
                        for (tmp = tinfo->data ; *tmp != NULL ; *tmp ++){
                                list = g_list_append(list,*tmp);
                        }
                        wig = gtk_combo_new();
                        gtk_combo_set_popdown_strings(GTK_COMBO(wig),list);
                        gtk_box_pack_start(GTK_BOX(hbox),wig,TRUE,TRUE,0);
                        break;
                default:
                        printf("Unknown type\n");
                        wig = NULL;
                }
                tinfo->widget = wig;
        }

        return popupinfo;
}


static void
tree_popup_button_cb(GtkWidget *button,gpointer data){
        struct popupinfo *popinfo;

        popinfo = (struct popupinfo *)data;

        gtk_widget_hide(popinfo->window);
        tree_save_popup(popinfo);
}

/*
 * tree_save_popup
 *
 * Called when a popup closes, saves any modified data back to the iter.
 *
 * Args:
 *  popupinfo describing the popup
 */
void
tree_save_popup(struct popupinfo *popupinfo){
        gboolean val;
        const char *text;
        int digit;
        GtkToggleButton *tog;
        struct treepopupinfo *tinfo = popupinfo->tinfo;
        GtkTreeStore *store = popupinfo->store;
        GtkTreeIter *iter = &popupinfo->iter;
        GtkTextIter start,end;
        GtkTextBuffer *buffer;

        for ( ; tinfo->field != -1 ; tinfo ++){
                if (tinfo->widget == NULL){
                        printf("Null widget - bad\n");
                        continue;
                }
                switch (tinfo->type){
                case POPUP_TOGGLE:
                        tog = GTK_TOGGLE_BUTTON(tinfo->widget);
                        val = gtk_toggle_button_get_active(tog);
                        gtk_tree_store_set(store,iter,tinfo->field,val,-1);
                        break;
                case POPUP_ENTRY:
                        text = gtk_entry_get_text(GTK_ENTRY(tinfo->widget));
                        if (text == NULL)
                                text = "";
                        gtk_tree_store_set(store,iter,tinfo->field,text,-1);
                        break;
                case POPUP_DIGIT:
                        text =  gtk_entry_get_text(GTK_ENTRY(tinfo->widget));
                        if (text == NULL)
                                text = "";
                        digit = strtol(text,NULL,0);
                        gtk_tree_store_set(store,iter,tinfo->field,digit,-1);
                        break;
                case POPUP_TEXTVIEW:
                        buffer = gtk_text_view_get_buffer(
                                                GTK_TEXT_VIEW(tinfo->widget));
                        gtk_text_buffer_get_bounds(buffer,&start,&end);
                        text = gtk_text_buffer_get_text(buffer,&start,&end,0);
                        if (text == NULL)
                                text = "";
                        gtk_tree_store_set(store,iter,tinfo->field,text,-1);
                        break;
                case POPUP_COMBO:
                        text = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(
                                                        tinfo->widget)->entry));
                        if (text == NULL)
                                text = "";
                        gtk_tree_store_set(store,iter,tinfo->field,text,-1);
                        break;
                default:
                        printf("tree_save_popup: Bad type %d\n",tinfo->type);
                }
        }
        file_modify();
}

/*
 * tree_set_popup
 *
 * Updates the popup from the iter in popupinfo
 *
 * Args:
 *  popupinfo to show
 */
void
tree_set_popup(struct popupinfo *popupinfo){
        gboolean val;
        char *text;
        int digit;
        GtkToggleButton *tog;
        GtkTextBuffer *buffer;
        struct treepopupinfo *tinfo = popupinfo->tinfo;
        GtkTreeStore *store = popupinfo->store;
        GtkTreeIter *iter = &popupinfo->iter;
        GtkTextIter start,end;
        GList *taglist;
        GtkTextTag *texttag;
        struct taginfo *tag;

        for ( ; tinfo->field != -1 ; tinfo ++){
                if (tinfo->widget == NULL){
                        printf("NULL widget- bad\n");
                        continue;
                }
                switch (tinfo->type){
                case POPUP_TOGGLE:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        tinfo->field,&val,-1);
                        tog = GTK_TOGGLE_BUTTON(tinfo->widget);
                        gtk_toggle_button_set_active(tog,val);
                        break;
                case POPUP_ENTRY:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        tinfo->field,&text,-1);
                        if (text == NULL)
                                text = "";
                        gtk_entry_set_text(GTK_ENTRY(tinfo->widget),text);
                        break;
                case POPUP_DIGIT:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        tinfo->field,&digit,-1);
                        entry_set_digit(tinfo->widget,digit);
                        break;
                case POPUP_TEXTVIEW:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        tinfo->field,&text,-1);
                        if (text == NULL)
                                text = "";
                        buffer = gtk_text_view_get_buffer(
                                        GTK_TEXT_VIEW(tinfo->widget));
                        gtk_text_buffer_set_text(buffer,text,-1);
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        (tinfo->field) + 1,&taglist,-1);
                        for ( ; taglist != NULL ; taglist = taglist->next){
                                tag = taglist->data;
                                gtk_text_buffer_get_iter_at_offset(buffer,
                                                &start,tag->start);
                                gtk_text_buffer_get_iter_at_offset(buffer,
                                                &end,tag->end);
                                texttag = gtk_text_buffer_create_tag(buffer,
                                                NULL,
                                                "style",PANGO_STYLE_ITALIC,
                                                NULL);
                                gtk_text_buffer_apply_tag(buffer,texttag,
                                                &start,&end);
                                break;
                        }
                        break;
                case POPUP_COMBO:
                        gtk_tree_model_get(GTK_TREE_MODEL(store),iter,
                                        tinfo->field,&text,-1);
                        if (text == NULL)
                                text = "";
                        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(tinfo->widget)
                                                ->entry),text);
                        break;
                default:
                        printf("NYI\n");
                }
                gtk_widget_set_sensitive(tinfo->widget,popupinfo->editable);
        }


}


/*
 * listinfo (this is a bad name)
 *
 * Takes top level info from a XML file and puts then in individual
 * fields
 */
GtkWidget *
tree_make_listinfo(struct listinfo *listinfo){
        GtkWidget *table;
        GtkWidget *box;
        GtkWidget *label;
        GtkWidget *frame;
        int pos = 0;

        box = gtk_hbox_new(FALSE,0);
        table = gtk_table_new(G_N_ELEMENTS(listinfo),2,FALSE);
        gtk_box_pack_start(GTK_BOX(box),table,TRUE,TRUE,0);

        for ( ; listinfo->xmltag != NULL ; pos ++, listinfo ++){
                if (listinfo->type & XML_LONG){
                        frame = gtk_frame_new(listinfo->label);
                        gtk_table_attach_defaults(GTK_TABLE(table),frame,0,2,
                                        pos,pos+1);
                        listinfo->widget = gtk_text_view_new();
                        gtk_text_view_set_wrap_mode(
                                        GTK_TEXT_VIEW(listinfo->widget),
                                        GTK_WRAP_WORD);
                        gtk_container_add(GTK_CONTAINER(frame),
                                        listinfo->widget);
                        continue;

                }
                label = gtk_label_new(listinfo->label);
                gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_RIGHT);
                gtk_table_attach(GTK_TABLE(table),label,0,1,pos,pos+1,
                                GTK_FILL,GTK_FILL,6,0);

                listinfo->widget = gtk_entry_new();
                gtk_table_attach_defaults(GTK_TABLE(table),listinfo->widget,
                                1,2,pos,pos+1);
        }

        return box;
}

void
tree_listinfo_insert(struct listinfo *listinfo,element_t *el){
        unsigned int pos;
        char *msg;
        struct listinfo *cur;
        GtkTextBuffer *buffer;

        if (el == NULL)
                return;
        for (pos = 0 ; listinfo[pos].xmltag != NULL ; pos ++){
                cur = &listinfo[pos];
                if (strcmp(cur->xmltag,el->name) != 0){
                        continue;
                }
                if (cur->type & XML_MULTIPLE){
                        if (strlen(gtk_entry_get_text(GTK_ENTRY(cur->widget)))){                                gtk_entry_append_text(GTK_ENTRY(cur->widget),", ");
                        }
                        gtk_entry_append_text(GTK_ENTRY(cur->widget),el->text);
                } else if (cur->type & XML_LONG){
                        buffer = gtk_text_view_get_buffer(
                                                GTK_TEXT_VIEW(cur->widget));
                        gtk_text_buffer_set_text(buffer,el->text,-1);
                } else {
                        if (strlen(gtk_entry_get_text(GTK_ENTRY(cur->widget)))){                                msg = g_strdup_printf("<%s> mulitply "
                                                "defined\n",el->name);
                                gui_warning_msg(msg);
                                g_free(msg);
                        } else {
                                if (el->text != NULL)
                                        gtk_entry_set_text(GTK_ENTRY(cur->widget),
                                                el->text);
                        }
                }
        }


}

void
tree_listinfo_save(struct listinfo *listinfo, element_t *root){
        assert(listinfo != NULL);
        assert(root != NULL);

        for ( ; listinfo->label != NULL  ; listinfo ++){
                tree_listinfo_save_node(listinfo,root);
        }

}

void
tree_listinfo_clear(struct listinfo *listinfo){
        GtkTextBuffer *buffer;
        GtkTextIter start,end;

        assert(listinfo != NULL);

        for ( ; listinfo->label != NULL ; listinfo ++){
                if (listinfo->type & XML_LONG){
                        buffer = gtk_text_view_get_buffer(
                                        GTK_TEXT_VIEW(listinfo->widget));
                        gtk_text_buffer_get_bounds(buffer,&start,&end);
                        gtk_text_buffer_delete(buffer,&start,&end);
                } else {
                        gtk_entry_set_text(GTK_ENTRY(listinfo->widget),"");
                }
        }
}

/*
 * tree_listinfo_save_node
 *
 * Saves a single listinfo node into the output XML tree.
 *
 * FIXME: The inconsistent handles long and multiples
 *      - this code should probably be seperated into different
 *      functions
 */
static void
tree_listinfo_save_node(struct listinfo *linfo,element_t *root){
        GtkTextBuffer *buffer;
        GtkTextIter start,end;
        const char *text;
        char *msg;
        char **strs,*tmp;
        int pos;

        if (linfo->type & XML_LONG){
                buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(linfo->widget));
                gtk_text_buffer_get_bounds(buffer,&start,&end);
                text = gtk_text_buffer_get_text(buffer,&start,&end,FALSE);
        } else if (linfo->type & XML_MULTIPLE){
                text = gtk_entry_get_text(GTK_ENTRY(linfo->widget));
                strs = g_strsplit(text,",",-1);
                for (pos = 0 ; strs[pos] != 0 ; pos ++){
                        tmp = strs[pos];
                        while (isspace(*tmp))
                                tmp ++;
                        create_text_element(root,linfo->xmltag,tmp);
                }
                g_strfreev(strs);
                return;
        } else
                text = NULL;

        if (text == NULL){
                text = gtk_entry_get_text(GTK_ENTRY(linfo->widget));
        }

        if (text == NULL){
                if (linfo->type & XML_COMPULSARY){
                        msg = g_strdup_printf("You must enter a value for %s",
                                        linfo->label);
                        gui_warning_msg(msg);
                        g_free(msg);
                }
                return;
        }
        create_text_element(root,linfo->xmltag,text);

        return;
}

