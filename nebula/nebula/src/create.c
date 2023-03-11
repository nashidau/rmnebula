#include "nebula.h"
#include "create.h"
#include "parse.h"
#include "nebfile.h"
#include "libneb.h"

/*
 * New creation fucntions 
 */
static void parse_choice(element_t *root);




static void create_next(void);
GList *list_grep(char *str,GList *src);
void print_list(GList *list);

GtkWidget *creation_window;
static int stage = -1;

char *prefix;
const char *path; /* FIXME: Seedy hardcodedness */

extern struct tags parsers[];

struct create_steps {
        void (*start)(void);
};

struct create_steps create_steps[] = {
        { begin_genre },
        { begin_name },
        { begin_race },
        { begin_profession },
        { begin_realm },
        { begin_realm_subtype },
        { begin_weapons },
        { begin_spells },
        { begin_spell_setup },
        { begin_stats },
        { begin_hobby_ranks },
        //{ begin_talents },
        //{ begin_adolescent_development},
        { begin_end },
        { NULL },
};

struct fileinfo *
begin_create(void){
        struct fileinfo *finfo;
        int rv = 0;
        char *msg;

        if (stage != -1) {
                printf("Currently in stage %d\n",stage);
                printf("Can't create new character\n");
                return NULL;
        }

        file_set_modified();

        rv = nebfile_open_data_file(BASE_XML,parsers);
        if (rv == -1){
                msg = g_strdup_printf("Could not find the file %s\n"
                                "Check the file path.",BASE_XML);
                gui_error_msg(msg);
                g_free(msg);
                return NULL;
        }

        stage = -1;

        set_prefix("");

        path = config_get_string(FILE_OPTION "datapath");
        if (path == NULL){
                config_set_string(FILE_OPTION "datapath",CONFIG_PATH);
                path = CONFIG_PATH;
        }

        finfo = g_malloc0(sizeof(struct fileinfo));
        finfo->name = NULL;
        finfo->state = FILE_MODIFIED;

        create_next();


        return finfo;

}

static void
create_next(void){

        /* Move forward to the next step */
        stage ++;

        /* Is that the end? */
        if (create_steps[stage].start == NULL){
                stage = -1;
                return;
        }


        /* Call function */
        (create_steps[stage].start)();


}

/*
 * begin_genre
 *
 * Pops up a genre selection dialog.  A genre contains all skills used in the
 * setting (Supposidly)
 */
static void
begin_genre(void){
        const char *str;
        char *filename;
        int result;

        genre_info.datapath = path;

        combo_dialog(&genre_info);

        /* Show it */
        gtk_widget_show_all(genre_info.window);

        /* Run it */
        result = gtk_dialog_run(GTK_DIALOG(genre_info.window));

        if (result != GTK_RESPONSE_OK){
                printf("Error: Invalid response\n");
                return;
        }

        /* get the list */
        str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(genre_info.combo)->entry));
        filename = g_strconcat(GENRE_PATH,str,".xml",NULL);

        /* load the selected file */
        nebfile_open_data_file(filename,parsers);

        /* kill the window */
        destroy_window(genre_info.window);

        /* Move along */
        create_next();
}

void
set_prefix_xml(element_t *el){
        set_prefix(get_text("prefix",el));
}

void
set_prefix(char *str){
        char *p;
        int len;

        if (prefix != NULL){
                free(prefix);
                prefix = NULL;
        }

        p = strchr(str,':');

        /* handle xml version */
        if (p == NULL){
                p = str;
        } else {
                p ++;
        }

        while (isspace(*p))
                p ++;

        len = strlen(p);
        prefix = calloc(len + 2,sizeof(char));
        strcpy(prefix,p);
/*
        strcat(prefix,"_");
*/
        strcat(prefix,"/");
}


static void
begin_name(void){
        GtkWidget *box,*cwindow;
        GtkWidget *name_box;
        char *name;
        int result;

        /* Open the creation window */
        cwindow = gtk_dialog_new_with_buttons("Name",GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK,
                        GTK_RESPONSE_OK,
                        NULL);

        box = new_row(GTK_DIALOG(cwindow)->vbox);
        name_box = new_text_box_large(box,"Name: ",TRUE);

        gtk_widget_show_all(cwindow);

        /* Run it */
        result = gtk_dialog_run(GTK_DIALOG(cwindow));

        if (result != GTK_RESPONSE_OK){
                printf("An error occured\n");
                return;
        }

        /* Get the results */
        name = g_strdup(gtk_entry_get_text(GTK_ENTRY(name_box)));
        characteristic_set("name",name);

        /* Clean up */
        destroy_window(name_box);

        create_next();

}

/* Race selection */
static void
begin_race(void){
        GtkWidget *box,*label,*cwindow,*race_combo;
        GList *list;
        char *filename;
        const char *race;

        cwindow = gtk_dialog_new_with_buttons("Choose Race",
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK,
                        GTK_RESPONSE_OK,
                        NULL);

        list = get_files_specific(RACE_PATH,path,prefix,".xml",NULL);


        box = new_row(GTK_DIALOG(cwindow)->vbox);
        label = gtk_label_new("Race: ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        race_combo = new_combo(box,list);

        gtk_widget_show_all(cwindow);

        gtk_dialog_run(GTK_DIALOG(cwindow));

        /* get the list */
        race = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(race_combo)->entry));

        filename = g_strconcat(RACE_PATH,prefix,race,".xml",NULL);

        /* kill the window */
        destroy_window(race_combo);

        /* load the selected file */
        nebfile_open_data_file(filename,parsers);

        free(filename);
        create_next();

}

static void
begin_profession(void){
        GtkWidget *box,*label,*cwindow,*prof_combo;
        GList *list;
        char *filename;
        const char *prof;

        cwindow = gtk_dialog_new_with_buttons("Choose Profession",
                        GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK,
                        GTK_RESPONSE_OK,
                        NULL);

        list = get_files(PROFESSION_PATH,path,NULL);


        box = new_row(GTK_DIALOG(cwindow)->vbox);
        label = gtk_label_new("Professions: ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        prof_combo = new_combo(box,list);


        gtk_widget_show_all(cwindow);

        /* Run it */
        gtk_dialog_run(GTK_DIALOG(cwindow));

        /* get the list */
        prof = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(prof_combo)->entry));

        filename = g_strconcat(PROFESSION_PATH,prof,".xml",NULL);

        /* kill the window */
        destroy_window(prof_combo);

        /* load the selected item */
        nebfile_open_data_file(filename,parsers);

        free(filename);
        create_next();
}

/*
 * begin_realm
 *
 * Select a realm for Non spell user
 */
static void
begin_realm(void){
        char *realm;
        GtkWidget *box,*label;
        GList *realms;


        realm = characteristic_get("fixed\\realm");
        if (realm != NULL){
                set_realm_stat(realm);
                create_next();
                return;
        }

        printf("Warning - Realm not set - this should be a <choice>"
                       " in the profession file\n");

        realms = list_realms();

        creation_window = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(creation_window),"Realm Selection");
        gtk_window_set_transient_for(GTK_WINDOW(creation_window),
                        GTK_WINDOW(window));


        box = new_row(GTK_DIALOG(creation_window)->vbox);
        label = gtk_label_new("Realm");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        realm_combo = new_combo(box,realms);

        /* let the user finish */
        new_button(GTK_DIALOG(creation_window)->action_area,
                        GTK_STOCK_OK,end_realm);

        gtk_widget_show_all(creation_window);

}

static void
end_realm(void){
        const char *realm;

        realm = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(realm_combo)->entry));

        characteristic_set("Fixed\\Realm",realm);

        set_realm_stat(realm);

        destroy_window(creation_window);

        create_next();
}

/*
 *
 */
static void
begin_realm_subtype(void){
        GList *sub,*sub2 = NULL;
        char *realm = NULL,*realm2 = NULL;
        GtkWidget *box,*label;
        char *rs;
        struct realm_subtype *subtype;
        GList *temp;

        creation_window = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(creation_window),"SubRealm Selection");
        gtk_window_set_transient_for(GTK_WINDOW(creation_window),
                        GTK_WINDOW(window));

        /* Get the realms */
        rs = characteristic_get("Fixed\\Realm");
        if (rs != NULL){
                realm = g_strdup(rs);
                if (strcmp(realm,"Unknown") == 0)
                        realm = NULL;
        }
        if ((realm2 = strchr(realm,'/')) != 0){
                *realm2 = 0;
                realm2 ++;
        }

        sub = prof_get_subtypes(realm);
        if (realm2 != NULL)
                sub2 = prof_get_subtypes(realm2);

        if (g_list_length(sub) < 2 && g_list_length(sub2) < 2){
                create_next();
                return;
        }


        /* Create stuff in the window */
        temp = NULL;
        while (sub != NULL){
                subtype = sub->data;
                temp = g_list_append(temp,subtype->name);
                sub = g_list_next(sub);
        }

        box = new_row(GTK_DIALOG(creation_window)->vbox);
        label = gtk_label_new(realm);
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        realm_combo = new_combo(box,temp);

        if (realm2 != NULL && g_list_length(sub2) > 1){
                g_list_free(temp);
                temp = NULL;
                while (sub2 != NULL){
                        subtype = sub2->data;
                        temp = g_list_append(temp,subtype->name);
                        sub2 = g_list_next(sub2);
                }
                box = new_row(GTK_DIALOG(creation_window)->vbox);
                label = gtk_label_new(realm2);
                gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
                realm2_combo = new_combo(box,temp);
        }
        g_list_free(temp);

        /* let the user finish */
        new_button(GTK_DIALOG(creation_window)->action_area,
                        GTK_STOCK_OK,end_realm_subtype);

        gtk_widget_show_all(creation_window);

}

/*
 * FIXME: Doesn't build the string correctly for hybrids
 *      if there are no sub types for the second realm
 */
static void
end_realm_subtype(void){
        char *str,*str2 = NULL;
        const char *sub,*sub2 = NULL;

        /* get the list */
        sub = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(realm_combo)->entry));
        if (realm2_combo != NULL)
                sub2 = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(realm2_combo)
                                        ->entry));

        /* Get the old realm */
        str = characteristic_get("fixed\\realm");
        str = g_strdup(str);

        if (realm2_combo){
                str2 = strchr(str,'/');
                *str2 = 0;
                str2 ++;
        }
        if (strcmp(str,sub) != 0)
                str = g_strconcat(str," - ",sub,NULL);
        if (str2 != NULL){
                if (strcmp(str2,sub2) != 0)
                        str = g_strconcat(str,"/",str2," - ",sub2,NULL);
                else
                        str = g_strconcat(str,"/",str2,NULL);
        }

        characteristic_set("fixed\\realm",str);

        /* kill the window */
        destroy_window(realm_combo);

        /* Clean up */
        free(str);

        create_next();
}

/*
 * begin_weapons
 *
 * Allows the user to select the order of weapons skills
 *
 * FIXME: is everything deleted corerctly?
 */
static void
begin_weapons(void){
        GtkTreeStore *weaponstore,*coststore;
        GtkWidget *weaponlist,*costlist;
        GtkWidget *weaponwindow;
        GtkWidget *row;
	GtkWidget *bdown;
	GtkWidget *bup;
	GtkWidget *box;
        int result;
        GList *costs,*tmp,*cats;

        cats = skill_get_cat_list("Weapon");

        tmp = cats;
        costs = NULL;
        while (tmp != NULL){
                costs = g_list_append(costs,skill_get_cost((char *)tmp->data));
                tmp = tmp->next;
        }


        weaponstore = make_tree_store(weapon_store_info);
        coststore = make_tree_store(weapon_store_info);
        weaponlist = tree_create_view(GTK_TREE_MODEL(weaponstore),
                        weapon_view_info);
        costlist = tree_create_view(GTK_TREE_MODEL(coststore),weapon_cost_info);

        weaponwindow = gtk_dialog_new_with_buttons("Prioritize Weapons",
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK,
                        GTK_RESPONSE_OK,
                        NULL);

	box = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (box), 2);
	bup = create_arrow_button (GTK_ARROW_UP, GTK_SHADOW_IN);
	bdown =  create_arrow_button (GTK_ARROW_DOWN, GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (box), bup, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (box), bdown, TRUE, TRUE, 3);

        row = new_row(GTK_DIALOG(weaponwindow)->vbox);
        gtk_box_pack_start(GTK_BOX(row),costlist,FALSE,FALSE,0);
        gtk_box_pack_start(GTK_BOX(row),weaponlist,FALSE,FALSE,0);
        gtk_box_pack_start(GTK_BOX(row),box,FALSE,FALSE,0);

        gtk_tree_view_set_reorderable(GTK_TREE_VIEW(weaponlist),TRUE);
        tree_set_from_list(weaponstore,NULL,WEAPON_NAME,cats);
        tree_set_from_list(coststore,NULL,WEAPON_NAME,costs);

        gtk_signal_connect (GTK_OBJECT (bup), "clicked",
                        GTK_SIGNAL_FUNC(button_up),
                        weaponlist);
        gtk_signal_connect (GTK_OBJECT (bdown), "clicked",
                        GTK_SIGNAL_FUNC(button_down),
                        weaponlist);

        gtk_widget_show_all(weaponwindow);

        /* Let the dialog run for a while */
        result = gtk_dialog_run(GTK_DIALOG(weaponwindow));

        if (result == GTK_RESPONSE_OK){
                g_list_free(cats);
                cats = tree_get_from_list(GTK_TREE_MODEL(weaponstore),
                                WEAPON_NAME);
                while (cats != NULL){
                        skill_set_cost(cats->data,costs->data);
                        cats = cats->next;
                        costs = costs->next;
                }
        }
        gtk_widget_destroy(weaponwindow);

        create_next();
}

/*
 * begin_spells
 *
 * Allows a user to select spell lists for the character if a Pure spell user
 *      (fixed/type == "Pure")
 * Selects from Open & Closed own realm.
 *
 * FIXME: Doesn't handle other spell lists
 * FIXME: Doesn't handle Alchemists (common theme ;-)
 * FIXME: This is as ugly
 */
void
begin_spells(void){
        char *str,*realm,*buf;
        GtkTreeStore *spellstore;
        GtkWidget *spellwindow,*spelllist,*scrollwin,*row,*label;
        GtkWidget *button,*display;
        GList *allspells,*realmspells,*extrabase;
        GList *spells,*base;
        char *basegroup;
        struct choice *select;
        int spellcount;
        GtkTreeIter parent;

        str = characteristic_get("fixed\\type");
	if (str == NULL){
		printf("No spell caster type has been set for this\n"
				"profession.  Report this as a bug\n");
		create_next();
		return;
	}
        if (g_strcasecmp("pure",str) != 0){
                /* Not a Pure - no base lists */
                create_next();
                return;
        }

        /* Work out what realm */
        realm = characteristic_get("fixed\\realm");
        realm = g_strdup(realm);
        strtok(realm," -/");

        spellstore = make_tree_store(base_list_store);

        select = g_malloc0(sizeof(struct choice));
        select->store = spellstore;
        select->count = 0;
        select->max = select->min = EXTRA_BASE;
        select->display = NULL;

        spelllist = tree_create_view_data(GTK_TREE_MODEL(spellstore),
                        base_list_view, select);
        spellwindow = gtk_dialog_new_with_buttons("Select Base Lists",
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        NULL);
        gtk_window_set_default_size(GTK_WINDOW(spellwindow),300,315);
        button = gtk_dialog_add_button(GTK_DIALOG(spellwindow),
                        GTK_STOCK_OK,GTK_RESPONSE_OK);
        gtk_widget_set_sensitive(button,FALSE);
        select->button = button;

        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(spellwindow)->vbox),
                        scrollwin);
        gtk_container_add(GTK_CONTAINER(scrollwin),spelllist);

        allspells = skill_get_cat_list("Spells");
        realmspells = list_grep(realm,allspells);
        extrabase = list_grep("Open",realmspells);
        extrabase = g_list_concat(extrabase,list_grep("Closed",realmspells));

        spellcount = 0;
        for ( ; extrabase != NULL ; extrabase = extrabase->next){
                spells = skill_get_skill_list(extrabase->data);
                spellcount += g_list_length(spells);
                gtk_tree_store_append(spellstore,&parent,NULL);
                gtk_tree_store_set(spellstore,&parent,
                                SPELL_LIST,extrabase->data,-1);
                tree_set_from_list(spellstore,&parent,SPELL_LIST,spells);
                tree_set_flag(spellstore,&parent,SPELL_CAN_SELECT,TRUE);
        }

        if (spellcount < EXTRA_BASE){ /* FIXME: Hardcoded */
                select->max = select->min = spellcount;
        }

        base = list_grep("Base",realmspells);
        base = list_grep(characteristic_get("fixed\\profession"),base);
        if (base == NULL){
                printf("Didn't find own base group (%s)\n",
                                characteristic_get("fixed\\profession"));
                printf("For now this is a bug\n");
                exit(1);
                create_next();
        }

        basegroup = (char *)(base->data);

        /* Put in a display widget */
        row = new_row(GTK_DIALOG(spellwindow)->vbox);
        label = gtk_label_new("Lists selected: ");
        gtk_box_pack_start(GTK_BOX(row),label,FALSE,FALSE,2);
        display = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(display),2);
        gtk_entry_set_width_chars(GTK_ENTRY(display),2);
        gtk_entry_set_text(GTK_ENTRY(display),"0");
        gtk_entry_set_editable(GTK_ENTRY(display),FALSE);
        select->display = display;
        gtk_box_pack_start(GTK_BOX(row),display,FALSE,FALSE,2);
        buf = g_strdup_printf(" of %d",EXTRA_BASE);
        label = gtk_label_new(buf);
        gtk_box_pack_start(GTK_BOX(row),label,FALSE,FALSE,2);

        /* Show it and run it */
        gtk_widget_show_all(spellwindow);
        gtk_dialog_run(GTK_DIALOG(spellwindow));

        /* Set the base list */
        gtk_tree_model_foreach(GTK_TREE_MODEL(spellstore),set_base,basegroup);

        /* Clean up */
        gtk_widget_destroy(spellwindow);
        g_free(realm);
        g_free(select);

        /* Done */
        create_next();

}

static gboolean
set_base(GtkTreeModel *model,GtkTreePath *path,
                GtkTreeIter *iter, gpointer data){
        gboolean base;
        char *list,*pname,*skillname;
        GtkTreeIter *parent;

        gtk_tree_model_get(model,iter,SPELL_LIST,&list,
                        SPELL_SELECTED,&base, -1);

        if (base == FALSE)
                return FALSE; /* done */

        parent = get_parent(model,iter);
        gtk_tree_model_get(model,parent,SPELL_LIST,&pname,-1);

        skillname = g_strconcat(pname,"\\",list,NULL);

        skill_move(skillname,(char *)data);

        g_free(parent);
        g_free(skillname);
        return FALSE;
}

/*
 * Set up spell lists
 *
 * Sets the correct cost for spell lists based on Profession
 * Removes any unwanted lists (eg other realm bases) based
 * on what the user want and the characters realm and profession
 *
 */
void
begin_spell_setup(void){
        GList *allspells,*spell;
        GtkTreeIter iter,*first = NULL;
        gboolean deleted;
        char *prof,*realm;
        GtkTreeStore *spellstore;
        GtkWidget *spellwindow,*spelllist,*scrollwin;
        struct choice *select;

        select = g_malloc0(sizeof(struct choice));

        allspells = skill_get_cat_list("Spells");
        spellstore = make_tree_store(base_list_store);
        spelllist = tree_create_view_data(GTK_TREE_MODEL(spellstore),
                        hidden_list_view,select);

        spellwindow = gtk_dialog_new_with_buttons("Select Visible Lists",
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        NULL);
        select->button = gtk_dialog_add_button(GTK_DIALOG(spellwindow),
                                GTK_STOCK_OK,GTK_RESPONSE_OK);
        select->store = spellstore;
        select->count = g_list_length(allspells);
        select->max = INT_MAX;
        select->min = INT_MIN;
        select->display = NULL;
        gtk_window_set_default_size(GTK_WINDOW(spellwindow),460,315);
        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(spellwindow)->vbox),
                        scrollwin);
        gtk_container_add(GTK_CONTAINER(scrollwin),spelllist);

        /* Set the data */
        realm = characteristic_get("fixed\\realm");
        realm = g_strdup(realm);
        realm = strtok(realm," -");
        prof = characteristic_get("fixed\\profession");
        if (realm == NULL || prof == NULL){
                printf("Error: realm is %s, prof is %s\n",realm,prof);
                return;
        }
        for (spell = allspells ; spell != NULL ; spell = spell->next){
                if (strstr(spell->data,realm) &&
                                (strstr(spell->data,"Open") ||
                                 strstr(spell->data,"Closed") ||
                                 strstr(spell->data,"TP"))){
                        deleted = FALSE;
                } else if (strstr(spell->data,prof)){
                        deleted = FALSE;
                } else {
                        deleted = TRUE;
                }
                if (deleted)
                        gtk_tree_store_append(spellstore,&iter,NULL);
                else {
                        gtk_tree_store_insert_after(spellstore,&iter,
                                        NULL,first);
                        g_free(first);
                        first = gtk_tree_iter_copy(&iter);
                }
                gtk_tree_store_set(spellstore,&iter,SPELL_LIST,spell->data,
                                SPELL_CAN_SELECT,TRUE,
                                SPELL_SELECTED,deleted,-1);
        }

        /* Show it and run it */
        gtk_widget_show_all(spellwindow);
        gtk_dialog_run(GTK_DIALOG(spellwindow));

        /* Delete some skills */
        gtk_tree_model_foreach(GTK_TREE_MODEL(spellstore),delete_skills,NULL);

        /* Clean up */
        g_free(select);
        gtk_widget_destroy(spellwindow);

        spells_set_cost();

        create_next();


}

static gboolean
delete_skills(GtkTreeModel *model,GtkTreePath *path,
                GtkTreeIter *iter, gpointer data){
        gboolean todelete;
        char *list;

        gtk_tree_model_get(model,iter,SPELL_LIST,&list,
                        SPELL_SELECTED,&todelete, -1);

        if (todelete == TRUE){
                skill_delete(list);
        }

        return FALSE;
}


/*
 * FIXME: Assumes the first 10 stats are the important ones
 */
void
begin_stats(void){
        int i = -1;
        GtkWidget *table,*label,*row,*sep;
        GtkObject *adjustment;
        int initial = -1;
        char *name;
        GSList *statnames,*cur;
        int statcount;
        int prcount;
        gboolean useprs;

        useprs = !(config_get_bool(CREATION_OPTION "noprs"));

        /* Roll stats */
        if (config_get_bool(CREATE_FIXED_POINTS)){
                stat_points = 660;
        } else {
                stat_points = 600;
                for (i = 0 ; i < 10 ; i++)
                        stat_points += d10();
        }

        statnames = stat_names();
        statcount = g_slist_length(statnames);

        if (statcount < 10){
                return;
        } else if (statcount > 10){
                statcount = 10;
        }

        creation_window = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(creation_window),"Allocate Stats");
        gtk_window_set_transient_for(GTK_WINDOW(creation_window),
                        GTK_WINDOW(window));

        table = gtk_table_new(statcount,2,FALSE);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(creation_window)->vbox),table,
                        TRUE,TRUE,2);

        cur = statnames;
        i = 0;
	prcount = 0;
        while (cur != NULL && i < statcount){
                name = cur->data;
                label = gtk_label_new(name);
                gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,i,i+1);


                if (stat_is_prime_requisite(name) && useprs == TRUE){
                        initial = 90;
			prcount++;
                } else {
                        initial = 20;
                }
                stat_points -= initial;
                stat_values[i] = initial;
                cur = g_slist_next(cur);
                i ++;
	}

	initial = stat_points / (statcount - prcount);

	for (i = 0 ; i < statcount ; i++){
		int old_initial = stat_values[i];
		if(stat_values[i] == 20){
			stat_values[i] += initial;
			stat_points -= initial;
		}
                adjustment = gtk_adjustment_new(stat_values[i], old_initial,
                                100,1,10,0);
                stat_boxes[i] = gtk_spin_button_new(
                                GTK_ADJUSTMENT(adjustment),0.5,0);
                gtk_spin_button_set_update_policy(
                                GTK_SPIN_BUTTON(stat_boxes[i]),
                                GTK_UPDATE_IF_VALID);
                gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
                                GTK_SIGNAL_FUNC(change_value),
                                GINT_TO_POINTER(i));
                gtk_table_attach_defaults(GTK_TABLE(table),
                                stat_boxes[i],1,2,i,i+1);
        }
        g_slist_free(statnames);

        sep = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(creation_window)->vbox),
                        sep,FALSE,TRUE,2);

        row = new_row(GTK_DIALOG(creation_window)->vbox);
        point_box = new_text_box(row,"Points Remaining: ",TRUE);
        entry_set_digit(point_box,stat_points);

        /* let the user finish */
        stat_button = new_button(GTK_DIALOG(creation_window)->action_area,
                        GTK_STOCK_OK,end_stats);

        /* Make it insensitive */
        if (stat_points != 0)
                gtk_widget_set_sensitive(stat_button,FALSE);

        gtk_widget_show_all(creation_window);
}


static void
change_value(GtkAdjustment *adj, gpointer ip){
        int i = 0;
        int new = 0;
        int old = 0;

        i = GPOINTER_TO_INT(ip);

        old = stat_values[i];
        new = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(stat_boxes[i]));

        if (stat_points - stat_to_points(new) + stat_to_points(old) >= 0){
                stat_points += stat_to_points(old);
                stat_points -= stat_to_points(new);
                stat_values[i] = new;
                entry_set_digit(point_box,stat_points);
        } else {
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(stat_boxes[i]),(float)old);
        }

        if (stat_points == 0)
                gtk_widget_set_sensitive(stat_button,TRUE);
        else
                gtk_widget_set_sensitive(stat_button,FALSE);

        adj = NULL;
}


static int
stat_to_points(int stat){
        int tmp;

        if (stat < 90)
                return stat;

        tmp = stat - 90;
        tmp *= tmp;
        return 90 + tmp;
}

/*
 * FIXME: Assumes stats do not change...
 */
static void
end_stats(void){
        int i;
        char *name;
        GSList *statnames,*cur;

        if (stat_points != 0)
                return;

        statnames = cur = stat_names();
        for (i = 0 ; i < 10 ; i ++){
                name = cur->data;
                stat_set_temp(name,stat_values[i]);
                stat_set_potential(name,gen_pot(stat_values[i]));
                cur = g_slist_next(cur);
        }
        g_slist_free(statnames);

        /*
        if (gets  tat("Ap") != NULL){
                create_appearance();
        }
         */

        destroy_window(creation_window);

        create_next();
}

static int
gen_pot(int temp){
        gboolean fixedpots;

        if (temp > 100)
                temp = 100;

        fixedpots = config_get_bool(CREATION_OPTION "fixed");

        if (fixedpots == TRUE){
                if (temp < 25)  return temp + 44;
                if (temp < 35)  return temp + 39;
                if (temp < 45)  return temp + 33;
                if (temp < 55)  return temp + 28;
                if (temp < 65)  return temp + 22;
                if (temp < 75)  return temp + 17;
                if (temp < 85)  return temp + 11;
                if (temp < 92)  return temp + 6;
                if (temp == 92)  return temp + 5;
                if (temp == 93)  return temp + 4;
                if (temp == 94)  return temp + 4;
                if (temp == 95)  return temp + 3;
                if (temp == 96)  return temp + 3;
                if (temp == 97)  return temp + 2;
                if (temp == 98)  return temp + 2;
                if (temp == 99)  return temp + 1;
                if (temp == 100) return temp + 1;
        } else {
                if (temp < 25)  return 20 + nd10(8);
                if (temp < 35)  return 30 + nd10(7);
                if (temp < 45)  return 40 + nd10(6);
                if (temp < 55)  return 50 + nd10(5);
                if (temp < 65)  return 60 + nd10(4);
                if (temp < 75)  return 70 + nd10(3);
                if (temp < 85)  return 80 + nd10(2);
                if (temp < 92)  return 90 + nd10(1);
                if (temp == 92)  return 91 + dX(9);
                if (temp == 93)  return 92 + dX(8);
                if (temp == 94)  return 93 + dX(7);
                if (temp == 95)  return 94 + dX(6);
                if (temp == 96)  return 95 + dX(5);
                if (temp == 97)  return 96 + dX(4);
                if (temp == 99)  return 98 + dX(2);
                if (temp == 100) return 99 + dX(2);
        }
        return 0;
}

/*
 * begin_hobby_ranks
 *
 * Get hobby ranks from character info and do gain of that many
 * ranks.  Defaults to 12 if none supplied
 *
 */
static void
begin_hobby_ranks(void){
        int ranks;
        char *str,*realm,*prog;


        realm = characteristic_get("Fixed\\Realm");
        if (realm == NULL){
                printf("Uh-Oh - no realm!!\n");
        } else {
                realm = g_strconcat("raceinfo\\",realm,NULL);
                realm = strtok(realm," ");
                prog = characteristic_get(realm);
                if (prog == NULL){
                        printf("No progression for '%s'\n",realm);
                } else {
                        skill_set_progression("Power Point Development",prog);
                        g_free(prog);
                }
        }
        free(realm);

        prog = characteristic_get("raceinfo\\bodydevelopment");
        if (prog == NULL){
                printf("No progression for %s\n","raceinfo\\bodydevelopment");
        } else {
                skill_set_progression("Body Development",prog);
                g_free(prog);
        }

        skill_update();

        str = characteristic_get("raceinfo\\hobbyranks");
        if (str == NULL){
                ranks = 12;
                printf("Warning: Defaulting to 12 for hobby ranks\n");
        } else {
                ranks = strtol(str,NULL,10);
        }

        skill_buy_ranks(BUY_RANKS,ranks,0);

        create_next();
}

/*
 * begin_end
 *
 * Set progressions for special skills based on race/profession and
 * display 'Character created' dialog
 *
 * FIXME: Body Development & PPDev are hard coded.
 * FIXME: Need to handle realm stat for Hybrids
 * FIXME: Need to handle PPDev for Hybrids
 *
 */
static void
begin_end(void){
        GtkWidget *dialog;

        /* Set some random things */
        characteristic_set("player\\application",VERSION_STRING);
        characteristic_set("experience\\level","1");
        characteristic_set("experience\\experiencepoints","10000");

        /* Recalculate everything */
        refresh_all();

        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  "Character Created");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        free(prefix);
        prefix = NULL;

	stage=-1;

        return;

}

/*
 * list_grep
 *
 * Greps a list (ie searches) a list
 *
 * Args:
 *      Pattern to search for
 *      List ot search
 * Return:
 *      Matching strings
 */
GList *
list_grep(char *pattern,GList *list){
        GList *result = NULL;

        while (list != NULL){
                if (strstr((char *)(list->data),pattern)){
                        result = g_list_prepend(result,list->data);
                }
                list = list->next;
        }

        result = g_list_reverse(result);
        return result;
}

void
print_list(GList *list){
        printf("List begin: [\n");
        while (list != NULL){
                printf("  - '%s'\n",(char *)list->data);
                list = list->next;
        }
        printf("]\n");
}

/* Create an Arrow widget with the specified parameters
 * and pack it into a button */
GtkWidget *create_arrow_button( GtkArrowType  arrow_type,
                                GtkShadowType shadow_type )
{
  GtkWidget *button;
  GtkWidget *arrow;

  button = gtk_button_new ();
  arrow = gtk_arrow_new (arrow_type, shadow_type);

  gtk_container_add (GTK_CONTAINER (button), arrow);

  gtk_widget_show (button);
  gtk_widget_show (arrow);

  return button;
}

static void
button_down(GtkWidget *button, GtkTreeView *weapons)
{
	GtkTreeSelection *selection;
	GtkTreeStore *model;
	GtkTreeIter iter,new_iter;
	GValue *v1,*v2;

	selection = gtk_tree_view_get_selection(weapons);
	if(!selection) return;
	if(!gtk_tree_selection_get_selected(selection,(GtkTreeModel **)&model,&iter))
		return;

	new_iter = iter;
	v1 = (GValue *)calloc(1,sizeof(GValue));
	v2 = (GValue *)calloc(1,sizeof(GValue));
	
	if(gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &new_iter))
	{
	  gtk_tree_model_get_value(GTK_TREE_MODEL(model), &iter, 0, v1);
	  gtk_tree_model_get_value(GTK_TREE_MODEL(model), &new_iter, 0, v2);
	  gtk_tree_store_set_value(model, &iter, 0, v2);
	  gtk_tree_store_set_value(model, &new_iter, 0, v1);
	  gtk_tree_selection_select_iter(selection,&new_iter);
	}
	free(v1);
	free(v2);
}

gboolean compare_iter(GtkTreeIter *it1, GtkTreeIter *it2)
{
  return ((it1->user_data == it2->user_data) &&
          (it1->user_data2 == it2->user_data2) &&
	  (it1->user_data3 == it2->user_data3));
}

gboolean gtk_tree_model_iter_prev(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
  GtkTreeIter it1,it2;

  if(!gtk_tree_model_get_iter_first(tree_model, &it1))
    return FALSE;

  if(compare_iter(&it1, iter))
    return FALSE;
  while(!compare_iter(&it1, iter))
  {
    it2 = it1;
    if(!gtk_tree_model_iter_next (tree_model, &it1))
      return FALSE;
  }

  *iter = it2;
  return TRUE;

}

static void
button_up(GtkWidget *button, GtkTreeView *weapons)
{
	GtkTreeSelection *selection;
	GtkTreeStore *model;
	GtkTreeIter iter,new_iter;
	GValue *v1,*v2;

	selection = gtk_tree_view_get_selection(weapons);
	if(!selection) return;
	if(!gtk_tree_selection_get_selected(selection,(GtkTreeModel **)&model,&iter))
		return;

	v1 = (GValue *)calloc(1,sizeof(GValue));
	v2 = (GValue *)calloc(1,sizeof(GValue));
	
	new_iter = iter;
	if(gtk_tree_model_iter_prev (GTK_TREE_MODEL(model), &new_iter))
	{
	  gtk_tree_model_get_value(GTK_TREE_MODEL(model), &iter, 0, v1);
	  gtk_tree_model_get_value(GTK_TREE_MODEL(model), &new_iter, 0, v2);
	  gtk_tree_store_set_value(model, &iter, 0, v2);
	  gtk_tree_store_set_value(model, &new_iter, 0, v1);
	  gtk_tree_selection_select_iter(selection,&new_iter);
	}
	free(v1);
	free(v2);
}


/*****************************************************
 * Creation Code - Version 2                         *
 *****************************************************
 *  Note that most of this code relies on choice
 *  files, and its main job is to make sure things
 *  run sanely 
 */

static void create_first(GtkWidget *druid);

/*
 * create_begin
 *
 * Create the creation window, and kick of the creation of the
 * character
 */
/* FIXME: this name will need to be changed */
#define BASE_CREATE "base/newcreate.xml"
GtkWidget *druid;

void
create_begin(void){
        GtkWidget *druidwindow;
        const char *paths;
        char *file = NULL,**strs;
        int i,rv;
        struct stat st;
        char *msg;
printf("starting new creation\n");
	/* Create a window */
        druid = gnome_druid_new_with_window(_("Create Character"),
                        GTK_WINDOW(window),TRUE,&druidwindow);
printf("creating first page\n");
        /* Plonk in the first page */
        create_first(druid);
printf("getting path\n");
        /* Find the creation file */
        /* FIXME: find data file needs to be a fucntion */
        paths = config_get_string_default(FILE_OPTION "datapath",CONFIG_PATH);
printf("path is %s\n",path);
        strs = g_strsplit(paths,":",-1);
        for (i = 0 ; strs[i] != NULL ; i ++){
                file = g_strconcat(strs[i],"/",BASE_CREATE,NULL);
                if (stat(file,&st) == -1 ||
                                S_ISREG(st.st_mode) == FALSE){
                        g_free(file);
                        file = NULL;
                        continue;
                }
                break;
        }
        g_strfreev(strs);
printf("file is %s\n",file);
        if (file == NULL){
                gui_warning_msg("<b>Could not load Base Genre File</b>\n\n"
                                "Check your data path\n");
                gtk_widget_destroy(druid);
                return ;
        }
printf("opening base file\n");
        rv = nebfile_open_data_file(BASE_XML,parsers);
        if (rv == -1){
                msg = g_strdup_printf("Could not find the file %s\n"
                                "Check the file path.",BASE_XML);
                gui_error_msg(msg);
                g_free(msg);
                return;
        }

printf("opening main file\n");
        parse_file(file,parsers);
printf("show all druid\n");
        gtk_widget_show_all(druid);


}


/*
 * create_choice
 *
 * We just hit the <create> tag - time to parse the choice windows 
 */
void
create_choice(element_t *root){
        GList *data;
        element_t *cur;

        for (data = root->elements ; data != NULL ; data = data->next){
                cur = data->data;
                assert(cur != NULL);
                if (strcmp(cur->name,"choice") == 0){
                        parse_choice(cur);
                } else {
                        printf("Unknown element %s\n",cur->name);
                }
        }

}

/* Move to create.h */
enum createmode {
        CREATEMODE_NONE,
        CREATEMODE_SELECT,  /* Select a file to load */
        CREATEMODE_SELECTVAL,  /* Select an item from a DB, and insert */
        CREATEMODE_ENTRY,   /* Enter some data into a field */
        CREATEMODE_ENTRYLONG,
        CREATEMODE_SORT,
        CREATEMODE_SPEND,
};

struct createnode {
        enum createmode mode;
        gboolean hasdescription;

        /* Window data */
        char *title;  /* Window title */
        char *heading; /* window heading */
        char *description; /* Window description */

        /* If the mode is CREATEMODE_SELECTION the options are in this list */
        GList *selections; /* struct selection */

        /* If the mode is CREATEMODE_ENTRY the entries are here */
        GList *entries;
        
        /* Internal widgets */
        GtkWidget *combo;
        GtkTextBuffer *descwid; /* Widget for the description */
        
        /* Links to children and siblings */
        struct createnode *next,*prev;
        struct createnode *children;
        
        
};

struct selection {
        char *name; /* To be displayed on the list */
        char *file; /* Name of the file where this can be found */
        char *description; /* The description of this selection */
        char *descriptionpath; /* Where to get the descritpion */
};
struct entry {
        char *label; /* label to desiplay next to extry */
        char *field;
        char *(*get_func)(const char *);
        void (*set_func)(const char *,const char *);
        GtkTreeStore *store;
        GtkTreeIter pos;
        GtkWidget *entry;
};

struct createfuncs {
        char *name;
        int (*func)(struct createnode *,element_t *);
};

static int set_mode(struct createnode *,element_t *);
static int has_description(struct createnode *node, element_t *el);
static int add_files(struct createnode *node, element_t *el);
static int add_field(struct createnode *node, element_t *el);
static int set_window_options(struct createnode *node, element_t *el);


static void create_from_node(struct createnode *node,GtkWidget *druid);
static void create_add_widgets(struct createnode *,GnomeDruidPageStandard *);
static GList *get_options(struct createnode *node);
static void combo_changed(GtkWidget *entry,void *userdata);

static gboolean create_next_page(GnomeDruidPage *druidpage, GtkWidget *widget,
                                            gpointer user_data);
static void prepare_page(GnomeDruidPage *druidpage, GtkWidget *widget,
                                            gpointer user_data);


struct selection *find_selection(GList *list,const char *str);

struct createfuncs createfuncs[] = {
        /* Generic window control options */
        { "title",       set_window_options },
        { "description", set_window_options },
        { "heading",     set_window_options },

        /* Modes - there can be only one */
        { "selection",   set_mode },  /* select a file */
        { "entry",       set_mode },  /* set a value */         

        /* General options - some only make sense in certain modes */
        { "hasdescription", has_description },
        { "files",       add_files },
        { "field",       add_field }, /* field where data comes from */
        { NULL,         NULL },
};

static void
parse_choice(element_t *root){
        GList *data;
        element_t *cur;
        struct createnode *node;
        struct createfuncs *fns;

        if (druid == NULL){
                g_warning("New style choice in old style create");
                return;
        }

        assert(root != NULL);
        assert(root->name != NULL);
        assert(strcmp(root->name,"choice") == 0);

        
        node = g_malloc0(sizeof(struct createnode));

        /* First create all the data for the choice */
        for (data = root->elements ; data != NULL ; data = data->next ){
                cur  = data->data;
                assert(cur != NULL);
                for (fns = createfuncs ; fns->func != NULL ; fns ++){
                        if (fns->func == NULL)
                                break; /* DEBUG */
                        assert(fns->func != NULL);
                        if (strcmp(cur->name,fns->name) == 0)
                                fns->func(node,cur);
                }
        }

        /* Now create the window */
        create_from_node(node,druid);

}

struct set_mode { 
        char *name; 
        enum createmode mode;
};
struct set_mode set_modes[] = {                
        { "selection",  CREATEMODE_SELECT },
        { "entry",      CREATEMODE_ENTRY },
        { "entrylong",  CREATEMODE_ENTRYLONG },
        { "sort" ,      CREATEMODE_SORT },
        { "spend",      CREATEMODE_SPEND },
        { NULL,         CREATEMODE_NONE },
};
 

static int
set_mode(struct createnode *node,element_t *el){
        struct set_mode *sm;

        assert(node != NULL);
        assert(node->mode == CREATEMODE_NONE);
        
        assert(el != NULL);
        assert(el->name != NULL);

        for (sm = set_modes ; sm->name != NULL ; sm ++){
                if (strcmp(sm->name,el->name) == 0){
                        node->mode = sm->mode;
                        return 0;
                }
        }
                                
        g_assert_not_reached();
        return -1;
}

static int
has_description(struct createnode *node, element_t *el){
        assert(node != NULL);
        assert(node->hasdescription != TRUE);
        node->hasdescription = TRUE;
        return 0;
}

static int
add_files(struct createnode *node, element_t *el){
        GList *files,*tmp;
        char *descpath;
        struct attribute *attr;
        struct selection *selection;
        char *path;
        
        assert(node != NULL);
        assert(el != NULL);
        assert(el->text != NULL);

        files = get_files(el->text,config_get_string(FILE_OPTION "datapath"),
                        &path);
        
        if (files == NULL){
                printf("No files found - dieing\n");
                return -1;
        }

        /* Do we have an attribute to find the description */
        descpath = NULL;
        for (tmp = el->attributes ; tmp != NULL ; tmp = tmp->next){
                attr = tmp->data;
                if (strcmp(attr->name,"descriptionpath") == 0){
                        descpath = g_strdup(attr->value);
                        break;
                }
        }
        
        
        for (tmp = files ; tmp != NULL ; tmp = tmp->next){
                selection = g_malloc0(sizeof(selection));

                selection->name = g_strdup(tmp->data);
                selection->file = g_strconcat(path,"/",tmp->data,".xml",NULL);
                selection->description = NULL;
                selection->descriptionpath = g_strdup(descpath);

                node->selections = g_list_append(node->selections,selection);
                
                /* Free the string */
                //g_free(tmp->data);

        }

        /* FIXME: Free the list */

        /* FIXME: turning this on causes it to lock up */
        //g_free(path);
        
        return 0;
}

struct add_field_support_funcs  {
        char *name;
        char *(*get_func)(const char *);
        void (*set_func)(const char *,const char *); 
};
struct add_field_support_funcs add_field_funcs[] = {
        { "[characteristics]", characteristic_get, characteristic_set },
        { "[char]",characteristic_get, characteristic_set },
        { "[characteristic]",characteristic_get, characteristic_set },
        { NULL,NULL,NULL }
};
        
static int 
add_field(struct createnode *node, element_t *el){
        struct entry *entry;
        char *str;
        GList *tmp;
        struct add_field_support_funcs *funcs;
        struct attribute *attr;

        printf("adding field\n");
        

        str = el->text;
        if (str == NULL || strlen(str) == 0){
                g_warning("string length 0  - this is bogus");
                return -1;
        }
        
        entry = g_malloc0(sizeof(struct entry));
        node->entries = g_list_append(node->entries,entry);

        /* Set the label field */
        for (tmp = el->attributes ; tmp != NULL ; tmp = tmp->next){
                attr = tmp->data;
                assert(attr != NULL);
                if (strcmp(attr->name,"label") == 0){
                        entry->label = g_strdup(attr->value);
                        break;
                }
        }
                        
        
        for (funcs = add_field_funcs ; funcs->name != NULL ; funcs ++){
                if (strncmp(funcs->name,str,strlen(funcs->name)) == 0){
                        entry->get_func = funcs->get_func;
                        entry->set_func = funcs->set_func;
                        str += strlen(funcs->name);
                        if (*str == '\\' || *str == '/')
                                str ++;
                        entry->field = g_strdup(str);
                        return 0;
                }
        }

        /* Fixme: handle generic data bases */
        
        g_warning("Unknown field: %s\n",str);
        return -1;
                                
        
}

static int
set_window_options(struct createnode *node, element_t *el){
        
        assert(node != NULL);
        assert(el != NULL);
        assert(el->name != NULL);

        if (strcmp(el->name,"title") == 0){
                if (node->title != NULL){
                        g_warning(_("The title for this window"
                                               " has already been set\n"));
                        return -1;
                }
                node->title = g_strdup(el->text);
        } else if (strcmp(el->name,"heading") == 0){
                if (node->heading != NULL){
                        g_warning(_("The heading for this window"
                                               " has already been set\n"));
                        return -1;
                }
                node->heading = g_strdup(el->text);
        } else if (strcmp(el->name,"description") == 0){
                if (node->description != NULL){
                        g_warning(_("The description for this window"
                                               " has already been set\n"));
                        return -1;
                }
                node->description = g_strdup(el->text);
        } else {
                g_assert_not_reached();
        }

        return 0;
}


/*
 * create from node
 *
 * Creates a druid window from the given createnode structure.
 *
 */
static void
create_from_node(struct createnode *node,GtkWidget *druid){
        GnomeDruidPageStandard *page;
        GtkWidget *label;
        PangoAttrList *list = NULL;
        PangoAttribute *attr;
        
        assert(node != NULL);
        assert(druid != NULL);

        page = (GnomeDruidPageStandard*)gnome_druid_page_standard_new();

        if (node->title != NULL)
                gnome_druid_page_standard_set_title(page,node->title);
        else
                gnome_druid_page_standard_set_title(page,_("No title"));

        if (node->heading != NULL){
                label = gtk_label_new(node->heading);
                list = pango_attr_list_new();
                attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
                attr->start_index = 0;
                attr->end_index = strlen(node->heading);
                pango_attr_list_insert(list,attr);
                attr = pango_attr_scale_new(PANGO_SCALE_LARGE);
                attr->start_index = 0;
                attr->end_index = strlen(node->heading);
                pango_attr_list_insert(list,attr);
                gtk_label_set_attributes(GTK_LABEL(label),list);
                pango_attr_list_unref(list); /* Don't need a ref anymore */
                
                gtk_label_set_use_markup(GTK_LABEL(label),FALSE);
                gtk_label_set_use_underline(GTK_LABEL(label),FALSE);

                
                /* Pack it in */
                gtk_box_pack_start(GTK_BOX(page->vbox),label,FALSE,FALSE,0);
        }

        if (node->description != NULL){
                char *s; /* FIXME: move */
                for (s = node->description ; *s != '\0' ; s ++){
                        if (*s == '\n' || *s == '\t')
                                *s = ' ';
                }
                
                label = gtk_label_new(node->description);
                gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
                
                gtk_box_pack_start(GTK_BOX(page->vbox),label,FALSE,FALSE,0);
        }

        /* Add the appropriate widgets */        
        create_add_widgets(node,page);

        /* Attach signal handlers */
        g_signal_connect(G_OBJECT(page),"next",
                        (GtkSignalFunc)create_next_page,node);
        g_signal_connect(G_OBJECT(page),"prepare",
        (GtkSignalFunc)prepare_page,node);

        /* Add it in */
        gtk_widget_show_all(GTK_WIDGET(page));
        gnome_druid_append_page(GNOME_DRUID(druid),GNOME_DRUID_PAGE(page));
        gtk_widget_show_all(GTK_WIDGET(druid));

         

}

static void
create_add_widgets(struct createnode *node,GnomeDruidPageStandard *page){
        GtkWidget *view;
        GtkWidget *wid,*label,*box;
        GtkSizeGroup *sgroup;
        GList *options;
        GList *list;
        struct entry *entry;
        
        assert(node != NULL);

        if (node->mode == CREATEMODE_NONE){
                g_warning("You must have a mode for creation\n");
                return;
        }

        switch (node->mode){
        case CREATEMODE_SELECT:
                wid = gtk_combo_new();
                options = get_options(node);
                gtk_combo_set_popdown_strings(GTK_COMBO(wid),options);
                g_list_free(options);

                if (node->hasdescription){
                        g_signal_connect(G_OBJECT(GTK_COMBO(wid)->list), 
                                        "selection-changed",
                                        G_CALLBACK(combo_changed), 
                                        node);
                }
                node->combo = wid;
                gtk_box_pack_start(GTK_BOX(page->vbox),wid,TRUE,TRUE,2);

                break;
        case CREATEMODE_ENTRY:
                sgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
                for (list = node->entries ; list != NULL ; list = list->next){
                        printf("adding a entry widget\n");
                        box = new_row(page->vbox);
                        entry = list->data;
                        label = gtk_label_new(entry->label);
                        gtk_size_group_add_widget(sgroup,label);
                        gtk_box_pack_start(GTK_BOX(box),label,FALSE,0,0);
                        wid = gtk_entry_new();
                        gtk_box_pack_start(GTK_BOX(box),wid,TRUE,TRUE,2);
                        entry->entry = wid;
                }
                g_object_unref(G_OBJECT(sgroup)); 
                break;
                
                
        default:
                wid = NULL;
                printf("Not implemented\n");
                g_assert_not_reached();        
        }


        if (node->hasdescription){
                /* Insert a text view and store for the description */
                view = gtk_text_view_new();
                node->descwid = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

                gtk_box_pack_start(GTK_BOX(page->vbox),view,TRUE,TRUE,2);
                if (node->mode == CREATEMODE_SELECT){
                        combo_changed(NULL,node);
                }
        }

        gtk_widget_show_all(GTK_WIDGET(page)); 
        
        
}

static GList *
get_options(struct createnode *node){
        GList *list;
        GList *cur;
        struct selection *sel;

        list = NULL;
        for (cur = node->selections; cur != NULL ; cur = cur->next){
                sel = cur->data;
                list = g_list_prepend(list,sel->name);
        }

        return g_list_reverse(list);
}


static void
combo_changed(GtkWidget *widget,void *userdata){
        struct createnode *node;
        GList *sellist;
        struct selection *sel;
        const char *newstr;

        node = userdata;
        assert(node != NULL);

        newstr = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(node->combo)->entry));

        if (newstr == NULL || strlen(newstr) == 0){
                return;
        }

        for (sellist = node->selections ; sellist != NULL ; 
                        sellist = sellist->next){
                sel = sellist->data;
                if (strcmp(newstr,sel->name) == 0){
                        break;
                        
                }
        }

        if (sellist == NULL){
                /* Didn't find it */
                return;
        }

        if (sel->description != NULL){
                /* We have a descritpion */
                gtk_text_buffer_set_text(node->descwid,sel->description,-1);
                return;
        }       
        
        if (sel->descriptionpath == NULL || sel->file == NULL){
                printf("can't update desription\n");
                return;
        }
        
        newstr = xml_grep(NULL,sel->file,sel->descriptionpath);
        if (newstr == NULL || strlen(newstr) == 0)
                newstr = "No description avialable";
        sel->description = g_strdup(newstr);

        /* set the new path */
        
        gtk_text_buffer_set_text(node->descwid,sel->description,-1);
        
         
        
}

static gboolean create_next_selection(struct createnode *node);
static gboolean create_next_entry(struct createnode *node);
/*
 * create_next_page
 *
 * The user clicked 'next'.  Work out the final state of the widget - 
 * and perform the appropriate actions as necessary.
 * 
 * FIXME: Also work out if we need to do things like
 * deactivate the next 'prev' button or not 
 *
 * Args:
 *      GnomeDruidPage *the page that got the signal
 *      GtkWidget *the druid it is in
 *      void* data: (should) be the node info data 
 * Returns:
 *      TRUE: If you CAN'T go on
 *      FALSE: Otherwise (if you can go on)
 */

static gboolean 
create_next_page(GnomeDruidPage *druidpage, GtkWidget *widget,
                                            gpointer user_data){
        struct createnode *node;

        node = user_data;

        assert(node != NULL);
        assert(widget == druid);
        assert(druidpage != NULL);

        switch (node->mode){
        case CREATEMODE_SELECT:
                return create_next_selection(node);
        case CREATEMODE_ENTRY:
                return create_next_entry(node);
        default:
                g_assert_not_reached();
        }  

        g_assert_not_reached();
        return FALSE;
}

static gboolean
create_next_selection(struct createnode *node){
        const char *str;
        struct selection *sel;

        str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(node->combo)->entry));
        
        if (str == NULL || strlen(str) == 0){
                g_warning("str is %s - expecting a valid selection\n",str);
                return TRUE; /* error */
        } 

        sel = find_selection(node->selections,str);
        if (sel == NULL){
                g_warning("Could not find selection (%s)\n",str);
                return TRUE;
        }

        
        if (sel->file == NULL){
                g_warning("No file - WTF am I supposed to do now\n");
                return TRUE;       
        }        

        parse_file(sel->file,parsers);

        return FALSE; /* We can load the next file */
}
static gboolean
create_next_entry(struct createnode *node){
        struct entry *entry;
        GList *tmp;
        const char *str;

        for (tmp = node->entries; tmp != NULL ; tmp = tmp->next){
                entry = tmp->data;
                
                str = gtk_entry_get_text(GTK_ENTRY(entry->entry));
                if (str == NULL || strlen(str) == 0){
                        /* FIXME: should throw a warning */
                        return TRUE;
                } 

                if (entry->set_func != NULL){
                        entry->set_func(entry->field,str);
                } else {
                        g_warning("this is not implemented\n");
                }
                
        }

        /* Can continue */
        return FALSE;
}

/*
 * prepare_page
 *
 * Prepare the next the page. Set an values that ned to be set,
 *
 */
static void prepare_entry_page(struct createnode *node);
static void
prepare_page(GnomeDruidPage *druidpage, GtkWidget *widget,
                gpointer user_data){
        struct createnode *node;

        node = user_data;
       
        assert(widget == druid);
        assert(druidpage != NULL);
        assert(node != NULL);

        switch (node->mode){
        case CREATEMODE_ENTRY:
                prepare_entry_page(node);
                break; 
        case CREATEMODE_SELECT:
                return;
                /* Nothing to do */
                break;
        case CREATEMODE_ENTRYLONG:
        case CREATEMODE_SORT:
        case CREATEMODE_SPEND: 
        case CREATEMODE_NONE:
                printf("not handled yet\n");
                break;
        default:
                g_assert_not_reached();        
                /* do nothing */
                break;
        }
        return;

        
}

static void
prepare_entry_page(struct createnode *node){
        GList *tmp;
        struct entry *entry;
        const char *str;

        for (tmp = node->entries ; tmp != NULL ; tmp = tmp->next){
                entry = tmp->data;
                
                if (entry->get_func != NULL){
                        str = entry->get_func(entry->field);
                        if (str != NULL && strlen(str) > 0){
                                gtk_entry_set_text(GTK_ENTRY(entry->entry),str);                        }
                }
        }
        return;
}




struct selection *
find_selection(GList *list,const char *str){
        struct selection *sel;
        
        assert(list != NULL);
        assert(str != NULL);
        
        for ( ; list != NULL ; list = list->next){
                sel = list->data;
                if (strcmp(str,sel->name) == 0)
                        return sel;
        }
                
        return NULL;
}
/*
 * create_first
 *
 * Called to create a fairly basic first page introduction character
 * creation.
 *
 * Arguments:
 *      druid: Widget to attach page to
 * Returns:
 *      nothing
 */

static void
create_first(GtkWidget *druid){
        GtkWidget *page;
        const char *msg = 
        "Welcome to character creation\n"
        "\n"
        "The following pages will guide you through character "
        "creation.  Most steps should be obvious, although you "
        "may need a copy of the appropriate rule books. "
        "\n"
        "However before you start make sure you have a good idea "
        "of the character you are about to create, as well as an "
        "understanding of the game you will be playing "
        "\n"
        "Have Fun!";
        
        
        page = /*gnome_druid_page_edge_new(GNOME_EDGE_START);*/

                gnome_druid_page_edge_new_with_vals(GNOME_EDGE_START,
                                TRUE,
                                "Create a new Character",
                                msg,
                                NULL,NULL,NULL);
        gnome_druid_append_page(GNOME_DRUID(druid),GNOME_DRUID_PAGE(page));
        
        gtk_widget_show_all(page); 
}
