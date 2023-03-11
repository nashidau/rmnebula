#ifndef AL_LOGWINDOW_H
#define AL_LOGWINDOW_H

#include <gtk/gtk.h>
#include <gnome.h>

void al_logwindow_create(GtkWidget * pan);
void al_logwindow_reset(void);
void al_logwindow_add_text(const char * text);

#endif
