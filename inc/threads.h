#ifndef _THREADS_H_
#define _THREADS_H_

#define MAX_THREAD_COUNT 10000l
#define STACK_SIZE 0x00f000l

void initThreads();

struct ThreadInfo {
    int id;
    int canSwitchThread;
    int threadLockCount;
    int threadState;
    struct Mutex* waitingFor;
    #define THREAD_STATE_RUNNABLE   0l
    #define THREAD_STATE_RUNNING    1l
    #define THREAD_STATE_WAIT       2l
    #define THREAD_STATE_WAIT_MUTEX 3l
    #define THREAD_STATE_FINISHED   4l
    struct Notifyer* joinedThreadsNotifyer;
    void* stackPtr;
};

extern struct ThreadInfo* currentThread;

struct ThreadInfo* newThread(void (*threadMain)(void*), void* arg);
void finishThread(struct ThreadInfo* thread);
void startThread(struct ThreadInfo*);
void switchThread();
void joinThread(struct ThreadInfo*);

#define beginSynchronize\
    static struct Mutex __synchronizeMutex;\
    lock(&__synchronizeMutex)

#define endSynchronize\
    unlock(&__synchronizeMutex)

#define lockThread() \
    beginSynchronize;\
    currentThread->canSwitchThread = 0;\
    currentThread->threadLockCount++

#define unlockThread() \
    currentThread->threadLockCount--;\
    if (currentThread->threadLockCount < 0) {\
        currentThread->threadLockCount = 0;\
    }\
    if (currentThread->threadLockCount == 0) {\
        currentThread->canSwitchThread = 1;\
    }\
    endSynchronize

struct Mutex {
    int claim[MAX_THREAD_COUNT];
    int turn[MAX_THREAD_COUNT];
    int isLocked;
};

extern struct Mutex mutexInitializer;

struct Mutex* newMutex();
void lock(struct Mutex* m);
int isLocked(struct Mutex* m);
void unlock(struct Mutex* m);
void freeMutex(struct Mutex* m);

struct Notifyer {
    struct ThreadInfo* waiters[MAX_THREAD_COUNT];
    int waitersCount;
};

struct Notifyer* newNotifyer();
void wait(struct Notifyer*);
void notify(struct Notifyer*);
void notifyAll(struct Notifyer*);

#endif
