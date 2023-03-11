#ifndef GUITOOLS_H
#include "guitools.h"
#endif

GtkWidget *
gui_new_label_in_table(const char * labelString, GtkWidget * table, guint left, guint right, guint top, guint bottom)
{
	GtkWidget * label;
	
	label = gtk_label_new(labelString);
	gtk_widget_show(label);
    gtk_table_attach (GTK_TABLE(table), label, left, right, top, bottom,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC(label), 0, 0.5);
	
	return label;
}

GtkWidget *
gui_new_checkbutton_in_table(const char * labelString, GtkWidget * table, guint left, guint right, guint top, guint bottom)
{
	GtkWidget * checkbutton;
    checkbutton = gtk_check_button_new_with_label (labelString);
    gtk_table_attach (GTK_TABLE(table), checkbutton, left, right, top, bottom,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
	return checkbutton;
}

GtkWidget *
gui_new_button_in_table(const char * labelString, GtkWidget * table, guint left, guint right, guint top, guint bottom)
{
	GtkWidget * button;
    button = gtk_button_new_with_label (labelString);
    gtk_table_attach (GTK_TABLE(table), button, left, right, top, bottom,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
	return button;
}


GtkWidget *
gui_new_entry_in_table(GtkWidget * table,guint left,guint right,guint top,guint bottom)
{
	GtkWidget * entry;
    entry = gtk_entry_new ();
    gtk_widget_show (entry);
    gtk_table_attach (GTK_TABLE(table), entry, left, right, top, bottom,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
	return entry;
}

GtkWidget *
gui_new_checkbutton_in_box(const char * labelString, GtkWidget * box)
{
	GtkWidget * checkbutton = gtk_check_button_new_with_label (labelString);
	gtk_box_pack_start(GTK_BOX(box),checkbutton,FALSE,FALSE,0);
	
	return checkbutton;
}
