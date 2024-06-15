/* msq_lib.h - header file for message queue library */
#ifndef _MSQ_LIB_H
#define _MSQ_LIB_H

#include	"msq.h"	/*	Vedere se e' meglio inserire il file qui' */

/* Max. serial number */
#define MAX_SERIAL		0xffff	/* 4 hexadecimal places */

typedef struct {
	long mtype;
	char mtext[INFO_LEN];
} MSGBUF;

/* Function prototypes */
extern int CreateMsgQ(int, int);
extern void DeleteMsgQ(int);
extern int OpenMsgQ(int);
extern int PackMsg(char *, int, int, int, int, int, int, char *, int);
extern int PackStructMsg(char *, MSGHDRINT *, char *, int);
extern int SendMsg(int, char *, int);
extern int SendMsgNoWait(int, char *, int);
extern int UnpackMsg(char *, int *, int *, int *,
					int *, int *, int *, int *, char *, int);
extern int UnpackStructMsg(char *, MSGHDRINT *, char *, int);
extern int ReceiveMsg(int, int *, char *);
extern int ReceiveMsgNoWait(int , int *, char *);
extern int ReceiveMsgTimed(int, int *, char *, int *);
extern int ExecModule(char *, ...);
extern int SendSpecificMsg(int, int, int, int, char *);
extern int SendSpecificMsgNoWait(int , int , int , int , char *,int ,int );
extern int SendSpecificMsgL(int, int, int, int, char *, int);
extern int SendSpecificMsgA(int, int, int, int, char *, ...);
extern int SendFileMsg(int, int, int, int, char *);
extern int SendAckMsg(int, int, int, int);
extern int GetSerial(void);
extern void PutField(char *, int, int);
extern int GetField(char *, int);
extern int PackInt (char *, int from, int);

#endif /* _MSQ_LIB_H */
