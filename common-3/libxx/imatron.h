/* imatron.h - Imatronic message constants */
#ifndef _IMATRON_H
#define _IMATRON_H

/* Imatronic NC unit messages */
/******************************/

/* Formats of messages received via serial line.  */
/* Received message indices */
#define IM_REC_STX			0
#define IM_REC_REPLY		1
#define IM_REC_DLE			2
#define IM_REC_NAK			3
/* Other codes */
#define IM_FRAME_ERROR		4	/* frame error received from NC unit */
#define IM_TO_WITHIN		5	/* timeout within message from NC unit */
/* Codes only used in message communication */
#define IM_DATA				6
#define IM_STAT				7

/* Formats of messages sent via serial line.
 * These codes are used for communication between the main module
 * and the Imatronic module.
 */
/* Sent message indices */
#define IM_SND_STX			0
#define IM_SND_DATAX		1
#define IM_SND_DATAB		2
#define IM_SND_DLE			3
#define IM_SND_NAK			4

/* Status errors which can be returned to Main Module by the
 * Imatronic Management Module.
 */
#define IM_STAT_OK		0	/* OK, Prog/Dist sent and correctly acknowledged */
#define IM_STAT_TIMING	1	/* Last transmission is still in progress */
#define IM_STAT_NORESP	2	/* NC unit fails to respond to initial ENQ */
#define IM_STAT_TIMEOUT	3	/* Failed to transfer message within the timeout */
#define IM_STAT_BUSY	4	/* NC unit signals that it is busy (NAK) */
#define IM_STAT_FORMAT	5	/* NC unit signals Prog/Dist format error */
#define IM_STAT_TIMEIN	6	/* Timeout within message */
#define IM_STAT_ERROR	7	/* Other error (repeated frame errors received) */

#endif	/* _IMATRON_H */
