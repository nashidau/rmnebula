#include <assert.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "main.h"
#include "comms.h"
#include "profession.h"

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_delete1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_professionmenu_changed              (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data)
{

}


void
on_profession_select_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *optionmenu;
	GtkWidget *menu,*menuitem;
	const char *filename;

	optionmenu = lookup_widget(state->MainWindow,"professionmenu");
	assert(optionmenu != NULL);


	printf("You've selected someone: Well done\n");

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optionmenu));
	assert(menu != NULL);

	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	assert(menuitem != NULL);

	filename = g_object_get_data(G_OBJECT(menuitem),"ref");

	printf("Filename is %s\n",filename);
}


gboolean
on_MainWindow_destroy_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


void
on_connect_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	assert(state != NULL);

	comms_connect(state);

	/* FIXME: String hardcoding */
	send_request("<professions>",RESPONSE_FILE,
			"professions.xml",profession_get_reply,NULL);

}

