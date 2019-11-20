
#include "bios.h"
#include "tinyos.h"
#include "util.h"


typedef enum socket_state
{
	UNBOUND,
	PEER,
	LISTENER
}sockTYPE;

typedef struct listener_socket
{
	rlnode request_queue;
	CondVar req;
}sockLis;

typedef struct peer_socket
{
	pipe_CB* readPipe;
	pipe_CB* writePipe;
	sockTYPE peer;
}

typedef struct socket_control_block
{
	sockType type;
}SCB;