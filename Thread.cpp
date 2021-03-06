//a instance of a thread




#include "Thread.h"

/**
 * @brief a constructor
 * @param tid the id of the thread
 */
Thread::Thread(int tid, void (*func)(void),int stackSize):_tid(tid),_runningTimes(0),_currState(Ready)
        ,_isDependent(-1) {
    _threadStack= new char[stackSize];
    _sp = (address_t)_threadStack + stackSize - sizeof(address_t);
    _pc = (address_t)func;
    sigsetjmp(_environment, 1);
    (_environment->__jmpbuf)[JB_SP] = translate_address(_sp); //todo to understand what the this does
    (_environment->__jmpbuf)[JB_PC] = translate_address(_pc);
    sigemptyset(&_environment->__saved_mask);
}

/**
 * a desctructor
 */
Thread::~Thread() {
    delete[] _threadStack;
}
/**
 * @brief get the thread id
 * @return thread id
 */
int Thread::getThreadId() const{
    return _tid;
}

/**
 * @brief  get the number of times the thread has been to running mode
 * @return the number of times the thread has been to running mode
 */
int Thread::getRunningTimes() const{
    return _runningTimes;
}


/**
 * change the state of the thread
 */
void Thread::changeStatus(State newState){
    _currState=newState;
    //if the thread is being the a state
    if(newState==Running) {
        _runningTimes+=1;
    }
}

/**
 * @brief get the state of the theard
 * @return the state of the theard
 */
State Thread::getStatus() const{
    return _currState;
}

/**
 * get the thread evironment
 * @return the thread evironment
 */
sigjmp_buf *Thread::getEnvironment() {
    return &_environment;
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

/**
 * @brief get if the thread is dependent on another thread
 * @return return if the thread is dependent on another thread
 */
int Thread::getDependency(){
    return _isDependent;
}

/**
 * set the dependency data member
 * @param isDependent
 */
void Thread::setDependency(int isDependent){
    _isDependent=isDependent;
}