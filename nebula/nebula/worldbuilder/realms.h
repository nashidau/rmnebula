
enum {
        REALM_TAG,
        REALM_NAME,
        REALM_DESCRIPTION,
};


void realms_load(element_t *el);
void realms_save(element_t *el);
void realms_clear(void);
GList *religions_get(void);
GList *realms_get(void);
void realms_new(struct rcmenu_info *minfo,GtkTreeIter *iter);
