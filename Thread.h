// a class of a  thread

#ifndef OS_EX2_THREAD_H
#define OS_EX2_THREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define STACK_SIZE 4096 /* stack size per thread (in bytes) */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/**
 * the set of state that the thread could be in
 */
enum State {Ready,Running,Blocked};

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
	char _threadStack[STACK_SIZE];

	/** the enviroment to save theard stack*/
	sigjmp_buf _environment;

	/** A translation is required when using an address of a variable.
    Use this as a black box in your code. */
	address_t translate_address(address_t addr);

	int _isDependent;

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
	int getThreadId() const;

	/**
	 * @brief  get the number of times the thread has been to running mode
	 * @return the number of times the thread has been to running mode
	 */
	int getRunningTimes() const;


	/**
	 * change the state of the thread
	 * @return
	 */
	void changeStatus(State newState);

	/**
	 * @brief get the state of the theard
	 * @return the state of the theard
	 */
	State getStatus() const;

	/**
	 * get the thread evironment
	 * @return the thread evironment
	 */
	sigjmp_buf* getEnvironment() ;

	/**
	 * @brief get if the thread is dependent on another thread
	 * @return return if the thread is dependent on another thread
	 */
	int getDependency();

	/**
	 * set the dependency data member
	 * @param isDependent
	 */
	void setDependency(int isDependent);


};

#endif //OS_EX2_THREAD_H
