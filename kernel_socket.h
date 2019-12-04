
#include "tinyos.h"
#include "util.h"
#include "kernel_dev.h"
#include "kernel_pipe.h"

typedef enum socket_state
{
	UNBOUND,
	PEER,
	LISTENER
}sockType;

typedef struct listener_socket
{
	rlnode request_queue;
	CondVar req;
}sockLis;

typedef struct peer_socket
{
	pipe_CB* readPipe;
	pipe_CB* writePipe;
	//sockType type; //perito 99%
	//ta kato isos theloun diaforetiko struct
}sockPee;

typedef struct socket_control_block
{
	sockType type;
	port_t port;


	union{
		sockLis listener;
		sockPee peer;
	};

}socketCB;

typedef struct queue_node
{
	Fid_t fid; //na tou anatheto timi kapos kapou
	socketCB* reqSock;
	rlnode node; 
  	CondVar cv;
  	int admitted;
}qNode;

//thread barrier gia sosto sugxronismo gia pollous purines
typedef struct barrier{
	int count;
	int epoch;
	CondVar reached;
}bar;


socketCB* PORT_MAP[MAX_PORT + 1] = { NULL };

int socket_read(void* this, char *buf, unsigned int size);
int socket_write(void* this, const char* buf, unsigned int size);
int socket_close(void* this);

