#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <gtk/gtk.h>
#include <gnome.h>

/* Creates a label and puts it in a table */
GtkWidget * gui_new_label_in_table(const char * labelString,GtkWidget * table,guint left,guint right,guint top,guint bottom);

/* Creates a checkbutton and puts it in a table */
GtkWidget * gui_new_checkbutton_in_table(const char * labelString, GtkWidget * table, guint left, guint right, guint top, guint bottom);

/* Creates a button and puts it in a table */
GtkWidget * gui_new_button_in_table(const char * labelString, GtkWidget * table, guint left, guint right, guint top, guint bottom);

/* Creates a text entry and puts it in a table */
GtkWidget * gui_new_entry_in_table(GtkWidget * table,guint left,guint right,guint top,guint bottom);

/* Creates a checkbutton and puts it in a box */
GtkWidget * gui_new_checkbutton_in_box(const char * labelString, GtkWidget * box);

#endif

