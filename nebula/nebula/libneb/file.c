/*
 * File.c
 *
 * File operations for Nebula
 *
 * FIXME:
 *  - Function names should be changed to be of form file_<action>
 *  - Comment this file
 */
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "libneb.h"

struct fileinfo *finfo;
GtkWidget *filewidget;
static GtkWidget *file_new_file_selection(void);
static const char *get_filename(struct fileinfo *);

static struct fileops fileops;

/*
 * Set the file operations to be called on save, load or close events
 */
void
file_set_fileops(struct fileops *nfileops){
        fileops = *nfileops;

        if (fileops.docname == NULL)
                fileops.docname = "'character'";
        if (fileops.getfilename == NULL)
                fileops.getfilename = get_filename;
}


void
file_preload(char *file){
        finfo = fileops.open(file);
}


static const char *
get_filename(struct fileinfo *finfo){
        char *name;

        assert(finfo != NULL);

        if (finfo->name == NULL)
                return "Unnamed";

        name = strrchr(finfo->name,'/');
        if (name != NULL)
                return name + 1;
        return finfo->name;
}

/*
 * file_open_cb
 *
 * Called from menu.  Requests user give a filename and attempts to
 * load that file.
 *
 * Args:
 *      None
 * Returns:
 *      Nothing
 * Will modify global finfo structure on a successful load
 */
void
file_open_cb(void){
        const char *filename;
        char *str;
        int res;

        if (filewidget == NULL){
                filewidget = file_new_file_selection();
        }

        if (finfo && finfo->state == FILE_MODIFIED){
                str = g_strdup_printf("Are you sure you want to load?\n"
                                "Your changes to %s will be lost.",
                                g_strrstr(finfo->name,"/") ?
                                g_strrstr(finfo->name,"/") + 1 :
                                finfo->name);
                res = gui_confirm_dialog(str);
                if (res == FALSE)
                        return;
        }

        gtk_widget_show_all(filewidget);
        res = gtk_dialog_run(GTK_DIALOG(filewidget));
        gtk_widget_hide(filewidget);
        if (res != GTK_RESPONSE_OK){
                return;
        }

        filename = gtk_file_selection_get_filename(
                                         GTK_FILE_SELECTION(filewidget));

        if (finfo){
                fileops.close();
                g_free(finfo->name);
                g_free(finfo);
        }

       	finfo = fileops.open(filename);

        if (finfo == NULL){
                gui_warning_msg("File failed to load - "
                                "file open returned NULL");
        }

        if (fileops.refresh != NULL)
                fileops.refresh();

        finfo->state = FILE_CLEAN; /* By definition */
}

void
file_save_cb(void){

        if (finfo == NULL || finfo->state == FILE_NONE){
                gui_warning_msg("No file to save!");
                return;
        }

        if (finfo->name == NULL){
                /* Same as file_save_as_cb */
                file_save_as_cb();
                return;
        }

        fileops.save(finfo);

        finfo->state = FILE_CLEAN;
}

void
file_save_as_cb(void){
        int res;
        const char *filename;

        if (finfo == NULL || finfo->state == FILE_NONE){
                gui_warning_msg("No file to save!");
                return;
        }
        if (filewidget == NULL){
                filewidget = file_new_file_selection();
        }

        gtk_widget_show_all(filewidget);
        res = gtk_dialog_run(GTK_DIALOG(filewidget));
        gtk_widget_hide(filewidget);
        if (res != GTK_RESPONSE_OK){
                return;
        }

        filename = gtk_file_selection_get_filename(
                                         GTK_FILE_SELECTION(filewidget));

        g_free(finfo->name);
        finfo->name = g_strdup(filename);
        fileops.save(finfo);

        finfo->state = FILE_CLEAN;

}



void
file_close_cb(void){
        int rv;
        char *msg;
        GtkWidget *dialog;
        GtkWidget *label;

        if (finfo == NULL)
                return;

        if (finfo->state == FILE_NONE)
                return;

        if (finfo != NULL && finfo->state == FILE_MODIFIED){
                msg = g_strdup_printf("<span weight=\"bold\" size=\"larger\">"
                                "Save changes to %s '%s' before "
                                "closing?</span>\n\n"
                                "If you close without saving, any changes "
                                "will be lost.",
                                fileops.docname,
                                fileops.getfilename(finfo));
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_QUESTION,
                                GTK_BUTTONS_NONE," ");
                label = gtk_label_new(NULL);
                gtk_widget_show(label);
                gtk_label_set_markup(GTK_LABEL(GTK_MESSAGE_DIALOG
                                        (dialog)->label),msg);
                gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,
                                TRUE,FALSE,0);
                gtk_dialog_add_button(GTK_DIALOG(dialog),
                                "Close without Saving",GTK_RESPONSE_CLOSE);
                gtk_dialog_add_button(GTK_DIALOG(dialog),
                                GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
                gtk_dialog_add_button(GTK_DIALOG(dialog),
                                GTK_STOCK_SAVE,GTK_RESPONSE_YES);
                rv = gtk_dialog_run(GTK_DIALOG(dialog));

                /* Now handle what was clicked */
                switch (rv){
                case GTK_RESPONSE_CLOSE:
                        gtk_widget_destroy(dialog);
                        /* Just close/quit - don't save */
                        break;
                case GTK_RESPONSE_CANCEL:
                        /* Don't close, don't quit */
                        gtk_widget_destroy(dialog);
                        /* Fall through */
                case GTK_RESPONSE_DELETE_EVENT:
                        return;
                case GTK_RESPONSE_YES:
                        gtk_widget_destroy(dialog);
                        /* Yes, I want to save the file */
                        file_save_cb();
                        if (finfo->state != FILE_CLEAN)
                                return;
                        break;
                default:
                        g_assert_not_reached();
                }

        }

        fileops.close();

        g_free(finfo->name);
        g_free(finfo);
        finfo = NULL;

        return;
}


void
file_revert_cb(void){
        gboolean res;
        char *name;

        if (finfo == NULL || finfo->state == FILE_NONE){
                gui_warning_msg("Can not Revert.\nNo file loaded.");
                return;
        }

        res = gui_confirm_dialog("Revert to Saved?\nAll changes will be lost.");
        if (res == FALSE)
                return;

        fileops.close();

        name = finfo->name;
        g_free(finfo);
        finfo = fileops.open(name);
        g_free(name);

        finfo->state = FILE_CLEAN;

        return;
}



/*
 * file_modify
 *
 * Should be called by any function who wishes to modify a file.
 *
 * Returns:
 *      true if modification allowed
 *      false if not allowed (r/o or not loaded etc)
 */
gboolean
file_modify(void){

        /* We should _always_ have a loaded file
        assert(finfo != NULL); */
  	if(finfo == NULL)
		return FALSE;

        if (finfo->state == FILE_NONE){
                printf("Can't modify nothing\n");
                return FALSE;
        }
        if (finfo->state == FILE_CLEAN){
                finfo->state = FILE_MODIFIED;
                return TRUE;
        }

        finfo->state = FILE_MODIFIED;
        return TRUE;
}
/*
 * Tells the system you are about to load a new
 * file.  Or create a new file.
 *
 * FIXME: Load on a dirty file should cause a dialog asking if you want to
 * save
 * FIXME: The return value is backwards
 *
 * Returns:
 *      true if you can create a new file
 *      false if a file is loaded
 */
gboolean
file_loaded(void){
        if (finfo == NULL)
                return TRUE;
        switch (finfo->state){
        case FILE_CLEAN:
                file_close_cb();
                /* Fall through */
        case FILE_NONE:
                return TRUE;
        case FILE_MODIFIED:
                return FALSE;
        default:
                printf("Unknown state\n");
                file_close_cb();
                return TRUE;
        }
}
void
file_set_modified(void){
        if (finfo == NULL){
                finfo = g_malloc0(sizeof(struct fileinfo));
        }
        finfo->state = FILE_MODIFIED;
}


static GtkWidget *
file_new_file_selection(void){
        return gtk_file_selection_new("Select File");
}

void
file_new_cb(void){

        if (fileops.create == NULL){
                printf("Create function not defined\n");
                return;
        }

        if (finfo != NULL){
                printf("We have a file\n");
                return;
        }

        finfo = fileops.create();

        return;

}

/*
 * file_quit_cb
 *
 * Confirms if the user wants to quit, then quits.
 * Follows Gnome HIG.
 *
 * Note: Does not correctly handle 'Character', 'Spell List'
 * etc - currently always says 'Character'
 */
void
file_quit_cb(void){
        int rv;
        char *msg;
        GtkWidget *dialog;
        GtkWidget *label;

        if (finfo != NULL && finfo->state == FILE_MODIFIED){
                msg = g_strdup_printf("<span weight=\"bold\" size=\"larger\">"
                                "Save changes to %s '%s' before "
                                "quitting?</span>\n\n"
                                "If you quit without saving, any changes "
                                "will be lost.",
                                fileops.docname,
                                fileops.getfilename(finfo));
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_QUESTION,
                                GTK_BUTTONS_NONE," ");
                label = gtk_label_new(NULL);
                gtk_widget_show(label);
                gtk_label_set_markup(GTK_LABEL(GTK_MESSAGE_DIALOG
                                        (dialog)->label),msg);
                gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,
                                TRUE,FALSE,0);
                gtk_dialog_add_button(GTK_DIALOG(dialog),
                                "Quit without Saving",GTK_RESPONSE_CLOSE);
                gtk_dialog_add_button(GTK_DIALOG(dialog),
                                GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
                gtk_dialog_add_button(GTK_DIALOG(dialog),
                                GTK_STOCK_SAVE,GTK_RESPONSE_YES);
                rv = gtk_dialog_run(GTK_DIALOG(dialog));

                /* Now handle what was clicked */
                switch (rv){
                case GTK_RESPONSE_CLOSE:
                        gtk_widget_destroy(dialog);
                        /* Just close/quit - don't save */
                        break;
                case GTK_RESPONSE_CANCEL:
                        /* Don't close, don't quit */
                        gtk_widget_destroy(dialog);
                        /* Fall through */
                case GTK_RESPONSE_DELETE_EVENT:
                        return;
                case GTK_RESPONSE_YES:
                        gtk_widget_destroy(dialog);
                        /* Yes, I want to save the file */
                        file_save_cb();
                        if (finfo->state != FILE_CLEAN)
                                return;
                        break;
                default:
                        g_assert_not_reached();
                }

        }

        /* Okay - any unwritten files have been saved - we can quit */
        gtk_exit(0);
}

void
file_print_cb(void){
        const char *xslt;

        if (fileops.printgconfkey == NULL){
                fprintf(stderr,"fileops.printgconfkey is NULL");
                return;
        }

        xslt = config_get_string_default(fileops.printgconfkey,
                        fileops.defaultprintkey);
        if (xslt == NULL){
                fprintf(stderr,"Couldn't get key\n");
                return;
        }

        /* Now lets do the print work */
        print_cb(xslt,fileops.save);
}




/*
 * file_set_none
 *
 * Called if the application has some reason to delete the file itself.
 * For instance if the user clicks cancel during creation the application
 * will now have no active file.
 *
 * Args:
 *	none
 * Returns:
 *	TRUE if there was a file
 *	FALSE otherwise
 * Side effects:
 *	sets current file state to none
 *	removes known file name
 */
gboolean
file_set_none(void){
	
	if (finfo == NULL)
		return FALSE;

	finfo->state = FILE_NONE;
	if (finfo->name != NULL)
		g_free(finfo->name);
	g_free(finfo);
	finfo = NULL;	
	
	return TRUE;
}
