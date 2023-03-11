/*
 * rr.c
 *
 * RR and defenses for nebula
 */
#include "nebula.h"
#include "rr.h"

void
rr_init(GtkWidget *box){
        rrstore = make_tree_store(rr_store_info);
        rrview = tree_create_view(GTK_TREE_MODEL(rrstore),rr_view_info);
        g_object_unref(G_OBJECT(rrstore));
        gtk_container_add(GTK_CONTAINER(box),rrview);
}

void
rr_insert(element_t *data){
        element_t *el;
        GList *elements;
        GtkTreeIter iter;

        for (elements = data->elements ; elements != NULL;
                        elements = g_list_next(elements)){
                el = elements->data;
                tree_insert(rrstore,rr_store_info,rrprimaries,el,NULL,NULL);
        }

        gtk_tree_model_get_iter_first(GTK_TREE_MODEL(rrstore),&iter);
        do {
                gtk_tree_store_set(rrstore,&iter,0,g_strdup("rr"),-1);
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(rrstore),&iter));
}

void
rr_update(void){
        tree_refresh(rr_store_info,rrstore);
}

int
rr_update_statbonus(GtkTreeIter *iter){
        char *str;

        gtk_tree_model_get(GTK_TREE_MODEL(rrstore),iter,RR_STATS,&str,-1);

        return get_stat_bonuses(str,3);

}

void
rr_save(element_t *root){
        root = create_element(root,"rrs",ETYPE_ELEMENT);

        tree_save(GTK_TREE_MODEL(rrstore),rr_store_info,root);
}

void
rr_clear(void){
        gtk_tree_store_clear(rrstore);
}

void
print_rrs(FILE *fp){
}
