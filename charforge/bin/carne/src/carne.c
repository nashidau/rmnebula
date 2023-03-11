#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include "comms.h"
#include "carne.h"
#include "levelup.h"

#define _(x) x

static void create_main_window(struct state *state);
static gint delete_event(GtkWidget *window, GdkEvent *event, gpointer data);

/* The Main menu */
/* FIXME: In future this will probably have to be built using sub menujs to
 * pass data around more effectively */

GtkItemFactoryEntry mainmenu[] = {
{ "/_File",	     NULL,	NULL,	0, "<Branch>" },
{ "/File/tear1",    NULL,      NULL,         0, "<Tearoff>" },
{ "/File/_Connect",     "<CTRL>N", (void*)comms_connect,     1, "<Item>" },
{ "/File/sep1",     NULL,      NULL,         0, "<Separator>" },
{ "/File/_Quit",    "<CTRL>Q", (void*)gtk_exit, 0, "<Item>"},
{ "/_Character",	     NULL,	NULL,	0, "<Branch>" },
{ "/Character/tear1",    NULL,      NULL,         0, "<Tearoff>" },
{ "/Character/_Levelup",     "<CTRL>L", (void*)levelup_start,     1, "<Item>" },

};



int 
main(int argc, char **argv) {
	const char *file;
	struct state *state;

	state = g_malloc0(sizeof(struct state));
	
	gtk_init(&argc, &argv);

	create_main_window(state);

	gtk_main();
	
	return 0;
}


/**
 * Create the tirno main window.
 *
 * Attach delete event, create menu, draw in main window
 *
 * Note this function will exit on error 
 *
 * Args: None
 * Returns: Nothing.
 */
static void
create_main_window(struct state *state){
	GtkWidget *window;
	GtkWidget *menuwidget;
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	GtkWidget *main_vbox;
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	

	g_signal_connect(G_OBJECT(window), "delete_event",
		       	G_CALLBACK(delete_event), NULL);


	/* Make an accelerator group (shortcut keys) */
	accel_group = gtk_accel_group_new ();

	/* Make an ItemFactory (that makes a menubar) */
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
			accel_group);

	/* This function generates the menu items. Pass the item factory,
	   the number of items in the array, the array itself, and any
	   callback data for the the menu items. */
	gtk_item_factory_create_items (item_factory, 
			(int)(sizeof(mainmenu) / sizeof(mainmenu[0])),
			mainmenu, state);

	/* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	/* Finally, return the actual menu bar created by the item factory. */
	menuwidget = gtk_item_factory_get_widget (item_factory, "<main>");
	main_vbox = gtk_vbox_new (FALSE, 1);
	gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
	gtk_container_add (GTK_CONTAINER (window), main_vbox);


	/* Pack it all together */
	gtk_box_pack_start(GTK_BOX(main_vbox), menuwidget, FALSE, TRUE, 0);
	
	
	gtk_widget_show_all(window);	

}




/**
 * Close window 
 *
 */
static gint 
delete_event(GtkWidget *window, GdkEvent *event, gpointer data) {
	GtkWidget *dialog,*label;
	gint run;

	dialog = gtk_dialog_new_with_buttons(_("Quit Carne"), 
			GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_YES,GTK_RESPONSE_YES,
			GTK_STOCK_NO,GTK_RESPONSE_NO,NULL);
	label = gtk_label_new(_("Do you wish to quit Carne?"));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show(label);
	
	run = gtk_dialog_run(GTK_DIALOG(dialog));

	if (run == GTK_RESPONSE_YES){
		/* Quit */
		gtk_exit(1);
		return FALSE;
	}
	
	/* Return TRUE: Don't destroy 
	 * Return FALSE: Destroy
	 * */
	return TRUE;
}


