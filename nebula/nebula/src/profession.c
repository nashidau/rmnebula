#include "nebula.h"
#include "profession.h"

void
prof_add_realm_xml(element_t *data){
        struct realms *r;
        struct realm_subtype *subtype;
        element_t *el;
        GList *elements;


        for (r = realms ; r->realm != NULL ; r ++){
                el = get_element(r->realm,data);
                if (el == NULL)
                        continue;

                elements = el->elements;
                while (elements != NULL){
                        el = elements->data;
                        subtype = calloc(1,sizeof(struct realm_subtype));
                        subtype->name = g_strdup(el->name);
                        subtype->description = g_strdup(el->text);
                        r->subtypes = g_list_append(r->subtypes,subtype);

                        elements = g_list_next(elements);
                }
        }
}

/*
 * prof_get_subtypes
 *
 * Get a list of all the realm subtypes of a particular realm.
 */
GList *
prof_get_subtypes(char *realm){
        struct realms *rp;

        rp = realms;
        while (rp->realm != NULL){
                if (strncasecmp(realm,rp->realm,strlen(rp->realm)) == 0){
                        if (rp->subtypes != NULL)
                                return rp->subtypes;
                        else
                                return rp->defaulttypes;
                }
                rp ++;
        }

        return NULL;
}

/*
 * set_realm_stat
 *
 * Sets the realm stat(s) for the given realm
 *
 * Args:
 *      char *  Realm name
 */
void
set_realm_stat(const char *rname){
        struct realms *realm;
        char *str;
        int i;

        if (rname == NULL)
                return;

        for (realm = realms ; realm->realm != NULL ; realm ++){
                if (strcmp(rname,realm->realm) == 0){
                        break; /* Found it */
                }
        }

        if (realm == NULL){
                printf("Warning: realm %s unknown\n",rname);
                return;
        }

        for (i = 0 ; i < 3 ; i ++){
                if (realm->stats[i]){
                        stat_set_realm_stat(realm->stats[i], TRUE);
                }
        }
        str = g_strjoin("/",realm->stats[0],realm->stats[1],
                        realm->stats[2],NULL);
        skill_set_realm_stat(str);

        free(str);

}

GList *
list_realms(void){
        GList *realmlist;
        struct realms *realm;

        realmlist = NULL;
        realm = realms;

        while (realm->realm != NULL){
                realmlist = g_list_append(realmlist,realm->realm);
                realm ++;
        }

        return realmlist;
}
