#include "kernel_cc.h"
#include "kernel_proc.h"
#include "kernel_pipe.h"
#include "kernel_streams.h"

int info_Read(void* this, char *buf, unsigned int size);
int info_Close(void* this);

typedef struct procinfo_cb{
	procinfo curinfo;	//tinyos.h:716
	int cursor;			//apo 0 eos MAX_PROC-1
}infoCB;

file_ops info_ops = {
  .Open = NULL,
  .Read = info_Read,
  .Write = nothingConst,
  .Close = info_Close
};
