/*
*	File sockets.h
*
*	Gestione sockets
*
* rm 17-11-2000 : ported from xnc library (courtesy of Maio)
*                 files : ca_sock.c ca_sckm.c ca_sckm0.c ca_sckm1.c
*
*/
#define BZERO	'\0';

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define BOOL_TYPE_DEFINED
#endif
#ifndef USHORT_TYPE_DEFINED
typedef unsigned short USHORT;
#define USHORT_TYPE_DEFINED
#endif
#ifndef ULONG_TYPE_DEFINED
typedef unsigned long ULONG;
#define ULONG_TYPE_DEFINED
#endif
#ifndef UINT_TYPE_DEFINED
typedef unsigned int UINT;
#define UINT_TYPE_DEFINED
#endif
#ifndef MIN_MAX_DEFINED
#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))
#endif
/*
*	File descriptor chiuso
*/
#define ID_SOCK_NO_FD			-1
/*
*	Numero massimo di client che gestisce il server (32)
*/
#define ID_MAX_FD					NFDBITS
typedef struct
{
	int			nFd;
#ifdef ELIMINATO
	XtInputId	FdInputId;
#endif
	void		*ptFather;
}	SCKTSRVRFD, *PTSCKTSRVRFD;

typedef struct
{
	int       nSocket;
	fd_set    Canali;
	BOOL      (*ptFunz)();
	void      *ClientData;
#ifdef ELIMINATO
	Widget    hWidget;
	XtInputId	SocketInputId;
#endif
	SCKTSRVRFD	ScktSrvrFd[ID_MAX_FD+1];
}	SCKTSRVR, *PTSCKTSRVR;


int		xncScktSrvrGetMaxFD		(fd_set);
int		xncScktSrvrRecv			(int *, char *, int, ULONG);
BOOL	xncScktSrvrSend			(int *, char *, int);
BOOL	xncScktSrvrLoop			(char *, char *, int, BOOL (*)(), void *);
PTSCKTSRVR	xncScktSrvrOpen		(char *, char *, int);
BOOL	xncScktSrvrGest			(PTSCKTSRVR, BOOL (*)(), void *, ULONG);
#ifdef ELIMINATO
void	xncScktSrvrFdInput		(PTSCKTSRVRFD, int *, XtInputId *);
void	xncScktSrvrSocketInput	(PTSCKTSRVR, int *, XtInputId *);
BOOL	xncScktSrvrAddInput		(Widget, PTSCKTSRVR, BOOL (*)(int *, void *, void *), void *);
#endif
BOOL	xncGetMyName			(char *);
BOOL	xncScktIsReady		(int *);
int		xncScktRecv				(int *, char *, int, ULONG);
BOOL	xncScktSend				(int *, char *, int);
BOOL	xncScktClose			(int *);
BOOL	xncScktOpen				(char *, char *, int, char *, int *);


