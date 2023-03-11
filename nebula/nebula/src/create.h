#ifndef CREATE_H
#define CREATE_H

#include "config.h"

#define CREATE_FIXED_POINTS "/apps/nebula/create_fixed_points"

struct field_info genre_fields[] = {
/*        { "About", "/description", NULL }, */
        { NULL, NULL, NULL }
};

struct combo_info genre_info = {
        "Choose Genre",  /* Title */
        GENRE_PATH,      /* Local Path */
        NULL,            /* Full path */
        NULL,            /* Prefix */
        ".xml",          /* Suffix */
        "Genre: ",       /* Genre label */
        NULL,    /* Field info list */
        /* genre_fields,   Disabled in 0.2.0 / * Field info list */
        NULL,            /* Window */
        NULL,            /* Combo */
};


enum { WEAPON_NAME };
enum { SPELL_LIST, SPELL_SELECTED, SPELL_CAN_SELECT };

struct store_info weapon_store_info[] = {
        { WEAPON_NAME, G_TYPE_STRING, "", 0, 0, SAVE_ELEMENT },
        { -1, 0, NULL, 0, 0, 0 },
};
struct view_info weapon_view_info[] = {
        { "Weapon",WEAPON_NAME,"text",0,0,0},
        { 0,0,0,0,0,0}
};
struct view_info weapon_cost_info[] = {
        { "Cost",WEAPON_NAME,"text",0,0,0},
        { 0,0,0,0,0,0}
};


struct store_info base_list_store[] = {
        { SPELL_LIST,     G_TYPE_STRING, "", 0, 0, 0 },
        { SPELL_SELECTED, G_TYPE_BOOLEAN,"", 0, 0, 0 },
        { SPELL_CAN_SELECT, G_TYPE_BOOLEAN,"", 0, 0, 0 },
        { -1,0,NULL,0,0,0 }
};
struct view_info base_list_view[] = {
        { "List",    SPELL_LIST, "text", 0,0,0 },
        { "Base?",   SPELL_SELECTED, "toggle", 0,SPELL_CAN_SELECT,0 },
        { 0,0,0,0,0,0 }
};
struct view_info hidden_list_view[] = {
        { "List",    SPELL_LIST, "text", 0,0,0 },
        { "Delete?", SPELL_SELECTED, "toggle", 0, SPELL_CAN_SELECT,0 },
        { 0,0,0,0,0,0 }
};

/* start... */
struct fileinfo *begin_create(void);

/* begin_create */
static void begin_genre(void);

/* Character Name */
static void begin_name(void);

/* Begin race */
static void begin_race(void);

/* Profession */
static void begin_profession(void);

/* Realm (for Non-speel users) */
static void begin_realm(void);
static void end_realm(void);

/* Realm Subtype */
GtkWidget *realm_combo;
GtkWidget *realm2_combo;
static void begin_realm_subtype(void);
static void end_realm_subtype(void);

/* Weapons */
static void begin_weapons(void);

/* Select base spell lists */
static void begin_spells(void);
static gboolean set_base(GtkTreeModel *,GtkTreePath *,GtkTreeIter *,gpointer);
static void begin_spell_setup(void);
static gboolean delete_skills(GtkTreeModel *model,GtkTreePath *path,
                GtkTreeIter *iter, gpointer data);

/* Stats */
/* Eek - fixme - hardcoded */
static GtkWidget *stat_boxes[10];
static int stat_values[10];
static GtkWidget *point_box;
static int stat_points;
static GtkWidget *stat_button;
static void change_value(GtkAdjustment *adj, gpointer i);
static int stat_to_points(int stat);
static int gen_pot(int temp);
static void end_stats(void);

/* Hobby Ranks */
static void begin_hobby_ranks(void);


/* End of Creation */
static void begin_end(void);

/* Misc functions to handle weapon selection */
GtkWidget *create_arrow_button(GtkArrowType, GtkShadowType);
gboolean compare_iter(GtkTreeIter *, GtkTreeIter *);
gboolean gtk_tree_model_iter_prev(GtkTreeModel *, GtkTreeIter *);
static void button_down(GtkWidget *, GtkTreeView *);
static void button_up(GtkWidget *, GtkTreeView *);

#endif
