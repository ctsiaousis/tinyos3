#include "tinyos.h"
#include "kernel_cc.h" //to prosthesame gia ta kernel_broadcast
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_streams.h"

//se poio ptcb anoikei to thread? (basei tou tid)
PTCB* find_ptcb(Tid_t tid){
//pairnoume to proto stoixeio tis listas ptcb apo to pcb tou CURTHREAD
  
  PTCB* ptcb = (PTCB*)tid;
  if(rlist_find(&CURPROC->thread_list, ptcb, NULL))
    return ptcb;
  else
    return NULL;
  //ulopoiisi me rlnode* rlist_find(rlnode* List, void* key, rlnode* fail);
}

//sunartisi pou kalei i spawn thread
//auti i func den stamataei pote!!
//allazei omos periexomeno meso tis yield kai
//tou context switching
void initialize_thread(){
//euresi tou current ptcb meso tou current tcb
  //PTCB* ptcb = find_ptcb((Tid_t)CURTHREAD);
  PTCB* ptcb = CURTHREAD->ptcb;
  assert(ptcb != NULL);
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
  TCB* tcb = CURPROC->main_thread;


  /* Initialize the other attributes */
  ptcb->ref_count = 0; //arxikopoiisi tou refCount sto 0
  ptcb->tcb = tcb; //to ptcb deixnei to tcb
  ptcb->task = task;
  ptcb->argl = argl;
  ptcb->args = (args==NULL)?NULL:args;
  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->exit_val = CURPROC->exitval;
  ptcb->exit_cv = COND_INIT;


  //auksano ton counter tou pcb, afou tou prostheto ena tcb
  CURPROC->thread_count++;

  CURTHREAD->ptcb = ptcb; //tcb deixnei ptcb

  //sundeseis ton pcb, ptcb kai tcb
  TCB *new_tcb = spawn_thread(CURPROC, initialize_thread); //na kanoume tin sunartisi
  //assert(new_tcb!=NULL);
  new_tcb->ptcb = ptcb; //to tcb deixnei to ptcb
  ptcb->tcb = new_tcb;
  ptcb->tcb->owner_pcb = CURTHREAD->owner_pcb;

  //initialize ton kombo tis listas ptcb
  rlnode_init(&ptcb->thread_list_node, ptcb); //o header ton ptcbs
  rlist_push_back(&CURPROC->thread_list, &ptcb->thread_list_node); //pcb deixnei ptcb

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
  
  //PTCB* ptcb = find_ptcb(tid);  //rlist_find();
  PTCB* ptcb = (PTCB*)tid;
  assert(ptcb != NULL);

//DEN MPORO NA BALO TO TID_T TOU EAUTOU MOU
  if((Tid_t)CURTHREAD==tid ||ptcb == NULL){
    return -1;
  }

  if(ptcb->exited==1 && ptcb->detached==1){
  	return -1;
  }//den mporo na kano join

  //afou den ekane return pio pano
  ptcb->ref_count++; //auksise ton refcount
  //assert(ptcb->exited==0);
  //assert(ptcb->detached==0);
  //assert((ptcb->exit_cv) == COND_INIT);
  while(ptcb->exited!=1 && ptcb->detached!=1){
    //kernel_unlock();
    //assert(1==2);
    kernel_wait(&ptcb->exit_cv,SCHED_USER);//perimene to condVar tou thread mexri na ginei exited i detached
    //assert(2==3);
    //kernel_lock();
  }
  ptcb->ref_count--;//afou bgika ap to loop kai to perimenei enas ligoteros

  //assert(exitval == NULL);
  if(exitval!=NULL){
  exitval = &ptcb->exit_val;
  }
  else{
    exitval=NULL;
  }

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
  //assert(ptcb == NULL); //debugem
  if(ptcb->exited != 1){
    if(ptcb->ref_count > 0){ //an kapoia nimata perimenoun to exit_cv
      kernel_broadcast(&ptcb->exit_cv); //kane broadcast to condition variable
      ptcb->ref_count = 0; //kane to refcount ksana 0
    }
    ptcb->detached = 1; //kanenas pleon den mas perimenei
    //to thread einai detached kai mporoume argotera na to diagrapsoume
    TCB* tcb = ptcb->tcb;
    ptcb->exit_val = tcb->owner_pcb->exitval;
    free(ptcb->tcb);
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
  PTCB* ptcb = CURTHREAD->ptcb;

  
//initialize ton exit metabliton tou ptcb
  ptcb->exited = 1;
  ptcb->exit_val = exitval;
  kernel_broadcast(&ptcb->exit_cv);//broadcast sto exit_cv gia na ksipnisoun osoi perimenoun
  //elegxos threadcount kai refcount
  if(ptcb->ref_count == 0)
  rlist_remove(&ptcb->thread_list_node);
  
//an thread count == 0 katharizo kai enimerono to PTCB opos tin sys_exit
  if(CURPROC->thread_count == 0){
    PCB *curproc = CURPROC;  /* cache for efficiency */

  /* Do all the other cleanup we want here, close files etc. */
  if(curproc->args) {
    free(curproc->args);
    curproc->args = NULL;
  }

  /* Clean up FIDT */
  for(int i=0;i<MAX_FILEID;i++) {
    if(curproc->FIDT[i] != NULL) {
      FCB_decref(curproc->FIDT[i]);
      curproc->FIDT[i] = NULL;
    }
  }

  /* Reparent any children of the exiting process to the 
     initial task */
  PCB* initpcb = get_pcb(1);
  while(!is_rlist_empty(& curproc->children_list)) {
    rlnode* child = rlist_pop_front(& curproc->children_list);
    child->pcb->parent = initpcb;
    rlist_push_front(& initpcb->children_list, child);
  }

  /* Add exited children to the initial task's exited list 
     and signal the initial task */
  if(!is_rlist_empty(& curproc->exited_list)) {
    rlist_append(& initpcb->exited_list, &curproc->exited_list);
    kernel_broadcast(& initpcb->child_exit);
  }

  /* Put me into my parent's exited list */
  if(curproc->parent != NULL) {   /* Maybe this is init */
    rlist_push_front(& curproc->parent->exited_list, &curproc->exited_node);
    kernel_broadcast(& curproc->parent->child_exit);
  }

  /* Disconnect my main_thread */
  curproc->main_thread = NULL;

  /* Now, mark the process as exited. */
  curproc->pstate = ZOMBIE;
  }
  CURPROC->exitval=exitval;
  CURPROC->thread_count--;
  //kernel_unlock();
  kernel_sleep(EXITED,SCHED_USER);//as paei to thread gia nani
  //kernel_lock();
}

