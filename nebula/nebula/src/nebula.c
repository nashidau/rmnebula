/*
 * Nebula
 *
 * Rolemaster Character Sheet Generator
 *
 * Written for gtk+
 *
 * Released under the GPL - See COPYING for details
 *
 * By Brett Nash <nash@nash.nu>
 */

#include "nebula.h"
#include "libneb.h"
#include "credits.h"
#include "nebfile.h"

struct tags parsers[] = {
        { "prefix",     set_prefix_xml },
        { "realms",     prof_add_realm_xml },
        { "data",       characteristic_add_xml },
        { "stats",      stat_insert },
        { "rrs",        rr_insert },
        { "skills",     skill_add_xml },
        { "items",      xml_insert_item },
        { "choices",    choice_parse },
        { "create",     create_choice },
        { NULL,         NULL }
};
struct option_info option_info[] = {
        {
                CREATION_OPTION "appearance",
                "Use RMSS Appearance",
                "Use the RMMS (Pr - 25 + 5d10) instead of\n"
                        "the mentalism companion method (d100 + 3*Pr)",
                G_TYPE_BOOLEAN, 0,
        },{
                CREATION_OPTION "fixed",
                "Use fixed points",
                "Use 660 points instead of the default\n"
                        "default of 600+10d10",
                G_TYPE_BOOLEAN, 0,
        },{
                CREATION_OPTION "fixedpotentials",
                "Use fixed potentials",
                "Use the RMSS option to give a fixed potential\n"
                        "rather then the dice based system",
                G_TYPE_BOOLEAN, 0,
        },{
                CREATION_OPTION "noprs",
                "Ignore PR minimums",
                "Ignore the requirement of 90 in your Prime Requisites",
                G_TYPE_BOOLEAN, 0,
        },{
                STAT_OPTION "potgain",
                "Allow Potential gain",
                "Allow Potential stats to increase with stat gain",
                G_TYPE_BOOLEAN, 0,
        },{
                STAT_OPTION "appearance",
                "Appearance",
                "Use the Appearance stat",
                G_TYPE_BOOLEAN, 0,
        },{
                STAT_OPTION "sanity",
                "Sanity",
                "Use the Sanity stat (from Mentalism Companion)",
                G_TYPE_BOOLEAN, 0,
        },{
                STAT_OPTION "willpower",
                "Willpower",
                "Use the Willpower stat (from Treasure Companion)",
                G_TYPE_BOOLEAN, 0,
        },{
                SKILL_OPTION "individualstats",
                "Individual skill stat bonuses",
                "Each individual skill has a stat bonus, instead of "
                        "the third listed in the category",
                G_TYPE_BOOLEAN, 0,
        },{
                FILE_OPTION "icon",
                "Icon",
                "The icon shown for Nebula",
                G_TYPE_STRING, 0,
        },{
                FILE_OPTION "logo",
                "Logo",
                "The logo shown for Nebula in About",
                G_TYPE_STRING, 0,
        },{
                FILE_OPTION "datapath",
                "Data path",
                "Where to look for data files\n"
                        "Use : to separate items",
                G_TYPE_STRING, 0,
        },{
                FILE_OPTION "printcommand",
                "Print Command",
                "What command to use for printing,\n"
                        "includes printer (-P<name>)",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "translator",
                "XSLT Processor",
                "Path to the XSLT processor\n"
                        "/usr/bin/xalan",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "xslopt",
                "XSLT option flag",
                "Flag translator uses to indicate XSLT file",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "inopt",
                "In option flag",
                "Flag translator uses to indicate input file",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "outopt",
                "Out option flag",
                "Flag translator uses to indicate input file",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "nebula/postscript",
                "Postscript XSLT",
                "XSLT to convert to postscript",
                G_TYPE_STRING, 0,
        },{
                XSL_OPTION "nebula/html",
                "HTML XSLT",
                "XSLT to convert to Hyper-Text Markup Language",
                G_TYPE_STRING, 0,
        },
        {        NULL,NULL,NULL,G_TYPE_NONE,NULL },
};


char *loadfile = NULL;

GtkWidget *window;
GdkPixbuf *icon;
GdkPixbuf *igor;

int main(int argc, char **argv){
        int i;

	   	/* Stuff to init i18n support */
		bindtextdomain(PACKAGE, GNOMELOCALEDIR);
		bind_textdomain_codeset (PACKAGE, "UTF-8");
		textdomain(PACKAGE);
		/* This shouldn't be needed, as gnome_program_init already does it */
        /* gtk_set_locale(); */

        /* Parse command line */
        gnome_program_init("Nebula",VERSION_STRING,
                        LIBGNOMEUI_MODULE,argc,argv,NULL);

        /* Initialse the file load/save callbacks */
        nebfile_init();

        /* init tool tips */
        init_tips();

        config_init();
        options_init(option_info);

        /* Seed random pool */
        dice_init();

        for (i = 1 ; i < argc ; i ++){
                if (*argv[i] != '-'){
                        loadfile = argv[i];
                        break; /* Stop looking */
                }
        }

        /* Open window */
        open_splash();
 
        /* Event Loop */
        gtk_main();
       
        /* Bye... */
        exit(EXIT_SUCCESS);
}

GtkWidget *showbutton;
void
open_splash(void){
        GtkWidget *dialog;
        const char *accepted;

        accepted = config_get_string_default(LICENCE,"None");
        if (strcmp(accepted,VERSION_STRING) == 0){
                create_window();
                return;
        }        

        dialog = msg_dialog("Nebula",_("I Agree"),GTK_SIGNAL_FUNC(accept_licence),
                        _("I Don't agree"),(GtkSignalFunc)gtk_exit);
        gtk_widget_set_usize(dialog,350,300);
        msg_add(dialog,_("Welcome to Nebula version " VERSION_STRING "\n\n"));
        msg_add(dialog,_("Copyright 2001-2002 Brett Nash\n\n"
                        "Nebula (the program and source files) comes "
                        "with ABSOLUTELY NO WARRANTY; "
                        "This is free software, and you are welcome to "
                        "redistribute it under certain conditions; "
                        "See COPYING for details.  This excludes data "
                        "files\n\n"));
        msg_add(dialog,_("Additionally certain components are copyright I.C.E "
                        "(Or other individuals).  Thus you agree that you "
                        "either own the relevant materials or have obtained "
                        "an appropriate licence from such bodies.  These "
                        "materials are not 'free'."));
        showbutton = gtk_check_button_new_with_label("Don't show this again");
        gtk_box_pack_start(GTK_BOX(dialog->parent->parent),showbutton,
                        FALSE,TRUE,2);
        gtk_widget_show(showbutton);

}



void
accept_licence(GtkWidget *widget){
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(showbutton)) == TRUE)
                config_set_string(LICENCE,VERSION_STRING);
        
        destroy_window(widget);
        create_window();
}


int
create_window(void){
        GtkWidget *mainbox;

        //window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        window = gnome_app_new("Nebula","Nebula");
        gtk_window_set_default_size(GTK_WINDOW(window),450,470);
        gtk_window_set_title(GTK_WINDOW(window),"Nebula");

        /* Register close window handler */
        gtk_signal_connect(GTK_OBJECT(window),"delete_event",
                        GTK_SIGNAL_FUNC(delete_event_cb),NULL);

        /* Add a box */
        mainbox = gtk_vbox_new(FALSE,2);
        gnome_app_set_contents(GNOME_APP(window),mainbox);

        /* Menu... */
        create_menu(window,mainbox);

        /* Create the Main Notebook */
        create_main_page(mainbox);

        /* Character info screen */
        characteristic_init(charpage);

        /* Stats */
        stat_init(statpage);

        /* Skill screen */
        skill_init(skillpage);

        /* Init RR boxes */
        rr_init(defensepage);

        /* item page */
        /* This crashes stuff */
        /*init_items(itempage); / * Code deactivated internally */

        /* Show it */
        gtk_widget_show_all(window);

        /* Load graphics and stuff */
        init_pixmaps();

        /* Anything to load? */
        if (loadfile != NULL){
                file_preload(loadfile);
        }

        return 0;

}

int delete_event_cb(GtkWidget *window,GdkEventAny *e, gpointer data){
        data = NULL;
        e = NULL;
        window = NULL;

        gtk_exit(0);
        return FALSE;
}


void
init_pixmaps(void){
        const char *iconfile;
        const char *logofile;
        GError *err = NULL;

        iconfile = config_get_string(FILE_OPTION "icon");
        if (iconfile == NULL){
                config_set_string(FILE_OPTION "icon",D10_PIXMAP);
                iconfile = D10_PIXMAP;
        }

        icon = gdk_pixbuf_new_from_file(iconfile,&err);
        if (icon == NULL)
                fprintf(stderr,"Warning: No app icon\n");
        else
                gtk_window_set_icon(GTK_WINDOW(window),icon);

        if (err != NULL){
                fprintf(stderr, "Unable to read %s: %s\n",
                                iconfile,err->message);
                g_error_free(err);
        }

        err = NULL;

        logofile = config_get_string(FILE_OPTION "logo");
        if (logofile == NULL){
                config_set_string(FILE_OPTION "logo",IGOR_PIXMAP);
                logofile = IGOR_PIXMAP;
        }

        igor = gdk_pixbuf_new_from_file(logofile,&err);

        if (err != NULL){
                fprintf(stderr, "Unable to read %s: %s\n",
                                logofile,err->message);
                g_error_free(err);
                igor = NULL;
        }
}

/*
 * nebula_about
 *
 * Shows the standard gnome_about_dialog for nebula.
 *
 */
void
nebula_about(void){
        static GtkWidget *about;

        if (about == NULL){
                about = gnome_about_new("Nebula", version,
                                copyright, comments,
                                (const char **)authors,
                                (const char **)documentation,
                                translation, igor);
        	gtk_signal_connect (GTK_OBJECT (about), "destroy",
				    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
				    &about);
	}

        gtk_window_set_transient_for(GTK_WINDOW(about),GTK_WINDOW(window));
	gtk_widget_show_now(about);
        gtk_widget_grab_focus(about);
}
