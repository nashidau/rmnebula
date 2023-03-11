#ifndef RR_H
#define RR_H


int rr_update_statbonus(GtkTreeIter *iter);

GtkTreeStore *rrstore;
GtkWidget *rrview;

enum {
        RR_RR, /* field for 'RR' tag */
        RR_NAME,
        RR_STATS,
        RR_STATBONUS,
        RR_RACIALBONUS,
        RR_SPECIALBONUS,
        RR_TOTAL,
};

/* What is summed to give total rr bonuses */
int rrbonuses[] = {
        RR_STATBONUS,
        RR_RACIALBONUS,
        RR_SPECIALBONUS,
        -1
        };

struct store_info rr_store_info[] = {
        { RR_RR, G_TYPE_STRING, "rr", UPDATE_NONE,0,SAVE_ELEMENT },
        { RR_NAME,  G_TYPE_STRING, "name", UPDATE_NONE,0,SAVE_ALWAYS }, /* primary key */
        { RR_STATS, G_TYPE_STRING, "statbonuses", UPDATE_NONE,0,SAVE_ALWAYS },
        { RR_STATBONUS,G_TYPE_INT, "statbonus", UPDATE_FUNCTION,
                rr_update_statbonus,SAVE_ALWAYS},
        { RR_RACIALBONUS,G_TYPE_INT,"racialbonus",UPDATE_NONE,0,SAVE_ALWAYS},
        { RR_SPECIALBONUS,G_TYPE_INT,"specialbonus",UPDATE_NONE,0,SAVE_ALWAYS},
        { RR_TOTAL, G_TYPE_INT, "totalbonus",UPDATE_SUM,rrbonuses,SAVE_ALWAYS},
        { -1,0,0,0,0,0},
};
int rrprimaries[] = { RR_NAME, -1};

struct view_info rr_view_info[] = {
        { "RR", RR_NAME, "text", 0,0,0},
        { "Stats", RR_STATS, "text", 0,0,0},
        { "Racial", RR_RACIALBONUS, "text", 0,0,0},
        { "Special", RR_SPECIALBONUS, "text", 0,0,0},
        { "Total", RR_TOTAL, "text", 0,0,0},
        { 0, 0, 0, 0,0,0},
};
#endif /* RR_H */
