#ifndef AL_ITEMS_H
#define AL_ITEMS_H

#include <gtk/gtk.h>
#include <gnome.h>

#define AL_ITEM_CONTAINER	0x0001
#define AL_ITEM_MISC		0x0002
#define AL_ITEM_GEM			0x0004
#define AL_ITEM_POISON		0x0008
#define AL_ITEM_HERB		0x0010
#define AL_ITEM_ARMOR		0x0020
#define AL_ITEM_WEAPON		0x0040
#define AL_ITEM_SHIELD		0x0080

#define AL_ITEM_SPELL_CONSTANT 0x01
#define AL_ITEM_SPELL_DAILY    0x02
#define AL_ITEM_SPELL_CHARGED  0x04
		
struct al_item_spell
{
	char *  spell_list_name;
	char *  spell_name;
	int		type;
	int		charges;
	int		level;
	int		cast_as_level;
};

struct al_item
{
	void *  widget_pointer;
	char *  name;
	char *  description;
	char *  weight;
	char *  size;
	char *  value;
	char *  misc_name;
	int     type;
	char    intelligent;
	char *  int_alignment;
	char *  int_purpose;
	char *  int_level;
	int	    spell_multiplier;
	float   spell_mul_times;
	int	    spell_adder;
	int	    spell_add_plus;
	GList * spell_list;
};

void             al_free_item          (struct al_item * an_item);
struct al_item * al_allocate_item      (void);
void             al_set_item_text_field(char ** field,const char * name);


#endif

