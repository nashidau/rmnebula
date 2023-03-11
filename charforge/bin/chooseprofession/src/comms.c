/**
 * Handle communication to the remote server.
 */
#include <assert.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>


#include "main.h"
#include "comms.h"
#include "gtk_support.h"

struct pending_callbacks {
	int type;
	xmlChar *match;
	void (*callback)(int type,xmlChar*,xmlNode *node,void *data);
	void *data;
	struct pending_callbacks *next,*prev;
};

struct pending_callbacks *callbacks;

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
	int flag = 1;

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

  	rv = setsockopt(sock,            /* socket affected */
                          IPPROTO_TCP,     /* set option at TCP level */
                          TCP_NODELAY,     /* name of option */
                          (char *) &flag,  /* the cast is historical 
                                                  cruft */
                          sizeof(int));    /* length of option value */
	
	memset(&theiraddr,0,sizeof(struct sockaddr_in)); 
	theiraddr.sin_family = AF_INET;
	theiraddr.sin_port = htons(PORT_NUM);
	theiraddr.sin_addr = *((struct in_addr *)he->h_addr);

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
comms_data_in(gpointer userdata, int fd, GdkInputCondition cond){
	xmlDoc *doc;
	xmlNode *node;
	struct pending_callbacks *cbs;
	struct state *state;
	xmlChar buf[BUFSIZ];
	xmlChar *response;
	int bytes;
	request_t type;

	state = userdata;

	assert(state != NULL);

	bytes = read(state->sock,buf,BUFSIZ); 
	if (bytes < 1){
		perror("read(sock)");
		return;
	}
	buf[bytes] = 0;
		
	doc = xmlParseMemory(buf,bytes);

	node = doc->children;	
	response = node_get_attribute(node,"response"); 
	
	if (xmlStrcmp(node->name,"file") == 0) {
		type = RESPONSE_FILE;
	} else {
		type = RESPONSE_DATA;
	}

	/* FIXME: I ignore type when searching */
	for (cbs = callbacks ; cbs != NULL ; cbs = cbs->next){
		if (strcmp(cbs->match,response) != 0)
			continue;
		cbs->callback(cbs->type,cbs->match,node,cbs->data);

		if (cbs->prev != NULL)
			cbs->prev->next = cbs->next;
		else
			callbacks = cbs->next;
		if (cbs->next != NULL)
			cbs->next->prev = cbs->prev;
		free(cbs->match);
		free(cbs);
		break;
	}

	if (cbs == NULL){
		printf("Did not find callback for %s\n",response);
		return;
	}
	
	xmlFreeDoc(doc);
}


int
send_request(const xmlChar *msg,int type,const xmlChar *reply,
		void (*callback)(int,xmlChar*,xmlNode*,void*),
		void *data){
	struct pending_callbacks *ncb;
	char buf[BUFSIZ];
	
	ncb = calloc(1,sizeof(struct pending_callbacks));

	ncb->type = type;
	ncb->callback = callback;
	ncb->match = g_strdup(reply);
	ncb->data = data;

	/* Insert */	
	ncb->next = callbacks;
	if (callbacks != NULL)
		callbacks->prev = ncb;
	callbacks = ncb;
	
	strncpy(buf,msg,BUFSIZ);
	if (strchr(buf,'\n') == NULL){
		strcat(buf,"\n");
	}
	write(state->sock,buf,strlen(buf));	
		
	return 0;
}




