//a instance of a thread

#include "Thread.h"

/**
 * @brief a constructor
 * @param tid the id of the thread
 */
Thread::Thread(int tid){
	_tid=tid;
	_runningTimes=0;
	_currState= Spawn;
}

/**
 * @brief get the thread id
 * @return thread id
 */
const int Thread::getThreadId(){
	return _tid;
}

/**
 * @brief  get the number of times the thread has been to running mode
 * @return the number of times the thread has been to running mode
 */
const int Thread::getRunningTimes(){
	return _runningTimes;
}

/**
 * @brief increase the number of times the thread was at running state
 * @return the increased runningTimes
 */
int Thread::increaseRunning(){
	_runningTimes+=1;
	return _runningTimes;
}

/**
 * change the state of the thread
 */
void Thread::changeStatus(State newState){
	_currState=newState;
}

/**
 * @brief get the state of the theard
 * @return the state of the theard
 */
const State Thread::getStatus(){
	return _currState;
}
