#include "nebula.h"
#include "skills.h"

/* Local functions */
int get_stat_bonuses(const char *str,int max);
static int get_rank_bonus(char *progname,gboolean iscat,int ranks);
static GtkTreeIter *skill_get_by_name(char *skill);
static void skill_cat_set_string(char *cat,int field,char *val);
static char *skill_cat_get_string(char *cat,int field);
static int bonus_from_prog(float *prog,int ranks);
static float *create_progression(char *name);
static void skill_cat_set_int(char *cat,int field,int value);
static int skill_cat_get_int(char *cat,int field);
static void skill_purchase_save(void);
/* static void skill_purchase_load(void); */

static int skill_edit_callback(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void
skill_create_new_cb(struct rcmenu_info *minfo,GtkTreeIter *iter);
/*static struct skill_edit_window *skill_window_create(void);*/
/*static void skill_window_update(GtkTreeIter *iter,gboolean active);*/
/*static void skill_window_okay(struct skill_edit_window *wininfo,GtkWidget *button);*/


static char *classstrings[] = {
        "Normal",
        "Everyman",
        "Occupational",
        "Restricted",
        NULL,
};

static struct popupinfo *popupinfo;
static struct treepopupinfo popup_info[] = {
        { SKILL_NAME, POPUP_ENTRY, "Name: ", 0,0 },
        { SKILL_STATS,POPUP_ENTRY, "Stats: ", 0,0 },
        { SKILL_CLASS,POPUP_COMBO, "Class: ", classstrings,0 },
        { SKILL_PROGRESSION,POPUP_ENTRY,"Progression: ",0,0 },
        { SKILL_COST,POPUP_ENTRY, "Cost: ", 0,0 },
        { SKILL_PROFESSION_BONUS,POPUP_DIGIT, "Profession Bonus: ", 0,0 },
        { SKILL_SPECIAL_BONUS,POPUP_DIGIT, "Special Bonus: ", 0,0 },
        { SKILL_RANKS,POPUP_DIGIT, "Ranks: ", 0,0 },
        { SKILL_BONUS,POPUP_DIGIT, "Total Bonus: ", 0,0 },
        { SKILL_PRIMARY,POPUP_TOGGLE, "Skill is frequently used", 0,0 },
        { SKILL_HALF_RANK,POPUP_TOGGLE, "Has a half rank", 0,0 },
        { SKILL_DESCRIPTION, POPUP_TEXTVIEW, "Description", 0,0 },
        { -1, 0, NULL, 0, 0 }, /* End the list */
};

/* The popup (right click) menus */
static GtkItemFactoryEntry skillmenu[] = {
        {
                N_("/_New Skill..."),NULL,
                (GtkItemFactoryCallback)rcmenu_new_callback,0,
                "<StockItem>", GTK_STOCK_NEW
        },
        {
                N_("/_Edit Skill..."), NULL,
                (GtkItemFactoryCallback)rcmenu_edit_callback,1,
                "<Item>", NULL
        },{
                "/sep1--------------", NULL,
                0, 0,
                "<Separator>",0
        },
	{
                N_("/_Delete Skill"), NULL,
                (GtkItemFactoryCallback)rcmenu_delete_callback,2,
                "<StockItem>", GTK_STOCK_DELETE
        },
};



/*******************************************
 * General module functions                *
 *******************************************/

/*
 * skill_init
 *
 * Create an inital widgets for skills.
 *
 * Args:
 *  GtkWidget * box to put stuff in - either a window or notebook page
 */
void
skill_init(GtkWidget *box){
        GtkWidget *scrollwin;
        GtkTreeSelection *select;
        struct choice *choice;
        struct rcmenu_info *minfo;

        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(box),scrollwin);

        skillstore = make_tree_store(skill_store_info);

        choice = g_malloc0(sizeof(struct choice));
        choice->min = choice->max = -1;
        choice->count = 0;
        choice->store = skillstore;

        skillview = tree_create_view_data(GTK_TREE_MODEL(skillstore),
                        skill_view_info,choice);
        g_signal_connect(G_OBJECT(skillview), "button_press_event",
                        (GtkSignalFunc)skill_edit_callback,NULL);
        g_object_unref(G_OBJECT(skillstore));

        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(skillview));
        gtk_tree_selection_set_mode(select,GTK_SELECTION_SINGLE);

        gtk_container_add(GTK_CONTAINER(scrollwin),skillview);

        popupinfo = tree_create_popup(popup_info,window);
        popupinfo->store = skillstore;

        /* Create a right click menu */
        minfo = g_malloc0(sizeof(struct rcmenu_info));
        minfo->store = skillstore;
        minfo->view = GTK_TREE_VIEW(skillview);
        minfo->storeinfo = skill_store_info;
        minfo->item = _("skill");
        minfo->name_field = SKILL_NAME;
        minfo->popupinfo = popupinfo;
        minfo->maxdepth = 2; /* Two deep children */
        minfo->set_defaults = skill_create_new_cb;
        tree_popup_add(skillview,skillmenu,G_N_ELEMENTS(skillmenu),minfo);
        

}


/*
 * Add all skills
 */
void
skill_add_xml(element_t *data){
        GList *elements;
        element_t *el;

        elements = data->elements;
        while (elements != NULL){
                el = elements->data;
                skill_insert_xml(el,NULL,skill_store_info);

                elements = g_list_next(elements);
        }
        tree_set_flag_recursive(skillstore,NULL,SKILL_TRUE,TRUE);
}

static void
skill_insert_xml(element_t *el,GtkTreeIter *parent,struct store_info *sinfo){
        char *recurse[] = {
                "skill",
                "skillgroup",
                0
                };

        tree_insert(skillstore,sinfo,skillprimaries,el,recurse,NULL);
}




/*
 * Callback on Edit Skills menu entry (toggle)
 */
void edit_skills(gpointer dummy, guint dummy_action, GtkWidget *widget)
{
}

/*
 * Works out the stat bonus from a given string
 *
 * FIXME: This should be in stats.h
 */
int
get_stat_bonuses(const char *instr,int max){
        char *p,*str;
        int total = 0;

        if (instr == NULL)
                return 0;

        str = g_strdup(instr);

        p = strtok(str,"+");
        while (p != NULL && max > 0){
                total += stat_get_bonus(p);

                max --;
                p = strtok(NULL,"+");
        }

        g_free(str);
        return total;
}

void
skill_clear(void){
        gtk_tree_store_clear(skillstore);
}



/***********************************************************/
/* Update Functions                                        */
/***********************************************************/
/*
 * skill_update
 *
 * Start the update process
 */

int maxstats;


void
skill_update(void){
        gboolean individualstats;
        individualstats = config_get_bool(SKILL_OPTION "individualstats");

        if (individualstats == TRUE){
                maxstats = 2;
                printf("using individual stats\n");
        } else
                maxstats = 3;

        tree_refresh(skill_store_info,skillstore);
}

/*
 * skill_update_statbonus
 *
 * Updates the skill bonus based on the given stats
 */
static int
skill_update_statbonus(GtkTreeIter *iter){
        char *str;
        char *name;
        int parentbonus = 0;
        gboolean hasparent;
        GtkTreeIter parent;

        hasparent = gtk_tree_model_iter_parent(GTK_TREE_MODEL(skillstore),
                                        &parent,iter);

        /* Is a skill?  And are we using skill specific stat bonuses */
        if (hasparent == TRUE && maxstats == 3)
                return 0;

        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),iter,SKILL_STATS,&str,
                        SKILL_NAME,&name,-1);

        if (maxstats < 3 && (str == NULL || strlen(str) == 0)){
                gtk_tree_model_get(GTK_TREE_MODEL(skillstore),&parent,
                                SKILL_STAT_BONUS,&parentbonus,
                                SKILL_STATS,&str,-1);
                if (str != NULL && strlen(str) > 0){
                        return get_stat_bonuses(str,3) - parentbonus;
                }
        }
        return get_stat_bonuses(str,maxstats);
}

/*
 * skill_update_rankbonus
 *
 * Updates the rankbonus for a given skill
 *
 * FIXME: Should handle all types of progressions - not just normal
 */
static int
skill_update_rankbonus(GtkTreeIter *iter){
        int ranks;
        char *prog = NULL;
        GtkTreeIter *parent;
        gboolean iscat = TRUE;

        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),iter,
                        SKILL_RANKS,&ranks,-1);

        parent = get_parent(GTK_TREE_MODEL(skillstore),iter);
        if (parent == iter)
                iscat = TRUE;
        else
                iscat = FALSE;
        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),parent,
                                SKILL_PROGRESSION,&prog,-1);

        if (prog == NULL){
                printf("prog not found\n");
                prog = "Standard";
        }

        return get_rank_bonus(prog,iscat,ranks);

}

/*
 * Can this field be ranks - if so how many ranks
 *
 * FIXME:  Needs to check progression != 0 for category.
 *      (ie if ranks > 30)
 */
static int
skill_update_can_buy(GtkTreeIter *iter){
        GtkTreeIter *parent;
        char *cost,*prog;
        int ranks = 1;
        gboolean iscat = FALSE;

        parent = get_parent(GTK_TREE_MODEL(skillstore),iter);
        if (parent != iter)
                iscat = FALSE;
        else
                iscat = TRUE;

        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),parent,SKILL_COST,&cost,
                        SKILL_PROGRESSION,&prog,-1);

        if (prog == NULL){
                printf("Progression is not defined\n");
                return 0;
        } else if (cost == NULL){
                // printf("Cost is not defined\n");
                return 0;
        }

        /* Check buying something means something */
        if (get_rank_bonus(prog,iscat,1) == 0)
                return 0;

        while ((cost = strpbrk(cost,"/")) != NULL){
                ranks ++;
                cost ++; /* jump over the '/' */
        }

        return ranks;

}

/*
 * get_rank_bonus
 *
 * Calculates the rank bonus for a particular skill
 *
 * FIXME: Should cache the results of create_progression
 */
static int
get_rank_bonus(char *progname,gboolean iscat,int ranks){
        struct progression *prog;
        float *cprog;
        int bonus;

        if (progname == NULL)
                return 0;

        for (prog = progressions ; prog->name != NULL ; prog ++){
                if (strcmp(prog->name,progname) == 0){
                        if (iscat == TRUE)
                                return bonus_from_prog(prog->cat,ranks);

                        else
                                return bonus_from_prog(prog->skill,ranks);
                }
        }

        /* All special progressions have 0 for cat progressions */
        if (iscat == TRUE)
                return 0;

        cprog = create_progression(progname);
        if (cprog == NULL){
                printf("Prog %s unknown\n",progname);
        }

        bonus = bonus_from_prog(cprog,ranks);
        g_free(cprog);
        return bonus;

}

/*
 * FIXME: Doesn't handle unicode digits correctly
 */
static float *
create_progression(char *name){
        float *digits;
        int pos;

        if (name == NULL || (!isdigit(*name) && *name != '-')){
                printf("%s is not valid\n",name);
                return NULL;
        }

        digits = calloc(5,sizeof(float));

        for (pos = 0 ; pos < 5 ; pos ++){
                digits[pos] = strtod(name,&name);
                if (isspace(*name)) /* FIXME: Should use unicode isspace */
                        name ++;
                /* Jump the seperator (&#x2022 or '*') */
                name = g_utf8_next_char(name);
        }

        return digits;


}

/*
 * Warning Seedy hard coding ahead
 */
static int
bonus_from_prog(float *prog,int ranks){
        int offset;
        float bonus = 0;

        if (prog == NULL)
                return 0;

        if (ranks == 0)
                return prog[0];

        for (offset = 1; ranks > 0 && offset < 4; offset ++ , ranks -= 10){
                if (ranks > 10)
                        bonus += prog[offset] * 10;
                else
                        bonus += prog[offset] * ranks;
        }

        if (ranks > 0)
                bonus += prog[4] * ranks;

        return (int)bonus;
}



/*
 * skill_save
 *
 * Creates an alena XML tree to be saved to a file
 *
 */
void
skill_save(element_t *el){
        el = create_element(el,"skills",ETYPE_ELEMENT);

        tree_save(GTK_TREE_MODEL(skillstore),skill_store_info,el);
}

/***********************************************
 * Various Action functions                    *
 ***********************************************/
/*
 * skill_set_cost
 *
 * Sets the cost on a skill/category
 *
 * Args:
 *      char * cat name
 *      char * new cost string
 */
void
skill_set_cost(char *cat,char *cost){
        skill_cat_set_string(cat,SKILL_COST,cost);
}

char *
skill_get_cost(char *cat){
        return skill_cat_get_string(cat,SKILL_COST);
}


/*
 * skill_set_progression
 *
 * Sets the progression string on a skill/category
 *
 * Args:
 *      char *cat name
 *      char *new progression string
 */
void
skill_set_progression(char *cat,char *progression){
        skill_cat_set_string(cat,SKILL_PROGRESSION,progression);
}

/*
 * skill_get_ranks
 *
 * Gets the current number of ranks in a skill
 *
 * Args:
 *      char *cat
 */
int
skill_get_ranks(char *cat){
        return skill_cat_get_int(cat,SKILL_RANKS);
}
void
skill_set_ranks(char *cat,int ranks){
        skill_cat_set_int(cat,SKILL_RANKS,ranks);
}

/*
 * skill_set_realm_stat
 *
 * Replaces stat 'realm' with the supplied realm stat
 *
 * Args:
 *      char *new realm stat
 */
void
skill_set_realm_stat(char *stat){
        char *oldstat;
        GtkTreeIter iter;
        gboolean valid;

        valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(skillstore),&iter);
        if (valid == FALSE)
                return;

        do {
                gtk_tree_model_get(GTK_TREE_MODEL(skillstore),&iter,
                                SKILL_STATS,&oldstat,-1);
                if (oldstat == NULL){
                        printf("Warning: Skill with no stat\n");
                        continue;
                }
                if (strcasecmp("realm",oldstat) == 0){
                        gtk_tree_store_set(skillstore,&iter,
                                        SKILL_STATS,stat,-1);
                }

        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(skillstore),&iter));

}

/*
 * skill_set_special_bonus
 *
 * Sets the special bnus of a skill
 *
 * Args:
 *      char *cat to set
 *      int new value
 */
void
skill_set_special_bonus(char *name,int value){
        skill_cat_set_int(name,SKILL_SPECIAL_BONUS,value);
}

/*
 * skill_set_class
 *
 * Sets the skills type to the given value
 *
 * FIXME: Should check the value is valid
 */
void
skill_set_class(char *name,char *value){
        skill_cat_set_string(name,SKILL_CLASS,value);
}

/*
 * skill_cat_set_string
 *
 * Sets a particular string element to be a certain value
 *
 * Args:
 *      char *name of cat/skill to be set
 *      int field of store to set
 *      char *val the new value to set
 */
static void
skill_cat_set_string(char *cat,int field,char *val){
        GtkTreeIter *iter;

        iter = skill_get_by_name(cat);
        if (iter == NULL)
                return;

        gtk_tree_store_set(skillstore,iter,field,val,-1);

        g_free(iter);

        return;
}

static char *
skill_cat_get_string(char *cat,int field){
        GtkTreeIter *iter;
        char *val;

        iter = skill_get_by_name(cat);
        if (iter == NULL)
                return NULL;

        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),iter,field,&val,-1);

        g_free(iter);

        return val;

}

static void
skill_cat_set_int(char *cat,int field,int value){
        GtkTreeIter *iter;

        iter = skill_get_by_name(cat);
        if (iter == NULL)
                return;

        gtk_tree_store_set(skillstore,iter,field,value,-1);

        g_free(iter);

        return;
}
static int
skill_cat_get_int(char *cat,int field){
        GtkTreeIter *iter;
        int val;

        iter = skill_get_by_name(cat);
        if (iter == NULL){
                printf("Skill %s is not found\n",cat);
                return -1;
        }

        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),iter,field,&val,-1);

        g_free(iter);

        return val;

}

/*
 * skill_get_by_name
 *
 * Gets the GtkTreeIter for the specific skill
 *
 * Args:
 *      char * name of skill <cat>\[<subskill>\..]<skill>
 * Returns
 *      GtkTreeIter * skill iter (to be freed by caller)
 */
static GtkTreeIter *
skill_get_by_name(char *skill){
        GtkTreeIter *iter;
        char **strs;
        char **part;

        iter = NULL;

        if (skill == NULL)
                return NULL;

        strs = g_strsplit(skill,"\\",-1); /* An unlimited number of tokens */

        for (part = strs; *part != NULL ; part ++){
                iter = tree_lookup(GTK_TREE_MODEL(skillstore),
                                skillprimaries,*part,iter);
                if (iter == NULL){
                        printf("Could not find:\n\t'%s' in \n\t'%s'\n",*part,skill);
                        break; /* Iter is NULL - cleanup and exit */
                }
        }

        g_strfreev(strs);
        return iter;
}

/***********************************************
 * Buying Related Functions                    *
 ***********************************************/
struct buyinfo buyinfo;

/*
 * skill_buy_cb();
 *
 * Callback for menu function
 * Does normal callback
 */
void
skill_buy_dp(void){
        if (file_modify() == FALSE){
                /* FIXME: Print a message */
                return;
        }

        skill_buy_ranks(BUY_DP,0,0);
}

/*
 * begin_buy
 *
 * Begin buying skills
 * FIXME: This function is huge - it should be made sane
 */
void
skill_buy_ranks(buymode_t buymode,int points,int limit){
        GtkWidget *buywindow;
        GtkWidget *scrollwin;
        GtkWidget *button;
        GtkWidget *label;
        GtkWidget *row;
        char *str;


        if (buymode != BUY_DP && buymode != BUY_RANKS){
                printf("Unknown buymode %d\n",buymode);
                return;
        }

        if (buymode == BUY_DP && points == 0)
                points = calculate_dp();
        if (buymode == BUY_RANKS && points == 0)
                points = 12;

        buyinfo.buymode = buymode;
        buyinfo.spent = 0;
        buyinfo.total = points;
        buyinfo.limit = limit;


        buywindow = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(buywindow),"Purchase List");
        gtk_window_set_transient_for(GTK_WINDOW(buywindow),GTK_WINDOW(window));
        gtk_window_set_default_size(GTK_WINDOW(buywindow),420,360);
        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(buywindow)->vbox),scrollwin);

        if (buymode == BUY_DP)
                buyview = tree_create_view(GTK_TREE_MODEL(skillstore),buy_info);
        else if (buymode == BUY_RANKS)
                buyview = tree_create_view(GTK_TREE_MODEL(skillstore),buy_info_ranks);

        gtk_container_add(GTK_CONTAINER(scrollwin),buyview);

        if (buymode == BUY_DP){
                row = new_row(GTK_DIALOG(buywindow)->vbox);
                button = gtk_button_new_with_label("Buy TP");
                gtk_box_pack_end(GTK_BOX(row),button,FALSE,TRUE,0);
                gtk_signal_connect_object (GTK_OBJECT(button), "clicked",
                        GTK_SIGNAL_FUNC(tp_select),GTK_OBJECT(button));
        }


        /* A total field */
        row = new_row(GTK_DIALOG(buywindow)->vbox);

        label = gtk_label_new("You have spent:");
        gtk_box_pack_start(GTK_BOX(row),label,FALSE,TRUE,2);
        buyinfo.spentbox = gtk_entry_new_with_max_length(3);
        gtk_entry_set_width_chars(GTK_ENTRY(buyinfo.spentbox),2);
        gtk_entry_set_editable(GTK_ENTRY(buyinfo.spentbox),FALSE);
        gtk_box_pack_start(GTK_BOX(row),buyinfo.spentbox,FALSE,TRUE,0);

        entry_set_digit(buyinfo.spentbox,0);

        if (buymode == BUY_DP){
                str = g_strdup_printf(" of %d DP\n",points);
        } else if (buymode == BUY_RANKS){
                str = g_strdup_printf(" of %d Hobby ranks",points);
        } else
                str = g_strdup(" of something");
        label = gtk_label_new(str);
        gtk_box_pack_start(GTK_BOX(row),label,FALSE,TRUE,2);
        g_free(str);

        /* Load and Save list of skills */
        button = new_button(GTK_DIALOG(buywindow)->action_area,
                        GTK_STOCK_OPEN,(GtkSignalFunc)skill_purchase_load);
        set_tip(button,"Load a list of skills");
        button = new_button(GTK_DIALOG(buywindow)->action_area,
                        GTK_STOCK_SAVE,(GtkSignalFunc)skill_purchase_save);
        set_tip(button,"Save this list of skills");

        /* OK and Cancel buttons */
        button = new_button(GTK_DIALOG(buywindow)->action_area,
                        GTK_STOCK_OK,(GtkSignalFunc)skill_purchase_skills);
        set_tip(button,"Apply these purchases");
        button = new_button(GTK_DIALOG(buywindow)->action_area,
                        GTK_STOCK_CANCEL,(GtkSignalFunc)skill_cancel_purchase);
        set_tip(button,"Cancel these purchases");
        
        /* Okay - show it */
        gtk_widget_show_all(buywindow);

        /* Second level of loop */
        gtk_main();

}

/*
 * skill_buy_rank
 *
 * Callback for editing 'new' column in callback.
 *
 * Buy some ranks in a skill - either hobby ranks or DP bonus
 *
 */
void
skill_buy_rank(GtkCellRendererText *cell, const gchar *path_string,
                const gchar *new_text, gpointer data){
        GtkTreeIter iter;
        int max;
        int cost,oldcost;
        int ranks,oldranks;

        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(skillstore),
                        &iter,path_string);
        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),&iter,
                        SKILL_CAN_BUY,&max,
                        SKILL_NEW_RANKS,&oldranks,-1);

        ranks = strtol(new_text,NULL,0);

        if (ranks > max || ranks < 0)
                return;

        if (buyinfo.buymode == BUY_DP){
                oldcost = skill_get_ranks_cost(&iter,oldranks);
                cost = skill_get_ranks_cost(&iter,ranks);
                buyinfo.spent = buyinfo.spent + cost - oldcost;
        } else if (buyinfo.buymode == BUY_RANKS){
                buyinfo.spent = buyinfo.spent + ranks - oldranks;
                cost = ranks;
        } else {
                cost = 99;
        }

        gtk_tree_store_set(skillstore,&iter,SKILL_NEW_RANKS,ranks,
                                SKILL_NEW_COST,cost,-1);
        entry_set_digit(buyinfo.spentbox,buyinfo.spent);
}


int
skill_get_ranks_cost(GtkTreeIter *iter,int ranks){
        char *coststr;
        int cost;

        iter = get_parent(GTK_TREE_MODEL(skillstore),iter);

        gtk_tree_model_get(GTK_TREE_MODEL(skillstore),iter,
                        SKILL_COST,&coststr,-1);

        cost = 0;
        while (*coststr != 0 && ranks > 0){
                cost += strtol(coststr,&coststr,10);
                ranks --;
                if (*coststr == '/')
                        coststr ++;
        }

        if (ranks > 0)
                printf("Warning: Attempt to buy %d extra ranks\n",ranks);

        return cost;
}

static void
skill_purchase_skills(void){
        gtk_tree_model_foreach(GTK_TREE_MODEL(skillstore),
                        skill_purchase_apply,&buyinfo);
        gtk_widget_destroy(gtk_widget_get_toplevel(buyview));
        skill_update();
        gtk_main_quit();
}

void
skill_purchase_skills_times(char *filename){
        gtk_tree_model_foreach(GTK_TREE_MODEL(skillstore),
                        skill_purchase_apply,&buyinfo);
	skill_purchase_add(filename);
}


static void
skill_cancel_purchase(void){
        buyinfo.buymode = BUY_CANCEL;

        gtk_tree_model_foreach(GTK_TREE_MODEL(skillstore),
                        skill_purchase_apply,&buyinfo);
        gtk_widget_hide(gtk_widget_get_toplevel(buyview));
        skill_update();
        gtk_main_quit();
}

/*
 * skill_purchase_save
 *
 * Saves a the current list of purchased skills (the ones in the
 *      new list)
 *
 */
static void
skill_purchase_save(void){
        static GtkWidget *select = NULL;
        int response;
        const char *file;
        element_t *el;

        if (select == NULL){
                select = gtk_file_selection_new("Save skill purchases");
                gtk_file_selection_set_filename(GTK_FILE_SELECTION(select),
                                "../characters/skillsets/");
                gtk_file_selection_complete(GTK_FILE_SELECTION(select),"*.xml");
        }
        gtk_widget_show_all(select);

        response = gtk_dialog_run(GTK_DIALOG(select));
        gtk_widget_hide(select);
        switch (response){
        case GTK_RESPONSE_OK:
                file = gtk_file_selection_get_filename(
                                GTK_FILE_SELECTION(select));
                break;
        case GTK_RESPONSE_CANCEL:
                return;
        default:
                g_warning("Got response %d from dialog run\n",response);
                return;
        }

        /* Save the file */
        el = create_element(NULL,"purchases",ETYPE_ELEMENT);
        tree_save(GTK_TREE_MODEL(skillstore),skill_store_purchases,el);
        /* FIXME: This needs to be defined in a DTD */
        alena_save(file,el,NULL);
        /*alena_save(file,el,SKILLSETDTD);*/
}

/* skill_purchase_add
 *
 * Adds the ranks purchased to the store.
 */
void
skill_purchase_add(const char *filename){
	element_t *el;
        GList *elements;
	
        /* Load the file */
        el = alena_parsefile(filename,WHITESPACE_IGNORE);
	if (el == NULL) {
		printf("el NULL for filename=%s\n", filename);
		exit(1);
	}


        elements = el->elements;
        while (elements != NULL){
                el = elements->data;
                skill_insert_xml(el,NULL,skill_store_purchases);

                elements = g_list_next(elements);
        }
}

/*
 * skill_purchase_load
 *
 * Loads a skill purchase list into the store.
 * Returns the name of the file loaded.
 *
 */
char *
skill_purchase_load(void){
        static GtkWidget *select = NULL; /* FIXME: Should I share this with
                                            save */
        int response;
        char *file;

        if (select == NULL){
                select = gtk_file_selection_new("Load skill purchases");
                gtk_file_selection_set_filename(GTK_FILE_SELECTION(select),
                                "../characters/skillsets/");
                gtk_file_selection_complete(GTK_FILE_SELECTION(select),"*.xml");
        }
        gtk_widget_show_all(select);

        response = gtk_dialog_run(GTK_DIALOG(select));
        gtk_widget_hide(select);
        switch (response){
        case GTK_RESPONSE_OK:
                file = (char *) gtk_file_selection_get_filename(
                                GTK_FILE_SELECTION(select));
                break;
        case GTK_RESPONSE_CANCEL:
                return NULL;
        default:
                g_warning("Got response %d from dialog run\n",response);
                return NULL;
        }

	skill_purchase_add(file);

	return file;
}



/*
 * skill_purchase_apply
 *
 * Applies or discards the purchases after 'OKAY' or 'CANCEL' has been
 * clicked.
 *
 * FIXME: Classes are hardcoded.  Should be read from an array
 *      and handle 'R' for restricted and the like
 *
 */
static gboolean
skill_purchase_apply(GtkTreeModel *model, GtkTreePath *path,
                GtkTreeIter *iter, gpointer data){
        int ranks,purchases;
        gboolean half;
        char *class;
        struct buyinfo *buy;

        if (data == NULL){
                printf("Buyinfo is null\n");
                return TRUE; /* end tree walk */
        }
        buy = (struct buyinfo *)data;

        gtk_tree_model_get(model,iter,SKILL_NEW_RANKS,&purchases,
                        SKILL_RANKS,&ranks,
                        SKILL_HALF_RANK,&half,
                        SKILL_CLASS,&class,-1);

        switch (buy->buymode){
        case BUY_DP:
                if (class == NULL){
                        /* Easy case */
                        ranks += purchases;
                } else if (strcmp(class,"Everyman") == 0){
                        ranks += purchases * 2;
                } else if (strcmp(class,"Occupational") == 0){
                        ranks += purchases * 3;
                } else if (strcmp(class,"Restricted") == 0){
                        if (half == TRUE)
                                purchases ++;
                        half = (gboolean)(purchases % 2);
                        ranks += (purchases / 2);
                } else {
                        printf("Class %s unknown\n",class);
                }
                break;
        case BUY_RANKS:
                /* Class is ignored */
                ranks += purchases;
                break;
        case BUY_CANCEL:
        default:
                /* nothing */
                break;
        };

        gtk_tree_store_set(GTK_TREE_STORE(model),iter,
                        SKILL_RANKS,ranks,
                        SKILL_NEW_RANKS,0,
                        SKILL_NEW_COST,0,
                        SKILL_HALF_RANK,half,
                        -1);
        /* To make the walk continue */
        return FALSE;
}

/*************************************
 * Accessor Methods                  *
 *************************************/

GList *
skill_get_cat_list(char *prefix){
        GtkTreeIter iter;
        gboolean valid;
        char *data;
        int len;
        GList *list = NULL;

        if (prefix == NULL){
                printf("Can't search for NULL prefix\n");
                return NULL;
        }

        len = strlen(prefix);

        valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(skillstore),&iter);

        while (valid){
                gtk_tree_model_get(GTK_TREE_MODEL(skillstore),&iter,
                                SKILL_NAME,&data,-1);
                if (data == NULL){
                        printf("Invalid name\n");
                        break;
                }

                if (strncmp(prefix,data,len) == 0){
                        list = g_list_prepend(list,data);
                }

                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(skillstore),
                                &iter);
        }
        list = g_list_reverse(list);

        return list;

}

/*
 * skill_get_skill_list
 *
 * Get the skill list for the supplied category
 *
 * Arg:
 *      cat to get the skills from
 * Returns:
 *      Glist* of the skills
 */
GList *
skill_get_skill_list(char *catname){
        GList *list;
        GtkTreeIter *cat;
        GtkTreeIter skill;
        gboolean flag;
        char *str;

        if (catname == NULL)
                return NULL;

        list = NULL;


        cat = skill_get_by_name(catname);
        if (cat == NULL)
                return NULL;

        flag = gtk_tree_model_iter_children(GTK_TREE_MODEL(skillstore),
                        &skill,cat);
        while (flag){
                gtk_tree_model_get(GTK_TREE_MODEL(skillstore),&skill,
                                SKILL_NAME,&str,-1);
                list = g_list_prepend(list,str);

                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(skillstore),
                                &skill);
        }
        list = g_list_reverse(list);

        g_free(cat);
        return list;

}

/*
 * skill_move
 *
 * Move the given skill from to the specified category
 *
 * Args:
 *      char *skill to move
 *      char *dest category
 * Return
 *      TRUE on success
 *      FAIL on failure
 */
gboolean
skill_move(char *srcname,char *destname){
        GtkTreeIter *dest;
        GtkTreeIter *src;

        src = skill_get_by_name(srcname);
        dest = skill_get_by_name(destname);

        tree_copy_item(skillstore,src,dest);
        tree_delete_item(skillstore,skill_store_info,src);

        g_free(src);
        g_free(dest);

        return TRUE;
}

/*
 * skill_delete
 *
 * Delete the given skill - will delete any subskills too
 *
 * Args:
 *      char *skill/cat to delete
 */
void
skill_delete(char *name){
        GtkTreeIter *skill;

        skill = skill_get_by_name(name);
        if (skill == NULL){
                printf("Skill %s not found\n",name);
                return;
        }

        tree_delete_item(skillstore,skill_store_info,skill);
        g_free(skill);
}

/*
 * skill_edit_callback
 *
 * New edit skill stuff based on popup info from tree.c
 *
 */
static int
skill_edit_callback(GtkWidget *widget, GdkEventButton *event, gpointer data){
        gboolean sel;

        if (event->type != GDK_2BUTTON_PRESS){
                return FALSE; /* Do normal handling */
        }

        sel = gtk_tree_selection_get_selected(gtk_tree_view_get_selection
                                        (GTK_TREE_VIEW(skillview)),
                                        NULL, &(popupinfo->iter));
        if (sel == FALSE){
                /* Nothing selected - ignore it */
                return FALSE;
        }

        tree_set_popup(popupinfo);
        gtk_widget_show_all(popupinfo->window);

        return TRUE;
}



static void
skill_create_new_cb(struct rcmenu_info *minfo,GtkTreeIter *iter){
        int depth;
        const char *element_name = NULL;
        GtkTreeIter parent;
        char *stats = "";
        char *name = "New Skill";

        depth = gtk_tree_store_iter_depth(minfo->store,iter);
        assert(depth < 3);

        if (depth == 0){
                element_name = "category";
                stats = "St+St+St";
                name = "New Category";
        } else if (depth == 1){
                element_name = "skill";
        } else if (depth == 2){
                element_name = "skill";
                /* Make sure the parent is a skillgroup */
                gtk_tree_model_iter_parent(GTK_TREE_MODEL(minfo->store),
                                &parent,iter);
                gtk_tree_store_set(minfo->store,&parent,
                                SKILL_ELEMENT_NAME,"skillgroup",
                                SKILL_RANKS,0, /* Groups have no ranks */
                                SKILL_CAN_BUY,FALSE,
                                -1);
        }

        assert(element_name != NULL);

        /* FIXME: Should be more intelligent about setting these */
        gtk_tree_store_set(minfo->store,iter,
                        SKILL_ELEMENT_NAME, element_name,
                        SKILL_NAME, name,
                        /* FIXME: only set if a skill */
                        SKILL_STATS,  stats,
                        SKILL_CLASS,  "Normal",
                        SKILL_PROGRESSION, "Standard",
                        SKILL_SPECIAL_BONUS, 0,
                        SKILL_DESCRIPTION, "Put your description here",
                        SKILL_TRUE, TRUE,
                        SKILL_CAN_BUY,TRUE,
                        -1);
}

