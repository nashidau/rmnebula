#include <sys/types.h>
#include <dirent.h>
#include <gtk/gtk.h>

#include "libneb.h"
#include "config.h"
#include "unix.h"

GList *
get_files(const char *name,const char *path,char **finalpath){
        return get_files_specific(name,path,NULL,".xml",finalpath);
}

GList *
get_files_specific(const char *name,const char *path,
                const char *prefix,const char *suffix,char **finalpath){
        DIR *dir;
        struct dirent *dirent;
        GList *list = NULL;
        char *p;
        char **paths,*fullpath;
        int i;
        struct stat st;

        paths = g_strsplit(path,":",-1);

        /* For each entry in the path - search */
        for (i = 0 ; paths[i] != NULL ; i ++){
                fullpath = g_strconcat(paths[i],"/",name,NULL);
                if (stat(fullpath,&st) == -1 ||
                                S_ISDIR(st.st_mode) == 0){
                        g_free(fullpath);
                        continue;
                }

		if (prefix != NULL)
			fullpath = g_strconcat(fullpath, prefix, NULL);
		
                dir = opendir(fullpath);
                if (dir == NULL){
                        perror(fullpath);
                        continue;
                }

                while((dirent = readdir(dir)) != NULL){
                        if (dirent->d_name[0] == '.')
                                continue;
                        if (strcmp(dirent->d_name,"CVS") == 0)
                                continue;
                        if (suffix != NULL){
                                p = strstr(dirent->d_name,suffix);
                                if (p == NULL)
                                        continue;
                                if (strcmp(suffix,p) != 0)
                                        continue;
                                *p = 0; /* Hide the suffix */

                        }
			list = g_list_append(list,
					g_strdup(dirent->d_name));
                }
                closedir(dir);
                if (finalpath != NULL)
                        *finalpath = fullpath;
                else 
                        g_free(fullpath);

			
        }
        g_strfreev(paths);

        return list;
}

