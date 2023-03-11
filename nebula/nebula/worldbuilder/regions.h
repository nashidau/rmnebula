
enum {
        REGION_TAG,
        REGION_NAME,
        REGION_DESCRIPTION,
};



void regions_init(GtkNotebook *);
void regions_load(element_t *el);
void regions_save(element_t *el);
void regions_clear(void);
GList *region_get(void);

/* Called from create */
void region_new(struct rcmenu_info *minfo,GtkTreeIter *iter);
