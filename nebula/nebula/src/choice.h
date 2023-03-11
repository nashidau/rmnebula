#ifndef CHOICE_H
#define CHOICE_H

enum {	CHOICE_ITEM,
	CHOICE_SELECTED, 
	CHOICE_VALUE,
	CHOICE_CAN_SELECT,
	CHOICE_DATA 
};

struct store_info choice_info[] = {
        { CHOICE_ITEM,       G_TYPE_STRING,  "", 0, 0, 0 },
        { CHOICE_SELECTED,   G_TYPE_BOOLEAN, "", 0, 0, 0 },
	{ CHOICE_VALUE,	     G_TYPE_INT,     "", 0, 0, 0 },
        { CHOICE_CAN_SELECT, G_TYPE_BOOLEAN, "", 0, 0, 0 },
        { CHOICE_DATA,       G_TYPE_STRING,  "", 0, 0, 0 },
        { -1,                0,            NULL, 0, 0, 0 },
};

struct view_info choice_view[] = {
        { "Item",  CHOICE_ITEM,     "text",   0, 0, 0 },
        { "Choice",CHOICE_SELECTED, "toggle", 0, CHOICE_CAN_SELECT, 0 },
        { 0,0,0,0,0,0 },
};

struct view_info choice_view_value[] = {
	{ "Choice", CHOICE_VALUE,  "text", 0,
		CHOICE_CAN_SELECT, (GtkSignalFunc)choice_set_value },
	{ 0, 0, 0, 0, 0, 0 },
};

#endif /* CHOICE_H */
