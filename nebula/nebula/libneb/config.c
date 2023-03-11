/*
 * gconf.c
 *
 * GConf interfacing functions for nebula
 * Provides general preferences for all of nebula
 *
 * Copyright 2002 Brett Nash
 */

#include <gconf/gconf-client.h>

#include "libneb.h"

GConfClient *client;

void
config_init(void){
        client = gconf_client_get_default();

        gconf_client_add_dir(client,NEBULA_GCONF_ROOT,
                        GCONF_CLIENT_PRELOAD_RECURSIVE,NULL);

}

gboolean
config_get_bool(const char *path){
        return gconf_client_get_bool(client,path,NULL);
}
void
config_set_bool(const char *path,gboolean value){
        gconf_client_set_bool(client,path,value,NULL);

}
void
config_set_string(const char *path,const char *value){
        gconf_client_set_string(client,path,value,NULL);
}

const char *
config_get_string(const char *path){
        return gconf_client_get_string(client,path,NULL);
}

/*
 * config_get_string_default
 *
 * Gets the current value of a string from GConf.  If the value is NULL,
 * sets the supplied value (the default) in gconf and returns
 *
 * Args:
 *  cosnt char *path: Path of key
 *  const char *def: Defualt value if key is unset
 */
const char *
config_get_string_default(const char *path,const char *def){
        const char *str;

        str = config_get_string(path);
        if (str == NULL){
                config_set_string(path,def);
                str = def;
        }
        return str;
}


