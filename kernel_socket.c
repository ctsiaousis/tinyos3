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

  	//dimiourgia pinaka mias thesis gia fcb kai fid
	Fid_t fid[1];
	FCB* fcb[1];
	//dunamiki desmeusi tou socket
	socketCB* cb = (socketCB*) xmalloc(sizeof(socketCB));
	//pare fcb (peirazei kai to fid)
	int reservedFCB = FCB_reserve(1,fid,fcb);
	//an apotuxei to reserve return -1	
	if(!reservedFCB)
		return NOFILE;

	//kane to port na deixnei to orisma
	cb->port = port;
	//kane ton tupo tou socket unbound
	cb->type = UNBOUND;

	//sto paidio antikeimenou tou fcb bale to socket
	fcb[0]->streamobj = cb;
	//kai sto pedio sunartiseon tou fcb bale to socketfunc
  	fcb[0]->streamfunc = &socket_func;

  	//an uparxei idi kati sto port
  	if(PORT_MAP[port] != NULL){
  		//epestrepse to fid
  		return fid[0];
  	}
  	//allios bale to socket ston pinaka
  	PORT_MAP[port] = cb;
  	//kai epestrepse to fid
	return fid[0];
}

int sys_Listen(Fid_t sock)
{
	//pare to fcb pou deixnei to fid
	FCB* fcb = get_fcb(sock);
		//fprintf(stderr, "%s\n","geisu stel");

	//an to fcb uparxei ki exei socket operants
	if(fcb != NULL && fcb->streamfunc == &socket_func){
		//pare to socket ap to antikeimeno pou deixnei to fcb
		socketCB* cb = fcb->streamobj;

		if(cb == NULL) 
			return -1; //den uparxei to socket
		if(cb->port <= NOPORT || cb->port > MAX_PORT) 
			return -1; //to port den einai mesa sto range
		if((PORT_MAP[cb->port])->type == LISTENER) 
			return -1; //den uparxei listener sto port pou akouo
		if(cb->type != UNBOUND) 
			return -1; //den eimai unbound		

		//kane to tupe listener
		cb->type = LISTENER;
		//arxikopoiise to condVar pou ksupna otan exo kainourio requestNode stin lista
		cb->listener.req = COND_INIT;
		//arxikopoiise tin oura sto unionTypeListener
		rlnode_init(&(cb->listener.request_queue), NULL);
		return 0;						//ola kalos
	}
	return -1;							//ola kakos
}


Fid_t sys_Accept(Fid_t lsock)
{
	//pare to fcb pou deixnei to fid
	FCB* fcb = get_fcb(lsock);
	//elegkse an einai valid
	if(fcb != NULL && fcb->streamfunc == &socket_func){
		//pare to socket apo to antikeimeno tou fcb
		socketCB* cb = fcb->streamobj;

		if(cb == NULL) return -1; 								//den uparxei to socket
		if(cb->port <= NOPORT) return -1;						//den einai to port
		if(cb->port > MAX_PORT) return -1;						//entos tou eurous
		if(cb->type == PEER) return -1;							//exei idi sundesi
		if((PORT_MAP[cb->port])->type != LISTENER) return -1;	//den deixnei se valid listener

		socketCB* l = PORT_MAP[cb->port];
		//oso i requestList tou L einai adeia
		while(is_rlist_empty(&(PORT_MAP[cb->port]->listener.request_queue))){
			kernel_wait(&(PORT_MAP[cb->port]->listener.req), SCHED_USER);	//perimene na se ksupnisei kapoios request
			if(PORT_MAP[cb->port] == NULL)
				return -1;
		}

	//apo do kai kato exoume ksupnisei pano sto reqCV tou listener
		//dimiourgo ena peer gia na enoso me ton requester
		Fid_t peerID = sys_Socket(cb->port);
		if(peerID == NOFILE) return -1;

		//pairno to fcb gia to kainourio socket
		FCB* peerFCB = get_fcb(peerID);
		//kai pairno to kainourio socket ap to antikeimeno pou deixnei to fcb
		socketCB* peer = peerFCB->streamobj;

		//pairno ton proto node ap to reqList tou L
		rlnode* requestNode = rlist_pop_front(&(l->listener.request_queue));
		//pairno to struct tupou qNode ap to rlnode
		qNode* reqNode = requestNode->obj;
		//pairno to peer apo auto
		socketCB* reqPeer = reqNode->reqSock;
		//kai to fid tou gia na sundeso sosta ta pipes xexe
		Fid_t reqPeerID = reqNode->fid;

		if(reqPeer == NULL || peer == NULL) return -1; //an fao tapa, dino tapa

//exoume dimiourgisei ta 2 peers ora na baloume 2 pipes
	//1
		pipe_CB* pipe1 = (pipe_CB*) xmalloc(sizeof(pipe_CB));
		pipe1->pit.read = reqPeerID;		//den ftaiei to pit gia to seg
		pipe1->pit.write = peerID;			//testarismeno me 2 malloc
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

//i enosi egine!!
		if(pipe1 != NULL && pipe2 != NULL){
			peer->type = PEER;
			peer->peer.readPipe = pipe2;
			peer->peer.writePipe = pipe1;

			reqPeer->type = PEER;
			reqPeer->peer.readPipe = pipe1;
			reqPeer->peer.writePipe = pipe2;
		}

	//dilono ston kombo oti eksupiretithike, basei autou kanei free ap to reqnode i connect
		reqNode->admitted = 1;
		//ksupna requester, s eftiaksa
		kernel_signal(&(reqNode->cv)); //isos signal

		return peerID;		//ola kalos
	}
	return -1;				//ola kakos
}


int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	//pare fcb ap to fid
	FCB* fcb = get_fcb(sock);
	int timedOut; //den mporoume na perimenoume gia panta tin sundesi

	if(fcb==NULL) return -1;							//akuro fcb
	if(fcb->streamfunc != &socket_func) return -1;		//akuro pedio sunartisis
	if(port <= NOPORT || port > MAX_PORT) return -1;	//akuro port

	//pare socket apo fcb
	socketCB* peer = fcb->streamobj;
	//pare Lsocket ap to port
	socketCB* listener = PORT_MAP[port];
	

	if(peer->type != UNBOUND) return -1; 		//einai idi se xrisi
	if(listener == NULL) return -1;				//den uparxei L sto port
	if(listener->type != LISTENER) return -1;	//uparxei socket alla oxi listener sto port

	//dunamiki desmeusi tis domis reqNode
	qNode* node = (qNode*) xmalloc(sizeof(qNode));
	//to socket tou reqNode deixnei to peer
	node->reqSock = peer;
	//to xreiazomaste gia na kseroume to fid tou requester enonontas ta pies
	node->fid = sock; 
	//initialize tou kombou
	rlnode_init(&node->node, node);
	//arxikopoiisi sto 0 afou den to xei strosei o listener
	node->admitted = 0;
	//initialize tou condition variable
	node->cv = COND_INIT;
	//bale to requestNode stin lista tou listener
	rlist_push_back(&(PORT_MAP[port]->listener.request_queue), &node->node);

	//ksupna ton listener etoimos na enotho!!!11!!!!
	kernel_broadcast(&(PORT_MAP[port]->listener.req));
	//oso den mou dinei simasia o xazos
	while(node->admitted == 0){
		//nani nani to pedi mas kanei mexri time out
		timedOut = kernel_timedwait(&node->cv, SCHED_PIPE, timeout);
		if(!timedOut) 
			return -1; //telos xronou
	}

	//yas me estrose! den xreiazetai o kombos giati exo enosi
	node = NULL;
	//free(node);
	return 0;		//ola kalos =)
}


int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	FCB* fcb = get_fcb(sock);
	if(fcb==NULL) return -1;							//akuro fcb
	if(fcb->streamfunc != &socket_func) return -1;		//akuro pedio sunartisis
	if(how < 1 || how > 3) return -1;					//akuro mode

	socketCB* cb = fcb->streamobj;

	if(cb != NULL && cb->type == PEER){ //an uparxei socket ki exei sundesi
		int r,w;
		switch(how) //case pano sto shutdown mode
		{
		case SHUTDOWN_READ: 		//an exo kleisimo read
			return reader_Close(cb->peer.readPipe);	//kleino to readPipe tou socket
			break;
		case SHUTDOWN_WRITE:
			return writer_Close(cb->peer.writePipe); //kleino to writePipe tou socket
			break;
		case SHUTDOWN_BOTH:		//kleino kai ta duo
			r = reader_Close(cb->peer.readPipe);
			w = writer_Close(cb->peer.writePipe);
			if((r+w) == 0)	//me elegxo an egine sosta
				return 0;		//ola kalos
			else
				return -1;		//ola kakos
			break;
		default:		//kapos perase lathos timi!
			fprintf(stderr, "%s\n","Unknown shutdown_mode.\n");
			return -1;
		}
	}
	return -1;	//ola kakos
}

int socket_read(void* this, char *buf, unsigned int size)
{
	//par to control block
	socketCB* cb = (socketCB*) this;

	//an ontos exei ontos sundesi kai ontos anoixto read
	if(cb->type == PEER && cb->peer.readPipe != NULL){
		//pasare tin pipeRead pou exoume sta pipes
		return pipe_read(cb->peer.readPipe, buf, size); //
	}else
		//alios dose prama
		return NOFILE;
}

int socket_write(void* this, const char* buf, unsigned int size)
{
	//par to control block
	socketCB* cb = (socketCB*) this;

	//an ontos exei ontos sundesi kai ontos anoixto write
	if(cb->type == PEER && cb->peer.writePipe != NULL){
		//pasare tin pipeWrite pou exoume sta pipes
		return pipe_write(cb->peer.writePipe, buf, size);
	}else
		//alios dose prama
		return NOFILE;
}

int socket_close(void* this)
{
	//an mou dineis valid object
	if(this!=NULL){
		//par to socket
		socketCB* cb = (socketCB*) this;

	//an exei sundesi
		if(cb->type == PEER){
			int r,w;
			r = reader_Close(cb->peer.readPipe); //0 an ola kalos
			w = writer_Close(cb->peer.writePipe); //0 an ola kalos
			if(r+w != 0) //0+0=0 an den kano lathos!?
		//den petuxan oi close ton pipes ara oute i close tou socket
				return -1;
		}

		if(cb->type == LISTENER){ //an einai kai listener
			PORT_MAP[cb->port] = NULL; 			//vgale ton L ap ton pinaka ton socket
			kernel_broadcast(&cb->listener.req); //ksupna ton listener
		}
		//free(cb);
		cb = NULL;
		return 0;		//ola kalos
	}
	return NOFILE;		//ola kakos
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