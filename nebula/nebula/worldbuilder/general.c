#include <gtk/gtk.h>
#include <assert.h>
#include "libneb.h"
#include "general.h"

GtkWidget *box;
struct listinfo listinfo_array[] = {
        {"genre",  "Genre",       XML_COMPULSARY, 0 },
        {"prefix", "Race Prefix", XML_COMPULSARY, 0 },
        {"description", "Description", XML_COMPULSARY | XML_LONG, 0 },
        {NULL,NULL,-1,0}
};
struct listinfo *listinfo = listinfo_array;


gboolean wb_get_leaves_internal(GtkTreeModel *store,GtkTreePath *path,
                GtkTreeIter *iter, gpointer data);


void
wb_general_init(GtkNotebook *notebook){
        GtkWidget *frame;
        GtkWidget *label;
        GtkWidget *desc;

        frame = gtk_frame_new("Genre Description");
        label = gtk_label_new("General");
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
        box = gtk_vbox_new(FALSE,2);
        gtk_container_add(GTK_CONTAINER(frame),box);

        desc = tree_make_listinfo(listinfo);
        gtk_box_pack_start(GTK_BOX(box),desc,FALSE,FALSE,0);

        return;
}

/*
 * Get the genre name
 *
 * FIXME: This and set_genre_name should be generalised for listinfo's
 */
const char *
wb_get_genre_name(struct fileinfo *finfo){
        const char *name = NULL;
        struct listinfo *linfo;

        assert(finfo != NULL);

        for (linfo = listinfo ; linfo->xmltag != NULL ; linfo ++){
                if (strcmp("genre",linfo->xmltag) == 0){
                        name = gtk_entry_get_text(GTK_ENTRY(linfo->widget));
                        if (name != NULL && strlen(name) > 0)
                                return name;
                }
        }

        if (finfo->name != NULL){
                name = strrchr(finfo->name,'/');
                if (name != NULL && strlen(name) > 1){
                        name ++;
                        return name;
                }
        }

        return "Un-named";
}

gboolean
wb_set_genre_name(const char *name){
	struct listinfo *linfo;
	
	assert(name != NULL);
	assert(strlen(name) > 0);

	for (linfo = listinfo ; linfo->xmltag != NULL ; linfo ++){
		if (strcmp("genre",linfo->xmltag) == 0){
			gtk_entry_set_text(GTK_ENTRY(linfo->widget),name);
			return TRUE;
		}
	}
	return FALSE;
}

struct get_leaves {
        GList *leaves;
        int namefield;
        int description;
};

GList *
wb_get_leaves(GtkTreeModel *store,int namefield, int description){
        struct get_leaves leaves;

        leaves.leaves = NULL;
        leaves.namefield = namefield;
        leaves.description = description;

        gtk_tree_model_foreach(store,wb_get_leaves_internal,&leaves);

        leaves.leaves = g_list_reverse(leaves.leaves);

        return leaves.leaves;
}

gboolean
wb_get_leaves_internal(GtkTreeModel *store,GtkTreePath *path,
                GtkTreeIter *iter, gpointer data){
        struct get_leaves *leaves;
        struct leaf *leaf;
        const char *name,*desc;

        leaves = data;
        assert(leaves != NULL);

        if (gtk_tree_model_iter_has_child(store,iter) == FALSE){
                /* Is a leaf */
                gtk_tree_model_get(store,iter,leaves->namefield,&name,
                                leaves->description,&desc,-1);
                assert(name != NULL);
                leaf = g_malloc(sizeof(leaf));
                assert(leaf != NULL);
                leaf->name = name;
                leaf->description = desc;
                leaves->leaves = g_list_prepend(leaves->leaves,leaf);
        }


        return FALSE;
}

