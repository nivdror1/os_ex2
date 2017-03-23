// a class of a  thread

#ifndef OS_EX2_THREAD_H
#define OS_EX2_THREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/**
 * the set of state that the thread could be in
 */
enum State {Spawn,Ready,Running,Blocked, Terminated};

class Thread{
private:
	/** a id of the thread*/
	int _tid;

	/** the number of times the thread was at running state*/
	int _runningTimes;

	/** the current state of the thread*/
	State _currState;

	/** the current place in stack*/
	address_t _sp;

	/** the current place in the program counter*/
	address_t _pc;

	/** the thread stack*/
	char _threadStack[stackSize];

	/** the enviroment to save theard stack*/
	sigjmp_buf _environment;

	/** A translation is required when using an address of a variable.
    Use this as a black box in your code. */
	address_t translate_address(address_t addr);

public:
	/**
	 * @brief a constructor
	 * @param tid the id of the thread
	 * @param func the function of the thread
	 */
	Thread(int tid, void (*func)(void),int stackSize);

	/**
	 * @brief get the thread id
	 * @return thread id
	 */
	const int getThreadId();

	/**
	 * @brief  get the number of times the thread has been to running mode
	 * @return the number of times the thread has been to running mode
	 */
	const int getRunningTimes();

	/**
	 * @brief increase the number of times the thread was at running state
	 * @return the increased runningTimes
	 */
	int increaseRunning();

	/**
	 * change the state of the thread
	 * @return
	 */
	void changeStatus(State newState );

	/**
	 * @brief get the state of the theard
	 * @return the state of the theard
	 */
	const State getStatus();

	/**
	 * get the thread evironment
	 * @return the thread evironment
	 */
	const sigjmp_buf getEnvironment();


};

#endif //OS_EX2_THREAD_H
