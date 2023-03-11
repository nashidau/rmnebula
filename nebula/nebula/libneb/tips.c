#include <gtk/gtk.h>
#include "tips.h"

GtkTooltips *tooltips;

void
init_tips(void){
        tooltips = gtk_tooltips_new ();
}

void
set_tip(GtkWidget *widget,char *tip){
        gtk_tooltips_set_tip(tooltips,widget,tip,NULL);

}
