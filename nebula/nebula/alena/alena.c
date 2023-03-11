#include <assert.h>
#include "alena.h"
#include "token.h"

static element_t *recursive_parse(element_t *el);
static void parse_attributes(char *token,element_t *el);
static char *d_spaces(int depth);
static void alena_save_internal(element_t *el,int depth);
static void save_attributes(element_t *el);
static void print_element(element_t *el);

char *point_to_utf8(unsigned int point,char *dest);
char *utf8_encode(int bytes, int point,char *dest);

/*
 * alena_parsefile
 *
 * Parses a given file into a tree of element_t's.
 *
 * Args:
 *      filename to be opened
 *      whitespec: Should whitespace be ignored?
 * Returns:
 *      NULL on error
 *      element_t tree otherwise
 */
element_t *
alena_parsefile(const char *filename,int whitespace){
        FILE *fp;
        element_t *el;
        struct stat st;

        if (stat(filename,&st) == -1){
                perror(filename);
                return NULL;
        }
        if (S_ISREG(st.st_mode) == 0){
                fprintf(stderr,"alena: Can only open files\n");
                return NULL;
        }

        fp = fopen(filename,"r");

        if (fp == NULL){
                fprintf(stderr,"alena: open(\"%s\",\"r\"): %s",
                                filename,strerror(errno));
                return NULL;
        }

        el = alena_parse(fp,whitespace);

        fclose(fp);

        return el;
}

/*
 * alena_parse
 *
 * As alena_parsefile, except takes a FILE * instead of the file name
 */
element_t *
alena_parse(FILE *fp,int whitespace){
        element_t *el;
        char *token;

        set_whitespace(whitespace);
        map_file(fp);

        token = gettoken();
        if (strncmp("<?xml",token,5) != 0){
                fprintf(stderr,"token: %s\n",token);
                fprintf(stderr,"not a valid XML doc\n");
                return NULL;
        }

        check_utf8(token);

        el = calloc(1,sizeof(element_t));
        if (el == NULL){
                perror("calloc");
                return NULL;
        }

        do {
                token = gettoken();
        } while (token[0] == '<' && (token[1] == '?' || token[1] == '!'));

        parse_attributes(token,el);

        recursive_parse(el);

        return el;
}

static element_t *
recursive_parse(element_t *el){
        element_t *new = NULL;
        char *token;

        while ((token = gettoken())){
                if (token[0] == '<' && token[1] == '/'){
                        strtok(token,">");
                        if (strcmp(el->name,&token[2])){
                                fprintf(stderr,"Expecting </%s> got %s>\n",
                                                el->name,
                                                token);
                                /* FIXME: Should return with error */
                                exit(1);
                        }
                        if (el->type == ETYPE_NONE)
                                el->type = ETYPE_EMPTY;
                        return el;
                }
                if (token[0] == '<' && (token[1] == '!'
                                        || token[1] == '?')){
                        /* FIXME: Just throwing it away! */
                        continue;
                }
                if (*token == '<'){
                        new = calloc(1,sizeof(element_t));
                        el->type = ETYPE_ELEMENT;
                        if (strstr(token,"/>")){
                                parse_attributes(token,new);
                                new->type = ETYPE_EMPTY;
                                el->elements = g_list_append(el->elements,new);
                        } else {
                                parse_attributes(token,new);
                                el->elements = g_list_append(el->elements,
                                                        recursive_parse(new));
                        }
                } else if (el->type != ETYPE_NONE){
                        new = calloc(1,sizeof(element_t));
                        new->type = ETYPE_TEXT;
                        new->text = strdup(token);
                        el->elements = g_list_append(el->elements,new);
                } else {
                        el->type = ETYPE_TEXT;
                        el->text = strdup(token);
                }

        }
        printf("Invalid XML !\n");

        return el;
}

static void
parse_attributes(char *token,element_t *el){
        char *p,*q;
        int end = 0;
        attribute_t *attr;
        char start;

        p = token;
        p ++; /* get past the '<' */

        while(*p != '>' && !isspace(*p))
                p ++;
        if (*p == '>')
                end = 1;
        *p = 0;

        if (el->name == 0){
                el->name = strdup(&token[1]);
        }

        p ++;

        while(!end){
                q = strchr(p,'=');
                if (q == NULL)
                        return;
                attr = calloc(1,sizeof(attribute_t));
                *q = 0;
                attr->name = strdup(p);

                q ++;
                while (*q != '\'' && *q != '"')
                        q ++;
                start = *q;
                q ++;
                p = q;
                while (*q != '\'' && *q != '"')
                        q ++;
                *q = 0;
                attr->value = strdup(p);
                q ++;
                p = q;
                while (isspace(*p))
                        p ++;
                if (*p == '>' || *p == '/')
                        end = 1;
                if (*p == '/')
                        *p = 0;

                el->attributes =
                        g_list_append(el->attributes,attr);
        }
        return;
}

static FILE *fp;

/*
 * alena_save
 *
 * Saves the supplied element tree into a file (filename).
 * If provided also inserts the given headers at the top (great for
 * DTDs and similar)
 *
 * Args:
 *   char *filename: File to save the output too
 *   element_t *el: Elements to save into the file
 *   char *headers: string to insert before root node
 */
void
alena_save(const char *filename,element_t *el, const char *headers){
        fp = fopen(filename,"w");

        if (fp == NULL){
                fprintf(stderr,"Couldn't open %s: %s\n",filename,
                                strerror(errno));
                return;
        }

        if (0)
                print_element(el);

        fprintf(fp,"<?xml version='1.0'?>\n");
        if (headers != NULL)
                fputs(headers,fp);

        alena_save_internal(el,0);
        fclose(fp);
}
static void
alena_save_internal(element_t *el,int depth){
        GList *item;

        /* FIXME: Should indent any spaces here to make it more useful */
        if (el->type == ETYPE_TEXT){
                if (el->text == NULL)
                        fprintf(fp,"%s<%s></%s>\n",d_spaces(depth),el->name,
                                                        el->name);
                else
                        fprintf(fp,"%s<%s>%s</%s>\n",d_spaces(depth),
                                                        el->name,
                                                        el->text,
                                                        el->name);
                return;
        }

        fprintf(fp,"%s<%s",d_spaces(depth),el->name);
        if (el->attributes)
                save_attributes(el);
        if (el->elements == NULL){
                fprintf(fp," />\n");
                return;
        }

        fprintf(fp,">\n");

        for (item = el->elements; item != NULL; item = g_list_next(item)){
                alena_save_internal((element_t *)item->data,depth + 1);
        }

        fprintf(fp,"%s</%s>\n",d_spaces(depth),el->name);


}

/*
 * Returns a list of spaces - used for 'tabing' of output strings
 * Note: This is seedy
 */
static char *
d_spaces(int depth){
        static char spaces[MAX_SPACES + 1];
        if (spaces[1] != ' '){
                memset(spaces,' ',MAX_SPACES);
                spaces[MAX_SPACES] = 0;
        }
        return &spaces[MAX_SPACES - TAB_SIZE *depth];
}

static void
save_attributes(element_t *el){
        GList *item;
        attribute_t *attr;

        item = el->attributes;
        while(item != NULL){
                attr = item->data;
                fprintf(fp," %s=\"%s\"",attr->name,attr->value);
                item = g_list_next(item);
        }
}

/*
 * print_element
 *
 * Non-recursivlely prints a single element to stdout
 */
static void
print_element(element_t *el){
        printf("* Element : %s\n",el->name);
        printf("\tType: %s\n",(el->type == 0) ? "None" :
                                (el->type == 1) ? "Text" :
                                "Elements");
        if (el->text)
                printf("\tText value is %s\n",el->text);
        if (el->elements)
                printf("\tHas elements (%p)\n",el->elements);
        if (el->attributes)
                printf("\tHas attributes (%p)\n",el->attributes);
}

/*
 * get_element_depth
 *
 * Find an element in the tree with the supplied name.  If there are multiple,
 * will find the first one.  Performs depth first search.
 *
 * Recursive function
 *
 * Arguments:
 *      name:   the element to search for
 *      el:     the tree
 *      depth:  maximum depth to search, or -1 for no limit
 * Returns:
 *      element_t * if found
 *      NULL if not found
 */
element_t *
get_element_depth(const char *name,element_t *el,int depth){
        GList *item;
        element_t *rv = NULL;

        if (strcmp(el->name,name) == 0)
                return el;
        if (el->type != ETYPE_ELEMENT)
                return NULL;
        item = el->elements;
        if (depth != 0){
                while (item != NULL && rv == NULL){
                        rv = get_element_depth(name,item->data,depth - 1);
                        item = g_list_next(item);
                }
        }
        return rv;
}

/*
 * get_element
 *
 * Finds an element at any depth
 * See get_element_depth for more detail
 * equiv to get_element_depth(name,el,-1);
 */
element_t *
get_element(const char *name,element_t *el){
        return get_element_depth(name,el,-1);
}
/*
 * get_text
 *
 * Returns the string value for a certain child element.  Must be a direct
 * child as this does not descend the tree.  If multiple elements exist -
 * returns the first found.
 *
 * Args:
 *      name    the name of the element with the text
 *      root      where to search from
 *
 * returns:
 *      char *  the string
 *      NULL    if element not found or ETYPE_EMPTY
 */
char *
get_text(const char *name,element_t *root){
        GList *elements;
        element_t *el;

        /* Is it this one? */
        if (root->type == ETYPE_TEXT){
                if (strcmp(root->name,name) == 0){
                        return root->text;
                }
                return NULL; /* Can't recurse from here */
        }

        /* search the list */
        elements = root->elements;
        while(elements != NULL){
                el = elements->data;
                if (strcmp(el->name,name) == 0){
                        if (el->type == ETYPE_TEXT)
                                return el->text;
                        else
                                return NULL;
                }
                elements = g_list_next(elements);
        }
        return NULL;
}

/*
 * get_int
 *
 * Returns the integer value for a certain child element.  Must be a direct
 * child as this does not descend the tree.  If multiple elements exist -
 * returns the first found.
 *
 * Args:
 *      name    the name of the element with the text
 *      root      where to search from
 *
 * returns:
 *      int     the value
 *      0       if element is 0 or not found
 */
int
get_int(const char *element,element_t *el){
        char *p;

        p = get_text(element,el);

        return strtol(p,NULL,0);

}


/*
 * create_element
 *
 * Creates an element in given element tree.
 *
 * Args:
 *      tree:   the tree to add it under (or NULL if none)
 *      name:   name of element
 *      type:   type
 *      value:  value (if valid) (type == ETYPE_TEXT)
 *
 * Returns:
 *      New element or tree
 *      NULL on error
 */
element_t *
create_element(element_t *parent,const char *name,etype_t type){
        element_t *el;

        el = calloc(1,sizeof(element_t));

        if (el != NULL)
                el->name = strdup(name);

        el->type = type;

        if (parent != NULL){
                parent->type = ETYPE_ELEMENT;
                parent->elements = g_list_append(parent->elements,el);
        }
        return el;
}

/*
 * create_text_element
 *
 * Creates an element and sets its text value
 *
 * Args:
 *      parent: parent element
 *      name:   element name
 *      value:  string to be put in element
 *
 * Returns:
 *      new element
 */
element_t *
create_text_element(element_t *parent,const char *name,const char *value){
        element_t *el;

        el = create_element(parent,name,ETYPE_TEXT);

        if (value != NULL && el != NULL)
                el->text = strdup(value);

        return el;
}

/*
 * create_numeric_element
 *
 * Another create_element wrapper.  Creates a text element and sets its
 * value to the string of the numeric valud given
 *
 * Args:
 *      parent
 *      name
 *      val     numeric value to be stored as a string
 * Returns
 *      new element
 *      NULL on error
 */
element_t *
create_numeric_element(element_t *parent,const char *name,int value){
        element_t *el;
        char buf[10];

        el = create_element(parent,name,ETYPE_TEXT);

        if (el != NULL){
                sprintf(buf,"%d",value);
                el->text = strdup(buf);
        }
        return el;
}

/*
 * add_attribute
 *
 * Adds an attribute to an element
 * If there is no value supplied the value is the attribute name
 *
 * Bugs:
 *      Doesn't prevent addition of a pre-exisiting attribute
 *
 * Args:
 *      el      element to add values to
 *      attribute attribute name to add
 *      value   optional attribute
 *
 * Returns
 *      0       on success
 *      -1      on error
 */
int
set_attribute(element_t *el,const char *attribute,const char *value){
        attribute_t *attr;

        if (attribute == NULL || el == NULL)
                return -1;
        if (value == NULL)
                value = attribute;

        attr = calloc(1,sizeof(attribute_t));
        attr->name = strdup(attribute);
        attr->value = strdup(value);

        el->attributes = g_list_append(el->attributes,attr);

        return 0;
}
