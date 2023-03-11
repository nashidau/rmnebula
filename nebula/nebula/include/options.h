#ifndef OPTIONS_H
#define OPTIONS_H 1

#include <gtk/gtk.h>

struct option_info {
        char *path;
        char *desc;
        char *longdesc;
        GType type;
        GtkWidget *button;
};


#define NEBULA_GCONF_ROOT       "/apps/nebula"
#define CREATION_OPTION         NEBULA_GCONF_ROOT "/creation/"
#define STAT_OPTION             NEBULA_GCONF_ROOT "/stats/"
#define SKILL_OPTION            NEBULA_GCONF_ROOT "/skills/"
#define FILE_OPTION             NEBULA_GCONF_ROOT "/file/"
#define XSL_OPTION              NEBULA_GCONF_ROOT "/xsl/"
#define LICENCE                 NEBULA_GCONF_ROOT "/licence"

void options_init(struct option_info *options);
void options_creation(void);

const char *config_get_string_default(const char *path,const char *def);

void config_init(void);
gboolean config_get_bool(const char *path);
const char *config_get_string(const char *path);
void config_set_bool(const char *path,gboolean value);
void config_set_string(const char *path,const char *value);
int gonfig_get_int(const char *path);

#endif /* OPTIONS_H */
