/*
 * Statgain 
 *
 * For a supplied temp and potential stat, statgain generates a new pair of
 * temps and potentials
 *
 * Not this uses nashs rules for stat gain of potentials, on pair of 6s, 7s 8
 * 9 or 10s, potential is increased by d10.
 * 
 * Input format:
 *   curtmp,curpot
 * Output:
 *   newtmp,newpot
 */
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

/* XPath expressions */
const char *highestlevel = "/rmsscharacter/sequence/step[last()]";
const char *statlist = "/rmsscharacter/stats/stat";

static int stat_gain(int,int,int*,int*);
static int d10(void);
static int xml_statgain(char **argv);
static char *get_highest_level(xmlXPathContext *context);
static void stat_gain_node(xmlXPathContext *context,char *tag, xmlNode *node);

int 
main(int argc,char **argv){
	char buf[BUFSIZ],*p;
	int tmp,pot;
	int ntmp,npot;

	if (argv[1] != NULL){
		return xml_statgain(argv);
	}
	
	while (fgets(buf,BUFSIZ,stdin) != NULL){
		tmp = pot = -1;

		tmp = strtol(buf,&p,10);
		while (!isdigit(*p) && *p != 0)
			p ++;
		if (*p == 0){
			printf("E: You must supply both temp and pot\n");
			continue;
		}
		pot = strtol(p,NULL,10);

		if (tmp < 1 || pot < 1){
			printf("E: Ignoring tmp/pots of %d/%d\n",tmp,pot);
			continue;
		}

		stat_gain(tmp,pot,&ntmp,&npot);

		printf("%d,%d\n",ntmp,npot);
	}

	return 0;	
}


/**
 * xml_statgain
 *
 * If a file is parsed statgain loads and parse the given file as an xml file
 * and produces a statgain XMl document for the character.
 *
 * if (argv[2] is supplied this output is written to the supplied file.
 *
 * @param argv Array of statgains
 * @return 0 on success, -1 on error
 */
static int
xml_statgain(char **argv){
	char *infile, *outfile = NULL;
	xmlDoc *doc;
	xmlXPathContext *xpcon;
	char *level;
	xmlNodeSet *set;
	xmlXPathObject *res;
	xmlNode *node;
	int i;
	
	infile = strdup(argv[1]);
	assert(infile != NULL);
	if (argv[2] != NULL)
		outfile = strdup(argv[2]);
	else {
		outfile = calloc(strlen(infile) + strlen(".sg"),1);
		strcpy(outfile,infile);
		strcat(outfile,".sg");
	}
	if (strcmp(infile,outfile) == 0){
		printf("Urg: outfile and infile the same (%s)\n",outfile);
		return -1;
	}

	doc = xmlParseFile(infile);
	if (doc == NULL){
		printf("Could not parse infile\n");
		return -1;
	}

	xpcon = xmlXPathNewContext(doc);	
	if (xpcon == NULL){
		printf("Could not create XPath context\n");
		return -1;
	}
	
	level = get_highest_level(xpcon);

	printf("Level is %s\n",level);

	res = xmlXPathEval(statlist,xpcon);
	set = res->nodesetval;
	printf("%d nodes\n",set->nodeNr);
	for (i = 0 ; i < set->nodeNr ; i ++){
		node = set->nodeTab[i];
		stat_gain_node(xpcon,level,node);
	}
		
	
	return 0;	
}


static char *
get_highest_level(xmlXPathContext *context){
	xmlXPathObject *res;
	xmlNode *node;
	xmlAttr *attr;

	res = xmlXPathEval(highestlevel,context);

	if (res == NULL){
		printf("Empty node set\n");
		return NULL;
	}
	if (xmlXPathNodeSetGetLength(res->nodesetval) != 1){
		printf("Expecting one node\n");
		return NULL;
	}

	node = res->nodesetval->nodeTab[0];	

	for (attr = node->properties; attr != NULL ; attr = attr->next){
		if (xmlStrcmp("seq",attr->name) == 0){
			/* Found it */
			assert(attr->children != NULL);
			return attr->children->content;
		}
	}
	
	return NULL;
}

static void
stat_gain_node(xmlXPathContext *context,char *tag, xmlNode *node){
	xmlXPathObject *temp,*pot;
	const char *tval;
	char buf[BUFSIZ];
	int i;

	xmlXPathRoot(context);
	context->node = node;	

	snprintf(buf,BUFSIZ,"stat/temp[@seq='%s']",tag);
	temp = xmlXPathEval(highestlevel,context);

	snprintf(buf,BUFSIZ,"stat/potential[@seq='%s']",tag);
	pot = xmlXPathEval(highestlevel,context);
	
	{
		xmlNode *n;
		n =  temp->nodesetval->nodeTab[0];
		i = temp->nodesetval->nodeNr;
		tval = xmlNodeListGetString(context->doc, n, 1);
		printf(" - %s\n",n->name);
	}
	printf("tmp/pot: (%s) %d %s/%s\n",tval,i,temp->stringval,pot->stringval);

	/* Extract current temp and pot values using XPATH */

	/* Call stat gain function */

	/* Save 'statgain' items in levelup tree */
	
	/* Hint: xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); */
}


static int
stat_gain(int tmp,int pot,int *ntmp,int *npot){
	int roll1,roll2,diff;
	
	assert(ntmp != NULL);
	assert(npot != NULL);

	roll1 = d10();
	roll2 = d10();

	*ntmp = tmp;
	*npot = pot;
	diff = pot - tmp;
	
	if (roll1 == roll2 && roll1 < 6){
		/* Goes down */
		*ntmp -= roll1;
	} else if (roll1 == roll2){
		/* Go up */
		*ntmp += 2 * roll1;
		if (*npot > 90)
			*npot = 90 + (int)pow(*npot - 90,2);
		*npot += d10() + d10();
		if (*npot > 90)
			*npot = 90 + (int)sqrt(*npot - 90);
	} else if (diff > 20){
		/* Both dice */
		*ntmp += roll1 + roll2;	
	} else if (diff > 10){
		/* High die */
		*ntmp += (roll1 > roll1) ? roll1 : roll2;
	} else /* diff <= 10) */{
		/* Low die */
		*ntmp += (roll1 > roll1) ? roll2 : roll1;
	}

	if (*ntmp > *npot)
		*ntmp = *npot;

	return *ntmp;	
}

static int
d10(void){
	static int init = 0;
	if (init == 0){
		init = 1;
		srand(time(NULL) ^ getpid());
	}

	return 1 + (int)(10.0 * rand() / (RAND_MAX + 1.0));
}
