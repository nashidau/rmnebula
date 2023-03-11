#ifndef FILE_H
#define FILE_H
#define CHARACTER_BASE_PATH "../characters/"

typedef enum {
        FILE_NONE, /* Nothing loaded - nothing saved */
        FILE_CLEAN, /* Has been loaded - not modified */
        FILE_MODIFIED, /* Has been loaded - not modified */
} file_state_t;

struct fileinfo {
        char *name; /* File name - in local filename encoding (not UTF8) */
        file_state_t state;
};

struct fileops {
        char *docname;  /* "Character", "Genre" etc.. */
        char *printgconfkey; /* Key to look up to print */
        char *defaultprintkey; /* Key to look up to print */
        struct fileinfo *(*create)(void);
        struct fileinfo *(*open)(const char *);
        int (*close)(void);
        void (*save)(struct fileinfo *);
        void (*refresh)(void);
        const char *(*getfilename)(struct fileinfo *);
};

void file_set_fileops(struct fileops *);


void file_preload(char *name);
void file_new_cb(void);
void file_open_cb(void);
void file_save_cb(void);
void file_save_as_cb(void);
void file_print_cb(void);
void file_quit_cb(void);

void file_close_cb(void);
void file_revert_cb(void);
gboolean file_loaded(void);
gboolean file_modify(void);
void file_set_modified(void);
gboolean file_set_none(void);



#define FILE_MENU_ITEM                                  \
        {                                               \
                "/_File",NULL,                          \
                NULL,0,                                 \
                "<Branch>",0                            \
        },                                              \
                /* File opening options */              \
                {                                       \
                        "/File/_New",    "<control>N",  \
                        (GtkItemFactoryCallback)file_new_cb,0,\
                        "<StockItem>", GTK_STOCK_NEW\
                },{ \
                        "/File/_Open",   "<control>O",\
                        (GtkItemFactoryCallback)file_open_cb,    0, \
                        "<StockItem>" ,GTK_STOCK_OPEN\
                },{\
                        "/File/sep1",  NULL,\
                        NULL, 0,\
                        "<Separator>", 0\
                },\
                /* Saving Functions */\
                /*  Todo: Revert function */\
                {\
                        "/File/_Save","<control>S",\
                        (GtkItemFactoryCallback)file_save_cb,0,\
                        "<StockItem>", GTK_STOCK_SAVE\
                },{ \
                        "/File/Save _As","<control>A",\
                        (GtkItemFactoryCallback)file_save_as_cb, 0, \
                        "<StockItem>", GTK_STOCK_SAVE_AS\
                },{\
                        "/File/_Revert",NULL,\
                        (GtkItemFactoryCallback)file_revert_cb, 0, \
                        "<StockItem>", GTK_STOCK_REVERT_TO_SAVED\
                },{     \
                        "/File/sep2",NULL,\
                        NULL,     0,\
                        "<Separator>",0\
                },\
                /* Printing */\
                {\
                        "/File/_Print","<control>P",\
                        (GtkItemFactoryCallback)file_print_cb, 0, \
                        "<StockItem>", GTK_STOCK_PRINT,\
                },{     \
                        "/File/sep3",NULL,\
                        NULL,     0,\
                        "<Separator>",0\
                },\
                /* Close operations */\
                {\
                        "/File/_Close", "<control>W",\
                        (GtkItemFactoryCallback)file_close_cb,0,\
                        "<StockItem>",GTK_STOCK_CLOSE\
                },\
                { \
                        "/File/_Quit","<control>Q",\
                        (GtkItemFactoryCallback)file_quit_cb,0,\
                        "<StockItem>", GTK_STOCK_QUIT\
                }


#endif /* FILE_H */
