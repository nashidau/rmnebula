#ifndef TREE_H
#define TREE_H 1

#include "alena.h"

typedef enum {
        LIST,
        TREE
} treeview_t;

typedef enum {
        UPDATE_NONE,  /* no data */
        UPDATE_SUM,   /* Sum the field in updatedata */
        UPDATE_AVG,   /* Avg the fields in updatedata */
        UPDATE_PARENT,/* Copy data from top level parent data, field in
                                updatadata (0 in parent) */
        UPDATE_FUNCTION, /* Call updatedata with (iter as arg) */
} update_t;

typedef int (*updatefunc)(GtkTreeIter *stat);

typedef enum {
        SAVE_ALWAYS,
        SAVE_PARENT,
        SAVE_CHILDREN,
        SAVE_BOTTOM,
        SAVE_DEFINED,
        SAVE_NEVER,
        SAVE_ELEMENT,
        SAVE_NOT_MIDDLE, /* Parent or Bottom */
        SAVE_SPECIAL = 0x1000,
} save_t;

struct store_info {
        int pos;
        GType datatype;
        char *xmlname;
        update_t update;
        void *updatedata;
        save_t save;
        /* If the SAVE_SPECIAL flag is set, we use the loadfn when loading
         * or saveing to determine how to make a tree from it */
        int (*loadfn)(GtkTreeStore *,GtkTreeIter *,int col,element_t *el);
        element_t *(*savefn)(GtkTreeStore *,GtkTreeIter *);
};

struct view_info {
        char *title;
        int col;  /* which col in the store info */
        char *displaytype;
        char *renderinfo;
        /* Which col in the store is the editable flag for
           this item: ignored is editfunc == NULL */
        int editflag;
        GtkSignalFunc editfunc;
};


/*
 * Information for popup items to edit/display item
 */
typedef enum {
        POPUP_TOGGLE, /* A toggle button */
        POPUP_ENTRY,  /* A Entry box */
        POPUP_DIGIT,  /* A Entry box for digits */
        POPUP_TEXTVIEW, /* A multiline text box */
        POPUP_COMBO,  /* A combo box of limited choices */
} popuptype_t;

struct treepopupinfo {
        int field;  /* Field in the store to get the data out of it */
        popuptype_t type; /* What to display it as */
        char *label; /* Display name */
        char **data; /* Combo info (only currently) */
        GtkWidget *widget; /* Where to store the widget */
};
struct popupinfo {
        GtkWidget *window;
        struct treepopupinfo *tinfo;
        GtkTreeStore *store;
        GtkTreeIter iter;
        gboolean editable;
};


typedef enum {
        ACTION_NONE,
        SET_CLASS,
        SET_REALM,
        SET_RANKS,
} choice_act_t;

struct choice {
        int min;
        int max;
        int count;
        GtkTreeStore *store;
        GtkWidget    *view;
        GtkWidget    *window;
        GtkWidget    *button;
        GtkWidget    *display;
        choice_act_t action;
        void         *data;
};

/* For list info */
typedef enum {
        XML_COMPULSARY = 0x00,
        XML_OPTIONAL   = 0x01,
        XML_MULTIPLE   = 0x02,
        XML_LONG       = 0x10,
        XML_LAST       = 0x20,
} xmltype_t;

struct listinfo {
        char *xmltag; /* Tag to be seen in the file */
        char *label;  /* Label to display in gui */
        xmltype_t type; /* COmpulsary, multi (, seperated), optional */
        GtkWidget *widget; /* Displayed widget */
};

/*
 * struct taginfo
 *
 * Used for inserting text into poppups.  Each taginfo contaisn an offset and
 * end for updating a text buffer 
 */
struct taginfo {
        GtkTextTag *tag;
        int start,end;
};



GtkTreeStore *make_tree_store(struct store_info *store);
GtkWidget *tree_create_view(GtkTreeModel *store,struct view_info *view_info);
GtkWidget *tree_create_view_data(GtkTreeModel *store,struct view_info *view_info,gpointer data);

GtkTreeIter *tree_lookup(GtkTreeModel *store,int *primaries,char *key,GtkTreeIter *parent);
void list_insert(GtkListStore *store,struct store_info *sinfo,
                int *primaries,element_t *el);
void tree_insert(GtkTreeStore *store,struct store_info *sinfo,
                int *primaries,element_t *el,char **recurse,GtkTreeIter *par);

/* Refresh functions */
void tree_refresh(struct store_info *,GtkTreeStore *store);
void tree_refresh_item(struct store_info *sinfo, GtkTreeStore *store,
		GtkTreeIter *item);



void tree_save(GtkTreeModel *store,struct store_info *sinfo,element_t *root);
GtkTreeIter *get_parent(GtkTreeModel *store,GtkTreeIter *iter);

void tree_set_from_list(GtkTreeStore *store,GtkTreeIter *parent,int field,GList *list);
void tree_set_from_list_data(GtkTreeStore *store,GtkTreeIter *parent,
                int col,GList *list,int field,char *data);
GList *tree_get_from_list(GtkTreeModel *store,int field);
void tree_set_flag(GtkTreeStore *store,GtkTreeIter *parent,int col,gboolean flag);
void
tree_set_flag_recursive(GtkTreeStore *store,GtkTreeIter *parent,int col,gboolean value);
void tree_delete_item(GtkTreeStore *,struct store_info *,GtkTreeIter *);
void tree_copy_item(GtkTreeStore *store,GtkTreeIter *src,GtkTreeIter *parent);

/* For popup menus... */
void tree_popup_add(GtkWidget *view,GtkItemFactoryEntry *items,int n,gpointer);


/*
 * popup views
 */
struct popupinfo *tree_create_popup(struct treepopupinfo *tinfo,
		GtkWidget *parentwindow);
void tree_save_popup(struct popupinfo *pinfo);
void tree_set_popup(struct popupinfo *pinfo);

/* List infos */
GtkWidget *tree_make_listinfo(struct listinfo *);
void tree_listinfo_insert(struct listinfo *,element_t *);
void tree_listinfo_save(struct listinfo *,element_t *el);
void tree_listinfo_clear(struct listinfo *);

/* Adding and removing columns */
GtkTreeViewColumn *get_text_column(struct view_info *vinfo,
                GtkTreeModel *store,gpointer data);
GtkTreeViewColumn *get_toggle_column(struct view_info *vinfo,gpointer data);



#endif /* TREE_H */
