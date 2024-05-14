#define _XOPEN_SOURCE 500

#include "nthread-impl.h"

#include "rwlock.h"

struct rwlock {
  int r_count;
  int w_count;
  NthQueue *r_queue;
  NthQueue *w_queue;
};

nRWLock *nMakeRWLock() {
  nRWLock *lock = nMalloc(sizeof(nRWLock));
  lock->r_count = 0;
  lock->w_count = 0;
  lock->r_queue = nth_makeQueue();
  lock->w_queue = nth_makeQueue();
  return lock;
}

void nDestroyRWLock(nRWLock *rwl) {
  nth_destroyQueue(rwl->w_queue);
  nth_destroyQueue(rwl->r_queue);
  free(rwl);
}

int nEnterRead(nRWLock *rwl, int timeout) {
  START_CRITICAL;
  if(!(rwl->w_count) && (nth_emptyQueue(rwl->w_queue))){
    rwl->r_count++; 
  }else{
    nth_putBack(rwl->r_queue, nSelf());
    suspend(WAIT_RWLOCK);
    schedule();
  }
  END_CRITICAL;
  return 1;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
  START_CRITICAL;
  if(!(rwl->r_count) && !(rwl->w_count)){
    rwl->w_count++;
  }else{
    nth_putBack(rwl->w_queue, nSelf());
    suspend(WAIT_RWLOCK);
    schedule();
  }
  END_CRITICAL;
  return 1;
}

void nExitRead(nRWLock *rwl) {
  START_CRITICAL;
  rwl->r_count--;
  if(!(rwl->r_count) && !nth_emptyQueue(rwl->w_queue)){
    setReady(nth_getFront(rwl->w_queue));
    rwl->w_count++;
    schedule();
  }
  END_CRITICAL;
}

void nExitWrite(nRWLock *rwl) {
  START_CRITICAL;
  rwl->w_count--;
  if(!nth_emptyQueue(rwl->r_queue)){
    while(!nth_emptyQueue(rwl->r_queue)){
      setReady(nth_getFront(rwl->r_queue));
      rwl->r_count++;
      schedule();
    }
  }else if(!nth_emptyQueue(rwl->w_queue)){
    setReady(nth_getFront(rwl->w_queue));
    rwl->w_count++;
    schedule();
  }
  END_CRITICAL;
}