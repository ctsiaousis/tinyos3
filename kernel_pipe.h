
#include "bios.h"
#include "tinyos.h"
#include "util.h"


typedef struct pipe_control_block
{
	pipe_t pipa;
	char* buffer;
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
