#include "alena.h"
#include "token.h"

static char *copy_to(char *dest,char *src,char end,int max,int inclusive);

static int whitespace = 0; /* Is white space significant?? */
static char *buf;    /* where we mmaped to.. */
static int length;
int utf8;

char *point_to_utf8(unsigned int point,char *dest);
char *utf8_encode(int bytes, int point,char *dest);


/* For gettoken */
static char *p = NULL,*q = NULL;

void
check_utf8(char *token){

        /* Assume it is */
        utf8 = TRUE;

        /* Currently only one exception */
        if (strstr(token,"iso-8859") != NULL){
                utf8 = FALSE;
        }

        return;
}

char *
point_to_utf8(unsigned int point, char *dest){

        if (point < 0x7f){
                return utf8_encode(1,point,dest);
        } else if (point < 0x000007FF){
                return utf8_encode(2,point,dest);
        } else if (point < 0x0000FFFF){
                return utf8_encode(3,point,dest);
        } else if (point < 0x001FFFFF){
                return utf8_encode(4,point,dest);
        } else if (point < 0x03FFFFFF){
                return utf8_encode(5,point,dest);
        } else if (point < 0x7FFFFFFF){
                return utf8_encode(6,point,dest);
        } else {
                printf("Unknown point %d\n",point);
                return 0;
        }
}

#define CONTINUE_BYTE 0x80
#define CLEAR_MASK 0x3F
char *
utf8_encode(int bytes, int point,char *dest){
        unsigned char mask;
        unsigned char tmp;
        int i;

        tmp = 0x80;
        mask = tmp;
        for (i = 1 ; i < bytes ; i ++){
                tmp = tmp >> 1;
                mask |= tmp;
        }

        *dest = 0;

        *dest |= mask;


        /* now do the actual data */
        *dest |= (char)(point >> ((bytes - 1)* 6));
        *dest ++;

        bytes --;
        while (bytes > 0){
                *dest = 0;
                tmp = (char)(point >> ((bytes - 1) * 6));
                *dest |= tmp;
                *dest &= CLEAR_MASK;
                *dest |= CONTINUE_BYTE;
                *dest ++;
                bytes --;
        }

        return dest;
}


void
map_file(FILE *fp){
        int fd;
        struct stat stat;

        fd = fileno(fp);

        fstat(fd,&stat);
        length = stat.st_size;

        buf = mmap(0,length,PROT_READ,MAP_PRIVATE,fd,0);

        /* set up gettoken */
        p = buf;
}

void
unmap_file(void){
        munmap(buf,length);
}

void
set_whitespace(int newwhitespace){
        whitespace = newwhitespace;
}

/*
 * gettoken
 *
 * returns the next token in the xml file.
 * FIXME:  doesn't handle <!-- --> <!-- --> properly
 */
char *
gettoken(){
        static char rbuf[ALENA_BUF];

        while(isspace(*p) || !strncmp(p,"<!--",4)){
                while(isspace(*p))
                        p ++;

                if (strncmp(p,"<!--",4) == 0){
                        /* A comment */
                        p = strstr(p,"-->");
                        p += 3;
                }
        }

        q = rbuf;
        if (*p == '<'){
                p = copy_to(q,p,'>',ALENA_BUF,1);
        } else {
                p = copy_to(q,p,'<',ALENA_BUF,0);
        }

        return rbuf;

}

/*
 * copy_to
 *
 * Copies from buffer to buffer diong XML entity translations
 * Stops whenit reaches end or maxbytes
 *
 * Args
 *      dest
 *      src
 *      endcharacter
 *      max     maxcharacters copied into dest buffer - 1
 *      inclusive   Should we copy the last character?
 * Resturns
 *      new src pointer
 */
static char *
copy_to(char *dest,char *src,char end,int max,int inclusive){
        int value;
        int move;

        while (max -- && *src != 0 && *src != end){
                if (*src == '&'){
                        src ++;
                        if (*src == '#'){
                                src ++;
                                if (*src == 'x'){
                                        src ++;
                                        value = strtol(src,&src,16);
                                } else
                                        value = strtol(src,&src,10);
                                if (*src == ';')
                                        src ++;
                                else
                                        printf("Invalid numeric attribute\n");
                                dest = point_to_utf8(value,dest);
                                dest --; // hack to avoid ++ at end ;-)
                        } else {
                                if (strncmp("amp;",src,4) == 0){
                                        *dest = '&';
                                        src += 4;
                                } else if (strncmp("lt;",src,3) == 0){
                                        *dest = '<';
                                        src += 3;
                                } else if (strncmp("gt;",src,3) == 0){
                                        *dest = '>';
                                        src += 3;
                                } else if (strncmp("quot;",src,5) == 0){
                                        *dest = '"';
                                        src += 5;
                                } else if (strncmp("apos;",src,5) == 0){
                                        *dest = '\'';
                                        src += 5;
                                } else {
                                        printf("Unknown char %5.5s\n",src);
                                        *dest = '?';
                                }
                        }
                        dest ++;
                } else if (*src == '\n'){
                        *dest ++ = *src ++;
                        while (*src == ' ' || *src == '\t')
                                *src ++;
                } else if ((*src & 0x80) > 0) {
                        if (utf8){
                                /* Eek! */
                                g_utf8_strncpy(dest,src,1);
                                src = g_utf8_next_char(src);
                                dest = g_utf8_next_char(dest);
                        } else {
                                value = *((unsigned char *)src);
                                move = g_unichar_to_utf8(value, dest);
                                dest += move;
                                src ++;
                        }
                } else {
                        *dest ++ = *src ++;
                }
        }
        if (inclusive)
                *dest ++ = *src ++;

        *dest = 0;

        return src;

}
