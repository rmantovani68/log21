/* mq.h -- header file for mq_server.c and mq_client.c */
#ifndef _MQ_H
#define _MQ_H

#define MSGSZ		256
#define MSGQNAME	1234

#define MSGTYPEALL	0
#define MSGTYPE1	1
#define MSGTYPE2	2

/* Message structure */
typedef struct {
	long mtype;
	char mtext[MSGSZ];
} msgbuf;

#endif	/* _MQ_H */

