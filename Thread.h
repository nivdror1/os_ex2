// a class of a  thread

#ifndef OS_EX2_THREAD_H
#define OS_EX2_THREAD_H

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


public:
	/**
	 * @brief a constructor
	 * @param tid the id of the thread
	 */
	Thread(int tid);

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

};

#endif //OS_EX2_THREAD_H
