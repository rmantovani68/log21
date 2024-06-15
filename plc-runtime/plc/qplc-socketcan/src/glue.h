/*
* @(#) glue.h 1.1 Thu Jul 10 11:22:21 MET DST 1997
*
* Definizioni delle primitive Posix e corrispondebti SysV.
*
* 30/10/96 GG 1.0 Prima stesura.
* 10/07/97 GG 1.1 Aggiunte le funzioni "posix_npoll" e "sysv_npoll".
*/

#ifndef _GLUE_H_

/*
* Primitive per l'attesa temporizzata di dati da un file descriptor.
*/

int posix_poll(int fd, long to);
int sysv_poll(int fd, long to);
int posix_npoll(int *fds, int *rfds, int nfd, long to);
int sysv_npoll(int *fds, int *rfds, int nfd, long to);

extern int (*glue_poll)(int fd, long to);
extern int (*glue_npoll)(int fds[], int rfds[], int nfd, long to);

/*
* Funzione da utilizzare al posto di "poll" o "select".
*/

#define test_fd(fd,to) ((*glue_poll)((fd),(to)))
#define test_nfd(fds,rfds,nfd,to) ((*glue_npoll)((fds),(rfds),(nfd),(to)))

/*
* Funzione di inizializzazione della modalita` posix/sysv.
*/

void glue_init(int posix);

#define _GLUE_H_

#endif

