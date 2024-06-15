
/*
* @(#) glue.h 1.1 Thu Jul 10 12:21:49 MET DST 1997
*
* Definizioni delle primitive Posix e corrispondenti SysV.
*
* 30/10/96 GG 1.0 Prima stesura.
* 10/07/97 GG 1.1 Aggiunta gestione delle funzioni *_npoll.
*/

#include "glue.h"

int (*glue_poll)(int fd, long to) = sysv_poll;
int (*glue_npoll)(int *fds, int *rfds, int nfd, long to) = sysv_npoll;

void glue_init(int posix)
{
	if (posix) {
		glue_poll = posix_poll;
		glue_npoll = posix_npoll;
	}
	else {
		glue_poll = sysv_poll;
		glue_npoll = sysv_npoll;
	}
}

