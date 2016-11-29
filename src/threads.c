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

struct ThreadInfo* currentThread;
static struct ThreadInfo kernelThread;
static struct ThreadInfo* threads[MAX_THREAD_COUNT];
static struct ThreadInfo* threadQueue[MAX_THREAD_COUNT + 1];
static int queueEnd, queuePos;

//Other functions
void callThreadMain(void (*threadMain)(void*), void* arg) {
  out8(0x20, 1 << 5); //Master EOI
  out8(0xA0, 1 << 5); //Slave EOI
  threadMain(arg);
  finishThread(currentThread);
  switchThread();
  while(1);
}

//Work with thread queue
void __addThreadToQueue(struct ThreadInfo* thread) {
    lockThread();
    threadQueue[queueEnd] = thread;
    queueEnd = (queueEnd + 1) % MAX_THREAD_COUNT;
    unlockThread();
}

struct ThreadInfo* __getNextThread() {
    lockThread();
    struct ThreadInfo* thread = threadQueue[queuePos];
    queuePos = (queuePos + 1) % MAX_THREAD_COUNT;
    if (thread -> threadState != THREAD_STATE_FINISHED) {
        __addThreadToQueue(thread);
    }
    unlockThread();
    return thread;
}

//Init function
struct Mutex mutexInitializer;

void initThreads() {
    currentThread = &kernelThread;
    currentThread -> id = 0;
    currentThread -> stackPtr = 0;
    currentThread -> threadLockCount = 0;
    currentThread -> canSwitchThread = 1;
    currentThread -> threadState = THREAD_STATE_RUNNING;
    threadQueue[0] = currentThread;
    threads[0] = currentThread;
    queueEnd = 1;
    queuePos = 0;
}

//Thread

struct ThreadInfo* newThread(void (*threadMain)(void*), void* arg) {
  lockThread();
  
  struct ThreadInfo *newThread = (struct ThreadInfo*) mem_alloc(sizeof(struct ThreadInfo));
  for (int freeID = 0; threads[freeID] != 0; freeID++) {
    newThread -> id = freeID;
  }
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
 
  unlockThread(); 
  return newThread;
}

void startThread(struct ThreadInfo* thread) {
    thread -> threadState = THREAD_STATE_RUNNABLE;
    __addThreadToQueue(thread);
}
 
void finishThread(struct ThreadInfo* thread) {
    lockThread();
    thread -> threadState = THREAD_STATE_FINISHED;
    notifyAll(thread -> joinedThreadsNotifyer);
    unlockThread();
}

void switchThread() {
    disable_ints();
    if (currentThread -> canSwitchThread) {
        if (currentThread -> threadState == THREAD_STATE_RUNNING) {
            currentThread -> threadState = THREAD_STATE_RUNNABLE; //current Thread already at the end of queue
        }
        lockThread();
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
        unlockThread();
        thread -> threadState = THREAD_STATE_RUNNING;
        if (currentThread != thread) {
            struct ThreadInfo* cur = currentThread;
            currentThread = thread;
            asm_switch_threads(&(cur->stackPtr), thread -> stackPtr);
            printf("Switch thread %d to thread %d\n", cur->id, thread->id);
        }
    }
    enable_ints();
}
void joinThread(struct ThreadInfo* thread) {
    if (thread -> threadState != THREAD_STATE_FINISHED) {
        wait(thread -> joinedThreadsNotifyer);
    }    
}

//Mutex

void initiateMutex(struct Mutex* mutex) {
    for (int i = 0; i < MAX_THREAD_COUNT; i++) {
        mutex -> claim[i] = 0;
        mutex -> turn[i] = 0;
    }
    mutex -> isLocked = 0;
}

struct Mutex* newMutex() {
    lockThread();
    struct Mutex* mutex = (struct Mutex*) mem_alloc(sizeof(struct Mutex));
    initiateMutex(mutex); 
    unlockThread();
    return mutex;
}

void lock(struct Mutex* m) {
    currentThread -> waitingFor = m; //если мы ждем какого-то mutex, то это он.
    for (int level = 0; level < MAX_THREAD_COUNT - 1; ++level) {
        m->claim[currentThread->id] = level + 1;
        struct ThreadInfo* prevThread = threads[m->turn[level]];
        m->turn[level] = currentThread->id;
        if (prevThread != 0)
            prevThread -> threadState = THREAD_STATE_RUNNABLE; //У этого потока теперь выполнено одно из условий прерывания цикла
        
        while (1) {
            int found = 0;
            for (int thread = 0; !found && thread < MAX_THREAD_COUNT; thread++) {
                if (thread == currentThread->id) continue;
                found = m->claim[thread] > level;
            }
            if (!found) break;
            if (m->turn[level] != currentThread->id) break;
            printf("Turning into a sleep\n");
            currentThread->threadState = THREAD_STATE_WAIT_MUTEX; //Проснется только если MUTEX не залочен или может попасть на новый уровень.
            switchThread();
        }
    }
    m -> isLocked = 1;
}
int isLocked(struct Mutex* m) {
    printf("isLocking check\n");
    return m -> isLocked;
}
void unlock(struct Mutex* m) {
    m -> claim[currentThread->id] = 0;
    m -> isLocked = 0;
}

//Notifyer

struct Notifyer* newNotifyer() {
    lockThread();
    struct Notifyer* notifyer = (struct Notifyer*) mem_alloc(sizeof(struct Notifyer));
    notifyer -> waitersCount = 0;
    unlockThread();
    return notifyer;
}
void wait(struct Notifyer* notifyer) {
    lockThread();
    notifyer -> waiters[notifyer -> waitersCount++] = currentThread;
    currentThread->threadState = THREAD_STATE_WAIT;
    unlockThread();
    switchThread();
}
void notify(struct Notifyer* notifyer) {
    lockThread();
    notifyer -> waiters[notifyer -> waitersCount--] -> threadState = THREAD_STATE_RUNNABLE;
    unlockThread();
}
void notifyAll(struct Notifyer* notifyer) {
    lockThread();
    for (int i = 0; i < notifyer -> waitersCount; i++) {
        notifyer -> waiters[i] -> threadState = THREAD_STATE_RUNNABLE;
    }
    notifyer -> waitersCount = 0;
    unlockThread();
}