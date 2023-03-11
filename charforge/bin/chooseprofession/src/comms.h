
#define SERVER "oblivion.nash.id.au"

enum {
	/* Standard port to connect to server on */
	PORT_NUM = 8374,
};

typedef enum {
	RESPONSE_DATA = 27,
	RESPONSE_FILE,
} request_t;


struct state;

int comms_connect(struct state *state);
int send_request(const xmlChar *msg,int type,const xmlChar *reply,
		void (*callback)(int,xmlChar*,xmlNode *,void*),
		void *data);

