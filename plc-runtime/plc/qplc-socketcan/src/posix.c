/*
* @(#) posix.c 1.1 Thu Jul 10 10:52:42 MET DST 1997
*
* Questo file contiene le primitive POSIX utilizzate da qplc.
* Sarebbe bello poterle sempre usare, se alcune di esse non sparissero
* togliendo "streams" !
*
* 30/10/96 GG 1.0 Prima stesura.
* 10/07/97 GG 1.1 Aggiunto un piccolo cast. Aggiunta la funzione "posix_npoll"
*             per seguire piu` canali.
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
// #include <stropts.h>
#include <poll.h>

#include "glue.h"

#define MAX_FD 16

int posix_poll(int fd, long to)
{
struct pollfd fdlist;

	fdlist.fd = fd;
	fdlist.events = POLLIN;
	return poll(&fdlist,1,(int)to);
}

int posix_npoll(int *fds, int *resfds, int nfd, long to)
{
int n,rv;
struct pollfd fdlist[MAX_FD];

	for (n = 0; n < nfd; ++n, ++fds) {
		fdlist[n].fd = *fds;
		fdlist[n].events = POLLIN;
	}

	rv = n = poll(fdlist,nfd,(int)to);

	while (n > 0 && nfd--) {
		if (fdlist[nfd].revents & POLLIN) {
			*(resfds++) = nfd;
			--n;
		}
	}

	return rv;
}

