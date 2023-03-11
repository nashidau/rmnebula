/*
 * stats.c
 *
 * Stat handling functions for nebula
 *
 * $Id: stats.c,v 1.42 2003/01/08 22:27:29 nash Exp $
 *
 * Log at bottom
 */

#include "nebula.h"
#include "tree.h"
#include "libneb.h"
#include "stats.h"

struct special_stats {
        char *name;
        char *abbr;
        char *key;
        gboolean used;
        gboolean exists;
        void (*create_fn)(void);
        void (*update_fn)(GtkTreeIter *);
};



static gboolean update_special_stats(GtkTreeIter *iter);
static int get_stat_bonus(int temp);
void stat_to_xml_tree(element_t *el,GtkTreeIter *iter);
static void stat_gain_roll(int,int,int *,int *);
static int stat_get_int(GtkTreeIter *iter,int field);
static GtkTreeIter *stat_get(char *name);
static int update_stat_bonus(GtkTreeIter *iter);

static void stat_set_integer(char *name,int pos,int val);
static void stat_set_flag(char *name,int pos,gboolean flag);

/* Creation and update of special stats */
static void create_appearance(void);
static void create_will(void);
static void create_sanity(void);
static void update_appearance(GtkTreeIter *iter);
static void update_will(GtkTreeIter *iter);
static void update_sanity(GtkTreeIter *iter);


struct special_stats specials[] = {
        { "Appearance","Ap", STAT_OPTION "appearance",
                0, 0, create_appearance, update_appearance },
        { "Will","Wl", STAT_OPTION "willpower",
                        0, 0, create_will, update_will },
        { "Sanity","Sn", STAT_OPTION "sanity",
                                0, 0, create_sanity, update_sanity },
        { NULL, NULL, NULL, 0, 0, NULL, NULL },
};




/*
 * stat_init
 *
 * Create the store and normal view for stats
 *
 * Args:
 *      GtkWidget:      The notepage to put the stat list in.
 *
 */
void
stat_init(GtkWidget *box){
        GtkTreeSelection *select;

        statstore = make_tree_store(stat_store_info);
        statview = tree_create_view(GTK_TREE_MODEL(statstore),stat_view_info);
        g_object_unref(G_OBJECT(statstore));

        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(statview));
        gtk_tree_selection_set_mode(select,GTK_SELECTION_SINGLE);

        gtk_container_add(GTK_CONTAINER(box),statview);
}

void
stat_clear(void){
        gtk_tree_store_clear(statstore);
}

void
stat_insert(element_t *el){
        GList *elements;

        elements = el->elements;
        while (elements != NULL){
                el = elements->data;
                tree_insert(statstore,stat_store_info,statprimaries,el,
                                NULL,NULL);

                elements = g_list_next(elements);
        }
}
/*
 * stat_gain
 *
 * Perform stat gain on a characters stats
 *
 *
 * note: Changed stats should be red or something
 * note: handles sanity, will and appearance by ignoring them
 */
GtkWidget *gainwindow;
void
stat_gain(void){
        int temp,pot;
        int newtemp,newpot;
        int stats;
        GtkWidget *gainview;
        int result;
        GtkTreeIter iter;
        gboolean flag;

        if (gainwindow != NULL)
                return;

        if (file_modify() == FALSE){
                /* FIXME: Print a message */
                return;
        }

        /* New version */
        flag = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(statstore),&iter);
        if (flag == FALSE){
                /* there are no stats to gain */
                return;
        }
        stats = 0;
        while (flag && stats < 10){
                gtk_tree_model_get(GTK_TREE_MODEL(statstore),&iter,
                                STAT_TEMP,&temp,
                                STAT_POT,&pot,
                                -1);
                stat_gain_roll(temp,pot,&newtemp,&newpot);
                gtk_tree_store_set(statstore,&iter,
                                STAT_GAIN_TEMP,newtemp,
                                STAT_GAIN_POT,newpot,
                                -1);
                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),
                                &iter);
                stats ++;
        }

        /* Prevent appearance from messing up */
	while(flag){
                gtk_tree_model_get(GTK_TREE_MODEL(statstore),&iter,
                                STAT_TEMP,&temp,
                                STAT_POT,&pot,
                                -1);
                gtk_tree_store_set(statstore,&iter,
                                STAT_GAIN_TEMP,temp,
                                STAT_GAIN_POT,pot,
                                -1);
                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),
                                &iter);
	}

        gainwindow = gtk_dialog_new_with_buttons("Stat Gain",
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK,
                        GTK_RESPONSE_OK,
                        NULL);

        gainview = tree_create_view(GTK_TREE_MODEL(statstore),stat_gain_view_info);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(gainwindow)->vbox),
                        gainview);

        gtk_widget_show_all(gainwindow);

        /* Let the dialog run for a while */
        result = gtk_dialog_run(GTK_DIALOG(gainwindow));

        gtk_widget_destroy(gainwindow);

        if (result == GTK_RESPONSE_OK) {
                apply_gain();
        }

        gainwindow = NULL;

        return;
}

static void
apply_gain(void){
        GtkTreeIter iter;
        int temp,pot;

        /* Apply the changes */
        gtk_tree_model_get_iter_first(GTK_TREE_MODEL(statstore),&iter);
        do {
                gtk_tree_model_get(GTK_TREE_MODEL(statstore),&iter,
                                STAT_GAIN_TEMP,&temp,
                                STAT_GAIN_POT,&pot,
                                -1);
                gtk_tree_store_set(statstore,&iter,
                                STAT_TEMP,temp,
                                STAT_POT,pot,
                                -1);
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),&iter));
}

/*
 * stat_gain_roll
 *
 * Roll one set of stat gains
 *
 * If newpot != NULL statgain is done on potentials too
 */
void
stat_gain_roll(int temp,int pot,int *newtemp,int *newpot){
        int diff,gain;
        int die1,die2;
        gboolean potgain;

        assert(newtemp != NULL);
        assert(newpot  != NULL);

        diff = pot - temp;

        die1 = d10();
        die2 = d10();

        if (die1 == die2 && die1 > 5){
                gain = die1 + die2;
        } else if (die1 == die2 && die1 < 6){
                gain = -die1;
        } else if (diff <= 10){
                gain = MIN(die1,die2);
        } else if (diff <= 20){
                gain = MAX(die1,die2);
        } else { /* diff > 20) */
                gain = die1 + die2;
        }

        /* Calculate the new temp */
        *newtemp = MIN(temp + gain,pot);

        /* Now calculate potential gain */
        *newpot = pot;
        potgain = config_get_bool(STAT_OPTION "potgain");

        /* Calculate the new pot (if needed) */
        if (potgain == TRUE && die1 == 10 && die2 == 10){
                if (*newpot > 90)
                        *newpot = 90 + (*newpot - 90) * (*newpot - 90);
                *newpot += d10() + d10();
                if (*newpot > 90){
                        *newpot -= 90;
                        *newpot = 90 + (int)sqrt((double)*newpot);
                }
        }

        return;
}

void
stat_update(void){
	GtkTreeIter item;
	gboolean valid;
	gboolean handled;
        struct special_stats *stats;

        /* Prepare special stats */
        for (stats = specials ; stats->name != NULL ; stats ++){
                stats->used = config_get_bool(stats->key);
        }
        
        
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(statstore),&item);

	while (valid == TRUE){
		handled = update_special_stats(&item);
                if (item.stamp == 0)
                        break; /* Just handled the last one */
                if (handled == TRUE){
                        continue;
                }
                
                tree_refresh_item(stat_store_info,statstore,&item);
                valid = gtk_tree_model_iter_next(
                                        GTK_TREE_MODEL(statstore), &item);
	}

        /* Do we need to create any special stats */
        for (stats = specials ; stats->name != NULL ; stats ++){
                if (stats->used == TRUE && stats->exists == FALSE){
                        if (stats->create_fn == NULL){
                                printf("Warning: Can't create %s\n",stats->
                                                name);
                                continue;
                        }
                        stats->create_fn();
                }
        }

        for (stats = specials ; stats->name != NULL ; stats ++){
                stats->used = FALSE;
                stats->exists = FALSE;
        }
                
        
}

int
calculate_dp(void){
        int total = 0;

        total = stat_get_temp("Ag") +
                stat_get_temp("Co") +
                stat_get_temp("Me") +
                stat_get_temp("Re") +
                stat_get_temp("SD");

        return round_div(total,5);
}

/* Find the average bonus from all the realm
 * stats.
 */
int
get_realm_bonus(void){
        GtkTreeIter iter;
        gboolean flag;
        gboolean realm;
        int val;
        int stats = 0;
        int total = 0;

        /* New version */
        flag = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(statstore),&iter);
        while (flag){
                gtk_tree_model_get(GTK_TREE_MODEL(statstore),&iter,
                                STAT_REALM,&realm,
                                STAT_TOTAL_BONUS,&val,
                                -1);

                if (realm == TRUE){
                        stats ++;
                        total += val;
                }

                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),
                                &iter);
        }

        return round_div(total,stats);
}

/* Straight from the table ... hardcoded numbers .. bleah :-P */
static int
update_stat_bonus(GtkTreeIter *stat){
        int val;

        gtk_tree_model_get(GTK_TREE_MODEL(statstore),stat,STAT_TEMP,&val,-1);

        return get_stat_bonus(val);
}



static int
get_stat_bonus(int temp){
        if (temp < 0){
                printf("Can't handle stats less then 0\n");
                exit(1);
        }

        if (temp > 100)  return (temp - 95) * 2;

        return stat_bonuses[temp];
}


void
stat_save(element_t *root){
        root = create_element(root,"stats",ETYPE_ELEMENT);

        tree_save(GTK_TREE_MODEL(statstore),stat_store_info,root);
}

/*
 * Seeds Appearance after presence and other stats have been done
 */
void
stat_create_appearance(void){
        gboolean rmssappearance;

        rmssappearance = config_get_bool(CREATION_OPTION "appearance");

        if (rmssappearance == TRUE){
                fprintf(stderr,"Apperance currently not implemented\n");
                /*
                // PR Pot based appearance
                tmpap = pr->pot;
                tmpap -= 25;
                tmpap += nd10(5);
                */
        } else {
                /*
                // PR Bonus based appearance
                tmpap = d100();
                tmpap += pr->total_bonus;
                */
        }
        /*
        ap = getstat("Ap");
        ap->temp = tmpap;
        ap->pot = tmpap;
        */
}


gboolean
stat_is_prime_requisite(char *name){
        GtkTreeIter *stat;
        gboolean rv;

        stat = stat_get(name);
        gtk_tree_model_get(GTK_TREE_MODEL(statstore),stat,
                                        STAT_PRIME,&rv,-1);
        return rv;
}

int
stat_get_temp(char *stat){
        GtkTreeIter *iter;

        iter = stat_get(stat);
        if (iter == NULL)
                return -1;

        return stat_get_int(iter,STAT_TEMP);
}

int
stat_get_potential(char *stat){
        GtkTreeIter *iter;

        iter = stat_get(stat);

        if (iter == NULL)
                return -1;

        return stat_get_int(iter,STAT_POT);
}

int
stat_get_bonus(char *stat){
        GtkTreeIter *iter;

        if (strcmp(stat,"none") == 0)
                return 0;
        
        iter = stat_get(stat);

        if (iter == NULL){
                g_warning("Couldn't find stat %s\n",stat);
                return 0;
        }

        return stat_get_int(iter,STAT_TOTAL_BONUS);
}

static int
stat_get_int(GtkTreeIter *iterp,int field){
        int val;

        gtk_tree_model_get(GTK_TREE_MODEL(statstore),iterp,
                                        field,&val,-1);
        return val;
}

static GtkTreeIter *
stat_get(char *name){
        return tree_lookup(GTK_TREE_MODEL(statstore),statprimaries,name,NULL);
}

/*
*/
void
stat_set_prime_requisite(char *name,gboolean pr){
        stat_set_flag(name,STAT_PRIME,pr);
}
void
stat_set_realm_stat(char *name,gboolean realm){
        stat_set_flag(name,STAT_REALM,realm);
}
void
stat_set_racial_bonus(char *name,int bonus){
        stat_set_integer(name,STAT_RACIAL_BONUS,bonus);
}
void
stat_set_temp(char *name,int temp){
        stat_set_integer(name,STAT_TEMP,temp);
}
void
stat_set_potential(char *name,int pot){
        stat_set_integer(name,STAT_POT,pot);
}


void
stat_set_integer(char *name,int pos,int val){
        GtkTreeIter *stat;
        GType type;

        stat = stat_get(name);
        if (stat == NULL){
                printf("stat.c: There is not stat %s\n",name);
                return;
        }

        if (pos > STAT_GAIN_TEMP){
                printf("stat.c: stat field %d is not valid\n",pos);
                return;
        }
        type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(statstore),pos);
        if (type != G_TYPE_INT){
                printf("stat.c: field %d is a %s, not an %s\n",pos,
                                g_type_name(type),g_type_name(G_TYPE_INT));
                return;
        }

        gtk_tree_store_set(statstore,stat,pos,val,-1);
}

void
stat_set_flag(char *name,int pos,gboolean flag){
        GtkTreeIter *stat;
        GType type;

        stat = stat_get(name);
        if (stat == NULL){
                printf("stat.c: There is not stat %s\n",name);
                return;
        }

        if (pos > STAT_GAIN_POT){
                printf("stat.c: Field %d is not a boolean\n",pos);
                return;
        }
        type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(statstore),pos);
        if (type != G_TYPE_BOOLEAN){
                printf("stat.c: field %d is a %s, not an %s\n",pos,
                                g_type_name(type),g_type_name(G_TYPE_BOOLEAN));
                return;
        }
        gtk_tree_store_set(statstore,stat,pos,flag,-1);
}

/*
 * stat_names
 *
 * Returns a list of all stat names - can be used with stat_set to
 * set or get values in order
 *
 * Returns:
 *      GSList * A list of stat names (char *)
 *              Must be freed by caller
 */
GSList *
stat_names(void){
        GtkTreeIter iter;
        char *str;
        gboolean flag;
        GSList *list = NULL;

        flag = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(statstore),&iter);
        while (flag){
                gtk_tree_model_get(GTK_TREE_MODEL(statstore),&iter,
                                        STAT_NAME,&str,
                                        -1);
                list = g_slist_prepend(list,str);

                flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),
                                &iter);
        }

        list = g_slist_reverse(list);

        return list;
}

/*******************************************************
 * Edit Stats                                          *
 *******************************************************/
void
stat_edit(void){
        GtkWidget *editview;
        GtkTreeIter iter;
        int result;
        int temp,pot;

        if (file_modify() == FALSE)
                return;

        if (gainwindow != NULL)
                return; /* re-entrant */

        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(statstore),&iter)
                        == FALSE)
                return;

        do {
                gtk_tree_model_get(GTK_TREE_MODEL(statstore),&iter,
                                STAT_TEMP,&temp,STAT_POT,&pot,-1);
                gtk_tree_store_set(statstore,&iter,
                                STAT_GAIN_TEMP,temp,STAT_GAIN_POT,pot,-1);
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),&iter));

        gainwindow = gtk_dialog_new_with_buttons("Edit Stats",
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK,
                        GTK_RESPONSE_OK,
                        GTK_STOCK_CANCEL,
                        GTK_RESPONSE_CANCEL,
                        NULL);

	tree_set_flag(statstore,NULL,STAT_EDIT_FLAG,TRUE);
	
        editview = tree_create_view(GTK_TREE_MODEL(statstore),stat_edit_view);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(gainwindow)->vbox),
                        editview);

        gtk_widget_show_all(gainwindow);

        /* Let the dialog run for a while */
        result = gtk_dialog_run(GTK_DIALOG(gainwindow));

        gtk_widget_destroy(gainwindow);
        gainwindow = NULL;

        if (result == GTK_RESPONSE_OK) {
                apply_gain();
        }

	tree_set_flag(statstore,NULL,STAT_EDIT_FLAG,FALSE);

}

void
stat_set_value_temp_cb(GtkCellRendererText *cell, const gchar *path_string,
                const gchar *new_text, gpointer data){
        int temp;
        GtkTreeIter iter;

        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(statstore),
                        &iter,path_string);
        temp = strtol(new_text,NULL,0);
        gtk_tree_store_set(statstore,&iter,STAT_GAIN_TEMP,temp,-1);

}

void
stat_set_value_pot_cb(GtkCellRendererText *cell, const gchar *path_string,
                const gchar *new_text, gpointer data){
        int pot;
        GtkTreeIter iter;

        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(statstore),
                        &iter,path_string);
        pot = strtol(new_text,NULL,0);
        gtk_tree_store_set(statstore,&iter,STAT_GAIN_POT,pot,-1);
}

/*
 * Handle special stats
 *
 * Deals with the special stats:
 *	Will
 *	Sanity
 *	Appearance
 *
 * 
 *
 * Returns:
 *	TRUE if handled, iter is moved to next item
 *	FALSE if not handled
 *
 */
static gboolean
update_special_stats(GtkTreeIter *iter){
        char *str;
        struct special_stats *stats;


        assert(iter != NULL);
        
	gtk_tree_model_get(GTK_TREE_MODEL(statstore),iter,STAT_NAME,&str,-1);
	if (str == NULL || strlen(str) == 0){
		printf("Warning: no skill name\n");
		return TRUE; /* Errors are handling ;-) */
	}

        
        for (stats = specials; stats->name != NULL ; stats ++){
                if (strcmp(str,stats->name) != 0 && 
                                strcmp(str,stats->abbr) != 0)
                        continue;

                stats->exists = TRUE;
                
                if (stats->used == FALSE){
                        gtk_tree_store_remove(statstore,iter);
                        return TRUE;
                }
                
                stats->update_fn(iter);
                
                if (!(gtk_tree_model_iter_next(GTK_TREE_MODEL(statstore),iter)))
                        iter->stamp = 0;
                
                return TRUE;
        }
	
	return FALSE;
}

static void
update_will(GtkTreeIter *iter){
        int temp;
        int racial,special;
        int total;
        
        assert(iter != NULL);
        assert(iter->stamp != 0);

        temp =  stat_get_bonus("Reasoning") +
                stat_get_bonus("Self Discipline") +
                stat_get_bonus("Empathy") +
                stat_get_bonus("Intuition") +
                stat_get_bonus("Presence");

        gtk_tree_model_get(GTK_TREE_MODEL(statstore),iter,
                        STAT_RACIAL_BONUS,&racial,
                        STAT_SPECIAL_BONUS,&special,
                        -1);

        total = temp + racial + special;

        gtk_tree_store_set(statstore,iter,
                        STAT_TEMP,temp,
                        STAT_POT,temp,
                        STAT_TOTAL_BONUS,total,
                        -1);
} 

static void
create_will(void){
        GtkTreeIter will;
        
        gtk_tree_store_append(statstore,&will,NULL);

        assert(will.stamp != 0);

        gtk_tree_store_set(statstore,&will,
                        STAT_ELEMENT_NAME,"stat",
                        STAT_NAME,"Will",
                        STAT_ABBR,"Wl",
                        STAT_RACIAL_BONUS,0,/* FIXME: shou;d be based on race */
                        STAT_SPECIAL_BONUS,0,
                        STAT_PRIME,FALSE,
                        STAT_REALM,FALSE,
                        STAT_GAIN_TEMP,0,
                        STAT_GAIN_POT,0,
                        STAT_EDIT_FLAG,FALSE,
                        -1);
                        
        update_will(&will);
}

static void
update_sanity(GtkTreeIter *iter){
        int sd,pr;
        int cur;
        int pot,racial,special;
        int bonus;

        sd = stat_get_potential("Self Discipline");
        pr = stat_get_potential("Presence");
        pot = round_div(sd + pr,2);
        
        gtk_tree_model_get(GTK_TREE_MODEL(statstore),iter,
                        STAT_RACIAL_BONUS,&racial,
                        STAT_SPECIAL_BONUS,&special,
                        STAT_TEMP,&cur,
                        -1);

        if (pot < cur)
                cur = pot;
        
        bonus = racial + special + get_stat_bonus(cur);

        gtk_tree_store_set(statstore,iter,
                        STAT_TEMP,cur,
                        STAT_POT,pot,
                        STAT_TOTAL_BONUS,bonus,
                        -1);
}
static void
create_sanity(void){
        GtkTreeIter sanity;
        int val;
        
        gtk_tree_store_append(statstore,&sanity,NULL);

        assert(sanity.stamp != 0);

        val = stat_get_potential("Self Discipline")+
                stat_get_potential("Presence");

        gtk_tree_store_set(statstore,&sanity,
                        STAT_ELEMENT_NAME,"stat",
                        STAT_NAME,"Sanity",
                        STAT_ABBR,"Sn",
                        STAT_RACIAL_BONUS,0,/* FIXME: shou;d be based on race */
                        STAT_SPECIAL_BONUS,0,
                        STAT_PRIME,FALSE,
                        STAT_REALM,FALSE,
                        STAT_TEMP,val,
                        STAT_GAIN_TEMP,0,
                        STAT_GAIN_POT,0,
                        STAT_EDIT_FLAG,FALSE,
                        -1);
                        
        update_sanity(&sanity);
}

/* 
 * As far as I am concerned appearance doesn't change due to presence chaning
 */
static void
update_appearance(GtkTreeIter *iter){
        int app;
        char *buf;
        
        gtk_tree_model_get(GTK_TREE_MODEL(statstore),iter,
                        STAT_POT,&app,-1);

        buf = g_strdup_printf("%d",app);
        characteristic_set("\\appearance\\roll",buf);
        g_free(buf);
}

static void
create_appearance(void){
        int app;
        char *str;
        GtkTreeIter appearance;
        
        str = characteristic_get("\\appearance\\roll");
        
        if (str == NULL || strlen(str) == 0){
                stat_create_appearance();
                return;
        }

        app = strtol(str,NULL,0);
                
        if (app == 0){
                stat_create_appearance();
                return;
        }
        
        gtk_tree_store_append(statstore,&appearance,NULL);
        assert(appearance.stamp != 0);
        gtk_tree_store_set(statstore,&appearance,
                        STAT_ELEMENT_NAME,"stat",
                        STAT_NAME,"Appearance",
                        STAT_ABBR,"Ap",
                        STAT_RACIAL_BONUS,0,/* FIXME: shou;d be based on race */
                        STAT_SPECIAL_BONUS,0,
                        STAT_PRIME,FALSE,
                        STAT_REALM,FALSE,
                        STAT_TEMP,app,
                        STAT_POT,app,
                        STAT_TOTAL_BONUS,app,
                        STAT_GAIN_TEMP,0,
                        STAT_GAIN_POT,0,
                        STAT_EDIT_FLAG,FALSE,
                        -1);
 
        
}
