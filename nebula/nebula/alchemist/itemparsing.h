#ifndef AL_ITEMPARSING_H
#define AL_ITEMPARSING_H

#include "alena_types.h"

#ifndef AL_ITEMS_H
#include "items.h"
#endif

#define AL_PARSING_NEVER	0x0
#define AL_PARSING_UNIQUE	0x1
#define AL_PARSING_MUSTHAVE	0x2
#define AL_PARSING_SEVERAL	0x4
#define AL_PARSING_DEFAULT	0x8

struct al_xml_parsing
{
	char *	match_name;
	int		cardinality; // see defines above
	void 	(*callback)(struct al_item * an_item, element_t * el);
	int		times;
};

void al_item_general_parsing(struct al_item * an_item, element_t * xml_root, struct al_xml_parsing * parse_structure);

#endif

