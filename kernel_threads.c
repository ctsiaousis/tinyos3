
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"

//se poio ptcb anoikei to thread? (basei tou tid)
PTCB* find_ptcb(Tid_t tid){
//pairnoume to proto stoixeio tis listas ptcb apo to pcb tou CURTHREAD
  rlnode* test = CURTHREAD->owner_pcb->thread_list.next;
  while(test != &(CURTHREAD->owner_pcb->thread_list)){
    if((Tid_t)(test->ptcb->tcb) == tid){
      return test->ptcb;
    }else{
      test=test->next;
    }
  }
  return NULL;
}

//sunartisi pou kalei i spawn thread
//auti i func den stamataei pote!!
//allazei omos periexomeno meso tis yield kai
//tou context switching
void initialize_thread(){
//euresi tou current ptcb meso tou current tcb
  PTCB* ptcb = find_ptcb((Tid_t)CURTHREAD);
  //arxikopoiisi task,argl,args
  Task task = ptcb->task;
  int argl = ptcb->argl;
  void* args = ptcb->args;
  int exitval = task(argl, args);

  ThreadExit(exitval);
}
/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{
  //to megethos tou allocation prepei na nai pol/sio tou PTCB size
  PTCB *ptcb = (PTCB *)xmalloc(sizeof(PTCB));
  //gia taxutita kanoume ena instance tou pcb
  TCB* tcb = CURTHREAD;

  /* Set the owner */
  //ptcb->owner_pcb = pcb;

  /* Initialize the other attributes */
  ptcb->ref_count = 0; //arxikopoiisi tou refCount sto 0
  ptcb->tcb = tcb;
  ptcb->task = task;
  ptcb->argl = argl;
  ptcb->args = args;
  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->exit_val = tcb->owner_pcb->exitval;
  ptcb->exit_cv = COND_INIT;

  //initialize ton kombo tis listas ptcb
  rlnode_init(&ptcb->thread_list_node, ptcb);
  rlist_push_back(&CURTHREAD->owner_pcb->thread_list, &ptcb->thread_list_node);
  //auksano ton counter tou pcb, afou tou prostheto ena tcb
  tcb->owner_pcb->thread_count++;

  //sundeseis ton pcb, ptcb kai tcb
  TCB *new_tcb = spawn_thread(CURTHREAD->owner_pcb, initialize_thread); //na kanoume tin sunartisi
  ptcb->tcb = new_tcb;
  //ptcb->tcb->owner_pcb
  wakeup(ptcb->tcb);

	return (Tid_t)ptcb;
}

/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) CURTHREAD;
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
	return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
	return -1;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{
  //euresi tou current ptcb meso tou current tcb
  PTCB* ptcb = find_ptcb((Tid_t)CURTHREAD);
//initialize ton exit metabliton tou ptcb
  ptcb->exited = 1;
  ptcb->exit_val = exitval;

  Cond_Broadcast(&ptcb->exit_cv); //to broadcast apo _cc.h
//to eixame kernel_broadcast(xoris to &) alla ebgaze warning kai to allaze automata
}

