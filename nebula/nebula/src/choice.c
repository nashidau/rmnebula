#include "nebula.h"

static void
choice_set_value(GtkCellRendererText *cell, const gchar *path_string,
                const gchar *new_text, gpointer data);
#include "choice.h"

static void show_choice(element_t *el);
static void create_choice_window(char *title,struct choice *choice);
static void choice_add_options(struct choice *choice,char *group,GList *list);
static void apply_all_actions(struct choice *choice,GtkTreeIter *parent);
static void apply_action(struct choice *choice,GtkTreeIter *iter);
static void choice_add_skills(struct choice *choice,char *group,GtkTreeIter *p);
static gboolean choice_count_leaf(GtkTreeModel *model,GtkTreePath *path,
		GtkTreeIter *iter, gpointer data);
static void set_multiple_select(struct choice *choice);


void
choice_parse(element_t *elements){
        GList *list;
        element_t *el;

        for (list = elements->elements ; list != NULL ; list = list->next){
                el = list->data;
                if (strcmp(el->name,"choice") != 0){
                        printf("Element %s unknown\n",el->name);
                        continue;
                }
                show_choice(el);
        }
}

/*
 * show_choice
 *
 * parses a <choice> tag to display a window and show a list
 * to allow the user to select the skills
 *
 * FIXME: The various tags should be done more intelligently
 *      (ie a proper parser)
 * FIXME: Add a 'title option' to give better feedback
 * FIXME: Add 'desctiption' to allow you to be a little speal in
 *
 * Args:
 *      elements
 */
static void
show_choice(element_t *choicetag){
        element_t *el;
        GList *list;
        char *search,*title;
        GList *choices;
	struct choice *choice;
	int leafcount;

        choice = g_malloc0(sizeof(struct choice));
        /* Eek, hardcoded */
        choice->min = 1;
        choice->max = 1;
        choice->count = 0;
        create_choice_window("Selection",choice);

        for (list = choicetag->elements ; list != NULL ; list = list->next){
                el = list->data;
                if (strcmp(el->name,"group") == 0){
                        search = el->text;
                        choice_add_skills(choice,search,NULL);
                } else if (strcmp(el->name,"class") == 0){
                        choice->action = SET_CLASS;
                        choice->data = el->text;
                } else if (strcmp(el->name,"select") == 0){
                        choice->min = choice->max = strtol(el->text,0,0);
                } else if (strcmp(el->name,"option") == 0){
                        choices = g_list_append(NULL,el->text);
                        choice_add_options(choice,NULL,choices);
                } else if (strcmp(el->name,"realm") == 0){
                        choice->action = SET_REALM;
                } else if (strcmp(el->name,"ranks") == 0){
                        choice->action = SET_RANKS;
                        choice->max = strtol(el->text,0,0);
                        choice->min = choice->max;
		} else if (strcmp(el->name,"title") == 0){
			title = get_text("title",el);
			printf("new title '%s'\n",title);
			if (title != NULL)
				gtk_window_set_title(GTK_WINDOW(window),title);
		} else if (strcmp(el->name,"multiple") == 0){
			set_multiple_select(choice);
                } else {
                        printf("Tag %s unknown\n",el->name);
                }
        }

	/* Check there are enough items to choose from */
	if (choice->min > 0){
		leafcount = 0;
		gtk_tree_model_foreach(GTK_TREE_MODEL(choice->store),
				choice_count_leaf,&leafcount);
		if (leafcount < choice->min){
			printf("Not enough options in tree: please fix\n");
			printf("leafcount is %d\n",leafcount);
			choice->min = 0;
		}
	}
	
        gtk_widget_show_all(choice->window);
        gtk_dialog_run(GTK_DIALOG(choice->window));

        apply_all_actions(choice,NULL);

        gtk_widget_destroy(choice->window);


}

static gboolean
choice_count_leaf(GtkTreeModel *model,GtkTreePath *path,
		GtkTreeIter *iter, gpointer data){
	int *leafcount;
	
	if (gtk_tree_model_iter_has_child(model,iter) == FALSE){
		leafcount = data;
		(*leafcount)++;
	}
	return FALSE;
}	

/*
 * create_choice_window
 *
 * Allocates a store, creates a window, and tree view.  Also creates
 * an okay button.
 *
 * Will also display a count
 *
 * Args:
 *      char *title of window
 *      struct choice *choice structure
 */
static void
create_choice_window(char *title,struct choice *choice){
        GtkWidget *scrollwin;

        choice->store = make_tree_store(choice_info);
        choice->view = tree_create_view_data(GTK_TREE_MODEL(choice->store),
                        choice_view,choice);
        choice->window = gtk_dialog_new_with_buttons(title,
                        GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        NULL);
        gtk_window_set_default_size(GTK_WINDOW(choice->window),300,315);
        choice->button = gtk_dialog_add_button(GTK_DIALOG(choice->window),
                        GTK_STOCK_OK,GTK_RESPONSE_OK);

        gtk_widget_set_sensitive(choice->button,FALSE);

        scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(choice->window)->vbox),
                        scrollwin);
        gtk_container_add(GTK_CONTAINER(scrollwin),choice->view);

}

/*
 * choice_add_options
 *
 */
static void
choice_add_options(struct choice *choice,char *group,GList *list){
        tree_set_from_list_data(choice->store,NULL,CHOICE_ITEM,list,
                        CHOICE_DATA,group);
        tree_set_flag(choice->store,NULL,CHOICE_CAN_SELECT,TRUE);
}

/*
 * choice_add_skills
 *
 * Adds a list of skills.  Automatically adds subskills
 *
 *
 */
static void
choice_add_skills(struct choice *choice,char *group,GtkTreeIter *parent){
        GList *skills,*skill,*subskills;
        char *fullgroup;
        GtkTreeIter iter;

        skills = skill_get_skill_list(group);
	assert(skills != NULL);
        for (skill = skills; skill != NULL ; skill = skill->next){
                /* Add the skills */
                gtk_tree_store_append(GTK_TREE_STORE(choice->store),
                                &iter,parent);
                gtk_tree_store_set(GTK_TREE_STORE(choice->store),&iter,
                                CHOICE_ITEM,skill->data,
                                CHOICE_SELECTED,FALSE,
                                CHOICE_DATA,group,
                                CHOICE_CAN_SELECT,TRUE,-1);

                /* Does it have any subskills? */
                fullgroup = g_strconcat(group,"\\",skill->data,NULL);
                subskills = skill_get_skill_list(fullgroup);
                if (subskills != NULL){
                        gtk_tree_store_set(GTK_TREE_STORE(choice->store),&iter,
                                CHOICE_CAN_SELECT,FALSE,-1);
                        choice_add_skills(choice,fullgroup,&iter);
                }
        }

        g_list_free(skills);

}

static void
apply_all_actions(struct choice *choice,GtkTreeIter *parent){
        GtkTreeIter iter;
        GtkTreeModel *model;
        gboolean flag;
        gboolean selected;
	int count;

        model = GTK_TREE_MODEL(choice->store);

        if (parent == NULL)
                flag = gtk_tree_model_get_iter_first(model,&iter);
        else
                flag = gtk_tree_model_iter_children(model,&iter,parent);

        while (flag){
		gtk_tree_model_get(model,&iter,CHOICE_SELECTED,&selected,
				CHOICE_VALUE,&count,-1);

                if (selected || count > 0)
                        apply_action(choice,&iter);

                if (gtk_tree_model_iter_has_child(model,&iter))
                        apply_all_actions(choice,&iter);

                flag = gtk_tree_model_iter_next(model,&iter);
        }


}

static void
apply_action(struct choice *choice,GtkTreeIter *iter){
        char *item,*data;
        int ranks,sel;
	int value;

        switch (choice->action){
        case SET_CLASS:
                gtk_tree_model_get(GTK_TREE_MODEL(choice->store),iter,
                                CHOICE_ITEM,&item,CHOICE_DATA,&data,-1);
                if (data != NULL)
                        item = g_strconcat(data,"\\",item,NULL);
                else
                        item = g_strdup(item);
                skill_set_class(item,choice->data);
                g_free(item);
                break;
        case SET_REALM:
                gtk_tree_model_get(GTK_TREE_MODEL(choice->store),iter,
                                CHOICE_ITEM,&item,-1);
                characteristic_set("fixed\\realm",item);
                break;
        case SET_RANKS:
                gtk_tree_model_get(GTK_TREE_MODEL(choice->store),iter,
				CHOICE_ITEM,&item,CHOICE_DATA,&data,
				CHOICE_SELECTED,&sel,
				CHOICE_VALUE,&value,-1);
                if (data != NULL)
                        item = g_strconcat(data,"\\",item,NULL);
                else
                        item = g_strdup(item);
                ranks = skill_get_ranks(item);
                if (ranks != -1){
                        ranks += sel + value;
                        skill_set_ranks(item,ranks);
                }
        default:
                break;
        }
}


/*
 * set_multiple_select
 *
 * Determines if you can select more then one of a particular item
 * For somethings this makes no sense, for others it is necessary.
 *
 * By default it is set to off 
 *
 * Args:
 *	struct choice *choice
 * Returns:
 *	nothing
 */
static void 
set_multiple_select(struct choice *choice){
	GtkTreeViewColumn *col;
	
	/* Delete the toggle column */
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(choice->view),1);
	gtk_tree_view_remove_column(GTK_TREE_VIEW(choice->view),col);

	/* Add the text column */	
	col = get_text_column(choice_view_value,GTK_TREE_MODEL(choice->store),
			choice);	
	gtk_tree_view_append_column(GTK_TREE_VIEW(choice->view), col);	

}


static void
choice_set_value(GtkCellRendererText *cell, const gchar *path_string,
		const gchar *new_text, gpointer data){
	struct choice *choice;
	int temp;
	int old;
        GtkTreeIter iter;

	choice = data;
	assert(choice != NULL);

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(choice->store),
			&iter,path_string);
	
	gtk_tree_model_get(GTK_TREE_MODEL(choice->store),&iter,
			CHOICE_VALUE,&old, -1);
	choice->count -= old;
	
        temp = strtol(new_text,NULL,0);
	gtk_tree_store_set(choice->store,&iter,CHOICE_VALUE,temp,-1);
	choice->count += temp;

	printf("count is %d  (%d/%d)\n",choice->count,choice->min,choice->max);
		
	if ((choice->min == -1 || choice->count >= choice->min) &&
			(choice->max == -1 || choice->count <= choice->max)){
		if (choice->button != NULL)
			gtk_widget_set_sensitive(choice->button,TRUE);
	} else {
		if (choice->button != NULL)
			gtk_widget_set_sensitive(choice->button,FALSE);	
	}

	
}

