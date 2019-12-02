#include "kernel_socket.h"
#include "tinyos.h"
#include "kernel_dev.h"
#include "kernel_streams.h"
#include "kernel_sched.h"
#include "kernel_cc.h"

file_ops socket_func = {
  .Open = NULL,
  .Read = socket_read,
  .Write = socket_write,
  .Close = socket_close
};



Fid_t sys_Socket(port_t port)
{
	/*fcb_reserv me ena (anti gia 2, pipe)
	sto fcb bazo to SCB kai to socket_func*/

	if(port < NOPORT || port > MAX_PORT)
  		return NOFILE;

	Fid_t fid[1];
	FCB* fcb[1];
	socketCB* cb = (socketCB*) xmalloc(sizeof(socketCB));
	int reservedFCB = FCB_reserve(1,fid,fcb);
	//an apotuxei to reserve return -1	
	if(!reservedFCB)
		return NOFILE;

	cb->port = port;
	cb->type = UNBOUND;

	fcb[0]->streamobj = cb;
  	fcb[0]->streamfunc = &socket_func;

  	if(PORT_MAP[port] != NULL){
  		return fid[0];
  	}
  	
  	PORT_MAP[port] = cb;

	return fid[0];
}

int sys_Listen(Fid_t sock)
{
	FCB* fcb = get_fcb(sock);
		//fprintf(stderr, "%s\n","geisu stel");
	if(fcb != NULL && fcb->streamfunc == &socket_func){
		socketCB* cb = fcb->streamobj;

		if(cb == NULL) 
			return -1;
		if(cb->port <= NOPORT || cb->port > MAX_PORT) 
			return -1;
		if((PORT_MAP[cb->port])->type == LISTENER) 
			return -1;
		if(cb->type != UNBOUND) 
			return -1;
		//if(PORT_MAP[cb->port] != NULL) return -1;
		
		cb->type = LISTENER;
		rlnode_init(&cb->listener.request_queue, NULL);
		cb->listener.req = COND_INIT;
		return 0;
	}
	return -1;
}


Fid_t sys_Accept(Fid_t lsock)
{
	FCB* fcb = get_fcb(lsock);
	if(fcb != NULL && fcb->streamfunc == &socket_func){
		socketCB* cb = fcb->streamobj;

		if(cb == NULL) return -1;
		if(cb->port <= NOPORT) return -1;
		if(cb->port > MAX_PORT) return -1;
		if((PORT_MAP[cb->port])->type != LISTENER) return -1;

		while(is_rlist_empty(&cb->listener.request_queue))
			kernel_wait(&cb->listener.req, SCHED_PIPE);
//apo do kai kato exoume ksupnisei pano sto reqCV tou listener
	//ara kanoume ton listener kai peer?
		Fid_t peerID = sys_Socket(cb->port);

		if(peerID == NOFILE) return -1;
//kanoume ton listener kai peer gia na epikoinonisei
		FCB* peerFCB = get_fcb(peerID);
		socketCB* peer = peerFCB->streamobj;

		rlnode* requestNode = rlist_pop_front(&cb->listener.request_queue);
		qNode* reqNode = requestNode->obj;
		Fid_t reqPeerID = reqNode->fid;
		socketCB* reqPeer = reqNode->reqSock;

		if(reqPeer == NULL || peer == NULL) return -1;

//exoume dimiourgisei ta 2 peers ora na baloume 2 pipes
	//1
		pipe_CB* pipe1 = (pipe_CB*) xmalloc(sizeof(pipe_CB));
		pipe1->pit.read = reqPeerID;
		pipe1->pit.write = peerID;
		pipe1->readPTR = 0;
		pipe1->writePTR = 0;
		pipe1->hasSpace = COND_INIT;
		pipe1->hasData = COND_INIT;
	//2
		pipe_CB* pipe2 = (pipe_CB*) xmalloc(sizeof(pipe_CB));
		pipe2->pit.read = peerID;
		pipe2->pit.write = reqPeerID;
		pipe2->readPTR = 0;
		pipe2->writePTR = 0;
		pipe2->hasSpace = COND_INIT;
		pipe2->hasData = COND_INIT;

//i enosi egine!! den exo idea an trexei xexex
		if(pipe1 != NULL && pipe2 != NULL){
			peer->peer.type = PEER;
			peer->peer.readPipe = pipe2;
			peer->peer.writePipe = pipe1;

			reqPeer->type = PEER;
			reqPeer->peer.readPipe = pipe1;
			reqPeer->peer.writePipe = pipe2;
		}

		reqNode->admitted = 1;
		kernel_signal(&reqNode->cv); //isos signal

		return peerID;
	}
	return -1;
}


int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	FCB* fcb = get_fcb(sock);
	int timedOut;

	if(fcb==NULL) return -1;
	if(fcb->streamfunc == NULL) return -1;
	if(port <= NOPORT || port > MAX_PORT) return -1;

	socketCB* peer = fcb->streamobj;
	socketCB* listener = PORT_MAP[port];
	

	if(peer->type != UNBOUND) return -1; //isos !=
	if(listener == NULL) return -1;
	if(listener->type != LISTENER) return -1;


	qNode* node = (qNode*) xmalloc(sizeof(qNode));
	node->reqSock = peer;
	node->fid = sock; //maybe yes maybe no
	rlnode_init(&node->node, node);
	node->admitted = 0;
	node->cv = COND_INIT;
	rlist_push_back(&(listener->listener.request_queue), &node->node);

	kernel_signal(&(listener->listener.req));

	//if(is_rlist_empty(&listener->listener.request_queue)){
	
	//}

	while(node->admitted == 0){
		timedOut = kernel_timedwait(&node->cv, SCHED_PIPE, timeout);
		if(!timedOut) 
			return -1;
	}
	free(node);
	node = NULL;
	return 0;
}


int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	return -1;
}

int socket_read(void* this, char *buf, unsigned int size)
{
	socketCB* cb = (socketCB*) this;

	if(cb->type == PEER && cb->peer.readPipe != NULL){
		return pipe_read(cb->peer.readPipe, buf, size);
	}else
		return NOFILE;
}

int socket_write(void* this, const char* buf, unsigned int size)
{
	socketCB* cb = (socketCB*) this;

	if(cb->type == PEER && cb->peer.writePipe != NULL){
		return pipe_write(cb->peer.writePipe, buf, size);
	}else
		return NOFILE;
}

int socket_close(void* this)
{
	if(this!=NULL){
		socketCB* cb = (socketCB*) this;

		if(cb->type == PEER){
			int r,w;
			r = reader_Close(cb->peer.readPipe); //0 an ola kalos
			w = writer_Close(cb->peer.writePipe); //0 an ola kalos
			if(r+w != 0)
				return -1;
		}

		if(cb->type == LISTENER) //an einai kai listener tautoxrona
			kernel_broadcast(&cb->listener.req);
		
		free(cb);
		cb = NULL;
		return 0;
	}
	return NOFILE;
}

/*
o listener kalei tin socket ki auti kalei tin
bind() pou deixnei to port tou listener basei
tou port table pou ulopoiisame.
epeita kaleite i
listen() kai brisketai se katastasi anamonis perimenontas
nees sindeseis meso tis accept().

o listener krataei ola ta requests kai ta eksupiretei seiriaka
otan sundethei kapoios akolouthei mia diadikasia write kai read.
ara dimiourgo tin socket_write() kai socket_read() pou aksiopoiei 
ton mixanismo ton pipes.
otan teleiosei h diadikasia o requester kalei tin close kai 
auti stelnei EOF ston listener etsi oste na kalesei ki autos close()
an kai mono an exei eksupiretisei ola ta requests!!!

ta sockets einai listener, an akouei
				 peer, an exei connection (mporei kai listener tautoxrona)
				 unbound, an den exei sundethei

Prepei na uparxei elegxos gia to type tou socket
Prepei na balo union sto SCB gia diaforetiki ulopoiisi
analoga me listener kai peer. to type dld. 
*/