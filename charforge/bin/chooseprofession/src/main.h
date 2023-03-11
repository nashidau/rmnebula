
struct state {
	GtkWidget *MainWindow; /* Main window */
 
        int sock;       /* Remote connection socket */
        int sock_tag;   /* Used to remove watch callback */
};


extern GtkWidget *MainWindow;
extern struct state *state;
