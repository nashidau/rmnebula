
struct spell_costs {
        char *matches[3];
        char *costs[4];
        int bonus;
};

struct spell_costs costs[] = {
    /* If it matches */          /*  Pure   Hybrid  Semi    Non */  /* bonus */
 {{"<fixed\\profession>",NULL},    { "3/3/3","4/4/4","6/6/6", NULL  }, 10},
 {{"<fixed\\realm>","Open",NULL},  { "4/4/4","4/4/4","8/8",  "x1"   }, 5},
 {{"<fixed\\realm>","Closed",NULL},{ "4/4/4","4/4/4","10/10","x1"   }, 0},
 {{"<fixed\\realm>","Base",NULL},  { "8/8"  ,"10/10","25",   "x1"   },-10},
 {{"<fixed\\realm>","TP",NULL},    { "4/4/4","4/4/4","6/6/6","8/8"  }, 5}, /*?*/
 /* Apply the Arcanes */
 {{"Arcane","Open",NULL},         { "6/6",  "5/5",  "12",   "x1"    },-5},
 {{"Arcane","Closed",NULL},       { "10/10","8/8",  "18",   "x1"    },-5},
 {{"Arcane","Base",NULL},         { "25",   "12",   "40",   "x1"    },-5},
 /* Not own realm */
 {{"Open",NULL},                  { "10/10","12",   "30",   "x1"    },-10},
 {{"Closed",NULL},                { "20",   "25",   "45",   "x1"    },-20},
 {{"Base",NULL},                  { "50",   "60",   "80",   "x1"    },-30},
 {{"TP",NULL},                    { "8/8",  "8/8",  "12/12","16/16" },-10},/*?*/
 {{NULL},                         {NULL},0}
};

struct spell_costs arcane_costs[] = {
 /* Arcane within Arcane realm */
 {{"<fixed\\profession>",NULL},    { "3/3/3", NULL,  "6/6/6", NULL  }, 10},
 {{"Arcane","Open",NULL},         { "4/4/4", NULL,  "11/11", "x1"   },  5},
 {{"Arcane","Closed",NULL},       { "4/4/4", NULL,  "16/16", "x1"   },  0},
 {{"Arcane","Base",  NULL},       { "12/12", NULL,  "25",    "x1"   },-10},
 /* The SL realms */
 {{"Open",NULL},                  { "5/5",   NULL,  "12",    "x1"   },-10},
 {{"Closed",NULL},                { "6/6",   NULL,  "25",    "x1"   },-20},
 {{"Base",NULL},                  { "10/10", NULL,  "60",    "x1"   },-30},
 {{"TP",NULL},                    { "8/8",   NULL,  "12/12", "16/16"},-10},/*?*/
 {{NULL},                         {NULL},0}
};


