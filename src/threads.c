#include "threads.h"
#include "alloc.h"
#include "ioport.h"
#include "ints.h"

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
    currentThread -> threadLockCount = 0;
    currentThread -> canSwitchThread = 1;
    currentThread -> joinedThreadsNotifyer = &kernelNotifyer;
    currentThread -> threadState = THREAD_STATE_RUNNING;
    threadQueue[0] = currentThread;
    threads[0] = currentThread;
    queueEnd = 1;
    queuePos = 0;
}

//Thread

struct ThreadInfo* newThread(void (*threadMain)(void*), void* arg) {
  lock(&__threadMutex); currentThread -> canSwitchThread = 0;
  
  struct ThreadInfo *newThread = (struct ThreadInfo*) mem_alloc(sizeof(struct ThreadInfo));
  newThread->id = 0;
  for (int freeID = 0; threads[freeID] != 0; freeID++) {
    newThread -> id = freeID + 1;
  }
  printf("New thread id = %d\n", newThread->id);
  threads[newThread -> id] = newThread;
  newThread -> threadLockCount = 0;
  newThread -> canSwitchThread = 1;
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
 
  currentThread -> canSwitchThread = 1; unlock(&__threadMutex); 
  return newThread;
}

void startThread(struct ThreadInfo* thread) {
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
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
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
}
 
void finishThread(struct ThreadInfo* thread) {
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
    thread -> threadState = THREAD_STATE_FINISHED;
    notifyAll(thread -> joinedThreadsNotifyer);
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
}

void switchThread() {
    //printf("Start switchThread\n");
    if (currentThread -> canSwitchThread) { //We cant switch thread, which make some operations with threads.
        lock(&__threadMutex); currentThread -> canSwitchThread = 0;
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
            if (thread -> threadState == THREAD_STATE_WAIT_MUTEX && !isLocked(thread -> waitingFor)) {
                notFound = 0;
                thread -> threadState = THREAD_STATE_RUNNABLE;
            }
        }
        //printf("Found new thread\n");
        thread -> threadState = THREAD_STATE_RUNNING;
        if (currentThread != thread) {
            //printf("Threads are not the same\n");
            struct ThreadInfo* cur = currentThread;
            currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
            currentThread = thread;
            asm_switch_threads(&(cur->stackPtr), thread -> stackPtr);
            printf("Thread %d switched to Thread %d\n", cur->id, thread->id);
            return;
            //printf("Finish switch threads\n");
        } else {
            //printf("Threads are the same\n");
        }
        currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
    } else {
        printf("Cant switch this!\n");
    }
}
void joinThread(struct ThreadInfo* thread) {
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
    if (thread -> threadState != THREAD_STATE_FINISHED) {
        __addWaitor(thread -> joinedThreadsNotifyer);
        currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
        switchThread();
        return;
    }
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
}

//Mutex

void initiateMutex(struct Mutex* mutex) {
    for (int i = 0; i < MAX_THREAD_COUNT; i++) {
//        mutex -> claim[i] = 0;
//        mutex -> turn[i] = 0;
        mutex -> select[i] = 0;
        mutex -> ticket[i] = 0;
    }
    mutex -> isLocked = 0;
}

struct Mutex* newMutex() {
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
    struct Mutex* mutex = (struct Mutex*) mem_alloc(sizeof(struct Mutex));
    initiateMutex(mutex); 
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
    return mutex;
}

int __max(struct Mutex* m) {
    int rc = 0;
    for (int i = 0; i < MAX_THREAD_COUNT; i++) {
        if (m -> ticket[i] > rc) {
            rc = m -> ticket[i];
        }
    }
    return rc;    
}

void lock(struct Mutex* m) {
        __asm__ volatile("": : :"memory");
    currentThread -> waitingFor = m; //если мы ждем какого-то mutex, то это он.
    
    m->select[currentThread->id] = 1;
        __asm__ volatile("": : :"memory");
    m->ticket[currentThread->id] = __max(m) + 1;
        __asm__ volatile("": : :"memory");
    m->select[currentThread->id] = 0;
        __asm__ volatile("": : :"memory");
    
    for (int thread = 0; thread < MAX_THREAD_COUNT; thread++) {
        if (thread == currentThread -> id) {
            continue;
        }
        while (m->select[thread]) {
                __asm__ volatile("": : :"memory");     
        }
        while (m->ticket[thread] && ((m->ticket[thread] < m->ticket[currentThread->id]) || (m->ticket[thread] == m->ticket[currentThread->id] && thread < currentThread->id))) {
                __asm__ volatile("": : :"memory");        
        }
    }
    __asm__ volatile("": : :"memory");
    m -> isLocked = 1;
    
        __asm__ volatile("": : :"memory");
/*    for (int level = 0; level < MAX_THREAD_COUNT - 1; ++level) {
        m->claim[currentThread->id] = level + 1;
        struct ThreadInfo* prevThread = threads[m->turn[level]];
        __asm__ volatile("": : :"memory");
        m->turn[level] = currentThread->id;
        __asm__ volatile("": : :"memory");
        if (prevThread != 0)
            prevThread -> threadState = THREAD_STATE_RUNNABLE; //У этого потока теперь выполнено одно из условий прерывания цикла
        __asm__ volatile("": : :"memory");
        while (1) {
            int found = 0;
            for (int thread = 0; !found && thread < MAX_THREAD_COUNT; thread++) {
                if (thread == currentThread->id) continue;
                found = m->claim[thread] > level;
            }
            if (!found) break;
            if (m->turn[level] != currentThread->id) break;
            printf("Turning into a sleep\n");
            __asm__ volatile("": : :"memory");
            currentThread->threadState = THREAD_STATE_WAIT_MUTEX; //Проснется только если MUTEX не залочен или может попасть на новый уровень.
            __asm__ volatile("": : :"memory");
            switchThread();
        }
    }
    __asm__ volatile("": : :"memory");
    m -> isLocked = 1;*/
}
int isLocked(struct Mutex* m) {
    return m -> isLocked;
}
void unlock(struct Mutex* m) {
    m -> ticket[currentThread->id] = 0;
    //m -> claim[currentThread->id] = 0;
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
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
    __addWaitor(notifyer);
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
    switchThread();
}
void notify(struct Notifyer* notifyer) {
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
    lock(notifyer -> mutex);
    notifyer -> waiters[notifyer -> waitersCount--] -> threadState = THREAD_STATE_RUNNABLE;
    unlock(notifyer -> mutex);
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
}
void notifyAll(struct Notifyer* notifyer) {
    lock(&__threadMutex); currentThread -> canSwitchThread = 0;
    lock(notifyer -> mutex);
    for (int i = 0; i < notifyer -> waitersCount; i++) {
        notifyer -> waiters[i] -> threadState = THREAD_STATE_RUNNABLE;
    }
    notifyer -> waitersCount = 0;
    unlock(notifyer -> mutex);
    currentThread -> canSwitchThread = 1; unlock(&__threadMutex);
}
