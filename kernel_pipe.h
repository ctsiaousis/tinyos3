
#include "bios.h"
#include "tinyos.h"
#include "util.h"
#include "kernel_dev.h"
#include "kernel_streams.h"
#include "kernel_cc.h"

#define BUF_SIZE 8192

typedef struct pipe_control_block
{
	pipe_t pit;
	char buffer[BUF_SIZE];
	uint readPTR;
	uint writePTR;
	CondVar hasSpace;
	CondVar hasData;
}pipe_CB;

int nothing(void* this, char *buf, unsigned int size);

int nothingConst(void* this, const char *buf, unsigned int size);

int pipe_read(void* this, char *buf, unsigned int size);

int pipe_write(void* this, const char* buf, unsigned int size);

int reader_Close(void* streamobj);

int writer_Close(void* streamobj);

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
