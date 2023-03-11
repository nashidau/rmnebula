#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgnomeui/libgnomeui.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "options.h"
#include "gui.h"
#include "print.h"
#include "parse.h"
#include "file.h"
#include "xslt.h"

int print_dialog(void);
static GtkWidget *print_create_dialog(void);
static void print_dialog_printer(void);
static void print_dialog_file(void);

/* The dialog buttons to select printer or file */
static GtkWidget *printer,*file;
/* The values associated with the dialogs */
static GtkWidget *printername,*filename;

/* The main program window */
extern GtkWidget *window;

enum {
        PRINT_CANCEL = -1,
        PRINT_PREVIEW = 0,
        PRINT_PRINT = 1,
        PRINT_FILE = 2,
};


void
print_cb(const char *xslt,void (*savefn)(struct fileinfo *)){
        char savefile[20],*outfile;
        int fd,fd2,mode = 0;
        struct fileinfo finfo = { 0,0};
        char *command = NULL;
        const char *printcmd;

        /* Do the print dialog */
        mode = print_dialog();
        if (mode == -1)
                return;

        if (mode == PRINT_PRINT){
                outfile = g_strdup("/tmp/nebprnXXXXXX");
                fd2 = g_mkstemp(outfile);
        } else {
                outfile = g_strdup(gtk_entry_get_text(GTK_ENTRY(filename)));
                fd2 = creat(outfile,0666);
        }

        /* Set up the temp files */
        strcpy(savefile,"/tmp/nebXXXXXX");
        fd = g_mkstemp(savefile);
        finfo.name = savefile;

        /* Save file to a temp file */
        savefn(&finfo);
	
	/* Process with XSLT and save */
	xslt_process_file(xslt, savefile, fd2);

        if (mode == PRINT_PRINT){
                g_free(command);
                command = NULL;
                printcmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(printername)));
                if (printcmd != NULL){
                        command = g_strdup_printf("%s %s",printcmd,outfile);
                        system(command);
                }
                unlink(outfile);
        }

        g_free(command);
        close(fd);
        close(fd2);
        unlink(savefile);
        g_free(outfile);


}


int
print_dialog(void){
	static GtkWidget *dialog;
        guint response;

        if (dialog == NULL){
                dialog = print_create_dialog();
        }

        response = gnome_dialog_run(GNOME_DIALOG(dialog));
        gtk_widget_hide(dialog);

        if (response == 0){
                /* Cancel - just return */
                return PRINT_CANCEL;
        }

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(printer)) == TRUE){
                return PRINT_PRINT;
        } else {
                return PRINT_FILE;
        }


}

static GtkWidget *
print_create_dialog(void){
        GtkWidget *dialog;
        GtkWidget *table;


        dialog = gnome_dialog_new("Print",GTK_STOCK_CANCEL,GTK_STOCK_OK,NULL);
	gnome_dialog_set_parent(GNOME_DIALOG(dialog),GTK_WINDOW(window));
	gnome_dialog_close_hides(GNOME_DIALOG(dialog),TRUE);

        /* Now create a dialog */
        table = gtk_table_new(2,2,FALSE);
        gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox),
                        table,TRUE,TRUE,0);

        printer = gtk_radio_button_new_with_label(NULL,"Printer: ");
        g_signal_connect(G_OBJECT(printer),"toggled",
                        G_CALLBACK(print_dialog_printer),NULL);
        gtk_table_attach_defaults(GTK_TABLE(table),printer,0,1,0,1);
        printername = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(printername),
                        config_get_string_default(FILE_OPTION "printcommand",
                                "lpr"));
        gtk_table_attach_defaults(GTK_TABLE(table),printername,1,2,0,1);

        file = gtk_radio_button_new_with_label_from_widget(
                        GTK_RADIO_BUTTON(printer),"File: ");
        g_signal_connect(G_OBJECT(file),"toggled",
                        G_CALLBACK(print_dialog_file),NULL);
        gtk_table_attach_defaults(GTK_TABLE(table),file,0,1,1,2);
        filename = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(filename),"nebula.ps");
        gtk_table_attach_defaults(GTK_TABLE(table),filename,1,2,1,2);

        print_dialog_printer();

        gtk_widget_show_all(dialog);

        return dialog;
}

static void
print_dialog_printer(void){
        gtk_widget_set_sensitive(GTK_WIDGET(filename),FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(printername),TRUE);
}
static void
print_dialog_file(void){
        gtk_widget_set_sensitive(GTK_WIDGET(filename),TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(printername),FALSE);
}
