

/*
 * For all intents and purposes a nasty global - slight wrapped up into one
 * object.  Most callbacks take this as their data argument - a few will use a 
 * structure contianing a pointer to this and some other data.
 * In any case there is one of these strucutres per main window.
 */
struct state {
	int sock;	/* Remote connection socket */
};
