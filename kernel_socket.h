
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
	sockType peer;
	//ta kato isos theloun diaforetiko struct
	rlnode node; 
  	CondVar cv;
  	int admitted;
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




socketCB* PORT_MAP[MAX_PORT + 1] = { NULL };

int socket_read(void* this, char *buf, unsigned int size);
int socket_write(void* this, const char* buf, unsigned int size);
int socket_close(void* this);

