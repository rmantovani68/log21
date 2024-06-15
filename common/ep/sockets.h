/**
* @file sockets.h
*
* Gestione sockets
*
* rm 17-11-2000 : ported from  library (courtesy of Maio)
*                 files : ca_sock.c ca_sckm.c ca_sckm0.c ca_sckm1.c
*
*/
#define BZERO    '\0';

#ifndef    FALSE
#define    FALSE    (0)
#endif

#ifndef    TRUE
#define    TRUE    (!FALSE)
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
#define max(a,b)    ((a)>=(b)?(a):(b))
#define min(a,b)    ((a)<=(b)?(a):(b))
#endif

/*
*    File descriptor chiuso
*/
#define ID_SOCK_NO_FD            -1
/*
*    Numero massimo di client che gestisce il server (32)
*/
#define ID_MAX_FD                    NFDBITS

typedef struct
{
    int nFd;
    void *ptFather;
} SCKTSRVRFD, *PTSCKTSRVRFD;

typedef struct
{
    int nSocket;
    fd_set Canali;
    ep_bool_t (*ptFunz)();
    void *ClientData;
    SCKTSRVRFD    ScktSrvrFd[ID_MAX_FD+1];
} SCKTSRVR, *PTSCKTSRVR;


int ScktSrvrGetMaxFD(fd_set);
int ScktSrvrRecv(int *, char *, int, ULONG);
ep_bool_t ScktSrvrSend(int *, char *, int);
ep_bool_t ScktSrvrLoop(char *, char *, int, ep_bool_t (*)(), void *);
PTSCKTSRVR ScktSrvrOpen(char *, char *, int);
ep_bool_t ScktSrvrGest(PTSCKTSRVR, ep_bool_t (*)(), void *, ULONG);
ep_bool_t GetMyName (char *);
ep_bool_t ScktIsReady (int *);
int ScktRecv (int *, char *, int, ULONG);
ep_bool_t ScktSend (int *, char *, int);
ep_bool_t ScktClose (int *);
ep_bool_t ScktOpen (char *, char *, int, char *, int *);


