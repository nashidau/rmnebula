/**
 * Handle communication to the remote server.
 */
#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>


#include <gtk/gtk.h>

#include "carne.h"
#include "comms.h"

static void comms_data_in(gpointer, gint, GdkInputCondition);


/**
 * Connect to remote host.
 * 
 * @arg state System state structure
 * @returns Return value
 */ 
int
comms_connect(struct state *state){
	struct hostent *he;
	struct sockaddr_in theiraddr;
	int sock,rv;

	assert(state != NULL);
	
	he = gethostbyname(SERVER);
	if (he == NULL){
		herror("gethostbyname()");
		exit(1);
	}
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock == -1){
		perror("socket()");
		exit(1);
	}

	theiraddr.sin_family = AF_INET;
	theiraddr.sin_port = htons(PORT_NUM);
	memset(&(theiraddr.sin_zero),0,8);

	rv = connect(sock,(struct sockaddr *)&theiraddr,
			sizeof(struct sockaddr));
	if (rv == -1){
		perror("connect()");
		exit(1);
	}

	state->sock = sock;
	state->sock_tag = gdk_input_add(state->sock, GDK_INPUT_READ, 
			comms_data_in,state);
		
	return sock;
	
}

/**
 * Callback for incomiong data from server 
 */
static void 
comms_data_in(gpointer data, int fd, GdkInputCondition cond){
	struct state *state;
	char buf[BUFSIZ];
	int bytes;

	state = data;

	assert(state != NULL);

	printf("w00t - data in\n");	

	bytes = read(state->sock,buf,BUFSIZ); 
	if (bytes < 1){
		perror("read(sock)");
		return;
	}
	buf[bytes] = 0;
		
	printf("Got: %s",buf);	
}
	






