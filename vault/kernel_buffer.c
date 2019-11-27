
#include "kernel_buffer.h"
#include "kernel_pipe.h"
#include "tinyos.h"

/*
kodikas elafros allagmenos apo
http://www.csl.mtu.edu/cs4411.ck/www/NOTES/threads/buffer-2.html
*/

static int   Buf[BUF_SIZE];             /* the actual buffer        */
static int   in;                        /* next empty location      */
static int   out;                       /* next filled location     */
static int   count;                     /* no. of items in buffer   */

static Mutex  MonitorLock;            /* monitor lock             */

static CondVar   UntilNotFull;           /* wait until full cv       */
static CondVar   UntilNotEmpty;          /* wait until full cv       */

void  BufferInit(void)
{
     in = out = count = 0;

     MonitorLock = MUTEX_INIT;

     UntilNotFull = COND_INIT;
     UntilNotEmpty = COND_INIT;
}

int  GET(void)
{
     int  value;

     Mutex_Lock(&MonitorLock);          /* lock the monitor         */
          while (count == 0)            /* while nothing in buffer  */
               Cond_Wait(&MonitorLock, &UntilNotEmpty); /* then wait*/
          value = Buf[out];             /* retrieve an item         */
          out = (out + 1) % BUF_SIZE;   /* advance out pointer      */
          count--;                      /* decrease counter         */
          Cond_Signal(&UntilNotFull);   /* signal consumers         */
     Mutex_Unlock(&MonitorLock);        /* release monitor          */
     return  value;                     /* return retrieved vague   */
}

int  PUT(int  value)
{
     Mutex_Lock(&MonitorLock);          /* lock the buffer          */
          while (count == BUF_SIZE)     /* while buffer is full     */
               Cond_Wait(&MonitorLock, &UntilNotFull); /* then wait */
          Buf[in] = value;              /* add the value to buffer  */
          in = (in + 1) % BUF_SIZE;     /* advance in pointer       */
          count++;                      /* increase count           */
          Cond_Signal(&UntilNotEmpty);  /* signal producers         */
     Mutex_Unlock(&MonitorLock);        /* release the lock         */
     return  value;                     /* return the value         */
}