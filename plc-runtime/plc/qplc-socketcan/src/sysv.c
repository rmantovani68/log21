/*
* @(#) sysv.c 1.2 Fri Jul 28 17:00:24 MET DST 2000
*
* Questo file isola alcune "stranezze" del sistema di sviluppo di Lynx.
* Poiche` si sta compilando in modalita` POSIX, conviene chiudere qui le cose
* che a POSIX non piacciono.
*
* 30/10/96 GG 1.0 Prima stesura.
* 10/07/97 GG 1.1 Piccola modifica per realizzare attese infinite con to == -1.
*             Aggiunta la funzione "sysv_npoll" per seguire piu` canali.
* 28/07/00 GG 1.2 Aggiustamenti per compilazione su Linux.
*/

/* Truccaccio maledetto. */

#ifdef Linux
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#else

#ifdef _POSIX_SOURCE
#  define __tmp__
#  undef _POSIX_SOURCE
#endif
#include <stdio.h>
#include <types.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#ifdef __tmp__
#  define _POSIX_SOURCE
#  undef __tmp__
#endif

#endif

#include "glue.h"

int sysv_poll(int fd, long to)
{
fd_set fdlist;
struct timeval tm,*ptm;

	FD_ZERO(&fdlist);
	FD_SET(fd,&fdlist);
	if (to == -1)
		ptm = (struct timeval *) 0;
	else {
		tm.tv_sec = to / 1000L;
		tm.tv_usec = (to % 1000L) * 1000;
		ptm = &tm;
	}

	return select(fd+1,&fdlist,(fd_set *)0,(fd_set *)0, ptm);
}

int sysv_npoll(int *fds, int *resfds, int nfd, long to)
{
fd_set fdlist;
struct timeval tm,*ptm;
int rv,n,maxfd,fd,*pfds;

	FD_ZERO(&fdlist);
	for (n = nfd, maxfd = 0, pfds = fds; n--; ++pfds) {
		fd = *pfds;
		FD_SET(fd,&fdlist);
		if (fd > maxfd)
			maxfd = fd;
	}

	if (to == -1)
		ptm = (struct timeval *) 0;
	else {
		tm.tv_sec = to / 1000L;
		tm.tv_usec = (to % 1000L) * 1000;
		ptm = &tm;
	}

	rv = n = select(maxfd + 1,&fdlist,(fd_set *)0,(fd_set *)0, ptm);

#if 0
	printf("select = %d\n",n);
	if (n < 0) {
		perror("select");
		printf("maxfd = %d\nfdlist = 0x%08lx\n",
		       maxfd,*(long *)&fdlist);
		for (n = 0; n < nfd; ++n) {
			printf("fds[%d] = %d\n",fds[n]);
		}
		exit(1);
	}
#endif

	while (n > 0 && nfd--) {
		if (FD_ISSET(fds[nfd],&fdlist)) {
			*(resfds++) = nfd;
			--n;
		}
	}

	return rv;
}

