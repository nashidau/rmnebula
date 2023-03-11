/*
 * Special.c
 *
 * Controls special profession, racal or other skills needed for RM cahracters
 * For instance the number of Primal tongues a Nomenist has...
 *
 * Part of nebula
 */
#include "nebula.h"
#include "special.h"

void
update_special(void){
        if (0)
                nomenist_primal();

}

/*
 * Essence Nomenists require Primal ranks to determine
 * maximum rank of spell they can cast
 */
static GtkWidget *primalranks = NULL;

static void
nomenist_primal(void){
        char *realm;
        GtkWidget *box;
        int ranks = 0;
        //GSList *skills;

        realm = characteristic_get("Fixed/Realm");
        if (realm == NULL || strstr(realm,"Nomenist") == NULL)
                return;

        if (primalranks == NULL){
                box = new_row(specialpage);
                primalranks = new_text_box(box,"Primal Tongue",TRUE);
                gtk_entry_set_editable(GTK_ENTRY(primalranks),FALSE);
        }
        /*
        skills = get_skills("Communications/ *Spoken");
        while(skills != NULL){
                ranks += ((skill_t *)(skills->data))->ranks;
                skills = g_slist_next(skills);
        }
        */
        ranks = round_div(ranks,7);
        entry_set_digit(primalranks,ranks);

        gtk_widget_show_all(specialpage);
}




