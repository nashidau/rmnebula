#ifndef CHARACTER_H
#define CHARACTER_H

/* New Stuff */
void
characteristic_update(GtkCellRendererText * cell,
            const gchar * path_string,
            const gchar * new_text, gpointer data);
typedef enum {
        CHAR_NAME,
        CHAR_FULLNAME,
        CHAR_VALUE,
        CHAR_EDITABLE
} char_store_number;

struct store_info char_store_info[] = {
        { CHAR_NAME,     G_TYPE_STRING, "", 0, 0 ,0},
        { CHAR_FULLNAME, G_TYPE_STRING, "", 0, 0 ,0 },
        { CHAR_VALUE,    G_TYPE_STRING, "", 0, 0,0 },
        { CHAR_EDITABLE, G_TYPE_BOOLEAN, "", 0, 0,0 },
        { -1, 0, NULL, 0, 0,0 }
};

struct view_info char_view_info[] = {
        { "Characteristic", CHAR_NAME,  "text",0, 0,0 },
        { "Value",          CHAR_VALUE, "text",0, CHAR_EDITABLE,(GtkSignalFunc)characteristic_update },
        { NULL,             0,          NULL  , 0,0, 0 },
};


GtkTreeStore *charstore;
GtkWidget *charview;

void characteristic_init(GtkWidget *box);

void increase_level(int lvl_inc);

#endif /* CHARACTER_H */

