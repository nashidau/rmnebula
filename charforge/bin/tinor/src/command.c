#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "command.h"

#define UNKNOWN "Unknown command - try 'help'.\n" 
#define PROFESSIONFILENAME  "../data/professions.xml"

struct command {
        char *cmd;
        int (*func)(char *cmd,int fd);
        char *help;
};

int command_parse(char *buf,int fd);

static char *get_args(char *buf);

static int cmd_ack(char *cmd,int fd);
static int cmd_help(char *cmd,int fd);
static int cmd_exit(char *cmd,int fd);
static int cmd_levelup(char *buf, int fd);
static int cmd_professions(char *buf, int fd);


struct command commands[] = {
        { "help",    cmd_help,  "Displays this help" },
        { "exit",    cmd_exit,  "Exits the server" },
        { "quit",    cmd_exit,  "Alias for exit" },
        { "logout",  cmd_exit,  "Alias for exit" },
        { "bye",     cmd_exit,  "Exit for SMTP users" },
	{ "professions", cmd_professions, "Sends the profession file" },
	{ "levelup", cmd_levelup, "Begin Levelup procedure <levelup/>" },
        { NULL, NULL, NULL },
};


int
command_parse(char *buf,int fd){
        struct command *cmd;
	char *p;

	p = buf;
	while (isspace(*p)) p ++;
	if (*p == '<')
		p ++;
	while (isspace(*p)) p ++;

	
        for (cmd = commands ; cmd->cmd != NULL ; cmd ++){
                if (strncmp(cmd->cmd,p,strlen(cmd->cmd)) == 0){
                        if (cmd->func == NULL)
                                return 0;
                        return cmd->func(buf,fd);
                }
        }

        write(fd,UNKNOWN,strlen(UNKNOWN));

        return 0;        
}

static char *
get_args(char *cmd){
        char *p;
        
        /* Skip leading whitespace */
        while (isspace(*cmd))
                cmd ++;
        /* Skip the command */
        while (!isspace(*cmd))
                cmd ++;
        /* Skip any intercening whitespace */
        while (isspace(*cmd) && *cmd != 0 && *cmd != '\n' && *cmd != '\r')
                cmd ++;

        /* Terminate the string */
        for (p = cmd ; *p != 0 && *p != '\n' && *p != '\r' ; p ++)
                ;
        *p = 0;
        
        return cmd;
}

#define NOTIMPLEMENTED "\t[Not implemented]\n" 
static int
cmd_help(char *cmd,int fd){
        struct command *command;
        char buf[1024];
        int len;

        for (command = commands ; command->cmd != NULL ; command ++){
                len = snprintf(buf,1024,"%s:\t%s\n",command->cmd,command->help);
                write(fd,buf,len);
                if (command->func == NULL)
                        write(fd,NOTIMPLEMENTED,strlen(NOTIMPLEMENTED));
        }
        return 0;                        
}

#define GOODBYE "Thanks for visiting Tinor\n"
static int
cmd_exit(char *buf, int fd){
        write(fd,GOODBYE,strlen(GOODBYE));
        printf("Logging out - bye\n");
        return -1; /* force an exit */
}

static int
cmd_levelup(char *buf, int fd){
	levelup_begin(fd);
	
	return 0;
}

/**
 * cmd_professions
 *
 * The <professions> command returns a list of all the professions avialable
 * in the current campaign.
 *
 * TODO: Should actually load the appropriate file for the campaign, not just
 * the hardcoded file.
 * 
 * Response:
 *   #bytes\n
 *   <professionlist>
 *   ...
 *   </professionlist>
 * 
 * @param buf Pointer to input buffer
 * @param fd the file 
 */
static int 
cmd_professions(char *inbuf, int fd){
	struct stat st;
	char buf[BUFSIZ];
	int proffd,nbytes;

	
	
	if (stat(PROFESSIONFILENAME,&st) != 0){
		printf("Urg\n");
		return -1;
	}
		
	proffd = open(PROFESSIONFILENAME,O_RDONLY);
	if (proffd == -1){
		printf("Eek\n");
		return -1;
	}

	nbytes = snprintf(buf,BUFSIZ,"%d\n",st.st_size);
	/* FIXME: check error */
	write(fd,buf,nbytes);

	nbytes = 0;
	if (sendfile(fd,proffd,&nbytes,st.st_size) != st.st_size){
		printf("Urg2\n");
		return -1;
	}

	return 0;	
}
