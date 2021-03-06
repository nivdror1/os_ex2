//
// Created by ido.shachar on 3/23/17.
//



#include <stdlib.h>
#include <functional>
#include <vector>
#include <queue>
#include "uthreads.h"
#include "Thread.h"
#include <list>
#include <iostream>

#define MILLION 1000000
//---------------------- global variables------------------------------


std::priority_queue<int, std::vector<int>, std::greater<int> > availableThreadId;

Thread* threadsList[MAX_THREAD_NUM];

std::list<int> readyList;

int quantum_length;

int runningThreadId;

int totalQuantoms;

int beforeSwitching=1;

struct sigaction act;

struct itimerval timer;

sigset_t timerSet;
// in i-th index there is vector of all thread id of thread that blocked by the i-th thread
std::vector<std::vector<int>> dependOnThread(MAX_THREAD_NUM, std::vector<int>(0));

void uthreadFinalizer();

/**
 * set the timer so it will signal SIGVTALRM after quantum_usecs of the thread exceution.
 * @param quantum_usecs the micro second the set the timer
 */
void setTimer(int quantum_usecs) {

    // Configure the timer to expire after the specified micro sec... */
    timer.it_value.tv_sec = quantum_usecs/MILLION;        // first time interval, seconds part
    timer.it_value.tv_usec = quantum_usecs%MILLION;        // first time interval, microseconds part

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = quantum_usecs/MILLION;    // following time intervals, seconds part
    timer.it_interval.tv_usec = quantum_usecs%MILLION;    // following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing..
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        uthreadFinalizer();
        std::cerr<<"thread library error: timer set error."<<std::endl;
        exit(1);
    }

}



void updateThreadToReady(int tid)
{
    threadsList[tid]->changeStatus(Ready);
    readyList.push_back(tid);
}

/**
 * block or unblock the SIGVTALRM signal
 * @param signalStatus an int represent whether to block or to unblock the signal
 * @return -1 if some error had occured else return 0
 */
int changeSignalStatus(int signalStatus){
    int err=sigemptyset(&timerSet);
    err+=sigaddset(&timerSet,SIGVTALRM);
    err+=sigprocmask(signalStatus, &timerSet, NULL);
    if(err<0){
        std::cerr<<"could not block/unblock the signal"<<std::endl;
    }
    return 0;
}

/**
 * resume the dependent threads of some thread tid
 */
void releaseDependent(int tid){
    // after the current thread stop running, resume all the threads depend on him
    for (int i: dependOnThread[tid]){
        if (threadsList[i]->getStatus() == Sync) {
            updateThreadToReady(i);
        }
        threadsList[i]->setDependency(-1);
    }
    dependOnThread[tid].clear();
}

/**
 * swtich between the current running thread and the next thread that should run, update quantoms
 * calculation and release threads that depend on the current running thread.
 */
void switchThreads(){
    setTimer(0);
    if (threadsList[runningThreadId] != NULL)
    {
        int ret_val = sigsetjmp(*(threadsList[runningThreadId]->getEnvironment()), 1);
        if (ret_val == 1)
        {
            return;
        }
        //release all of his dependencies threads
        releaseDependent(runningThreadId);
    }

    // prepare to jump to the next thread on the ready list
    runningThreadId=readyList.front();
    readyList.pop_front();
    threadsList[runningThreadId]->changeStatus(Running);
    totalQuantoms++;
    setTimer(quantum_length);
    siglongjmp(*(threadsList[runningThreadId]->getEnvironment()),1);

}

/**
 * in case of reciving SIGVTALRM switch the thread
 * @param signal the signal SIGVTALRM
 */
void timer_handler(int signal)
{
    //move the running thread the end of the ready list
    readyList.push_back(runningThreadId);
    threadsList[runningThreadId]->changeStatus(Ready);
    switchThreads(); //call switch threads
}


/**
 * change SIGVTALRM to perform as the function timer_handler
 * @param act a sigaction instance
 */
void changeTimerSignal(){
    // Install timer_handler as the signal handler for SIGVTALRM.
    act.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &act,NULL) < 0) {
        std::cerr<<"thread library error: sigaction error."<<std::endl;
    }
}


/*
 * Description: This function initializes the thread library.
 * You may assume that this function is called before any other thread library
 * function, and that it is called exactly once. The input to the function is
 * the length of a quantum in micro-seconds. It is an error to call this
 * function with non-positive quantum_usecs.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_init(int quantum_usecs){
    if (quantum_usecs <= 0){
        std::cerr<<"thread library error: invalid quantum length"<<std::endl;
        return -1;
    }
    for (int i = 0; i < MAX_THREAD_NUM; ++i)
    {
        availableThreadId.push(i);
    }

    sigaddset(&timerSet, SIGVTALRM);
    // create main thread
    uthread_spawn(NULL);


    threadsList[0]->changeStatus(Running);
    runningThreadId = 0;
    totalQuantoms = 1;
    quantum_length = quantum_usecs;

    //change the signal
    changeTimerSignal();
    //set the timer
    setTimer(quantum_usecs);
    return 0;
}

/**
 * @brief releasing the assigned library memory
 */
void uthreadFinalizer(){
    setTimer(0);
    for (int i = 1; i < MAX_THREAD_NUM; ++i)
    {
        delete threadsList[i];
    }
}

/*
 * Description: This function creates a new thread, whose entry point is the
 * function f with the signature void f(void). The thread is added to the end
 * of the READY threads list. The uthread_spawn function should fail if it
 * would cause the number of concurrent threads to exceed the limit
 * (MAX_THREAD_NUM). Each thread should be allocated with a stack of size
 * STACK_SIZE bytes.
 * Return value: On success, return the ID of the created thread.
 * On failure, return -1.
*/
int uthread_spawn(void (*f)(void)){
    changeSignalStatus(SIG_BLOCK);
    if (availableThreadId.empty()){
        std::cerr << "system error: maximum threads exceed\n";
        return -1;
    }
    int newThreadid = availableThreadId.top();
    availableThreadId.pop();
    threadsList[newThreadid] = new (std::nothrow)Thread(newThreadid, f, STACK_SIZE);
    if (threadsList[newThreadid] == NULL){
        std::cerr << "system error: cannot allocate new thread\n";
        return -1;
    }
    if (newThreadid!=0)
    {
        readyList.push_back(newThreadid);
    }
    changeSignalStatus(SIG_UNBLOCK);
    return newThreadid;
}

/**
 * @brief check if given tid is valid, return matching thread if it does.
 * @param tid the id of the thread
 * @return matching thread if tid is valid id, NULL otherwise
 */
int checkTid(int tid){
    if (tid < 0 || tid > MAX_THREAD_NUM){
        return -1;
    }
    if (threadsList[tid] == NULL){
        return -1;
    }
    return tid;
}

/*
 * Description: This function terminates the thread with ID tid and deletes
 * it from all relevant control structures. All the resources allocated by
 * the library for this thread should be released. If no thread with ID tid
 * exists it is considered as an error. Terminating the main thread
 * (tid == 0) will result in the termination of the entire process using
 * exit(0) [after releasing the assigned library memory].
 * Return value: The function returns 0 if the thread was successfully
 * terminated and -1 otherwise. If a thread terminates itself or the main
 * thread is terminated, the function does not return.
*/
int uthread_terminate(int tid){
    changeSignalStatus(SIG_BLOCK);
    if (checkTid(tid) == -1){
        std::cerr << "thread library error: invalid thread id in terminate\n";
        return -1;
    }
    //if the thread is the main thread and terminate process
    if (tid == 0){
        uthreadFinalizer(); // terminate all of the threads
        exit(0);
    }
    //if the thread is in ready delete the thread from the readyList
    if(threadsList[tid]->getStatus()==Ready){
        readyList.remove(tid);
    }
    releaseDependent(tid); //release the thread that are depending on it
    delete threadsList[tid];
    threadsList[tid] = NULL;
    availableThreadId.push(tid);//add it to the available thread list
    if(tid==runningThreadId){
        changeSignalStatus(SIG_UNBLOCK);
        switchThreads();
    }
    changeSignalStatus(SIG_UNBLOCK);
    return 0;
}


/*
 * Description: This function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_block(int tid){
    changeSignalStatus(SIG_BLOCK);
    if (checkTid(tid) == -1 ){
        std::cerr << "thread library error: invalid thread id in block\n";
        return -1;
    }else if(tid==0){
        std::cerr << "thread library error: it's illegal to block the main thread\n";
        return -1;
    }

    State threadState = threadsList[tid]->getStatus();
    threadsList[tid]->changeStatus(Blocked);
    //if the thread is blocked while in ready state
    if (threadState ==Ready){
        readyList.remove(tid);
    }
    else if(threadState ==Running)
    {
        changeSignalStatus(SIG_UNBLOCK);
        switchThreads();
    }
    changeSignalStatus(SIG_UNBLOCK);
    return 0;
}


/*
 * Description: This function resumes a blocked thread with ID tid and moves
 * it to the READY state. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid){
    changeSignalStatus(SIG_BLOCK);
    if (checkTid(tid) == -1){
        std::cerr << "thread library error: invalid thread id in resume\n";
        return -1;
    }
    if (threadsList[tid]->getStatus() == Blocked){
        if (threadsList[tid]->getDependency() == -1){
            updateThreadToReady(tid);
        }
        else {
            threadsList[tid]->changeStatus(Sync);
        }
    }
    changeSignalStatus(SIG_UNBLOCK);
    return 0;
}


/*
 * Description: This function blocks the RUNNING thread until thread with
 * ID tid will move to RUNNING state (i.e.right after the next time that
 * thread tid will stop running, the calling thread will be resumed
 * automatically). If thread with ID tid will be terminated before RUNNING
 * again, the calling thread should move to READY state right after thread
 * tid is terminated (i.e. it won’t be blocked forever). It is considered
 * as an error if no thread with ID tid exists or if the main thread (tid==0)
 * calls this function. Immediately after the RUNNING thread transitions to
 * the BLOCKED state a scheduling decision should be made.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_sync(int tid){
    changeSignalStatus(SIG_BLOCK);
    if (checkTid(tid) == -1||tid==runningThreadId || runningThreadId == 0){
        std::cerr << "thread library error: invalid thread id in sync\n";
        return -1;
    }
    threadsList[runningThreadId]->changeStatus(Sync);
    dependOnThread[tid].push_back(runningThreadId);
    threadsList[runningThreadId]->setDependency(tid);
    changeSignalStatus(SIG_UNBLOCK);
    switchThreads();
    return 0;
}


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid(){
    return runningThreadId;
}


/*
 * Description: This function returns the total number of quantums that were
 * started since the library was initialized, including the current quantum.
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number
 * should be increased by 1.
 * Return value: The total number of quantums.
*/
int uthread_get_total_quantums(){
    return totalQuantoms;
}


/*
 * Description: This function returns the number of quantums the thread with
 * ID tid was in RUNNING state. On the first time a thread runs, the function
 * should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state
 * when this function is called, include also the current quantum). If no
 * thread with ID tid exists it is considered as an error.
 * Return value: On success, return the number of quantums of the thread with ID tid. On failure, return -1.
*/
int uthread_get_quantums(int tid){
    if (checkTid(tid) == -1){
        std::cerr << "thread library error: invalid thread id in get quantums\n";
        return -1;
    }
    return threadsList[tid]->getRunningTimes();
}

