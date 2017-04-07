#include <iostream>
#include "Thread.h"
#include "uthreads.h"
/*
 * sigsetjmp/siglongjmp demo program.
 * Hebrew University OS course.
 * Questions: os@cs.huji.ac.il
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define SECOND 1000000
#define STACK_SIZE 4096

char stack1[STACK_SIZE];
char stack2[STACK_SIZE];

sigjmp_buf env[2];

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
			"rol    $0x11,%0\n"
	: "=g" (ret)
	: "0" (addr));
	return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int addre#ifdef __x86_64__
/* code for 64 bit Intel arch */ss_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif

void f(void);
void g(void);

Thread t1= Thread(1,f,STACK_SIZE);
Thread t2= Thread(1,g,STACK_SIZE);

void switchThreads(Thread &curT,Thread& t)
{
	//static int currentThread = 0;

	int ret_val = sigsetjmp(*(curT.getEnvironment()),1);
	printf("SWITCH: ret_val=%d\n", ret_val);
	if (ret_val == 1) {
		return;
	}
	//currentThread = 1 - currentThread;
	siglongjmp(*(t.getEnvironment()),1);

}

void f(void)
{
	static c= 0;
	int i = 0;
	int j=5;
	while(1){
		++i;
		printf("in f (%d)\n",&i);
		printf("in f (%d)\n",&j);
		if (i % 3 == 0) {
			printf("f: switching\n");
			switchThreads(t1,t2);
		}
		c+=1;
		if(c=10){
			struct sigaction sa;
			struct itimerval timer;

			// Install timer_handler as the signal handler for SIGVTALRM.
			sa.sa_handler = &timer_handler;
			if (sigaction(SIGVTALRM, &sa,NULL) < 0) {
				printf("sigaction error.");
			}

			// Configure the timer to expire after 1 sec... */
			timer.it_value.tv_sec = 5;		// first time interval, seconds part
			timer.it_value.tv_usec = 0;		// first time interval, microseconds part

			// configure the timer to expire every 3 sec after that.
			timer.it_interval.tv_sec = 1;	// following time intervals, seconds part
			timer.it_interval.tv_usec = 0;	// following time intervals, microseconds part

			// Start a virtual timer. It counts down whenever this process is executing.
			if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
				printf("setitimer error.");
			}

			for(;;) {
				if (gotit) {
					printf("Got it!\n");
					gotit = 0;
				}
			}
		}
		usleep(SECOND);
	}
}

void g(void)
{

	int i = 0;
	while(1){
		++i;
		printf("in g (%d)\n",&i);
		if (i % 5 == 0) {
			printf("g: switching\n");
			switchThreads(t2,t1);
		}
		usleep(SECOND);
	}
}

void setup(void)
{
	address_t sp, pc;

	//sp = (address_t)stack1 + STACK_SIZE - sizeof(address_t);
	pc = (address_t)f;
	sigsetjmp(env[0], 1);
	(env[0]->__jmpbuf)[JB_SP] = translate_address(sp);
	(env[0]->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&env[0]->__saved_mask);

	//sp = (address_t)stack2 + STACK_SIZE - sizeof(address_t);
	pc = (address_t)g;
	sigsetjmp(env[1], 1);
	(env[1]->__jmpbuf)[JB_SP] = translate_address(sp);
	(env[1]->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&env[1]->__saved_mask);
}

int main(void)
{
	//uthread_init(34);
	//uthread_spawn(&uthread_init);

	//setup();
	printf("in f (%d)\n",&f);
	printf("in f (%d)\n",&g);

	siglongjmp(*(t1.getEnvironment()), 1);
	return 0;
}