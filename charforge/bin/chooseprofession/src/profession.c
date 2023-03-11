#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "comms.h"
#include "gtk_support.h"
#include "main.h"
#include "profession.h"
#include "support.h"

static void add_menu_item(GtkWidget *,const char *,const char *);
static void profession_select(const char *filename);

static void parse_name(xmlNode *);
static void parse_description(xmlNode *);
static void parse_lists(xmlNode *);
static void parse_category(xmlNode *);
static void parse_skills(xmlNode *parent,GtkTreeStore *tree,GtkTreeIter *iter);

static void profession_data_get(int,xmlChar *,xmlNode *, void *);

static void profession_remove_file(const char *filename);


#define QUOTE_STYLE "quote"

/* FIXME: Should be global/config related */
/* Double FIXME: Should really be getting it from network server */
#define DATADIR "../data"

struct  professionparsers {
	xmlChar *name;
	void (*parserfunc)(xmlNode *node);
};
struct professionparsers parsers[] = {
	{ "name",		parse_name },
	{ "description",	parse_description },
	{ "quote",		parse_description },
	{ "baselists",          parse_lists },
	{ "skills",		parse_category },
	{ NULL, NULL },
};


/**
 * Loads the list of professions to insert into the selection
 *
 */
void
profession_init(void){
	GtkWidget *widget;
	GtkTextBuffer *buffer;	

	/* Set up a tag table */
	widget = lookup_widget(MainWindow,"descriptiontextview");
	assert(widget != NULL);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));

	gtk_text_buffer_create_tag(buffer,QUOTE_STYLE,
			"indent",30,
			"style",PANGO_STYLE_ITALIC,
			NULL);

}

void
profession_callback(struct state *state, void *data){
	GtkWidget *optionmenu,*menu;
	xmlChar *name,*ref;
	xmlDoc *doc;
	xmlNode *cur;

	optionmenu = lookup_widget(MainWindow,"professionmenu");

	assert(optionmenu != NULL);

	menu = gtk_menu_new();
	
	/* FIXME: This should be dragged down from the server */
	doc = xmlParseFile("/tmp/chooseprofession/professions.xml");
	if (doc == NULL){
		error_dialog("Could not open file",
				"Could not open the file '"
				DATADIR "/professions.xml");
		exit(1);
	}
	cur = doc->children;
	if (xmlStrcmp(cur->name,"professionlist") != 0){
		error_dialog("Profession list file corrupt",
				"Did not get the expected tag parsing "
				"profession list");
		exit(1);
	}

	for (cur = cur->children; cur != NULL ; cur = cur->next){
		if (cur->type != XML_ELEMENT_NODE)
			continue;
		if (xmlStrcmp(cur->name,"profession") != 0){
			error_dialog("Unknown tag",cur->name);
			continue;
		}
		name = node_get_text(cur);
		ref = node_get_attribute(cur,"ref");
		if (name == NULL){
			error_dialog("Malformed file",
					"Professions should not be empty");
			exit(1);
		} else if (ref == NULL){
			error_dialog("Malformed file",
					"All professions need a reference");
			exit(1);
		}
		add_menu_item(menu,name,ref);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optionmenu),menu);

	xmlFreeDoc(doc);
	
	
}

/**
 * Add menu item
 *
 */
static void
add_menu_item(GtkWidget *menu,const char *name,const char *ref){
	GtkWidget *item;
	char *refcpy;
	static int first = 0;

	refcpy = g_strdup(ref);
	
	item = gtk_menu_item_new_with_label(name);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect_swapped(G_OBJECT(item), "activate",
		                      G_CALLBACK(profession_select), 
				      refcpy);
	gtk_widget_show(item);

	g_object_set_data(G_OBJECT(item),"name",g_strdup(name));
	g_object_set_data(G_OBJECT(item),"ref",g_strdup(ref));

	if (first == 0){
		first ++;
		profession_select(ref);
	}
}


#define PROFESSION_GET "<getprofession>%s</getprofession>"
static void
profession_select(const char *profession){
	static const char *lastfile = NULL;
	char buf[BUFSIZ];
	int len;

	if (lastfile != NULL && strcmp(profession,lastfile) == 0)
		/* Same file, just return */
		return;
	
	snprintf(buf,BUFSIZ,PROFESSION_GET,profession);
	len = strlen(buf);

	send_request(buf,RESPONSE_FILE,profession,profession_data_get,NULL);

	lastfile = profession;
}

static void
profession_data_get(int request,xmlChar *type,xmlNode *paramnode, void *data){
	char buf[BUFSIZ],filename[BUFSIZ];
	struct stat st;
	xmlDoc *doc;
	xmlNode *node;
	struct professionparsers *cur;
	char *url,*shortname;
	int rv;

	url = node_get_attribute(paramnode,"name");
	shortname = node_get_attribute(paramnode,"response");
	snprintf(filename,BUFSIZ,"/tmp/chooseprofession/professions/%s",shortname);
	snprintf(buf,BUFSIZ,"wget -q -O %s %s",filename,url);

	rv = system(buf);
	if (rv != 0){
		printf("wget failed: return code %d\n",WEXITSTATUS(rv));
		return;
	}	

		
	if (stat(filename,&st) == -1){
		const char *msg;
		if (errno == ENOENT)
			msg = "File does not exist";
		else if (errno == EACCES)
			msg = "Could not access file";
		else 
			msg = strerror(errno);
		error_dialog("Could not open file",msg);
		return;
	}
	
	doc = xmlParseFile(filename);
	if (doc == NULL){
		printf("Failed to load %s\n",filename);
		error_dialog("Could not parse file",
				"The file appears corrupted");
		return;
	}


	/* Drop down first level */	
	node = doc->children;	
	assert(node != NULL);
	while (node != NULL && node->type != XML_ELEMENT_NODE)
		node = node->next;
	assert(node != NULL);
	if (xmlStrcmp(node->name,"rmssprofession") != 0){
		printf("Removing file: Got %s instead of rmssprofession\n",
				node->name);
		profession_remove_file(filename);
		return;
	}
	node = node->children;
	
	/* Clear the fields I'm about to set */
	for (cur = parsers ; cur->name != NULL ; cur ++)
		if (cur->parserfunc != NULL)
			cur->parserfunc(NULL);
	
	for ( ; node != NULL ; node = node->next){
		if (node->type != XML_ELEMENT_NODE)
			continue;	
		for (cur = parsers ; cur->name != NULL ; cur ++){
			if (xmlStrcmp(cur->name,node->name) == 0)
				cur->parserfunc(node);
		}
	}

	xmlFreeDoc(doc);
}


/**
 * Removes profession associated with the given filename -
 *	only works if it was hte last item selected
 */
static void
profession_remove_file(const char *filename){
	GtkWidget *optionmenu;
        GtkWidget *menu,*menuitem;
        const char *objectfile;
 
        optionmenu = lookup_widget(MainWindow,"professionmenu");
        assert(optionmenu != NULL);
 
 
        menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optionmenu));
        assert(menu != NULL);
                                                                              
        menuitem = gtk_menu_get_active(GTK_MENU(menu));
	if (menuitem == 0)
		return;
        assert(menuitem != NULL);
                                                                              
        objectfile = g_object_get_data(G_OBJECT(menuitem),"filename");

	
}




void 
profession_get_reply(int request,xmlChar *type,
		xmlNode *node,void *data){
	xmlChar *url;
	char buf[BUFSIZ];

	printf("Woot - get reply\n");
	
	url = node_get_attribute(node,"name");
	if (url == NULL){
		printf("No url supplied\n");
		return;
	}

	/* FIXME: Should do this as a callback */
	snprintf(buf,BUFSIZ,"wget -q -O /tmp/chooseprofession/professions.xml %s",url);
	system(buf);

	profession_callback(state,data);
	
}








static void 
parse_name(xmlNode *node) { 
	GtkWidget *widget;

	widget = lookup_widget(MainWindow,"descriptionlabel");
	assert(widget != NULL);

	if (node == NULL)
		gtk_label_set_text(GTK_LABEL(widget),"None");
	else
		gtk_label_set_text(GTK_LABEL(widget),node_get_text(node));
		
}
static void 
parse_description(xmlNode *node) { 
	GtkWidget *widget;
	GtkTextBuffer *buffer;	
	xmlChar *str;

	widget = lookup_widget(MainWindow,"descriptiontextview");
	assert(widget != NULL);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));

	if (node == NULL){
		/* Clear buffer, and return */
		text_buffer_clear(buffer);	
		return;
	} 

	for (node = node->children ; node != NULL ; node = node->next){
		if (node->type != XML_TEXT_NODE)
			/* FIXME: Handle 'ref' and the like */
			continue;
		str = node_get_text(node);
		str_clean(str);
		if (xmlStrcmp(node->parent->name,"quote") == 0){
			/* FIXME: Should indent and italise */
			text_buffer_insert(buffer,NULL,"\n\n");
			text_buffer_insert(buffer,QUOTE_STYLE,str);
		} else {
			text_buffer_insert(buffer,NULL,str);
		}
	}

	
}


static void 
parse_lists(xmlNode *node) {
	GtkWidget *widget;
	GtkListStore *list;
	xmlNode *cur;
	char *listname,*description;
	static GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	static int pos = 0;

	widget = lookup_widget(MainWindow,"spelllistview");
	assert(widget != NULL);
	
	list = (GtkListStore *)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (list == NULL){
		list = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
		assert(list != NULL);
		gtk_tree_view_set_model(GTK_TREE_VIEW(widget),
				(GtkTreeModel *)list);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("List",
				renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
		column = gtk_tree_view_column_new_with_attributes("Description",
				renderer, "text", 1,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);

	}

	/* If node is NULL - empty list */
	if (node == NULL){
		gtk_list_store_clear(list);	
		pos = 0;
		return;
	}
	
	for (cur = node->children ; cur != NULL ; cur = cur->next){
		if (cur->type != XML_ELEMENT_NODE)
			continue; 
		if (xmlStrcmp(cur->name,"list") != 0){
			printf("Got a %s: Expected list\n",cur->name);
			continue;
		}
		description = node_get_text(cur);
		listname = node_get_attribute(cur,"name");
		if (listname == NULL) {
			/* 'Old' style */ 
			listname = description;
			description = NULL;
		}
		gtk_list_store_append(list,&iter);
		gtk_list_store_set(list,&iter,0,listname,
				1,description,-1);
		pos ++;
	}
}










static void 
parse_category(xmlNode *node) {
	GtkWidget *widget;
	GtkTreeStore *tree;
	xmlNode *cur;
	char *listname,*cost,*bonusstr;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	int bonus;

	widget = lookup_widget(MainWindow,"skillcostview");
	assert(widget != NULL);
	
	tree = (GtkTreeStore *)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (tree == NULL){
		tree = gtk_tree_store_new(3,G_TYPE_STRING,G_TYPE_STRING,
				G_TYPE_INT);
		assert(tree != NULL);
		gtk_tree_view_set_model(GTK_TREE_VIEW(widget),
				(GtkTreeModel *)tree);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("Category",
				renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
		column = gtk_tree_view_column_new_with_attributes("Cost",
				renderer, "text", 1,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
		column = gtk_tree_view_column_new_with_attributes("Bonus",
				renderer, "text", 2,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}

	/* If node is NULL - empty list */
	if (node == NULL){
		gtk_tree_store_clear(tree);	
		return;
	}
	
	for (cur = node->children ; cur != NULL ; cur = cur->next){
		if (cur->type != XML_ELEMENT_NODE)
			continue; 
		if (xmlStrcmp(cur->name,"category") != 0){
			printf("Got a %s: Expected category\n",cur->name);
			continue;
		}
		listname = node_get_attribute(cur,"name");
		cost = node_get_attribute(cur,"cost");
		bonusstr = node_get_attribute(cur,"bonus");
		gtk_tree_store_append(tree,&iter,NULL);
		if (bonusstr != NULL){
			bonus = strtol(bonusstr,0,0);
			gtk_tree_store_set(tree,&iter,0,listname,1,cost,
					2,bonus,-1);
		} else {
			gtk_tree_store_set(tree,&iter,0,listname,1,cost,-1);
		}
		if (cur->children != NULL)
			parse_skills(cur,tree,&iter);
			
	}
}


static void
parse_skills(xmlNode *node,GtkTreeStore *tree,GtkTreeIter *parent){
	xmlNode *cur;
	GtkTreeIter iter;
	const char *skill,*class,*bonusstr;
	int bonus;
	
	assert(parent != NULL);
	assert(tree != NULL);
	assert(node != NULL);

	for (cur = node->children ; cur != NULL ; cur = cur->next){
		if (node->type != XML_ELEMENT_NODE)
			continue;
		skill = node_get_attribute(cur,"name");
		class = node_get_attribute(cur,"class");
		bonusstr = node_get_attribute(cur,"bonus");
		gtk_tree_store_append(tree,&iter,parent);
		if (bonusstr != NULL){
			bonus = strtol(bonusstr,0,0);
			gtk_tree_store_set(tree,&iter,0,skill,1,class,
					2,bonus,-1);
		} else {
			gtk_tree_store_set(tree,&iter,0,skill,1,class,-1);
		}		
	}
	
}




