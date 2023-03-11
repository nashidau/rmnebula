#include <string.h>
#include <gtk/gtk.h>

#ifndef AL_LOGWINDOW_H
#include "logwindow.h"
#endif

/* The TextView buffer  */
static GtkTextBuffer *log_window = NULL;

void
al_logwindow_create(GtkWidget * pan)
{
        GtkWidget *textview;
                
	if(log_window)
	{
		printf("Log window already created. Internal Error\n");
		return;
	}

        textview = gtk_text_view_new();
        log_window = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview),GTK_WRAP_WORD);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textview),FALSE);
        
	gtk_paned_pack2 (GTK_PANED (pan), textview, FALSE, TRUE);
}

void
al_logwindow_reset()
{
        GtkTextIter start,end;

        gtk_text_buffer_get_bounds(log_window,&start,&end);
        gtk_text_buffer_delete(log_window,&start,&end);
}

void al_logwindow_add_text(const char * text)
{
        GtkTextIter end;
        
        gtk_text_buffer_get_end_iter(log_window,&end);
        gtk_text_buffer_insert(log_window,&end,text,-1);
        
}

