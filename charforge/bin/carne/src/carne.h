

/*
 * For all intents and purposes a nasty global - slight wrapped up into one
 * object.  Most callbacks take this as their data argument - a few will use a 
 * structure contianing a pointer to this and some other data.
 * In any case there is one of these strucutres per main window.
 */
struct state {
	GtkWidget *window; /* Main window */

	int sock;	/* Remote connection socket */
	int sock_tag;   /* Used to remove watch callback */
};

enum {
	CARNE_ERROR_NONE = 0,
	CARNE_ERROR_ILLEGAL_HOST = -100,
	CARNE_ERROR_NO_SOCKET,
	CARNE_ERROR_CONNECTION_FAILED,
};

