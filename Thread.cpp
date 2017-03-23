//a instance of a thread

#include "Thread.h"

/**
 * @brief a constructor
 * @param tid the id of the thread
 */
Thread::Thread(int tid, void (*func)(void),int stackSize){
	_tid=tid;
	_runningTimes=0;
	_currState= Spawn;
	_sp = (address_t)_threadStack + stackSize - sizeof(address_t);
	_pc = (address_t)func;
	(_environment->__jmpbuf)[JB_SP] = translate_address(_sp);
	(_environment->__jmpbuf)[JB_PC] = translate_address(_pc);

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

/**
 * get the thread evironment
 * @return the thread evironment
 */
const sigjmp_buf  Thread::getEnvironment(){
	return _environment;
}
/** A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t Thread::translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
			"rol    $0x11,%0\n"
	: "=g" (ret)
	: "0" (addr));
	return ret;
}