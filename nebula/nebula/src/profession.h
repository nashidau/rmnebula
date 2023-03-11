#ifndef PROFESSION_H
#define PROFESSION_H

struct realms {
        char *realm;
        char *stats[3];
        GList *subtypes;
        GList *defaulttypes;
};

/*
 * The fixed realm types
 */
struct realms realms[] = {
        {"Essence",{"Em",0,0},0,0},
        {"Channeling",{"In",0,0},0,0},
        {"Mentalism",{"Pr",0,0},0,0},
        {"Arcane",{"Em","In","Pr"},0,0},
        {0,{0,0,0},0,0}
};

#endif /* Profession.h */
