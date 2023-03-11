#include "nebula.h"
#include "spells.h"

/* Local functions */
static void spell_list_cost(char *list);
static char *spell_char_compare(char *teststr,char *list);
static void apply_cost(struct spell_costs *cost,char *list);


void
spells_set_cost(void){
        GList *spells;
        GList *spell;
        char *realm;

        spells = skill_get_cat_list("Spells ");
        spell = spells;
        while (spell != NULL){
                spell_list_cost((char *)(spell->data));
                spell = spell->next;
        }

        g_list_free(spells);


        /* Set channeling to be restrited if realm != Channeling or Arcane */
        realm = characteristic_get("fixed\\realm");
        if (realm == NULL){
                printf("Realm is NULL\n");
                return;
        }
        if (strstr(realm,"Channeling") == 0 && strstr(realm,"Arcane") == 0){
                skill_set_class("Power Manipulation\\Channeling","Restricted");
        }
}

static void
spell_list_cost(char *list){
        char **tests;
        char *res;
        char *realm;
        struct spell_costs *cost;
        gboolean valid;

        realm = characteristic_get("fixed\\realm");
        if (strstr(realm,"Arcane") != NULL){
                cost = arcane_costs;
        } else {
                cost = costs;
        }

        for ( /* cost */; cost->matches[0] != NULL; cost ++){
                valid = TRUE;
                for (tests = cost->matches ; *tests != NULL ; tests ++){
                        if (*tests[0] == '<'){
                                res = spell_char_compare(*tests,list);
                        } else {
                                res = strstr(list,*tests);
                        }
                        if (res == NULL)
                                valid = FALSE;
                }
                if (valid){
                        apply_cost(cost,list);
                        break;
                }
        }
}

static char *
spell_char_compare(char *teststr,char *list){
        char *charac;
        char *val;
        char *result;

        /* get rid of the '<' & '>' */
        charac = teststr + 1;
        charac = g_strdup(charac);
        charac = strtok(charac,">");

        val = g_strdup(characteristic_get(charac));

        /* Handle sub realm */
        val = strtok(val," -/");

        result = strstr(list,val);

        /* clean up */
        g_free(charac);
        g_free(val);

        return result;
}

static void
apply_cost(struct spell_costs *cost,char *list){
        char *dpcost;
        char *type;
        int pos = 0;
        char *scsmbonus;
        int bonus;

        type = characteristic_get("fixed\\type");
        if (strcmp("Pure",type) == 0)
                pos = 0;
        else if (strcmp("Hybrid",type) == 0)
                pos = 1;
        else if (strcmp("Semi",type) == 0)
                pos = 2;
        else if (strstr(type,"Non") != NULL) /* eek - different rule */
                pos = 3;
        else {
                printf("Type '%s' is unknown - assuming Non-spell user\n",type);
                pos = 3;
        }

        dpcost = cost->costs[pos];

        skill_set_cost(list,dpcost);

        /* set the base scsm bonus for the list */
        bonus = cost->bonus;
        scsmbonus = characteristic_get("fixed\\scsmbonus");
        if (scsmbonus != NULL)
                bonus += strtol(scsmbonus,NULL,10);
        bonus += 50; /* Everyone gets this */
        skill_set_special_bonus(list,bonus);
}

