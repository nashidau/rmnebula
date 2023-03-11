enum {
	/* Stabdard oirt to connect to server on */
	PORT_NUM = 8374,
};
#define SERVER "localhost"

struct state;

int comms_connect(struct state *state);
