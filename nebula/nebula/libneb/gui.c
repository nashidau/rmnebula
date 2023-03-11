#include <gtk/gtk.h>
#include <stdio.h>
#include <gnome.h>
#include <assert.h>

#include "libneb.h"


static void selection_changed(void *data);

extern GtkWidget *window;


void
gui_error_msg(char *msg){
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_OK,
                        msg);
        gtk_dialog_run(GTK_DIALOG(dialog));

        gtk_widget_destroy(dialog);
}

GtkWidget *
msg_dialog(char *title,char *msg1, GtkSignalFunc okay,char *msg2,GtkSignalFunc cancel){
        GtkWidget *dialog,*table;
        GtkWidget *button;
        //GtkWidget *vscrollbar;
        GtkWidget *textview;
        GtkTextBuffer *buffer;

        dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog),title);
        gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

        table = gtk_table_new(2, 2, FALSE);
        gtk_table_set_row_spacing(GTK_TABLE(table), 0, 2);
        gtk_table_set_col_spacing(GTK_TABLE(table), 0, 2);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, TRUE, TRUE, 0);

        button = gtk_button_new_with_label(msg1);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                button, TRUE, TRUE, 0);
        gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                        okay, GTK_OBJECT (dialog));

        button = gtk_button_new_with_label(msg2);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                button, TRUE, TRUE, 0);
        gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                        cancel, GTK_OBJECT (dialog));

        textview = gtk_text_view_new();
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview),GTK_WRAP_WORD);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textview),FALSE);
        gtk_table_attach_defaults(GTK_TABLE(table), textview, 0, 1, 0, 1);
        /*
        vscrollbar = gtk_vscrollbar_new(GTK_TEXT_VIEW(textview)->vadj);
        gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1,
                         GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
                */
        if (window != NULL)
                gtk_window_set_transient_for(GTK_WINDOW(dialog),
                                GTK_WINDOW(window));

        gtk_widget_show_all(dialog);

        return textview;
}

/*
 * Adds a message to an output buffer
 *
 * Args:
 *  GtkWidget *textview to add message to
 *  char *format: printf style format string
 *  ... Args for printf string
 */
void
msg_add(GtkWidget *textview,char *format, ...){
        va_list args;
        char *buf =NULL;
        GtkTextIter iter;
        GtkTextBuffer *buffer;

        va_start(args,format);
        buf = g_strdup_vprintf(format,args);
        va_end(args);

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_get_end_iter(buffer,&iter);
        gtk_text_buffer_insert(buffer,&iter,buf,-1);

        g_free(buf);
}

void
destroy(GtkWidget *widget, gpointer data) {
        data = NULL;
        gtk_widget_destroy(widget);
}

/*
 * Sets an entry in a gtk_entry to be the
 * digit passed
 */
void
entry_set_digit(GtkWidget *widget,int data){
        char buf[30];

        if (widget != NULL){
                sprintf(buf,"%2d",data);
                gtk_entry_set_text(GTK_ENTRY(widget),buf);
        }
}

/* gui_ask
 *
 * Pops up a dialog and asks for something. Receives a const char * to be used
 * as title for the window and input message ("what to ask"). 
 * Returns a char string pointer */
char *
gui_ask(const char *msg_string, GtkWidget *main_window) {
        GtkWidget *box,*cwindow;
        GtkWidget *ask_box;
	int result;
	char *message, *rtn_string;
        
	message = g_strdup(msg_string);

	cwindow = gtk_dialog_new_with_buttons(message,GTK_WINDOW(main_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		NULL);

        box = new_row(GTK_DIALOG(cwindow)->vbox);
        ask_box = new_text_box_large(box,message,TRUE);

        gtk_widget_show_all(cwindow);

        /* Run it */
        result = gtk_dialog_run(GTK_DIALOG(cwindow));

        if (result != GTK_RESPONSE_OK){
                printf("An error occured\n");
                return NULL;
        }

        /* Get the results */
        rtn_string = g_strdup(gtk_entry_get_text(GTK_ENTRY(ask_box)));

        /* Clean up */
        destroy_window(ask_box);
	free(message);

	return rtn_string;
}

/*
 * Generic Window packing functions
 */

/* create a text box */
GtkWidget *
new_text_box(GtkWidget *hbox,char *name,gboolean fill){
        GtkWidget *text = new_text_box_large(hbox,name,fill);

        gtk_widget_set_usize(text,50,-1);

        return text;
}
GtkWidget *
new_text_box_large(GtkWidget *hbox,char *name,gboolean fill){
        GtkWidget *label;
        GtkWidget *text;

        label = gtk_label_new(name);
        gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,TRUE,2);
        text = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(hbox),text,fill,TRUE,0);

        return text;
}


GtkWidget *
new_row(GtkWidget *vbox){
        GtkWidget *hbox = gtk_hbox_new(FALSE,2);
        gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);
        return hbox;
}
GtkWidget *
new_col(GtkWidget *hbox){
        GtkWidget *vbox = gtk_vbox_new(FALSE,2);
        gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE,TRUE,0);
        return hbox;
}

GtkWidget *
new_button(GtkWidget *box,char *name,GtkSignalFunc clicked){
        GtkWidget *button;

        button = gtk_button_new_from_stock(name);
        gtk_box_pack_start(GTK_BOX(box),button,TRUE,TRUE,2);
        if (clicked != NULL)
                gtk_signal_connect_object (GTK_OBJECT(button), "clicked",
                                clicked, GTK_OBJECT(button));

        return button;
}

/*
 * Destroys the window containing the widget
 */
void
destroy_window(GtkWidget *w){
        gtk_widget_destroy(gtk_widget_get_toplevel(w));
}



/*
 * Combo functions
 */
GtkWidget *
combo_dialog(struct combo_info *cinfo){
        GtkWidget *box,*label;
        GList *items;
        struct field_info *field;

        /* Sanity checking */
        if (cinfo == NULL)
                return NULL;


        items = get_files_specific(cinfo->datadirectory,cinfo->datapath,
                        cinfo->prefix,cinfo->suffix,NULL);

        cinfo->window = gtk_dialog_new_with_buttons(cinfo->title,
                              GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,
                              GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

        box = new_row(GTK_DIALOG(cinfo->window)->vbox);
        label = gtk_label_new(cinfo->label);
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        cinfo->combo = new_combo(box,items);

        g_list_free(items);

        for (field = cinfo->fieldinfo; field && field->label != NULL; field ++){
                box = new_row(GTK_DIALOG(cinfo->window)->vbox);
                label = gtk_label_new(field->label);
                field->display = gtk_entry_new();
                gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
                gtk_box_pack_start(GTK_BOX(box),field->display,TRUE,FALSE,2);
        }

        gtk_signal_connect_object(GTK_OBJECT(GTK_COMBO(cinfo->combo)->list),
                        "select-child",(GtkSignalFunc)selection_changed,
                        cinfo);

        return cinfo->combo;
}



static void
selection_changed(gpointer cinfodata){
        GtkEntry *entry;
        struct combo_info *cinfo;
        struct field_info *field;
        const char *str;
        char *data, *filename;
        element_t *el;

        if (cinfodata == NULL){
                printf("data is NULL\n");
                return;
        }
        cinfo = (struct combo_info *)cinfodata;
        //printf("cinfo is %p\n",cinfodata);

        entry = GTK_ENTRY(GTK_COMBO(cinfo->combo)->entry);

        if (cinfo->fieldinfo == NULL)
                /* Nothing to do - who cares */
                return;

        str = gtk_entry_get_text(entry);
        if (str == NULL)
                return;

        if (cinfo->prefix != NULL){
                filename = g_strconcat(cinfo->datadirectory,cinfo->prefix,str,
                                cinfo->suffix,NULL);
        } else {
                filename = g_strconcat(cinfo->datadirectory,str,
                                cinfo->suffix,NULL);
        }


        el = alena_parsefile(filename,WHITESPACE_IGNORE);
        if (el == NULL){
                printf("File failed to parse %s\n",filename);
                return;
        }

        field = cinfo->fieldinfo;
        while (field->label != NULL){
                data = field->filepath;
                data ++;
                str = get_text(data,el);
                gtk_entry_set_text(GTK_ENTRY(field->display),str);

                field ++;
        }


}


GtkWidget *
new_combo(GtkWidget *box,GList *list){
        GtkWidget *combo;

        combo = gtk_combo_new();
        gtk_combo_set_popdown_strings (GTK_COMBO (combo), list);
        gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry),FALSE);
        gtk_box_pack_start (GTK_BOX (box), combo, TRUE, FALSE, 2);

        return combo;
}

/*
 * gui_confirm_dialog
 *
 * Pops up a dialog of the form:
 * Are you sure you want to <msg>?
 * And waits for the user to click yes or no.
 *
 * Args:
 *      msg to display in dialog
 * Returns:
 *      True if the user clicked 'Yes'
 *      False otherwise
 */
gboolean
gui_confirm_dialog(const char *msg){
        GtkWidget *dialog;
        int result;

        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_QUESTION,
                        GTK_BUTTONS_YES_NO,
                        "Are you sure you want to %s",msg);

        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (result == GTK_RESPONSE_YES)
                return TRUE;
        else
                return FALSE;
}

void
gui_warning_msg(const char *msg){
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_WARNING,
                        GTK_BUTTONS_OK,
                        msg);

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        return;

}

void
gui_message(const char *msg,...){
        GtkWidget *dialog;
        va_list vargs;
        char *str;

        va_start(vargs,msg);
        str = g_strdup_vprintf(msg,vargs);
        va_end(vargs);

        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO,
                        GTK_BUTTONS_OK,
                        str);

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(str);

        return;

}




typedef enum {
        NEW_UNKNOWN,
        NEW_CHILD,
        NEW_SIBLING,
        NEW_ROOT,   /* A new tree */
} position_t;



static position_t choose_pos(struct rcmenu_info *minfo,GtkTreeIter iter);

extern GtkWidget *window;

void
rcmenu_delete_callback(gpointer callback_data,guint action,GtkWidget *widget){
        GtkTreeIter iter;
        struct rcmenu_info *minfo;
        gboolean issel,rv,children;
        GtkTreeSelection *selection;
        char *item,*msg;

        minfo = callback_data;

        if (file_modify() == FALSE){
                /* Debug */
                printf("Can't delete\n");
                return;
        }

        selection = gtk_tree_view_get_selection(minfo->view);

        issel = gtk_tree_selection_get_selected(selection,NULL,&iter);

        if (issel == FALSE){
                /* Debug */
                printf("nothing selected\n");
                return;
        }

        children = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(minfo->store),
                        &iter);
        
        gtk_tree_model_get(GTK_TREE_MODEL(minfo->store),&iter,
                        minfo->name_field,&item,-1);
        if (children == TRUE){
                msg = g_strdup_printf(_("delete the %s %s and its children?"),
                                minfo->item,item);
        } else {
                msg = g_strdup_printf(_("delete the %s %s?"),minfo->item,item);
        }
        rv = gui_confirm_dialog(msg);

        if (rv == TRUE){
                tree_delete_item(GTK_TREE_STORE(minfo->store),minfo->storeinfo,&iter);
                file_set_modified(); /* We modified it */
        }

        g_free(msg);
}

/* Adds a new CHILD to the current item - if it can */
void
rcmenu_new_callback(gpointer data,guint action,GtkWidget *widget){
        struct rcmenu_info *minfo;
        gboolean issel;
        GtkTreeSelection *selection;
        GtkTreeIter iter,new;
        int depth;
        position_t pos = NEW_UNKNOWN;

        minfo = data;

        assert(minfo != NULL);
        assert(minfo->popupinfo != NULL);
        assert(minfo->store != NULL);
        assert(minfo->item != NULL);

        selection = gtk_tree_view_get_selection(minfo->view);

        issel = gtk_tree_selection_get_selected(selection,NULL,&iter);

        if (issel == FALSE){
                pos = NEW_ROOT;
        } else if (minfo->maxdepth >= 0){
                /* Check the depth */
                depth = gtk_tree_store_iter_depth(minfo->store,&iter);
                if (depth >= minfo->maxdepth){
                        pos = NEW_SIBLING;
                }
        }

        if (action == RCMENU_NEW_CHILD && pos == NEW_SIBLING){
                /* FIXME: gui dialog waarnign should be here */
                printf("Can't add any more children\n");
                return;
        }

        if (action == RCMENU_NEW_CHILD && pos == NEW_UNKNOWN)
                pos = NEW_CHILD;
        if (action == RCMENU_NEW_SIBLING && pos == NEW_UNKNOWN)
                pos = NEW_SIBLING;
        
        if (pos == NEW_UNKNOWN){
                pos = choose_pos(minfo,iter);
                if (pos == NEW_UNKNOWN)
                        return; /* Clicked cancel */
        }

        switch (pos){
        case NEW_CHILD:
                gtk_tree_store_append(minfo->store,&new,&iter);
                break;
        case NEW_SIBLING:
                gtk_tree_store_insert_after(minfo->store,&new,NULL,&iter);
                break;
        case NEW_ROOT:
                /* Create a new root node */
                gtk_tree_store_append(minfo->store,&new,NULL);
                break;
        default:
                fprintf(stderr,"unknown position\n");
                g_assert_not_reached();
        }


        minfo->popupinfo->iter = new;

        /* Well we can say the file has been modified now */
        file_set_modified();

        /* Call back to get the default values */
        if (minfo->set_defaults != NULL)
                minfo->set_defaults(minfo,&new);

        /* Show the popup for the new item */
        minfo->popupinfo->editable = TRUE;
        tree_set_popup(minfo->popupinfo);
        gtk_widget_show_all(minfo->popupinfo->window);
}

/*
 * FIXME: Sanity check all fields
 */
void
rcmenu_edit_callback(gpointer data,guint action,GtkWidget *widget){
        struct rcmenu_info *minfo;
        gboolean issel;
        GtkTreeSelection *selection;

        minfo = data;

        assert(minfo != NULL);
        assert(minfo->popupinfo != NULL);

        if (file_modify() == FALSE){
                /* Debug */
                printf("Can't edit\n");
                return;
        }

        selection = gtk_tree_view_get_selection(minfo->view);

        issel = gtk_tree_selection_get_selected(selection,NULL,
                        &(minfo->popupinfo->iter));

        if (issel == FALSE){
                /* Debug */
                printf("nothing selected\n");
                return;
        }

        minfo->popupinfo->editable = TRUE;
        tree_set_popup(minfo->popupinfo);
        gtk_widget_show_all(minfo->popupinfo->window);


}


static position_t
choose_pos(struct rcmenu_info *minfo,GtkTreeIter iter){
        int rv;
        char *msg,*name;
        GtkWidget *dialog,*label;

        assert(minfo != NULL);
        assert(minfo->item != NULL);
        assert(minfo->name_field > -1);

        gtk_tree_model_get(GTK_TREE_MODEL(minfo->store),&iter,
                        minfo->name_field,&name,-1);


        msg = g_strdup_printf("<span weight=\"bold\" size=\"larger\">"
                        "Do you wish to create a sibling or a child?</span>\n\n"
                        "A sibling will be created at the same level as %s.\n"
                        "A child will be added beneath the %s '%s'.",
                        name,minfo->item,name);

        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,                                GTK_MESSAGE_QUESTION,
                                GTK_BUTTONS_NONE," ");

        label = gtk_label_new(NULL);
        gtk_widget_show(label);

        gtk_label_set_markup(GTK_LABEL(GTK_MESSAGE_DIALOG
                                        (dialog)->label),msg);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,
                                TRUE,FALSE,0);

        gtk_dialog_add_button(GTK_DIALOG(dialog),
                                GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);

        gtk_dialog_add_button(GTK_DIALOG(dialog),
                                _("Child"),NEW_CHILD);
        gtk_dialog_add_button(GTK_DIALOG(dialog),
                                _("Sibling"),NEW_SIBLING);



        rv = gtk_dialog_run(GTK_DIALOG(dialog));

        switch (rv){
                case GTK_RESPONSE_CANCEL:
                        gtk_widget_destroy(dialog);
                        /* Fall through */
                case GTK_RESPONSE_CLOSE:
                        break;
                case NEW_CHILD:
                        gtk_widget_destroy(dialog);
                        return NEW_CHILD;
                case NEW_SIBLING:
                        gtk_widget_destroy(dialog);
                        return NEW_SIBLING;
                default:
                        g_assert_not_reached();
        }

        return NEW_UNKNOWN;
}
