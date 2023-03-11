#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <libxml/xmlreader.h>
#include <libxml/parser.h>

#include "tirno.h"

static int stat_bonuses[] = {
          0, /* 0 - No idea what 'correct' is for this */
        -10,-9,-9,-8,-8,-7,-7,-6,-6,-5, /*  1 - 10 */
        -4,-4,-4,-4,-4,-3,-3,-3,-3,-3,  /* 11 - 20 */
        -2,-2,-2,-2,-2,-1,-1,-1,-1,-1,  /* 21 - 30 */
        0,0,0,0,0,0,0,0,0,0,            /* 31 - 40  */
        0,0,0,0,0,0,0,0,0,0,            /* 41 - 50 */
        0,0,0,0,0,0,0,0,0,0,            /* 51 - 60 */
        0,0,0,0,0,0,0,0,0,1,            /* 61 - 70 */
        1,1,1,1,2,2,2,2,2,3,            /* 71 - 80 */
        3,3,3,3,4,4,4,4,4,5,            /* 81 - 90 */
        5,6,6,7,7,8,8,9,9,10,           /* 91 -100 */
};
 
struct control {
        int id;
        xmlChar *name;
        xmlChar *attribute;
        struct control *next;
};
                                                                                
int 
main(int argc,char **argv){
	int rv,sock;

        sock = socket_create();
                                                                                
        if (sock == -1){
                fprintf(stderr,"Could not create socket\n");
                exit(1);
        }
                                                                                
        rv = socket_connect_loop(sock);
                                                                                
        exit(rv);

}
