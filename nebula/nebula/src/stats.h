#ifndef STATS_H
#define STATS_H

#ifndef LIBNEB_H
#include "libneb.h"
#endif /* LIBNEB_H */

static int update_stat_bonus(GtkTreeIter *iter);

static void apply_gain(void);
void stat_set_value_temp_cb(GtkCellRendererText *cell, const gchar *path_string,
                const gchar *new_text, gpointer data);
void stat_set_value_pot_cb(GtkCellRendererText *cell, const gchar *path_string,
                const gchar *new_text, gpointer data);
/* new stuff */
enum {
        STAT_ELEMENT_NAME,
        STAT_NAME,
        STAT_ABBR,
        STAT_TEMP,
        STAT_POT,
        STAT_BASIC_BONUS,
        STAT_RACIAL_BONUS,
        STAT_SPECIAL_BONUS,
        STAT_TOTAL_BONUS,
        STAT_PRIME,
        STAT_REALM,
        STAT_GAIN_TEMP,
        STAT_GAIN_POT,
        STAT_EDIT_FLAG,
};

int totalargs[] = {STAT_BASIC_BONUS,STAT_RACIAL_BONUS,STAT_SPECIAL_BONUS, -1};
int statprimaries[] =  {STAT_NAME,STAT_ABBR,-1};

struct store_info  stat_store_info[] = {
        {STAT_ELEMENT_NAME,G_TYPE_STRING,"",0,0,SAVE_ELEMENT },
        {STAT_NAME,  G_TYPE_STRING,    "name", 0,0,0},

        {STAT_ABBR,  G_TYPE_STRING,    "abbr" , 0,0,0},
        {STAT_TEMP,  G_TYPE_INT,       "temp" , 0,0,0},
        {STAT_POT,   G_TYPE_INT,       "potential", 0,0,0},
        {STAT_BASIC_BONUS,G_TYPE_INT, "basic", UPDATE_FUNCTION,update_stat_bonus,0},
        {STAT_RACIAL_BONUS,G_TYPE_INT,       "racial", 0,0,0},
        {STAT_SPECIAL_BONUS,G_TYPE_INT,       "special", 0,0,0},
        {STAT_TOTAL_BONUS,G_TYPE_INT,       "total", UPDATE_SUM, totalargs,0},
        {STAT_PRIME, G_TYPE_BOOLEAN,  "primestat",0,0,0},
        {STAT_REALM,G_TYPE_BOOLEAN, "realmstat",0,0,0},
        {STAT_GAIN_TEMP, G_TYPE_INT,    NULL,0,0,0},
        {STAT_GAIN_POT,  G_TYPE_INT,    NULL,0,0,0},
        {STAT_EDIT_FLAG,  G_TYPE_BOOLEAN,    NULL,0,0,0},
        {-1, G_TYPE_NONE,      NULL, UPDATE_NONE, 0,0},
};

struct view_info stat_view_info[] = {
        {"P",      STAT_PRIME,           "toggle", 0, 0, 0 },
        {"R",      STAT_REALM,           "toggle", 0, 0, 0 },
        {"Stat",   STAT_NAME,            "text",0, 0, 0 },
        {"Abbr",   STAT_ABBR,            "text",0, 0, 0 },
        {"Temp",   STAT_TEMP,            "text",0, 0, 0 },
        {"Pot",    STAT_POT,             "text",0, 0, 0 },
        {"Basic",  STAT_BASIC_BONUS,     "text",0, 0, 0 },
        {"Racial", STAT_RACIAL_BONUS,    "text",0, 0, 0 },
        {"Special",STAT_SPECIAL_BONUS,   "text",0, 0, 0 },
        {"Total",  STAT_TOTAL_BONUS,     "text",0, 0, 0 },
        {NULL,     -1,                   NULL  ,0, 0, 0 },
};


struct view_info stat_gain_view_info[] = {
        {"Stat",   STAT_NAME,      "text",0, 0, 0 },
        {"Abbr",   STAT_ABBR,      "text",0, 0, 0 },
        {"NewTemp",STAT_GAIN_TEMP, "text",0, 0, 0 },
        {"Temp",   STAT_TEMP,      "text",0, 0, 0 },
        {"NewPot", STAT_GAIN_POT,  "text",0, 0, 0 },
        {"Pot",    STAT_POT,       "text",0, 0, 0 },
        {NULL,     -1,             NULL  ,0, 0, 0 }
};

struct view_info stat_edit_view[] = {
        {"Stat",   STAT_NAME,      "text",0,0,0 },
        {"Temp",   STAT_GAIN_TEMP, "text",0,
                        STAT_EDIT_FLAG, (GtkSignalFunc)stat_set_value_temp_cb},
        {"Pot",    STAT_GAIN_POT,  "text",0,
                        STAT_EDIT_FLAG, (GtkSignalFunc)stat_set_value_pot_cb},
        {NULL,     -1,             NULL,0,0,0   }
};

GtkTreeStore *statstore;
GtkWidget *statview;


/* How many we use for DevPoint calculation */
#define DEV_STATS 5

int stat_bonuses[] = {
          0, /* 0 - No idea what 'correct' is for this */
        -10,-9,-9,-8,-8,-7,-7,-6,-6,-5, /*  1 - 10 */
        -4,-4,-4,-4,-4,-3,-3,-3,-3,-3,  /* 11 - 20 */
        -2,-2,-2,-2,-2,-1,-1,-1,-1,-1,  /* 21 - 30 */
        0,0,0,0,0,0,0,0,0,0,            /* 31 - 40  */
        0,0,0,0,0,0,0,0,0,0,            /* 41 - 50 */
        0,0,0,0,0,0,0,0,0,0,            /* 51 - 60 */
        0,0,0,0,0,0,0,0,0,1,            /* 61 - 70 */
        1,1,1,1,2,2,2,2,2,3,            /* 71 - 80 */
        3,3,3,3,4,4,4,4,4,5,            /* 81 - 90 */
        5,6,6,7,7,8,8,9,9,10,           /* 91 -100 */
};

#endif
