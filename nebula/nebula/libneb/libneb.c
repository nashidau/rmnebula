#include <stdarg.h>
#include "libneb.h"


int
libneb_init(char *name,char *version,int argc,char **argv,
                struct fileops *fileops,struct option_info *options){
        gtk_set_locale();
        gnome_program_init(name,version,LIBGNOMEUI_MODULE,argc,argv,NULL);
        file_set_fileops(fileops);
        init_tips();
        config_init();
        options_init(options);

	return 0;
}

int
libneb_icons_init(char *name,...){
        GdkPixbuf **icon;
        GError *error;
        va_list args;
        int rv = 0;

        va_start(args,name);
        while (name != NULL){
                icon = va_arg(args,GdkPixbuf **);
                error = NULL;

                *icon = gdk_pixbuf_new_from_file(name,&error);
                if (*icon == NULL){
                        printf("Couldn't load icon %s\n",name);
                        rv = -1;
                }
                name = va_arg(args,char *);
        }
        return rv;
}
