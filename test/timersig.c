#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)
pthread_t t_id;

print_siginfo(siginfo_t *si)
{
	timer_t *tidp;
	int or;

	tidp = si->si_value.sival_ptr;

	printf("    sival_ptr = %p; ", si->si_value.sival_ptr);
	printf("    *sival_ptr = 0x%lx\n", (long) *tidp);

	or = timer_getoverrun(*tidp);
	if (or == -1)
		errExit("timer_getoverrun");
	else
		printf("    overrun count = %d\n", or);
}

void printf_tid_pid(const char *s)
{
	pid_t pid;
	pthread_t tid;
	pid = getpid();
	tid = pthread_self();
	printf("%s pid %u tid %u (0x%x)\n",s,(unsigned)pid,(unsigned)tid,(unsigned)tid);

}

void *thread_fuction2(void *arg)
{
	int l;
	sigset_t mask;

	printf_tid_pid("new thread: ");
	l = sleep(5);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
		errExit("sigprocmask");

	while (1) {
		l = sleep(5);
		printf("sleep timeout %d\n", l);
	}
	return ((void*)0);
}

static void handler()
{
	printf("timer out \n");
}

void *thread_fuction1(void *arg)
{

	struct sigevent evp;
	struct itimerspec ts;
	timer_t timer;
	int ret;

	evp.sigev_value.sival_ptr = &timer;
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;
	signal(SIGUSR1, handler);

	ret = timer_create(CLOCK_REALTIME, &evp, &timer);
	if( ret )
		perror("timer_create");

	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 5000000;
	ts.it_value.tv_sec = 1;
	ts.it_value.tv_nsec = 0;

	ret = timer_settime(timer, 0, &ts, NULL);
	if( ret )
		perror("timer_settime");
}

int main(int argc,int argv)
{
	int err;
	struct sigevent sev;
	struct sigaction sa;
	sigset_t mask;

	err = pthread_create(&t_id,NULL,thread_fuction1,NULL);
	if(err != 0 )
	{
		printf("create thread fail: %s\n",strerror(err));
	}

	err = pthread_create(&t_id,NULL,thread_fuction2,NULL);
	if(err != 0 )
	{
		printf("create thread fail: %s\n",strerror(err));
	}
	printf_tid_pid("main thread");

	while (1);
	return 0;
}
