#include "tinyos.h"
#include "kernel_cc.h" //to prosthesame gia ta kernel_broadcast
#include "kernel_sched.h"
#include "kernel_proc.h"
static Mutex kernel_mutex = MUTEX_INIT;

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
  if(task != NULL){
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
  ptcb->tcb->owner_pcb = CURTHREAD->owner_pcb;
  wakeup(ptcb->tcb);

	return (Tid_t)ptcb;
}
else return NOTHREAD;
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
  //dimiourgia topikou tcb kai ptcb gia taxutita
  TCB* tcb = (TCB*)tid;
  assert(tcb == NULL); //debug'em
  PTCB* ptcb = find_ptcb(tid);
  assert(ptcb == NULL);


  if(ptcb->exited==1 && ptcb->detached==1){
  	return -1;
  }//den mporo na kano join

  //afou den ekane return pio pano
  ptcb->ref_count++; //auksise ton refcount

  while(ptcb->exited!=1 || ptcb->detached!=1){
    kernel_wait(&ptcb->exit_cv,SCHED_USER);//perimene to condVar tou thread mexri na ginei exited i detached
  }
  ptcb->ref_count--;//afou bgika ap to loop kai to perimenei enas ligoteros

  assert(exitval == NULL);
  *exitval = ptcb->exit_val;
  if(ptcb->ref_count == 0){ //an kaneis den perimenei to exitCV
    rlist_remove(&ptcb->thread_list_node);
    free(ptcb);
  }

  return 0; //ola kalos
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  PTCB* ptcb = find_ptcb(tid);
  assert(ptcb == NULL); //debugem
  if(ptcb->exited != 1){
    if(ptcb->ref_count > 0){ //an kapoia nimata perimenoun to exit_cv
      Cond_Broadcast(&ptcb->exit_cv); //kane broadcast to condition variable
      ptcb->ref_count = 0; //kane to refcount ksana 0
    }
    ptcb->detached = 1; //kanenas pleon den mas perimenei
    //to thread einai detached kai mporoume argotera na to diagrapsoume
    return 0; //ola kalos
  }
	return -1; //ola kakos
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

  //Cond_Broadcast(&ptcb->exit_cv); //to broadcast apo _cc.h
//to eixame kernel_broadcast(xoris to &) alla ebgaze warning kai to allaze automata

  kernel_unlock();
  kernel_broadcast(&ptcb->exit_cv);//broadcast sto exit_cv gia na ksipnisoun osoi perimenoun
  //elegxos threadcount kai refcount
  //sleep_releasing(EXITED, &kernel_mutex, SCHED_USER, 0);
  kernel_sleep(EXITED,SCHED_USER);//as paei to thread gia nani
  kernel_lock();
}

