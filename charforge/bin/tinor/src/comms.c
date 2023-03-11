
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>

#include "comms.h"
#include "command.h"
#include "levelup.h"

enum { BACKLOG = 10 }; /* Max pending connections */

static void sigchild_handler(int signal);
static void socket_connection(int sock);

int
socket_create(void){
        int sock,rv;
        struct sockaddr_in my_addr;
        struct sigaction sa;
        int yes = 1;

        
        sock = socket(AF_INET,SOCK_STREAM, 0);
        if (sock == -1){
                perror("socket()");
                return -1;
        }

        yes = 1;
        if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
                perror("setockopt()");
                return -1;
        }

        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(8374);
        my_addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(my_addr.sin_zero),0,8); /* 8?? */

        rv = bind(sock, (struct sockaddr *)&my_addr,sizeof(struct sockaddr));
        if (rv == -1){
                perror("bind()");
                return -1;
        }

        if (listen(sock, BACKLOG) == -1){
                perror("listen()");
                return -1;
        }

        sa.sa_handler = sigchild_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; /* Not portable */
        if (sigaction(SIGCHLD, &sa, NULL) == -1){
                perror("sigaction()");
                return -1;
        }

        return sock;

        
}


static void
sigchild_handler(int signal){
        assert(signal == SIGCHLD);

        /* FIXME: Should use waitpid */
        while (wait(NULL) > 0)
                ;
}


int
socket_connect_loop(int sock){
        struct sockaddr_in theiraddr;
        size_t sinsize;
        int newsock;
        int child;
        
        while (1){
                sinsize = sizeof(struct sockaddr_in);
                newsock = accept(sock,(struct sockaddr *)&theiraddr,&sinsize);
                if (newsock == -1){
                        perror("accept()");
                        fprintf(stderr,"Continuing anyway\n");
                        continue;
                }

                printf("Connection from %s\n",inet_ntoa(theiraddr.sin_addr));
                child = fork();
                if (child == -1){
                        /* An error */
                        perror("fork()");
                        return 1;
                }
                if (child == 0){
                        /* Child thread */
                        socket_connection(newsock);
                        exit(0);
                }
                close(newsock);
        }
                        

}


/*
 * socket connection
 *
 * Handles a single connected client 
 */
static void 
socket_connection(int sock){
        struct user *user;
        char buf[1024];

        do {
                read(sock,buf,1024);
        } while (command_parse(buf,sock) != -1);
}

