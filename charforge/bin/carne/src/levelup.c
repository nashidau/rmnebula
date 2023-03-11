#include <assert.h>

#include <gtk/gtk.h>

#include "levelup.h"
#include "carne.h"


#define LEVELUP_COMMAND "<levelup />\n"


gint
levelup_start(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	struct state *state;
	int bytes;

	state = data;
	assert(state != NULL);

	if (state->sock == -1){
		/* FIXME: Should be a dialog, or this menu item 
		 * should be deactivated */
		printf("Must connect first\n");
		return;
	}
	
	/* Send the levelup command */
	/* FIXME: Better error handling */
	write(state->sock,LEVELUP_COMMAND, strlen(LEVELUP_COMMAND));

	printf("Sent levelup command\n");

	return 0;

	
}
