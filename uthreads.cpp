//
// Created by ido.shachar on 3/23/17.
//

#include <queue>
#include <stdio.h>
#include "uthreads.h"
#include "Thread.h"

//---------------------- global variables------------------------------

int quantumLength;

std::priority_queue<int, std::vector<int>, std::greater<int> > availableTheardid;

Thread* threadsList[MAX_THREAD_NUM];

std::List<int> readyList;

std::vector<int> blockedList;

int runningThreadId;

int totalQuantoms;

// in i-th index there is vector of all thread id of thread that blocked by the i-th thread
std::vector<std::vector<int>> dependOnThread(MAX_THREAD_NUM, std::vector<int>(0));

/**
 * set the timer so it will signal SIGVTALRM after quantum_usecs of the thread exceution.
 * @param quantum_usecs the micro second the set the timer
 */
void setTimer(int quantum_usecs) {

    // Configure the timer to expire after the specified micro sec... */
    timer.it_value.tv_sec = quantum_usecs;        // first time interval, seconds part
    timer.it_value.tv_usec = 0;        // first time interval, microseconds part

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = quantum_usecs;    // following time intervals, seconds part
    timer.it_interval.tv_usec = 0;    // following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        std::cerr<<"thread library error: timer set error."<<std::endl;
    }

}

/**
 * in case of reciving SIGVTALRM switch the thread
 * @param signal the signal SIGVTALRM
 */
void timer_handler(int signal)
{
    switchThreads(); //call switch threads
}


/**
 * change SIGVTALRM to perform as the function timer_handler
 * @param act a sigaction instance
 */
void changeTimerSignal(sigaction act){
    // Install timer_handler as the signal handler for SIGVTALRM.
    act.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa,NULL) < 0) {
        std::cerr<<"thread library error: sigaction error."<<std::endl;
    }
}
/**
 * resume the dependent threads of some thread tid
 */
void releaseDependent(tid){
	// after the current thread stop running, resume all the threads depend on him
	for (int i: dependOnThread[tid]){
		uthread_resume(i);
        threadsList[i]->setDependency(-1);
	}
}

void switchThreads(){
    releaseDependent(runningThreadId);
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
    }
    //change the signal
    changeTimerSignal(act);
    //set the timer
    setTimer(quantum_usecs);

    quantumLength = quantum_usecs;
    for (int i = 1; i < MAX_THREAD_NUM; ++i)
    {
        availableThreadId.push(i);
    }
    runningThreadId = -1;
    totalQuantoms = 1;
}

/**
 * @brief releasing the assigned library memory
 */
void uthreadFinalizer(){
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
    if (availableTheardid.empty()){
        std::cerr << "system error: maximum threads exceed\n";
        return -1;
    }
    int newThreadid = availableTheardid.pop();
    threadsList[newThreadid] = new Thread(newThreadid, f, STACK_SIZE);
    if (threadsList[newThreadid] == NULL){
        std::cerr << "system error: cannot allocate new thread\n";
        return -1;
    }
    readyList.pushback(newThreadid);
}

/**
 * @brief check if given tid is valid, return matching thread if it does.
 * @param tid the id of the thread
 * @return matching thread if tid is valid id, NULL otherwise
 */
Thread* getThread(int tid){
    if (tid < 0 || tid > MAX_THREAD_NUM){
        return NULL;
    }
    if (threadsList[tid] == NULL){
        return NULL;
    }
    return threadsList[tid];
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
    Thread* currentThread = getThread(tid);
    if (currentThread == NULL){
        std::cerr << "thread library error: invalid thread id\n";
        return -1;
    }
	//if the thread is the main thread and terminate process
    if (tid == 0){
        uthreadFinalizer(); // terminate all of the threads
        exit(0);
    }
	//if the thread is in blocked delete the thread from the blockedList
	if(currentThread->getStatus()==Blocked){
		blockedList.erase(std::find(blockedList.begin(),blockedList.end(),tid));
	}
	//if the thread is in ready delete the thread from the readyList
	if(currentThread->getStatus()==Ready){
		readyList.remove(tid);
	}
	if(tid==runningThreadId){
		switchThreads();
	}
    releaseDependent(tid); //release the thread that are depending on it
    delete currentThread;
    threadsList[tid] = NULL;
	//todo check what we suppose to do when we do not need to return anything
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
    Thread* currentThread = getThread(tid);
    if (currentThread == NULL || tid == 0){
        std::cerr << "thread library error: invalid thread id\n";
        return -1;
    }
    if (tid != runningThreadId){
        threadsList[tid]->changeStatus(Blocked);
    }
    else
    {
        // todo switch threads
        switchTheards();
    }
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
    Thread* currentThread = getThread(tid);
    int dependOnTid = -1;
    int indexIndependencyList = -1;
    if (currentThread == NULL){
        std::cerr << "thread library error: invalid thread id\n";
        return -1;
    }
    if (currentThread->getStatus() == Blocked){
        blockedList.erase(std::find(blockedList.begin(), blockedList.end(), tid));
        currentThread->changeStatus(Ready);
        readyList.pushback(tid);
        dependOnTid = currentThread->getDependency();
        if (dependOnTid != -1){
            indexIndependencyList = std::find(dependOnThread[dependOnTid]
                                                      .begin(),dependOnThread[dependOnTid].end(),tid);
            dependOnThread[currentThread->getDependency()].erase(indexIndependencyList);
        }
    }
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
    Thread* currentThread = getThread(tid);
    if (currentThread == NULL){
        std::cerr << "thread library error: invalid thread id\n";
        return -1;
    }
    threadsList[runningThreadId]->changeStatus(Blocked);
    dependOnThread[tid].pushback(runningThreadId);
    currentThread->setDependency(tid);
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
    Thread* currentThread = getThread(tid);
    if (currentThread == NULL){
        std::cerr << "thread library error: invalid thread id\n";
        return -1;
    }
    return currentThread->getRunningTimes();
}

