#include <string.h>

#include "logwindow.h"

#ifndef AL_ITEMPARSING_H
#include "itemparsing.h"
#endif

void
al_item_general_parsing(struct al_item * an_item, element_t * xml_root, struct al_xml_parsing * parse_structure)
{
	GList *elems;
	element_t *el;
	/* Reseting numbers */
	{
		int i=0;
		while(parse_structure[i].match_name)
		{
			parse_structure[i].times = 0;
			i++;
		}
	}

	/* Parsing */
	for (elems = xml_root->elements; elems != NULL; elems = elems->next)
	{
		int i = 0;
		el = elems->data;

		while(parse_structure[i].match_name)
		{
			if(strcmp(el->name,parse_structure[i].match_name)==0)
			{
				parse_structure[i].times++;
				if((parse_structure[i].cardinality & AL_PARSING_UNIQUE) && parse_structure[i].times>1)
				{
					char logtext[100];
					sprintf(logtext,"Already got element : %s.\n",el->name);
					al_logwindow_add_text(logtext);
				}
				else if(parse_structure[i].callback)
				{
					parse_structure[i].callback(an_item,el);
				}
				break;
			}
			i++;
		}
		if(NULL==parse_structure[i].match_name)
		{
			char logtext[100];
			sprintf(logtext,"Element not parsed : %s\n",el->name);
			al_logwindow_add_text(logtext);
		}
	}
	
	/* Test cardinality */
	{
		int i=0;
		while(parse_structure[i].match_name)
		{
			if((parse_structure[i].cardinality & AL_PARSING_MUSTHAVE) && parse_structure[i].times==0)
			{
				char logtext[100];
				sprintf(logtext,"Element not found : %s\n",parse_structure[i].match_name);
				al_logwindow_add_text(logtext);
			}
			i++;
		}
	}
}
