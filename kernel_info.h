#include "kernel_cc.h"
#include "kernel_proc.h"
#include "kernel_pipe.h"
#include "kernel_streams.h"

int info_Read(void* this, char *buf, unsigned int size);
int info_Close(void* this);

file_ops info_ops = {
  .Open = NULL,
  .Read = info_Read,
  .Write = nothingConst,
  .Close = info_Close
};
