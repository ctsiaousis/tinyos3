
#include "bios.h"
#include "tinyos.h"
#include "util.h"
#include "kernel_dev.h"
#include "kernel_streams.h"
#include "kernel_cc.h"

void  BufferInit(void);       /* initializes the monitor  */
int   GET(void);              /* get an item from buffer  */
int   PUT(int  value);        /* add an item to buffer    */