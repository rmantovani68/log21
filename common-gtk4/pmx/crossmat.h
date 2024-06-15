/* crossmat.h - crossmatic message constants */
#ifndef _CROSSMAT_H
#define _CROSSMAT_H

/* Crossmatic NC unit messages */
/*******************************/

/* Formats of messages received via serial line.
 * These codes are also sent by the Crossmatic receive module to the
 * Crossmatic management module on receiving a message.
 */
#define BS_REC_XON			0	/* code: ACK XON (XON = DC1) */
#define BS_REC_XOFF			1	/* code: NAK */
/* 7 bytes in length */
#define BS_REC_STAT15		2	/* code: BEL 1 5 STX 0 0 ETX */
#define BS_REC_STAT04		3	/* code: BEL 0 4 STX 0 0 ETX */
#define BS_BAD_PROGRAMMA	4	/* code: BEL 1 5 STX 1 1 ETX */
#define BS_BAD_DISTINTA		5	/* code: BEL 0 4 STX 1 1 ETX */
/* Other codes */
#define BS_FRAME_ERROR		6	/* frame error received from NC unit */
#define BS_TO_WITHIN		7	/* timeout within message from NC unit */

/* The above codes are also used to indicate the type of message in
 * a message queue (Crossmat code). In addition, the following 2
 * Crossmat codes are used:
 */
#define BS_PROGRAMMA		8	/* reception of Programma or status */
#define BS_DISTINTA			9	/* reception of Programma or status */

/* Formats of messages sent via serial line.
 * These codes are used for communication between the main module
 * and the Crossmatic module.
 */
/* 2 bytes in length */
#define BS_SND_ENQ			0	/* code: ENQ 1 */
/* variable length */
#define BS_SND_PROG			1	/* code: BEL 1 5 ..... ETX */
#define BS_SND_DIST			2	/* code: BEL 0 4 ..... ETX */

/* Status errors which can be returned to Main Module by the
 * Crossmatic Management Module. These are sent as appropriate with
 * Crossmat codes of BS_PROGRAMMA or BS_DISTINTA.
 */
#define BS_STAT_OK		0	/* OK, Prog/Dist sent and correctly acknowledged */
#define BS_STAT_TIMING	1	/* Last transmission is still in progress */
#define BS_STAT_NORESP	2	/* NC unit fails to respond to initial ENQ */
#define BS_STAT_TIMEOUT	3	/* Failed to transfer message within the timeout */
#define BS_STAT_BUSY	4	/* NC unit signals that it is busy (NAK) */
#define BS_STAT_FORMAT	5	/* NC unit signals Prog/Dist format error */
#define BS_STAT_TIMEIN	6	/* Timeout within message */
#define BS_STAT_ERROR	7	/* Other error (repeated frame errors received) */

#endif	/* _CROSSMAT_H */
