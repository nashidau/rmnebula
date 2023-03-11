
#include "libneb.h"
#include "skills.h"
#include "general.h"
#include "menu.h"
#include <gtk/gtk.h>
#include <assert.h>

void skill_add_xml(element_t *data);
static void skill_init(struct rcmenu_info *,GtkTreeIter *iter);
GtkTreeIter *skill_get_by_name(char *skill);


enum {
        WB_SKILL_ELEMENT_NAME, /* slot 0 reserved for parser */
        WB_SKILL_NAME,
        WB_SKILL_STATS,
        WB_SKILL_CLASS, /* Everyman, Occ etc */
        WB_SKILL_PROGRESSION,
        WB_SKILL_SPECIAL_BONUS,
        WB_SKILL_DESCRIPTION,
        WB_SKILL_AUTOMATIC, /* Skill was inserted by an update,
                                either a region or a realm skill */
        WB_SKILL_TRUE, /* Set to true when loaded */
};

int skillprimaries[] = { WB_SKILL_NAME , -1 };
static void skill_insert_xml(element_t *,GtkTreeIter *,struct store_info *);

struct store_info skillstoreinfo[] = {
        { WB_SKILL_ELEMENT_NAME, G_TYPE_STRING, "", 0, 0, SAVE_ELEMENT },
        { WB_SKILL_NAME,   G_TYPE_STRING, "name", 0, 0, SAVE_ALWAYS },
        { WB_SKILL_STATS,  G_TYPE_STRING, "statbonuses", 0, 0, SAVE_NOT_MIDDLE},
        { WB_SKILL_CLASS,  G_TYPE_STRING, "class", 0, 0, SAVE_CHILDREN },
        { WB_SKILL_PROGRESSION, G_TYPE_STRING, "progression", 0, 0, SAVE_PARENT},
        { WB_SKILL_SPECIAL_BONUS,G_TYPE_INT,"specialbonus", 0 , 0,SAVE_DEFINED},
        { WB_SKILL_DESCRIPTION,G_TYPE_STRING,"description", 0,0,SAVE_DEFINED },
        { WB_SKILL_AUTOMATIC, G_TYPE_BOOLEAN, "auto", 0, 0, SAVE_DEFINED },
        { WB_SKILL_TRUE, G_TYPE_BOOLEAN, NULL, 0, 0, SAVE_NEVER },

        { -1, 0, NULL, 0, 0, 0 },
};

struct view_info skillviewinfo[] = {
        { "Name",WB_SKILL_NAME,"text",0,0,0},
        { "Stats",WB_SKILL_STATS,"text",0,0,0},
        { "Class",WB_SKILL_CLASS,"text",0,0,0},
        { "Prog",WB_SKILL_PROGRESSION,"text",0,0,0},
        { "Spec",WB_SKILL_SPECIAL_BONUS,"text",0,0,0},
        { 0,0,0,0,0,0}
};

/* THis is the same as Nebula/skills.c :-( */
char *classstrings[] = {
        "Normal",
        "Everyman",
        "Occupational",
        "Restricted",
        0,
};
/* Progression: In nebula this is free form - here we can be more restrictive
*/
char *progressionstrings[] = {
        "Standard",
        "Limited",
        "Combined",
        "Racial",
        0
};

static struct treepopupinfo popup_info[] = {
        { WB_SKILL_NAME,   POPUP_ENTRY, "Name: ", 0,0},
        { WB_SKILL_STATS,  POPUP_ENTRY, "Stats: ", 0, 0 },
        { WB_SKILL_CLASS,  POPUP_COMBO, "Class: ", classstrings, 0 },
        { WB_SKILL_PROGRESSION, POPUP_COMBO, "Progression: ",
                                                 progressionstrings, 0 },
        { WB_SKILL_SPECIAL_BONUS, POPUP_DIGIT,"Special Bonus: ", 0 , 0 },
        { WB_SKILL_DESCRIPTION, POPUP_TEXTVIEW,"Description", 0,0},
        { -1, 0, NULL, 0 , 0 },
};


enum { SKILL_NEW, SKILL_EDIT, SKILL_DELETE };

/* The popup (right click) menus */
static GtkItemFactoryEntry skillmenu[] = {
        {
                N_("/_New Skill..."),NULL,
                (GtkItemFactoryCallback)rcmenu_new_callback,SKILL_NEW,
                "<StockItem>", GTK_STOCK_NEW
        },
        {
                N_("/_Edit Skill..."), NULL,
                (GtkItemFactoryCallback)rcmenu_edit_callback,SKILL_EDIT,
                "<Item>", NULL
        },{
                "/sep1--------------", NULL,
                0, 0,
                "<Separator>",0
        },
	{
                N_("/_Delete Skill"), NULL,
                (GtkItemFactoryCallback)rcmenu_delete_callback,SKILL_DELETE,
                "<StockItem>", GTK_STOCK_DELETE
        },
};



static GtkTreeStore *skillstore;
static GtkWidget *skillview;

void skills_insert_list_item(GtkTreeStore *,GtkTreeIter *, struct leaf *leaf);

void
wb_skills_init(GtkNotebook *notebook){
        GtkWidget *scrollwin;
        GtkWidget *frame;
        GtkWidget *label;
        GtkWidget *box;
        struct rcmenu_info *minfo;
        struct popupinfo *popupinfo;

        frame = gtk_frame_new("Basic Skills");
        label = gtk_label_new("Skills");
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
        box = gtk_vbox_new(FALSE,2);


        gtk_container_add(GTK_CONTAINER(frame),box);


        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(box),scrollwin);

        skillstore = make_tree_store(skillstoreinfo);
        skillview = tree_create_view(GTK_TREE_MODEL(skillstore),skillviewinfo);

        /* Create a popup window */
        popupinfo = tree_create_popup(popup_info,window);
        popupinfo->store = skillstore;

        /* Create a right click menu */
        minfo = g_malloc0(sizeof(struct rcmenu_info));
        minfo->store = skillstore;
        minfo->view = GTK_TREE_VIEW(skillview);
        minfo->storeinfo = skillstoreinfo;
        minfo->item = _("skill");
        minfo->name_field = WB_SKILL_NAME;
        minfo->popupinfo = popupinfo;
        minfo->maxdepth = 2; /* Two deep children */
        minfo->set_defaults = skill_init;
        tree_popup_add(skillview,skillmenu,G_N_ELEMENTS(skillmenu),minfo);


/*
        g_signal_connect(G_OBJECT(realmview), "button_press_event",
                                (GtkSignalFunc)realm_double_click,NULL);
  */
        gtk_container_add(GTK_CONTAINER(scrollwin),skillview);

}



void
skill_add_xml(element_t *data){
        GList *elements;
        element_t *el;

        elements = data->elements;
        while (elements != NULL){
                el = elements->data;
                skill_insert_xml(el,NULL,skillstoreinfo);

                elements = g_list_next(elements);
        }
        tree_set_flag_recursive(skillstore,NULL,WB_SKILL_TRUE,TRUE);
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


void
skills_save(element_t *el){
        el = create_element(el,"skills",ETYPE_ELEMENT);

        tree_save(GTK_TREE_MODEL(skillstore),skillstoreinfo,el);
}


void
skills_clear(void){
        gtk_tree_store_clear(skillstore);
}


static void
skill_init(struct rcmenu_info *minfo,GtkTreeIter *iter){
        int depth;
        const char *element_name = NULL;
        GtkTreeIter parent;

        depth = gtk_tree_store_iter_depth(minfo->store,iter);
        assert(depth < 3);

        if (depth == 0){
                element_name = "category";
        } else if (depth == 1){
                element_name = "skill";
        } else if (depth == 2){
                element_name = "skill";
                /* Make sure the parent is a skillgroup */
                gtk_tree_model_iter_parent(GTK_TREE_MODEL(minfo->store),
                                &parent,iter);
                gtk_tree_store_set(minfo->store,&parent,
                               WB_SKILL_ELEMENT_NAME,"skillgroup",-1);
        }

        assert(element_name != NULL);

        gtk_tree_store_set(minfo->store,iter,
                        WB_SKILL_ELEMENT_NAME, element_name,
                        WB_SKILL_NAME, "New Skill",
                        /* FIXME: only set if a skill */
                        WB_SKILL_STATS,  "St+St+St",
                        WB_SKILL_CLASS,  "Normal",
                        WB_SKILL_PROGRESSION, "Standard",
                        WB_SKILL_SPECIAL_BONUS, 0,
                        WB_SKILL_DESCRIPTION, "Put your description here",
                        WB_SKILL_AUTOMATIC,FALSE,
                        WB_SKILL_TRUE, TRUE,
                        -1);
}


void
skills_insert_list(char *skillname,GList *items){
        GtkTreeIter *iter,child;
        gboolean automatic;
        char *name;
        struct leaf *leaf;
        GList *cur/*,*tmp*/;
        gboolean nodes;

        assert(skillname != NULL);

        if (items == NULL){
                g_warning("Nothing to insert\n");
                return;
        }

        iter = skill_get_by_name(skillname);

        if (iter == NULL){
                printf("Warning skill %s not available!\n",skillname);
                return;
        }

        /* Make a pricate copy of the lsi tso I can play with it */
        items = g_list_copy(items);
        
        /* Do any exist - if so update them */
        nodes = gtk_tree_model_iter_children(GTK_TREE_MODEL(skillstore),
                        &child,iter);
        while (nodes){
                gtk_tree_model_get(GTK_TREE_MODEL(skillstore),&child,
                                WB_SKILL_NAME,&name,
                                WB_SKILL_AUTOMATIC,&automatic,
                                -1);
                if (name == NULL || strlen(name) == 0){
                        printf("Error - invalid name\n");
                        return;
                }
                for (cur = items ; cur != NULL ; cur = cur->next){
                        leaf = cur->data;
                        if (leaf == NULL)
                                continue;
                        if (strcmp(name,leaf->name) == 0){
                                skills_insert_list_item(skillstore,&child,leaf);
                                cur->data = NULL;
                                break;
                        } 
                }

                if (cur == NULL && automatic == TRUE){
                        /* It was automatically added, but not updated
                         *      -> It no longer exists */
                        gtk_tree_store_remove(skillstore,&child);
                        if (child.stamp == 0)
                                /* Last item in list
                                 *  Hack: Relies on internal s of GTK */
                                nodes = FALSE; 
                } else {
                        nodes = gtk_tree_model_iter_next(
                                        GTK_TREE_MODEL(skillstore), &child); 
                }
        } 
        

        
        /* We have a list of skills to insert */
        for (cur = items ; cur != NULL ; cur = cur->next){
                leaf = cur->data;
                if (leaf == NULL){
                        /* We have dealt with it */
                        continue;
                }
                gtk_tree_store_append(skillstore,&child,iter);
                skills_insert_list_item(skillstore,&child,leaf);
        }

        g_free(iter);
        
}

void
skills_insert_list_item(GtkTreeStore *store,GtkTreeIter *iter,
                struct leaf *leaf){
        gtk_tree_store_set(store,iter,
                        WB_SKILL_ELEMENT_NAME,"skill",
                        WB_SKILL_NAME,leaf->name,
                        WB_SKILL_DESCRIPTION,leaf->description,
                        WB_SKILL_AUTOMATIC,TRUE,
                        -1);
}


/*
 * skill_get_by_name
 *
 * Gets the GtkTreeIter for the specific skill
 * Note: This is almost the same as the one in skills.c in Nebula
 *      Maybe there should be a common skills library
 *
 * Args:
 *      char * name of skill <cat>\[<subskill>\..]<skill>
 * Returns
 *      GtkTreeIter * skill iter (to be freed by caller)
 */     
GtkTreeIter *
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
                        printf("Could not find:\n\t'%s' in \n"
                                        "\t'%s'\n",*part,skill);
                        break; /* Iter is NULL - cleanup and exit */
                }
        } 
        
        g_strfreev(strs);
        return iter;
}
  


