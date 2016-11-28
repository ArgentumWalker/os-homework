#ifndef _THREADS_H_
#define _THREADS_H_

#define MAX_THREAD_COUNT 100000

struct ThreadInfo {
    int id;
    int canSwitchThread;
    int threadLockCount;
    void* stackPtr;
}

extern ThreadInfo currentThread;

ThreadInfo* newThread();
void finishThread(ThreadInfo* thread);
void switchThread();
void joinThread(struct ThreadInfo);

#define beginSyncronize()\
    static struct Mutex __syncronizeMutexValue;\
    lock(__syncronizeMutex&)

#define endSynchronize()\
    unlock(__synchronizeMutex&)

#define lockThread() \
    beginSynchronize();\
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
    endSynchronize()

struct Mutex {
    int claim[MAX_THREAD_COUNT];
    int turn[MAX_THREAD_COUNT];
}

struct Mutex* newMutex();
void lock(struct Mutex* m);
void unlock(struct Mutex* m);
void freeMutex(struct Mutex* m)

struct Notifyer {
    struct ThreadInfo waiters[MAX_THREAD_COUNT];
}

void 

#endif
