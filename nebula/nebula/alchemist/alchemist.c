/*
 * Alchemist
 *
 * Item File editor, and creator
 */
#include <gtk/gtk.h>
#include <stdlib.h>
#include <gnome.h>
#include <assert.h>
#include "libneb.h"
#include "alchemist.h"
#include "listitems.h"
#include "intelligence.h"
#include "general.h"
#include "logwindow.h"
#include "spells.h"

#ifndef AL_ITEM_SKILLS_H
#include "skills.h"
#endif

/* Functions */
static void al_create_window (void);
static void al_file_new (void);
static void al_print (void);
static void al_about (void);
static struct fileinfo *al_open (const char *file);
static int al_close (void);
static void al_save (struct fileinfo *finfo);
static void al_refresh (void);
void items_load (element_t * el);

void al_create_general (GtkNotebook * notebook);
void al_create_effects (GtkNotebook * notebook);
void al_create_skills (GtkNotebook * notebook);

struct tags parsers[] = {
	{"items", items_load},
	{NULL, NULL}
};



/* Variables */
GtkWidget *window;

GdkPixbuf *icon;
GdkPixbuf *logo;


static GtkItemFactoryEntry menu_items[] = {
	{
	 "/_File", NULL,
	 NULL, 0,
	 "<Branch>", 0},
	/* File opening options */
	{
		"/File/_New", "<control>N",
		 (GtkItemFactoryCallback) al_file_new, 0,
		 "<StockItem>", GTK_STOCK_NEW}, {
		"/File/_Open", "<control>O",
		 (GtkItemFactoryCallback)
		 file_open_cb, 0,
		 "<StockItem>", GTK_STOCK_OPEN}, {
		"/File/sep1",
		  NULL,
		  NULL,
		  0,
		"<Separator>",
		  0
	},
	
	/* Saving Functions */
	{
	 "/File/_Save", "<control>S",
	 (GtkItemFactoryCallback) file_save_cb, 0,
	 "<StockItem>", GTK_STOCK_SAVE}, {
	 "/File/Save _As", "<control>A",
	  (GtkItemFactoryCallback)
	  file_save_as_cb, 0,
	  "<StockItem>", GTK_STOCK_SAVE_AS}, {
	 "/File/_Revert",
	   NULL,
	   (GtkItemFactoryCallback) file_revert_cb, 0,
	   "<StockItem>",
		GTK_STOCK_REVERT_TO_SAVED},
		{
	 "/File/sep2", NULL,
	 NULL, 0,
	 "<Separator>", 0},
	/* Printing */
	{
	 "/File/_Print", NULL,
	 (GtkItemFactoryCallback) al_print, 0,
	 "<StockItem>", GTK_STOCK_PRINT,
	 }, {
	     "/File/sep3", NULL,
	     NULL, 0,
	     "<Separator>", 0},
	/* Close operations */
	{
	 "/File/_Close", "<control>W",
	 (GtkItemFactoryCallback) file_close_cb, 0,
	 "<StockItem>", GTK_STOCK_CLOSE},
	{
	 "/File/_Quit", "<control>Q",
	 (GtkItemFactoryCallback) gtk_exit, 0,
	 "<StockItem>", GTK_STOCK_QUIT},
	/* Edit Menu */
	{"/_Edit", NULL,
	 NULL, 0,
	 "<Branch>", 0},
	{"/Edit/_Preferences", NULL,
	 (GtkItemFactoryCallback) options_creation, 0,
	 "<StockItem>", GTK_STOCK_PREFERENCES},
	{"/_Help", NULL, NULL, 0, "<Branch>", 0},
	{
	 "/Help/_About", NULL,
	 (GtkItemFactoryCallback) al_about, 0,
	 "<StockItem>", GNOME_STOCK_ABOUT},
};


struct option_info options[] = {
	{
	 XSL_OPTION "translator",
	 "XSLT Processor",
	 "Path to the XSLT processor\n" "/usr/bin/xalan",
	 G_TYPE_STRING, 0,
	 }, {
	     XSL_OPTION "worldbuilder/postscript",
	     "Postscript XSLT",
	     "XSLT to convert to postscript",
	     G_TYPE_STRING, 0,
	     }, {
		 XSL_OPTION "worldbuilder/html",
		 "HTML XSLT",
		 "XSLT to convert to Hyper-Text Markup Language",
		 G_TYPE_STRING, 0,
		 },
	{NULL, NULL, NULL, G_TYPE_NONE, NULL},

};

struct fileops fileops;


/*
 * Code
 */



int
main (int argc, char **argv)
{
	/* Stuff to init i18n support */
	/*
	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE); */

	fileops.docname = g_strdup ("items");
	fileops.printgconfkey = g_strdup (XSL_OPTION "stardust/postscript");
	fileops.defaultprintkey = g_strdup ("/home/nash/work/rolemaster/XML/"
					    "spelllist/spellllisttops.xsl");
	fileops.create = NULL;
	fileops.open = al_open;
	fileops.save = al_save;
	fileops.close = al_close;
	fileops.refresh = al_refresh;
	fileops.getfilename = NULL;

	libneb_init ("Alchemist", "0.1", argc, argv, &fileops, options);

	//libneb_icons_init("../pixmaps/worldmap-small.png",&logo,
	//                "../pixmaps/jigsawpiece-48x48.png",&icon,NULL);

	al_create_window ();

	if (icon != NULL)
	{
		gtk_window_set_icon (GTK_WINDOW (window), icon);
	}

	gtk_main ();

	exit (EXIT_SUCCESS);
}


static void
al_create_window (void)
{
	GtkAccelGroup *accel_group;
	GtkNotebook   *notebook;
	GtkHPaned     *paned;
	GtkWidget     *widget;
	GtkWidget     *main_vertical_layout;
//        GtkWidget *mainbox;
	int n;
	GtkItemFactory *item_factory;


	/* Make a window */
	window = gnome_app_new ("Alchemist", "0.1");
	gtk_window_set_default_size (GTK_WINDOW (window), 450, 470);
	gtk_window_set_title (GTK_WINDOW (window), "Alchemist");

	/* Make a menu */
	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	g_object_unref (accel_group);
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR,
					     "<main>", accel_group);
	n = sizeof (menu_items) / sizeof (menu_items[0]);
	gtk_item_factory_create_items (item_factory, n, menu_items, NULL);
	widget = gtk_item_factory_get_widget (item_factory, "<main>");
	gnome_app_set_menus (GNOME_APP (window), GTK_MENU_BAR (widget));

	/* Main Vertical pan */
	main_vertical_layout = gtk_vpaned_new();
	gnome_app_set_contents (GNOME_APP (window), main_vertical_layout);

	/* Left/Right pan */
	paned = GTK_HPANED (gtk_hpaned_new ());
	gtk_paned_pack1 (GTK_PANED (main_vertical_layout), GTK_WIDGET (paned), FALSE, TRUE);
	
	/* The log window */
	al_logwindow_create(main_vertical_layout);

	/* Right pan -> item list */
	al_create_listitems (paned);

	/* Left pan -> notebook */
	notebook = GTK_NOTEBOOK (gtk_notebook_new ());

	al_create_general      (notebook);
	al_create_effects      (notebook);
	al_create_intelligence (notebook);
	al_create_skills       (notebook);
	al_create_spells       (notebook);
    
	gtk_paned_pack2 (GTK_PANED (paned), GTK_WIDGET (notebook), FALSE, TRUE);

	gtk_widget_show_all (window);
    gtk_notebook_set_current_page(notebook,0);
    
	return;
}

void
al_create_effects (GtkNotebook * notebook)
{
	GtkButton *button = GTK_BUTTON (gtk_button_new ());
	GtkLabel *label = GTK_LABEL (gtk_label_new ("Effects"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  GTK_WIDGET (button), GTK_WIDGET (label));
}

static void
al_file_new (void)
{
	al_logwindow_add_text ("New File - Not implemented\n");
}


static void
al_print (void)
{
	al_logwindow_add_text ("Print file - Not implemented\n");
}

static void
al_about (void)
{
	static GtkWidget *about;
	const char *copyright = "Copyright 2002 Brett Nash/Sylvain Glaize";
	const char *authors[] = {
		"Sylvain Glaize <mokona@puupuu.org>",
		"Brett Nash <nash@nash.nu>",
		NULL
	};
	const char *comments = NULL;
	const char *documentation = NULL;
	const char *translation = NULL;


	if (about == NULL)
	{
		about = gnome_about_new (ALCHEMIST_NAME, ALCHEMIST_VERSION,
					 copyright, comments,
					 (const char **) authors,
					 (const char **) documentation,
					 translation, logo);
		gtk_signal_connect (GTK_OBJECT (about), "destroy",
				    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
				    &about);
	}

	gtk_window_set_transient_for (GTK_WINDOW (about),
				      GTK_WINDOW (window));
	gtk_widget_show_now (about);

}


void
al_skills_init (GtkNotebook * notebook)
{
};




/*
 * File Operations
 */

static struct fileinfo *
al_open (const char *file)
{
	struct fileinfo *finfo;

	parse_file (file, parsers);

	finfo = g_malloc (sizeof (struct fileinfo));
	finfo->name = g_strdup (file);
	finfo->state = FILE_CLEAN;

	return finfo;
}

static int
al_close (void)
{
	al_logwindow_add_text ("File close - Not implemented\n");

	//tree_listinfo_clear(listinfo);

	return 0;
}


static void
al_save (struct fileinfo *finfo)
{
	//element_t *el;

	al_logwindow_add_text ("file save\n");

	assert (finfo != NULL);
	assert (finfo->name != NULL);

#if 0
	el = create_element (NULL, "rmssgenre", ETYPE_ELEMENT);
	set_attribute (el, "id", "worldbuilder-001");
	set_attribute (el, "version", WORLDBUILDER_VERSION);

	tree_listinfo_save (listinfo, el);

	realms_save (el);

	/* skills_save(el); */

	alena_save (finfo->name, el, NULL);
#endif

}

static void
al_refresh (void)
{
	al_logwindow_add_text ("refresh\n");
}


void
items_load (element_t * all)
{
	GList *elems;
	element_t *el;
	
	/* Reset the log window */
	al_logwindow_reset();

	/* Clear all loaded objects */
	al_item_list_empty();

	/* Lets load the basic bits first */
	elems = all->elements;
	for (elems = all->elements; elems != NULL; elems = elems->next)
	{
		el = elems->data;

		if (strcmp (el->name, "item") == 0)
		{
			al_list_add_one_item (el);
		}
		else
		{
			/* Local log buffer */
			char log_text[200];
			sprintf (log_text,"Unparsed element : %s\n", el->name);
			al_logwindow_add_text(log_text);
		}
	}
	
	al_item_list_commit();
}
