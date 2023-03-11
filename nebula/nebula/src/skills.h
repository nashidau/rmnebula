#ifndef SKILLS_H
#define SKILLS_H

/* FIXME:
 *      This file is UGLY!!
 */

static int skill_update_rankbonus(GtkTreeIter *iter);
static int skill_update_statbonus(GtkTreeIter *iter);
static int skill_update_can_buy(GtkTreeIter *iter);
void skill_buy_rank(GtkCellRendererText * cell, const gchar * path_string,
            const gchar * new_text, gpointer data);
int skill_get_ranks_cost(GtkTreeIter *iter,int ranks);
void skill_purchase_add(const char *filename);
char *skill_purchase_load(void);
static void skill_purchase_skills(void);
void skill_purchase_skills_times(char *filename);

static void skill_cancel_purchase(void);
static gboolean skill_purchase_apply(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);

struct buyinfo {
        buymode_t buymode;
        int spent; /* How many points have been spent */
        int total; /* Max points to spend */
        int limit; /* Max of any one point */
        GtkWidget *spentbox;
};

enum {
        SKILL_ELEMENT_NAME, /* slot 0 reserved for parser */
        SKILL_NAME,
        SKILL_STATS,
        SKILL_CLASS, /* Everyman, Occ etc */
        SKILL_PROGRESSION,
        SKILL_COST,
        SKILL_PROFESSION_BONUS,
        SKILL_STAT_BONUS,
        SKILL_SPECIAL_BONUS,
        SKILL_RANKS,
        SKILL_RANK_BONUS,
        SKILL_CATEGORY_BONUS,
        SKILL_BONUS,
        SKILL_DESCRIPTION,
        SKILL_PRIMARY, /* Is this a frequently used skill */
        SKILL_HIDDEN,
        SKILL_CAN_BUY,
        SKILL_NEW_RANKS,
        SKILL_NEW_COST,
        SKILL_HALF_RANK,
        SKILL_TRUE, /* Set to true when loaded */
};

int skillupdates[] = { SKILL_PROFESSION_BONUS,
        SKILL_STAT_BONUS,
        SKILL_SPECIAL_BONUS,
        SKILL_RANK_BONUS,
        SKILL_CATEGORY_BONUS, /* 0 for cats, non-zero skills */
        -1};


struct store_info skill_store_info[] = {
        { SKILL_ELEMENT_NAME, G_TYPE_STRING, "", 0, 0, SAVE_ELEMENT },
        { SKILL_NAME,   G_TYPE_STRING, "name", 0, 0, SAVE_ALWAYS },
        { SKILL_STATS,  G_TYPE_STRING, "statbonuses", 0, 0, SAVE_NOT_MIDDLE},
        { SKILL_CLASS,  G_TYPE_STRING, "class", 0, 0, SAVE_CHILDREN },
        { SKILL_PROGRESSION, G_TYPE_STRING, "progression", 0, 0, SAVE_PARENT},
        { SKILL_COST,   G_TYPE_STRING, "cost", 0, 0, SAVE_PARENT },
        { SKILL_PROFESSION_BONUS,G_TYPE_INT,"professionbonus",0,0,SAVE_PARENT},
        { SKILL_STAT_BONUS,G_TYPE_INT, "statbonus", UPDATE_FUNCTION,
                skill_update_statbonus ,SAVE_PARENT },
        { SKILL_SPECIAL_BONUS,G_TYPE_INT,"specialbonus", 0 , 0,SAVE_DEFINED},
        { SKILL_RANKS,  G_TYPE_INT, "ranks", 0, 0, SAVE_NOT_MIDDLE },
        { SKILL_RANK_BONUS, G_TYPE_INT, "rankbonus", UPDATE_FUNCTION,
                        skill_update_rankbonus,SAVE_NOT_MIDDLE },
        { SKILL_CATEGORY_BONUS, G_TYPE_INT, "categorybonus", UPDATE_PARENT,
                                (void *)SKILL_BONUS,SAVE_BOTTOM },
        { SKILL_BONUS,G_TYPE_INT,"total", UPDATE_SUM,skillupdates,SAVE_NOT_MIDDLE},
        { SKILL_DESCRIPTION,G_TYPE_STRING,"description", 0,0,SAVE_DEFINED },
        { SKILL_PRIMARY, G_TYPE_BOOLEAN, "primary" ,0 ,0,SAVE_DEFINED},
        { SKILL_HIDDEN, G_TYPE_BOOLEAN, "hidden" ,0 ,0,SAVE_DEFINED},
        { SKILL_CAN_BUY, G_TYPE_INT, NULL, UPDATE_FUNCTION,
                        skill_update_can_buy, SAVE_NEVER },
        { SKILL_NEW_RANKS, G_TYPE_INT, NULL, 0, 0, SAVE_NEVER },
        { SKILL_NEW_COST, G_TYPE_INT, NULL, 0, 0, SAVE_NEVER },
        { SKILL_HALF_RANK, G_TYPE_BOOLEAN, "halfrank", 0, 0, SAVE_DEFINED },
        { SKILL_TRUE, G_TYPE_BOOLEAN, NULL, 0, 0, SAVE_NEVER },

        { -1, 0, NULL, 0, 0, 0 },
};

struct view_info skill_view_info[] = {
        { "Name",SKILL_NAME,"text",0,0,0},
        { "Stats",SKILL_STATS,"text",0,0,0},
        { "Class",SKILL_CLASS,"text",0,0,0},
        { "Prog",SKILL_PROGRESSION,"text",0,0,0},
        { "Cost",SKILL_COST,"text",0,0,0},
        { "Prof",SKILL_PROFESSION_BONUS,"text",0,0,0},
        { "Stat",SKILL_STAT_BONUS,"text",0,0,0},
        { "Spec",SKILL_SPECIAL_BONUS,"text",0,0,0},
        { "Ranks",SKILL_RANKS,"text",0,0,0},
        { "Rank",SKILL_RANK_BONUS,"text",0,0,0},
        { "Bonus",SKILL_BONUS,"text",0,0,0},
        /*{ "Hidden",SKILL_HIDDEN,"toggle",0,0,0}, Does nothing currently */
        { "Primary",SKILL_PRIMARY,"toggle",0,SKILL_TRUE,0},

        /* Debug stuff */
        /* { "half",SKILL_HALF_RANK,"toggle",0,0,0}, */
        /*{ "Can Buy",SKILL_CAN_BUY,"text",0,0,0},*/
        { 0,0,0,0,0,0}
};

struct view_info buy_info[] = {
        { "Name", SKILL_NAME, "text", 0, 0, 0 },
        { "Cost", SKILL_COST, "text", 0, 0, 0 },
        { "Ranks",SKILL_RANKS,"text", 0, 0, 0 },
        { "New",  SKILL_NEW_RANKS,  "text", 0, SKILL_CAN_BUY, (GtkSignalFunc)skill_buy_rank },
        { "Spent",SKILL_NEW_COST, "text", 0, 0, 0 },
        { 0,0,0,0,0,0 }
};
struct view_info buy_info_ranks[] = {
        { "Name", SKILL_NAME, "text", 0, 0, 0 },
        { "Cost", SKILL_COST, "text", 0, 0, 0 },
        { "Ranks",SKILL_RANKS,"text", 0, 0, 0 },
        { "New",  SKILL_NEW_RANKS,  "text", 0, SKILL_CAN_BUY, (GtkSignalFunc)skill_buy_rank },
        { 0,0,0,0,0,0 }
};

/* Used for saving skills during level up procedure
 *   This is subset of skill_store_info
 */
struct store_info skill_store_purchases[] = {
        { SKILL_ELEMENT_NAME, G_TYPE_STRING, "", 0, 0, SAVE_ELEMENT },
        { SKILL_NAME, G_TYPE_STRING, "name", 0, 0, SAVE_ALWAYS },
        { SKILL_NEW_RANKS, G_TYPE_INT, "newranks",  0, 0, SAVE_ALWAYS },
        { -1, 0, NULL, 0,0,0 },
};

int skillprimaries[] = { SKILL_NAME , -1 };


GtkTreeStore *skillstore;
GtkWidget *skillview;
GtkWidget *buyview;

static void skill_insert_xml(element_t *el,GtkTreeIter *parent,struct store_info *);

struct progression {
        char *name;
        float *cat;
        float *skill;
};

float prog_standard_cat[] = {-15,2,1,0.5,0};
float prog_standard_skill[] = {-15,3,2,1,0.5};
float prog_limited_skill[] = { 0,1,1,0,5,0};
float prog_combined_skill[] = {-30,5,3,1.5,0.5};

struct progression progressions[] = {
        /* Name */   /* Cat Prog */     /* skill prog */
        {"Standard",  prog_standard_cat, prog_standard_skill },
        {"Limited",   NULL,              prog_limited_skill  },
        {"Combined",  NULL,              prog_combined_skill },
        { NULL,       NULL,            NULL }
};

/*
static GtkItemFactoryEntry popup_menu_items[] = {
	{"/_New skill...", NULL, NULL, 0,"<StockItem>", GTK_STOCK_NEW},
	{"/_Edit skill...", NULL, NULL, 0,"<StockItem>", GTK_STOCK_EDIT},
};
*/
struct skill_edit_window {
        GtkWidget *window;
        GtkWidget *name;
        GtkWidget *class;
        GtkWidget *text;
        GtkTreeIter *iter;
};
enum {
        POPUP_CALLBACK_SHOW,
        POPUP_CALLBACK_EDIT,
        POPUP_CALLBACK_NEW,
};

#endif /* SKILLS_H */
