/* nccom.h */
#ifndef _NCCOM_H_
#define _NCCOM_H_

/* Contains definitions for simulation
 * of the various devices
 */
#define MAX_NUM_MESSAGES	256
#define MAXLEN_START		80		/* max. length of start sequence */
#define RIDICULOUS_LEN		10000	/* max. possible length of a message */

/* Error codes for serial line routines */
#define TO_BEFORE		-1		/* timeout before reception */
#define TO_WITHIN		-2		/* timeout during reception */
#define FRAME_ERROR		-3		/* frame error during reception */
#define CHECKSUM_ERROR	-4		/* checksum error after reception */
#define LENGTH_ERROR	-5		/* check length error after reception */
#define CORRUPT_LENGTH	-6		/* incorrect check length (not number) */
#define INCONSIST_ERROR	-7		/* inconsistency in message reception */
#define WRITE_ERROR		-8		/* error in write call */
#define VER_ERROR1		-11		/* error in number of messages given */
#define VER_ERROR2		-12		/* error in start/end lengths */
#define VER_ERROR3		-13		/* error check lengths */
/* Where an error occurs, the message list contains one element:
 * the index of the current message which caused the error.
 */

struct msg_spec {
	char *msg_start;		/* message start string */
	int start_len;			/* start length to be checked exactly */
	int do_start_check;		/* check only for start char (but < start_len) */
	int start_char;			/* >0: start char to be checked for */
	int end_len;			/* >0: end length to be checked exactly */
	int do_end_check;		/* >0: check only for end char (but < end_len) */
	int end_char;			/* end char to be checked for (if any) */
	int do_checksum;		/* >0: add/read checksum after "end_char" */
	int do_checklen;		/* add/read check length just before "end_char" */
};
typedef struct msg_spec MSG_SPEC;

/* The general mode of processing for serial line routines is as follows:
 * 1) Send/read the start sequence, either as a fixed length sequence
 *		(start_len) or up to and including a given character 'start_char'.
 * 2) Send/read to the end of the message, either up to a total length
 *		(end_len) or up to and including a given character 'end_char'.
 * When using a check char, the sequence for start (start_len) is not
 * sent/checked character for character. However, checking for the
 * check char is only performed up to the corresponding length. After this,
 * it is assumed that the start sequence has finished.
 * The same applies to 'end_char', although in this case the message is
 * not sent/checked character by character.
 * The checksum and checklen are used as follows:
 * 1) Checksum: a XOR checksum is formed of all characters after 'start_char'
 * (or start_len) up to but not including 'end_char' (or end_len). This
 * checksum is added/read after 'end_char' (or end_len).
 * 2) Checklen: a BCD-coded length sequence (of length equal to the value
 *		of checklen) is inserted/read just before 'end_char' (or end_len).
 *		If a checksum is also used, it includes the length field.
 */

/* Function prototypes */
int VerSerMsg(MSG_SPEC *, int, int *);
int InitSer(char *szPort, int *fdp, int nBR, int nDB, int nP, int nSB, int oflag);
int InitSerHS(char *szPort, int *fdp, int nBR, int nDB, int nP, int nSB, int nHS, int oflag);
void CloseSer(void);
void XonXoff(int);
int RecSerMsg(int, char *, MSG_SPEC *, int *, int *, int *, int, int);
int SendSerMsg(int, char *, MSG_SPEC *, int *);
int FlushSer(int, int);
int ReadChar(int, char *, int);
int WriteChar(int, char);
void SetRxTimeout(int, int);

#endif /* _NCCOM_H_ */
