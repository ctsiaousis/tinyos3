
#include "tinyos.h"
#include "kernel_pipe.h"


int nothing(void* this, char *buf, unsigned int size){
	return -1;
}

int nothingConst(void* this, const char *buf, unsigned int size){
	return -1;
}

int pipe_read(void* this, char *buf, unsigned int size){
	pipe_CB* pipeCB = (pipe_CB*) this;

	//if invalid pipe_cb return -1
	if(pipeCB == NULL){
		return -1;
	}

	//if reader is closed return -1

	int count = 0;
	while(count < size){

	}
	return count;
}

int pipe_write(void* this, const char* buf, unsigned int size){
	pipe_CB* pipeCB = (pipe_CB*) this;

	//if invalid pipe_cb return -1
	if(pipeCB == NULL){
		return -1;
	}

	//if reader is closed return -1

	int count = 0;
	while(count < size){

	}
	return count;
}

int reader_Close(void* streamobj){

	return -1;
}

int writer_Close(void* streamobj){
	
	return -1;
}

int sys_Pipe(pipe_t* pipe)
{
	//topikes metablites gia fid fcb kai pipeControlBlock
	Fid_t fid[1];
	FCB* fcb[1];
	pipe_CB* cb = (pipe_CB*) xmalloc(sizeof(pipe_CB));

	//kane reserve 2 FCBs
	int reservedFCB = FCB_reserve(2,fid,fcb);
	//an apotuxei to reserve return -1	
	if(!reservedFCB){
		return -1;	
	}
	/*kane tous reader kai writer tou pipe_t na 
	deixnoun ta fid pou kaname reserve*/
	pipe->read = fid[0];
	pipe->write = fid[1];

	/*arxikopoiisi ton periexomenon tou pipeCB*/
	cb->pit = *pipe;
	cb->readPTR = 0;
	cb->writePTR = 0;
	cb->hasSpace = COND_INIT;
	cb->hasData = COND_INIT;

	/*streamobj kai func gia ta fcb*/
	fcb[0]->streamobj = cb;
	fcb[1]->streamobj = cb;
	fcb[0]->streamfunc = &reader_ops;
	fcb[1]->streamfunc = &reader_ops;
	return 0;
}

/*
I proti douleia tis pipe einai na kanei ena
FCB_reserve(Z,Fid_t *, FCB**)
na dimiourgisoume diladi to path ap to pcb
mexri to file table.
Theloume ena fidT gia read ki ena gia write.
An ola kalos epistrefei 0 allios -1.
Episis prepei na dimiourgoume ena pipe_CB to
opoio tha exei mesa enan buffer(char*)
SIMEIOSI o buffer einai mia kukliki lista (bounded buffer).
ki ena BUF_SIZE=8192 kai FCB* r,w kai 
uint readPTR,writePTR kai
CondVar hasSpace, hasData.
o reader koimatai pano sto hasData kai o writer pano stin hasSpace.

Episis edo ulopoiisi ton pipe_read(pipe_CB,buf,size) kai 
pipe_write(pipe_CB,buf,size) kathos kai
close_reader kai close_writer.
*/
