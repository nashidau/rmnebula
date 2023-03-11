#ifndef ITEM_H
#define ITEM_H

typedef enum carried {
        CARRY_ALWAYS,  /* Always carry it - even nude */
        CARRY_BASE,    /* Even when sleeping - rings et al */
        CARRY_CITY,    /* 'City' cloths - no armor or pack */
        CARRY_ARMOR,   /* Equipment - no pack, but armor */
        CARRY_NORMAL,  /* For the adventurer */
        CARRY_NEVER    /* At home */
} carried_t;

char *carry_type[] = {
        "Always",
        "Base",
        "City",
        "Armor",
        "Normal",
        "Home",
        NULL
};


struct item {
        char *name;
        char *desc;
        carried_t carried;
        int weight;
        struct bonus *bonus;
        int row;
};

struct bonus {
        int type;
        void *which;
        int bonus;
        struct bonus *next;
};

void add_item(void);

#endif /* ITEM_H */
