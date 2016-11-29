#ifndef _THREADS_H_
#define _THREADS_H_

#define MAX_THREAD_COUNT 1000l
#define STACK_SIZE 0x00f000l

#include "print.h"

void initThreads();

struct ThreadInfo {
    int id;
    int canSwitchThread;
    int wantSwitchThread;
    int threadState;
    int disabledInts;
    struct Mutex* waitFor;
    //struct __MutexNode mutexNode;
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
/*
struct __MutexNode {
    struct __MutexNode* _Atomic next;
    atomic_int wait;
}
*/
struct Mutex {
    //struct __MutexNode* _Atomic tail;
    int isLocked;
};

void initiateMutex(struct Mutex* mutex);
struct Mutex* newMutex();
void lock(struct Mutex* m);
int isLocked(struct Mutex* m);
void unlock(struct Mutex* m);
void freeMutex(struct Mutex* m);

struct Notifyer {
    struct Mutex* mutex;
    struct ThreadInfo* waiters[MAX_THREAD_COUNT];
    int waitersCount;
};

struct Notifyer* newNotifyer();
void wait(struct Notifyer*);
void notify(struct Notifyer*);
void notifyAll(struct Notifyer*);

#endif
