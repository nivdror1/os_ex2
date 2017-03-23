// a thread object

#ifndef OS_EX2_THREAD_H
#define OS_EX2_THREAD_H

class Thread{
private:
	/**
	int tid;


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
	const getThreadId();

	/**
	 * @brief  get the number of times the thread has been to running mode
	 * @return the number of times the thread has been to running mode
	 */
	const getQuanta();

	/**
	 * change the state of the thread
	 * @return
	 */
	changeStatus();

	/**
	 * @brief get the state of the theard
	 * @return the state of the theard
	 */
	const getStatus();

};

#endif //OS_EX2_THREAD_H
