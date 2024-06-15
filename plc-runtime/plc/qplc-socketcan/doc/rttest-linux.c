

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <pthread.h>

int main(int argc, char **argv)
{
int val = 0;
pthread_t th = pthread_self();
struct sched_param sch;
int policy;
struct itimerval v;
int sig;
sigset_t sm;
int c = 0;

	if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
		perror("mlockall");
		exit(1);
	}

	pthread_getschedparam(th,&policy,&sch);
	sch.sched_priority = 20;
	if (pthread_setschedparam(th,SCHED_FIFO,&sch)) {
		perror("setschedparam");
		exit(1);
	}

	signal(SIGALRM,SIG_IGN);

	sigemptyset(&sm);
	sigaddset(&sm,SIGALRM);

	v.it_value.tv_sec = 0;
	v.it_value.tv_usec = 100000;
	v.it_interval = v.it_value;
	if (setitimer(ITIMER_REAL,&v,NULL) < 0) {
		perror("setitimer");
		exit(1);
	}

	if (iopl(3) < 0) {
		perror("iopl");
		exit(1);
	}

	for (;;) {
		/* printf("%d\r",++c); fflush(stdout); */
		outb(val,0x378);
		val ^= 0xFF;
		if (sigwait(&sm,&sig) < 0) {
			perror("sigwait");
			exit(0);
		}
	}
}

