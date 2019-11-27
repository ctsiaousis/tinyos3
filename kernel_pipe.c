
#include "tinyos.h"
#include "kernel_pipe.h"
/*
file_ops reader_ops = {
  .Open = NULL,
  .Read = pipe_read,
  .Write = nothingConst,
  .Close = reader_Close
};


file_ops writer_ops = {
  .Open = NULL,
  .Read = nothing,
  .Write = pipe_write,
  .Close = writer_Close
};
*/
int nothing(void* this, char *buf, unsigned int size){
	return -1;
}

int nothingConst(void* this, const char *buf, unsigned int size){
	return -1;
}

int pipe_read(void* this, char *buf, unsigned int size){

	return -1;
}

int pipe_write(void* this, const char* buf, unsigned int size){

	return -1;
}

int reader_Close(void* streamobj){

	return -1;
}

int writer_Close(void* streamobj){
	
	return -1;
}

int sys_Pipe(pipe_t* pipe)
{
	return -1;
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
