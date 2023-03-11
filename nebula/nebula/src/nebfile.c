#include "alena.h"
#include "file.h"
#include "parse.h"
#include "nebfile.h"
#include "nebula.h"
#include <assert.h>

static const char *get_character_name(struct fileinfo *finfo);

void
nebfile_init(void){
        struct fileops fileops;

        fileops.docname = g_strdup("character");
        fileops.printgconfkey = g_strdup(XSL_OPTION "nebula/postscript");
        fileops.defaultprintkey = g_strdup(
                        "/home/nash/work/rolemaster/XML/rmsscharacter/ps.xsl");
        fileops.create = begin_create;
        fileops.open = nebfile_open;
        fileops.save = nebfile_save;
        fileops.close = nebfile_close;
        fileops.refresh = refresh_all;
        fileops.getfilename = get_character_name;

        file_set_fileops(&fileops);
}

static const char *
get_character_name(struct fileinfo *finfo){
        char *name;

        assert(finfo != NULL);

        name = characteristic_get("name");
        if (name != NULL && strlen(name) > 0)
                return name;

        name = strrchr(name,'/');
        if (name != NULL)
                if (strlen(name) > 1)
                        return ++name;

        return "Anonymous";

}

void
nebfile_save(struct fileinfo *finfo){
        element_t *el;

        assert(finfo != NULL);

        if (finfo->name == NULL){
                printf("Can't save NULL file\n");
                return;
        }

        el = create_element(NULL,"rmsscharacter",ETYPE_ELEMENT);
        set_attribute(el,"id","neb-00001");
        set_attribute(el,"version",VERSION_STRING);

        /* Save Characteristics */
        characteristics_save(el);

        /* Save stats */
        stat_save(el);

        /* Save cats & skills */
        skill_save(el);

        /* Some RRs */
        rr_save(el);

        /* put in the empty (unimplemented) stuff */
        create_element(el,"items",ETYPE_EMPTY);
        create_element(el,"other",ETYPE_EMPTY);

        alena_save(finfo->name,el,CHARACTERDTD);

        /* FIXME: I should free the tree now! */

        return;
}
/* Closes the file specified by the fileinfo.  Note
 * does not save or otherwise check the fileinfo */
int
nebfile_close(void){
        characteristics_clear();
        stat_clear();
        skill_clear();
        rr_clear();

        return 0;
}
/* Opens the file specified by the fileinfo.
 * Will close any current open file */
struct fileinfo *
nebfile_open(const char *filename){
        struct fileinfo *finfo;
        int res;
        GtkWidget *dialog;

        res = parse_file(filename,parsers);

        if (res == -1){
                /* FIXME: A better message would be nice */
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_MESSAGE_WARNING,
                                          GTK_BUTTONS_OK,
                                          "Failed to load file");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
                return NULL;
        }

        finfo = g_malloc0(sizeof(struct fileinfo));
        finfo->name = g_strdup(filename);
        finfo->state = FILE_CLEAN;

        return finfo;
}


int
nebfile_open_data_file(const char *filename,struct tags *parsers){
        struct stat st;
        const char *paths;
        char **strs;
        char *path = NULL;
        int i,rv = -1;

        paths = config_get_string(FILE_OPTION "datapath");
        if (paths == NULL){
                config_set_string(FILE_OPTION "datapath",CONFIG_PATH);
                paths = CONFIG_PATH;
        }

        strs = g_strsplit(paths,":",-1);
        for (i = 0 ; strs[i] != NULL ; i ++){
                path = g_strconcat(strs[i],"/",filename,NULL);
                if (stat(path,&st) == -1 ||
                                S_ISREG(st.st_mode) == FALSE){
                        g_free(path);
                        path = NULL;
                        continue;
                }
                break;
        }

        if (path != NULL){
                rv = parse_file(path,parsers);
        }

        g_free(path);
        g_strfreev(strs);

        if (rv == -1)
                fprintf(stderr,"Can't find file %s in path\n",filename);
        return rv;

}

