/* msq.h: File containing message queue definitions */
#ifndef _MSQ_H
#define _MSQ_H

/* Lengths of message structure fields */
#define ADR_LEN			2
#define ACK_LEN			1
#define STAT_LEN		2
#define CLASS_LEN		2
#define CODE_LEN		2
#define SER_LEN			4
#define HDR_LEN			(2 * ADR_LEN + STAT_LEN + ACK_LEN +\
							CLASS_LEN + CODE_LEN + SER_LEN)
#define INFO_LEN		4096	/* max. possible length */

/* Message class */
#define SPECIFIC_CLASS	1		/* specific message */
#define STRING_CLASS	2			/* message contains string */
#define SHARED_CLASS	3			/* message contains shared mem. handle */
#define FILE_CLASS		4			/* message contains file name */

/* Predefined SPECIFIC_CLASS message codes */
enum
{
	MSG_ACK=1,					/* Messaggio di OK */
	MSG_STARTMOD,				/* Avvia un modulo */
	MSG_ENDMOD,					/* Kill Software di un modulo */
	MSG_MODABORT,				/* Il modulo che lo ha inviato e' ABORTITO */
	MSG_RESET,					/* Reset di un modulo */
	MSG_KEYBD,					/* Carattere da tastiera */
	MSG_XCODE,					/* Messaggio con codice extra */

	MSG_OFFSET, 				/* Offset di partenza per codici messaggi **
								** privati di ogni applicazione che usa   **
								** le librerie delle code messaggi        */
};

/* Structure of a message, as it is sent */
typedef struct {
	char dest[ADR_LEN];		/* Module to which message is to be sent */
	char srce[ADR_LEN];		/* Module from which message originated */
	char ack[ACK_LEN];		/* =0: no confirm required, !=0, yes */
	char stat[STAT_LEN];	/* Status of module sending message */
	char _class[CLASS_LEN];/* Class of message (SPECIFIC_CLASS etc.) */
	char code[CODE_LEN];	/* Code of message */
	char serial[SER_LEN];	/* Serial no. of message */
	char info[INFO_LEN];	/* message information (not all sent) */
} MSG;

/* Structure for packing/unpacking of message header */
typedef struct {
	int dest;				/* Module to which message is to be sent */
	int srce;				/* Module from which message originated */
	int ack;				/* =0: no confirm required, !=0, yes */
	int stat;				/* Status of module sending message */
	int _class;			/* Class of message (SPECIFIC_CLASS etc.) */
	int code;				/* Code of message */
	int serial;			/* Serial no. of message */
} MSGHDRINT;

#endif	/* _MSQ_H */
