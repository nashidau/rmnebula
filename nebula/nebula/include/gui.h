#ifndef GUI_H
#define GUI_H

struct combo_info {
        const char *title;
        const char *datadirectory;
        const char *datapath;
        const char *prefix;
        const char *suffix;
        const char *label;
        struct field_info *fieldinfo;
        /* Written by callee */
        GtkWidget *window;
        GtkWidget *combo;
};


struct rcmenu_info {
        GtkTreeStore *store;
        GtkTreeView *view;
        struct store_info *storeinfo;
        char *item;
        int name_field;
        struct popupinfo *popupinfo;
        /* how deep we can create new nodes
         *      < 0 is unlimited */
        int maxdepth;
        void (*set_defaults)(struct rcmenu_info *,GtkTreeIter *);

};

struct field_info {
        char *label;
        char *filepath;      /* What to get out of the data file */
        GtkWidget *display;  /* Where to display the path */
};


enum {
        RCMENU_NONE = 0,
        RCMENU_NEW_CHILD = 100,
        RCMENU_NEW_SIBLING = 101,
        RCMENU_DELETE = 102
};



/* gui.c */
void destroy(GtkWidget *widget,gpointer data);
void gui_error_msg(char *msg);
GtkWidget *msg_dialog(char *title,char *msg1,GtkSignalFunc func1,char *msg2,GtkSignalFunc func2);
void msg_add(GtkWidget *text,char *format,...);
void entry_set_digit(GtkWidget *widget,int data);
char *gui_ask(const char *msg_string, GtkWidget *main_window);
GtkWidget *new_text_box_large(GtkWidget *hbox,char *name,gboolean fill);
GtkWidget *new_text_box(GtkWidget *hbox,char *name,gboolean fill);
GtkWidget *new_row(GtkWidget *vbox);
GtkWidget *new_col(GtkWidget *hbox);
GtkWidget *new_button(GtkWidget *button,char *label,GtkSignalFunc clicked);
void destroy_window(GtkWidget *w);
GtkWidget *new_combo(GtkWidget *box,GList *list);

GtkWidget *combo_dialog(struct combo_info *cinfo);
gboolean gui_confirm_dialog(const char *msg);

/* Displays a warning message that an error has occured */
void gui_warning_msg(const char *msg);
void gui_message(const char *msg,...);

void rcmenu_edit_callback(gpointer data,guint action,GtkWidget *widget);
void rcmenu_delete_callback(gpointer data,guint action,GtkWidget *widget);
void rcmenu_new_callback(gpointer data,guint action,GtkWidget *widget);


#endif /* GUI_H */
