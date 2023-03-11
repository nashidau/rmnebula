#ifndef NEBULA_H_
#define NEBULA_H_

/* beginning of i18n stuff */
#define ENABLE_NLS 1
#define PACKAGE "nebula"
#define GNOMELOCALEDIR "/usr/share/locale/"
/* end of i18n stuff */

#include <gnome.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <glib.h>
#include <sys/types.h>
#ifdef unix
#include <dirent.h>
#endif
#include <assert.h>

#include "tree.h"
#include "debug.h"
#include "types.h"
#include "options.h"
#include "alena.h"
#include "config.h"
#include "parse.h"
#include "gui.h"
#include "file.h"
#include "print.h"

#ifndef LIBNEB_H
#include "libneb.h"
#endif /* LIBNEB_H */

extern GtkWidget *charpage;
extern GtkWidget *skillpage;
extern GtkWidget *statpage;
extern GtkWidget *defensepage;

extern GtkWidget *the_notebook;
extern GtkWidget *window;

extern struct tags parsers[]; 

#define VERSION_STRING "0.3"
#define NEBULA_VERSION "Nebula " VERSION_STRING


int create_window(void);
int delete_event_cb(GtkWidget *window,GdkEventAny *e, gpointer data);

/* nebula.c */
int main(int argc,char **argv);
int create_window(void);
void init_pixmaps(void);
void open_splash(void);
void accept_licence(GtkWidget *widget);
void nebula_about(void);

/* Menu.c */
void create_main_page(GtkWidget *box);
GtkWidget *add_page(char *name,char *description);
GtkWidget *create_menu(GtkWidget *window,GtkWidget *box);
void refresh_all(void);

/* stats.c */
void stat_init(GtkWidget *box);
void stat_insert(element_t *el);
void stat_update(void);
void stat_clear(void);
void stat_seed_all(void);
void stat_save(element_t *root);
void stat_gain(void);
void stat_edit(void);
int calculate_dp(void);
int stat_get_temp(char *name);
int stat_get_potential(char *name);
int stat_get_bonus(char *name);
int get_realm_bonus(void);
void stat_set_prime_requisite(char *name,gboolean pr);
void stat_set_realm_stat(char *name,gboolean realm);
void stat_set_racial_bonus(char *name,int newval);
void stat_set_temp(char *name,int newval);
void stat_set_potential(char *name,int newval);

GSList *stat_names(void);
gboolean stat_is_prime_requisite(char *name);

void print_stats(FILE *fp,char *func,char *linefunc);
void stat_create_appearance(void);

/* Character.c */
void characteristic_init(GtkWidget *box);
void characteristic_set(const char *name,const char *value);
char *characteristic_get(const char *name);
void characteristic_add_xml(element_t *el);
void characteristics_save(element_t *parent);
void characteristics_clear(void);

void print_characteristics(FILE *fp);
void increase_level(int lvl_inc);

/* skills.c */
void skill_init(GtkWidget *box);
void skill_add_xml(element_t *el);
void skill_update(void);
void skill_clear(void);
void skill_save(element_t *el);
void skill_buy_dp(void);
void skill_buy_ranks(buymode_t buymode,int points,int limit);
void skill_buy_rank(GtkCellRendererText * cell, const gchar * path_string,
            const gchar * new_text, gpointer data);
void skill_set_cost(char *cat,char *cost);
char *skill_get_cost(char *cat);
void skill_set_progression(char *cat,char *progression);
void skill_set_realm_stat(char *stats);
void skill_set_special_bonus(char *skill,int value);
GList *skill_get_cat_list(char *pattern);
GList *skill_get_skill_list(char *cat);
void skill_set_class(char *name,char *value);
int get_stat_bonuses(const char *str,int max);
gboolean skill_move(char *srcname,char *destname);
void skill_delete(char *cat);
int skill_get_ranks(char *cat);
void skill_set_ranks(char *cat,int ranks);
void skill_purchase_skills_times(char *filename);
char *skill_purchase_load(void);

/* file.c */
void file_open_cb(void);
void file_save_cb(void);
void file_save_as_cb(void);

void file_close_cb(void);
void file_revert_cb(void);
gboolean file_loaded(void);
gboolean file_modify(void);
void file_set_modified(void);

/* dice.c */
void dice_init(void);
int d10(void);
int d100(void);
int d100OEH(void);
int d100OEL(void);
int d100OE(void);
int nd10(int n);
int dX(int X);
int round_div(int total,int divisor);

/* options.c */
void options_creation(void);
void options_stats(void);
void options_skills(void);
void options_print(void);
void options_file(void);

/* create.c */
/* New creation code - should move to create.h */
void create_begin(void);
void create_choice(element_t *el);

struct fileinfo *begin_create(void);
void begin_stats(void);
void set_prefix_xml(element_t *el);
void set_prefix(char *p);
void print_list(GList *list);

/* profession.c */
void parse_profession(char *filename);
void prof_add_realm_xml(element_t *el);
GList *prof_get_subtypes(char *prefix);
GList *list_realms(void);
void set_realm_stat(const char *str);

void prof_add_realm(char *realm); /* Of form "Realm:<realm>:<subtype>" */
/* item.c */
void init_items(GtkWidget *box);
void update_items(void);
void xml_insert_item(element_t *el);

/* rr.c */
void rr_init(GtkWidget *notepage);
void rr_insert(element_t *el);
void rr_update(void);
void print_rrs(FILE *fp);
void rr_save(element_t *el);
void rr_clear(void);

/* special.c */
void update_special(void);

/* print.c */
void print_micro(void);
void print_normal(void);
void print_full(void);

/* tips.c */
void init_tips(void);
void set_tip(GtkWidget *widget,char *tip);


/* spells.c */
void spells_set_cost(void);

/* choices.c */
void choice_parse(element_t *el);

/* Training packages */
void tp_select(GtkWidget *button);


#endif /* NEBULA_H_ */
