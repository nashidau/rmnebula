/*
 * item.c
 *
 * Item code for nebula
 */
#include "nebula.h"
#include "item.h"

extern GtkWidget *itempage;
GtkWidget *itemlist;

void
init_items(GtkWidget *box){
        /*
        GtkWidget *scrolled;
        char *titles[] = {"Item","Location","Weight"};
        GtkWidget *button;
        GtkWidget *hbox;

        scrolled = gtk_scrolled_window_new(NULL, NULL);

        itemlist = gtk_clist_new_with_titles(3, titles);
        gtk_container_add(GTK_CONTAINER(scrolled), itemlist);
        gtk_box_pack_start (GTK_BOX(box), scrolled, TRUE, TRUE, 0);

        hbox = new_row(box);
        button = new_button(hbox,"Add Item",add_item);
        */

}

void
xml_insert_item(element_t *el){
        struct item *item;
        char *text[3];
        char *p,*skill;
        int bonus;

        if (itemlist == NULL)
                return;

        item = calloc(1,sizeof(struct item));

        item->name = g_strdup(get_text("name",el));
        item->desc = g_strdup(get_text("location",el));

        p = get_text("weight",el);
        if (p != NULL)
                item->weight = strtol(p,NULL,0);

        /* Add any items */
        el = get_element("effect",el);
        if (el != NULL){
                skill = get_text("skill",el);
                bonus = get_int("bonus",el);
                //skill_insert_special(item->name,skill,bonus);
        }



        text[0] = item->name;
        text[1] = item->desc;
        text[2] = p;
        item->row = gtk_clist_append(GTK_CLIST(itemlist),text);
        return;
}

void
update_items(void){
        if (itemlist == NULL)
                return;
        gtk_clist_set_column_width(GTK_CLIST(itemlist),0,
                        gtk_clist_optimal_column_width(GTK_CLIST(itemlist),0));
}

void
add_item(void){
        GList *list;
        char **str;
        GtkWidget *item_window,*item_name;
        GtkWidget *box,*row;
        GtkWidget *label;
        GtkWidget *button;

        if (itemlist == NULL)
                return;

        list = NULL;
        str = carry_type;
        while (*str != NULL){
                list = g_list_append(list,*str);
                str ++;
        }

        item_window = gtk_dialog_new();
        box = GTK_DIALOG(item_window)->vbox;
        gtk_window_set_title(GTK_WINDOW(item_window),"Add Item");

        row = new_row(box);
        item_name = new_text_box_large(row,"Name: ",TRUE);
        row = new_row(box);
        item_name = new_text_box_large(row,"Description: ",TRUE);
        row = new_row(box);
        item_name = new_text_box_large(row,"Weight: ",TRUE);
        row = new_row(box);
        label = gtk_label_new("Carried");
        gtk_box_pack_start(GTK_BOX(row),label,FALSE,FALSE,2);
        item_name = new_combo(row,list);

        button = new_button(GTK_DIALOG(item_window)->action_area,"Add",
                        (GtkSignalFunc)destroy_window);

   /*     item_name = new_text_box_large(box,"Description: ",TRUE);
        item_name = new_text_box_large(box,"Description: ",TRUE);
        item_name = new_text_box_large(box,"Description: ",TRUE); */
        gtk_widget_show_all(item_window);

}
