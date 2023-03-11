

struct listinfo *listinfo;

struct leaf {
        const char *name;
        const char *description;
};

const char *wb_get_genre_name(struct fileinfo *finfo);
gboolean wb_set_genre_name(const char *name);
GList *wb_get_leaves(GtkTreeModel *store,int namefield, int description);
