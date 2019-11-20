
#include "tinyos.h"

typedef struct pipe_control_block
{

}pipe_CB;

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