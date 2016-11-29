#include "threads.h"
#include "alloc.h"
#include "ioport.h"
#include "ints.h"
//#include <stdatomic.h>

struct StackFrame {
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t rbx;
  uint64_t rbp;
  uint64_t flags;
  uint64_t retAddr;
} __attribute__((packed));

extern void asm_call_thread_main();
extern void asm_switch_threads (void** oldThreadStackPointer, void* newThreadStackPointer);


static struct Mutex __threadMutex;
struct ThreadInfo* currentThread;
static struct ThreadInfo kernelThread;
static struct ThreadInfo* threads[MAX_THREAD_COUNT];
static struct ThreadInfo* threadQueue[MAX_THREAD_COUNT + 1];
static int queueEnd, queuePos;
static struct Notifyer kernelNotifyer;

//Other functions


void __cantSwitchThread() {
    currentThread -> canSwitchThread = 0;
}

void __canSwitchThread() {
    currentThread -> canSwitchThread = 1;
    //if (currentThread -> wantSwitchThread) {
    //    currentThread -> wantSwitchThread = 0;
    //    __asm__ volatile("": : :"memory");
    //    switchThread();
    //}
}

void __addWaitor(struct Notifyer*);
void callThreadMain(void (*threadMain)(void*), void* arg) {
  //out8(0x20, 1 << 5); //Master EOI
  //out8(0xA0, 1 << 5); //Slave EOI
  threadMain(arg);
  finishThread(currentThread);
  switchThread();
  while(1);
}

//Work with thread queue
void __addThreadToQueue(struct ThreadInfo* thread) {
    threadQueue[queueEnd] = thread;
    queueEnd = (queueEnd + 1) % MAX_THREAD_COUNT;
}

struct ThreadInfo* __getNextThread() {
    struct ThreadInfo* thread = threadQueue[queuePos];
    queuePos = (queuePos + 1) % MAX_THREAD_COUNT;
    if (thread -> threadState != THREAD_STATE_FINISHED) {
        __addThreadToQueue(thread);
    }
    return thread;
}

//Init function

void initThreads() {
    currentThread = &kernelThread;
    currentThread -> id = 0;
    currentThread -> stackPtr = 0;
    currentThread -> disabledInts = 0;
    currentThread -> canSwitchThread = 1;
    currentThread -> wantSwitchThread = 0;
    __canSwitchThread();
    currentThread -> joinedThreadsNotifyer = &kernelNotifyer;
    currentThread -> threadState = THREAD_STATE_RUNNING;
    threadQueue[0] = currentThread;
    threads[0] = currentThread;
    queueEnd = 1;
    queuePos = 0;
}

//Thread


struct ThreadInfo* newThread(void (*threadMain)(void*), void* arg) {
  lock(&__threadMutex); __cantSwitchThread();
  
  struct ThreadInfo *newThread = (struct ThreadInfo*) mem_alloc(sizeof(struct ThreadInfo));
  newThread->id = 0;
  for (int freeID = 0; threads[freeID] != 0; freeID++) {
    newThread -> id = freeID + 1;
  }
  printf("New thread id = %d\n", newThread->id);
  threads[newThread -> id] = newThread;
  newThread -> disabledInts = 0;
  newThread -> canSwitchThread = 1;
  newThread -> wantSwitchThread = 0;
  newThread -> joinedThreadsNotifyer = newNotifyer();
  void* stack = mem_alloc(STACK_SIZE);
  newThread->stackPtr = (void*)((uintptr_t)stack + STACK_SIZE - sizeof(struct StackFrame));
  
  
  struct StackFrame *initialFrame = newThread->stackPtr;
  
  initialFrame->r15 = (uint64_t)threadMain;
  initialFrame->r14 = (uint64_t)arg;
  initialFrame->r13 = 0;
  initialFrame->r12 = 0;
  initialFrame->rbx = 0;
  initialFrame->rbp = 0;
  initialFrame->retAddr = (uint64_t)(&asm_call_thread_main);
 
  __canSwitchThread(); unlock(&__threadMutex); 
  return newThread;
}

void startThread(struct ThreadInfo* thread) {
    lock(&__threadMutex); __cantSwitchThread();
    thread -> threadState = THREAD_STATE_RUNNABLE;
    __addThreadToQueue(thread);
    if (queueEnd < queuePos) {
        printf("Queue size %d\n", (MAX_THREAD_COUNT - queuePos) + queueEnd);
    } else {
        printf("Queue size %d\n", queueEnd - queuePos);
    }
    for (int i = queuePos; i != queueEnd; i = (i + 1) % MAX_THREAD_COUNT) {
        printf("Thread %d\n", threadQueue[i]->id);
    }
    __canSwitchThread(); unlock(&__threadMutex);
}
 
void finishThread(struct ThreadInfo* thread) {
    lock(&__threadMutex); __cantSwitchThread();
    thread -> threadState = THREAD_STATE_FINISHED;
    notifyAll(thread -> joinedThreadsNotifyer);
    __canSwitchThread(); unlock(&__threadMutex);
}

void switchThread() {
    //printf("Start switchThread\n");
    if (currentThread -> canSwitchThread) { //We cant switch thread, which make some operations with threads.
        lock(&__threadMutex); __cantSwitchThread();
        if (currentThread -> threadState == THREAD_STATE_RUNNING) {
            currentThread -> threadState = THREAD_STATE_RUNNABLE; //current Thread already at the end of queue
        }
        int notFound = 1;
        struct ThreadInfo* thread;
        while (notFound) {
            thread = __getNextThread();
            if (thread -> threadState == THREAD_STATE_RUNNABLE) {
                notFound = 0;
            }
            if (thread -> threadState == THREAD_STATE_WAIT_MUTEX && !isLocked(thread -> waitFor)) {
                notFound = 0;
                thread -> threadState = THREAD_STATE_RUNNABLE;
            }
        }
        //printf("Found new thread\n");
        thread -> threadState = THREAD_STATE_RUNNING;
        if (currentThread != thread) {
            //printf("Threads are not the same\n");
            struct ThreadInfo* cur = currentThread;
            __canSwitchThread(); unlock(&__threadMutex);
            currentThread = thread;
            asm_switch_threads(&(cur->stackPtr), thread -> stackPtr);
            if (cur->disabledInts) { //If we came from lockMutex, we have disabled ints. We need to enable them
                cur->disabledInts = 0;
                __asm__ volatile("": : :"memory");
                enable_ints();
            }
            printf("Thread %d switched to Thread %d\n", cur->id, thread->id);
            return;
            //printf("Finish switch threads\n");
        } else {
            //printf("Threads are the same\n");
        }
        __canSwitchThread(); unlock(&__threadMutex);
    }
}
void joinThread(struct ThreadInfo* thread) {
    lock(&__threadMutex); __cantSwitchThread();
    if (thread -> threadState != THREAD_STATE_FINISHED) {
        __addWaitor(thread -> joinedThreadsNotifyer);
        __canSwitchThread(); unlock(&__threadMutex);
        switchThread();
        return;
    }
    __canSwitchThread(); unlock(&__threadMutex);
}


//Mutex

void initiateMutex(struct Mutex* mutex) {
    //for (int i = 0; i < MAX_THREAD_COUNT; i++) {
    //    mutex -> select[i] = 0;
    //    mutex -> ticket[i] = 0;
    //}
    mutex -> isLocked = 0;
}

struct Mutex* newMutex() {
    lock(&__threadMutex); __cantSwitchThread();
    struct Mutex* mutex = (struct Mutex*) mem_alloc(sizeof(struct Mutex));
    initiateMutex(mutex); 
    __canSwitchThread(); unlock(&__threadMutex);
    return mutex;
}

void lock(struct Mutex* m) {
    disable_ints();
    currentThread->canSwitchThread = 0;
    enable_ints();
    if (isLocked(m)) {
        currentThread -> threadState = THREAD_STATE_WAIT_MUTEX;
        currentThread -> waitFor = m;
        currentThread -> disabledInts = 1;
        __asm__ volatile("": : :"memory");
        switchThread(); //Wait for unlock
        disable_ints(); //After turning back we have enable ints.
    }
    m -> isLocked = 1;
    enable_ints();
    //I want to wite MCS, but people said, that I can't use <stdatomic.h>
 /*  __asm__ volatile("": : :"memory");
    currentThread -> waitingFor = m; //если мы ждем какого-то mutex, то это он.
    __asm__ volatile("": : :"memory");
    struct __MutexNode* m = &(currentThread -> mutexNode); 
    atomic_store_explicit(&self->next, 0, memory_order_relaxed);
    atomic_store_explicit(&self->wait, 0, memory_order_relaxed);
    struct node* tail = atomic_exchange_explicit(&lock->tail, self, memory_order_acq_rel);
    if (!tail) {
        m -> isLocked = 1;
        return;
    }
    atomic_store_explicit(&tail->next, self, memory_order_relaxed);
    while (!atomic_load_explicit(&self->wait, memory_order_acquire));
    //Заменить это на падение в сон и переключение на другой поток.
    __asm__ volatile("": : :"memory");
    m -> isLocked = 1;
    __asm__ volatile("": : :"memory");*/
}
int isLocked(struct Mutex* m) {
    return m -> isLocked;
}
void unlock(struct Mutex* m) {
    //m -> ticket[currentThread->id] = 0;
    m -> isLocked = 0;
}

//Notifyer
void __addWaitor(struct Notifyer* notifyer) {
    lock(notifyer -> mutex);
    notifyer -> waiters[notifyer -> waitersCount++] = currentThread;
    currentThread->threadState = THREAD_STATE_WAIT;
    unlock(notifyer -> mutex);
}

struct Notifyer* newNotifyer() {
    struct Notifyer* notifyer = (struct Notifyer*) mem_alloc(sizeof(struct Notifyer));
    notifyer -> mutex = newMutex();
    notifyer -> waitersCount = 0;
    return notifyer;
}
void wait(struct Notifyer* notifyer) {
    lock(&__threadMutex); __cantSwitchThread();
    __addWaitor(notifyer);
    __canSwitchThread(); unlock(&__threadMutex);
    switchThread();
}
void notify(struct Notifyer* notifyer) {
    lock(&__threadMutex); __cantSwitchThread();
    lock(notifyer -> mutex);
    notifyer -> waiters[notifyer -> waitersCount--] -> threadState = THREAD_STATE_RUNNABLE;
    unlock(notifyer -> mutex);
    __canSwitchThread(); unlock(&__threadMutex);
}
void notifyAll(struct Notifyer* notifyer) {
    lock(&__threadMutex); __cantSwitchThread();
    lock(notifyer -> mutex);
    for (int i = 0; i < notifyer -> waitersCount; i++) {
        notifyer -> waiters[i] -> threadState = THREAD_STATE_RUNNABLE;
    }
    notifyer -> waitersCount = 0;
    unlock(notifyer -> mutex);
    __canSwitchThread(); unlock(&__threadMutex);
}
