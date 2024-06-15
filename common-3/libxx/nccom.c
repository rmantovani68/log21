/* 								nccom.c*/
/* 	simple serial line routines */
/* */
/* 	-MC-	10.12.93	Modificata CloseSer non faceva flush e nemmeno close*/
/* */
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <setjmp.h>
#include <errno.h>
#include <termio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "nccom.h"

extern int sys_nerr;

#ifdef SCO
extern char **sys_errlist;
#endif

#ifdef TRACE
#include "trace.h"
#endif	/* TRACE */

#define TRACE_EXT	".trc"

static char sbuf[81];
static struct termio save_param;
static int fd;
static int port_to_close;

/* Holds long jump environment */
jmp_buf env;

/* Module-local function protypes */
static void alfun1();	/* requires empty argument lists */
static void alfun2();
static void pipeint();

/*****************************************************************************
  FUNCTION: int VerSerMsg(MSG_SPEC *mess, int num_msg, int *msg_err)
  PURPOSE: Verify a list of messages given in a message list
           Ensure that all messages have been verified using this
           routine before employing them in "RecSerMsg" or "SendSerMsg".
           This also sets up a routine to recognize a SIGPIPE.
  COMMENTS: (source nccom.c, library: pmx_lib.a)
  PARAMETERS: mess - list of message specs
              num_msg - no. of messages in list
              msg_err - if error: index of message containing error
  RETURNS: 0: all messages OK
           VER_ERROR1: incorrect no. of messages given (see nccom.h)
           VER_ERROR2: a message was incorrectly specified
 ****************************************************************************/
int VerSerMsg(MSG_SPEC *mess, int num_msg, int *msg_err)
{
	register	j;
	MSG_SPEC	*msg;

	if (num_msg <= 0 || num_msg > MAX_NUM_MESSAGES)
		return(VER_ERROR1);
	for (j=0; j<num_msg; j++) {
		*msg_err = j;
		msg = mess + j;
		if (msg->start_len > msg->end_len ||
			msg->start_len > MAXLEN_START ||
				msg->start_len < 0 || msg->end_len < 0)
			return(VER_ERROR2);
		if (msg->do_checklen < 0 || msg->do_checklen >= msg->end_len)
			return(VER_ERROR3);
	}
	/* Important if we are using a pipe */
	signal(SIGPIPE, pipeint);
	return(0);	/* OK */
}

/******************************************************************************
  FUNCTION: int InitSer(char *szPort, int *fdp, int nBR, int nDB, int nP, int nSB, int oflag)
  PURPOSE:  Initialize serial port
  COMMENTS: (source nccom.c, library: pmx_lib.a)
  PARAMETERS: szPort - "/dev/tty1a" etc.
              fdp - returned file descriptor
   BaudRate   nBR - 300, 600, 1200, 2400, 4800, 9600
   Data Bits  nDB - 6, 7, 8
   Parity     nP  - 0: none, 1: odd, 2: even
   Stop Bits  nSB - 1, 2
   Open Flag  oflag - O_RDONLY, O_WRONLY or O_RDWR (opt.: | O_NDELAY)
  RETURNS:
      0  : OK
     -1  : could not open port
     -2  : bad baudrate
     -3  : bad bits
     -4  : bad stop bits
     -5  : bad parity
     -6  : ioctl() failed
 *****************************************************************************/
int InitSer(char *szPort, int *fdp, int nBR, int nDB, int nP, int nSB, int oflag)
{
	char	*ptr;
	char	trace_name[81];
	int		baudrate;
	int		bits;
	int		parity;
	int		port;
	struct	termio	param;

	port = open(szPort, oflag);
	if (port < 0)
		return(-1);
	switch (nBR) {
	case 300:
		baudrate = B300;
		break;
	case 600:
		baudrate = B600;
		break;
	case 1200:
		baudrate = B1200;
		break;
	case 2400:
		baudrate = B2400;
		break;
	case 4800:
		baudrate = B4800;
		break;
	case 9600:
		baudrate = B9600;
		break;
	default:
		return(-2);
	}
	switch (nDB) {
	case 6:
		bits = CS6;
		break;
	case 7:
		bits = CS7;
		break;
	case 8:
		bits = CS8;
		break;
	default:
		return(-3);
	}
	switch (nSB) {
	case 1:
		break;
	case 2:
		bits |= CSTOPB;
		break;
	default:
		return(-4);
	}
	switch (nP) {
	case 0:
		parity = 0;
		break;
	case 1:
		parity = PARENB | PARODD;
		break;
	case 2:
		parity = PARENB;
		break;
	default:
		return(-5);
	}
	/* save previous settings of port */
	if (ioctl(port, TCGETA, &save_param) == -1)
		return(-6);
	/* Save terminal parameters */
	memcpy(&param, &save_param, sizeof(param));

/*
	param.c_cflag = (short)(parity | baudrate | bits | CREAD);
	param.c_cc[VMIN] = 1;
	param.c_cc[VTIME] = 1;
*/

	param.c_lflag &= ~(ISIG | ICANON | IEXTEN | XCASE | ECHO | ECHOE | ECHOK | ECHONL);
 	/*
  param.c_iflag |= IGNBRK | IGNPAR;
	*/
  param.c_iflag |= IGNBRK | IGNPAR | IXON | IXOFF;

	/*
	* da rimettere a posto (usare XonXoff() 
	param.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXOFF | IXANY);
	*/
	param.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXANY);

	param.c_oflag |= 0;
	param.c_oflag &= ~(OPOST);
	param.c_cflag &= ~(CBAUD | CSIZE | PARENB | PARODD);
	param.c_cflag |= (parity | baudrate | bits | CREAD | CLOCAL);

	param.c_cc[VMIN] = 1;
	param.c_cc[VTIME] = 0;


	if (ioctl(port, TCSETA, &param) == -1)
		return(-6);
	port_to_close++;

	/* Note file descriptor */
	fd = port;
	*fdp = port;
#ifdef TRACE
#ifdef ELIMINATO_RM
	/* Open a file to hold trace information */
	if (!(ptr = strrchr(szPort, '/')))
		ptr = szPort;
	else
		ptr++;	/* after last '/' */
	sprintf(trace_name, "%s%s", ptr, TRACE_EXT);
	if (open_trace(szPort, trace_name, TRACE_FILE_MAXSIZE))
		fprintf(stderr, "Cannot open trace file %s\n", trace_name);
#endif
#endif	/* TRACE */
	return(0);
}

/******************************************************************************
  FUNCTION: int InitSerHS(char *szPort, int *fdp, int nBR, int nDB, int nP, int nSB, int nHS, int oflag)
  PURPOSE:  Initialize serial port
  COMMENTS: (source nccom.c, library: pmx_lib.a)
  PARAMETERS: szPort - "/dev/tty1a" etc.
              fdp - returned file descriptor
   BaudRate   nBR - 300, 600, 1200, 2400, 4800, 9600
   Data Bits  nDB - 6, 7, 8
   Parity     nP  - 0: none, 1: odd, 2: even
   Stop Bits  nSB - 1, 2
   Handshake  nHS - 0:None , 1:CTS-RTS , 2:XON-XOFF
   Open Flag  oflag - O_RDONLY, O_WRONLY or O_RDWR (opt.: | O_NDELAY)
  RETURNS:
      0  : OK
     -1  : could not open port
     -2  : bad baudrate
     -3  : bad bits
     -4  : bad stop bits
     -5  : bad parity
     -6  : ioctl() failed
 *****************************************************************************/
int InitSerHS(char *szPort, int *fdp, int nBR, int nDB, int nP, int nSB, int nHS, int oflag)
{
	char	*ptr;
	char	trace_name[81];
	int		baudrate;
	int		bits;
	int		parity;
	int		port;
	struct	termio	param;

	port = open(szPort, oflag);
	if (port < 0)
		return(-1);
	switch (nBR) {
	case 300:
		baudrate = B300;
		break;
	case 600:
		baudrate = B600;
		break;
	case 1200:
		baudrate = B1200;
		break;
	case 2400:
		baudrate = B2400;
		break;
	case 4800:
		baudrate = B4800;
		break;
	case 9600:
		baudrate = B9600;
		break;
	default:
		return(-2);
	}
	switch (nDB) {
	case 6:
		bits = CS6;
		break;
	case 7:
		bits = CS7;
		break;
	case 8:
		bits = CS8;
		break;
	default:
		return(-3);
	}
	switch (nSB) {
	case 1:
		break;
	case 2:
		bits |= CSTOPB;
		break;
	default:
		return(-4);
	}
	switch (nP) {
	case 0:
		parity = 0;
		break;
	case 1:
		parity = PARENB | PARODD;
		break;
	case 2:
		parity = PARENB;
		break;
	default:
		return(-5);
	}
	/* save previous settings of port */
	if (ioctl(port, TCGETA, &save_param) == -1)
		return(-6);
	/* Save terminal parameters */
	memcpy(&param, &save_param, sizeof(param));

/*
	param.c_cflag = (short)(parity | baudrate | bits | CREAD);
	param.c_cc[VMIN] = 1;
	param.c_cc[VTIME] = 1;
*/

	param.c_lflag &= ~(ISIG | ICANON | IEXTEN | XCASE | ECHO | ECHOE | ECHOK | ECHONL);
 	/*
  param.c_iflag |= IGNBRK | IGNPAR;
	*/
  param.c_iflag |= IGNBRK | IGNPAR | IXON | IXOFF;

	/*
	* da rimettere a posto (usare XonXoff() 
	param.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXOFF | IXANY);
	*/
	param.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXANY);

	param.c_oflag |= 0;
	param.c_oflag &= ~(OPOST);
	param.c_cflag &= ~(CBAUD | CSIZE | PARENB | PARODD);
	switch(nHS){
		case 0:
			/* NONE */
			param.c_cflag |= (parity | baudrate | bits | CREAD | CLOCAL);
		break;
		case 1:
			/* RTS/CTS */
			param.c_cflag |= (parity | baudrate | bits | CREAD );
		break;
	}

	param.c_cc[VMIN] = 1;
	param.c_cc[VTIME] = 0;


	if (ioctl(port, TCSETA, &param) == -1)
		return(-6);
	port_to_close++;

	/* Note file descriptor */
	fd = port;
	*fdp = port;
#ifdef TRACE
#ifdef ELIMINATO_RM
	/* Open a file to hold trace information */
	if (!(ptr = strrchr(szPort, '/')))
		ptr = szPort;
	else
		ptr++;	/* after last '/' */
	sprintf(trace_name, "%s%s", ptr, TRACE_EXT);
	if (open_trace(szPort, trace_name, TRACE_FILE_MAXSIZE))
		fprintf(stderr, "Cannot open trace file %s\n", trace_name);
#endif
#endif	/* TRACE */
	return(0);
}

/******************************************************************************
  FUNCTION: void CloseSer()
  PURPOSE: Restore serial port to its previous settings
  COMMENTS: (source nccom.c, library: pmx_lib.a)
******************************************************************************/
void CloseSer()
{
	if (port_to_close)
	{
		ioctl(fd, TCFLSH, 2);
		ioctl(fd, TCSETA, &save_param);
		close(fd);
	}
}

/******************************************************************************
  FUNCTION: void XonXoff(int flag)
  PURPOSE: Enable/Disable Xon Xoff protocol 
  COMMENTS: (source nccom.c, library: pmx_lib.a)
******************************************************************************/
void XonXoff(int flag)
{
	struct	termio	param;

	/* Verifica che la porta sia stata aperta */
	if (port_to_close)
	{
		ioctl(fd, TCGETA, &param);

		if (flag)
			param.c_iflag |= IXON | IXOFF;
		else
			param.c_iflag &= ~(IXON | IXOFF);

		ioctl(fd, TCSETA, &param);
	}
}

/******************************************************************************
  FUNCTION: void SetRxTimeout(int vmin, int vtime)
  PURPOSE:  Set Timeout for RX
  COMMENTS: (source nccom.c, library: pmx_lib.a)
******************************************************************************/
void SetRxTimeout(int vmin, int vtime)
{
	struct	termio	param;

	/* Verifica che la porta sia stata aperta */
	if (port_to_close) {
		ioctl(fd, TCGETA, &param);

		param.c_cc[VMIN] = (unsigned char) vmin;
		param.c_cc[VTIME] = (unsigned char) vtime;

		ioctl(fd, TCSETA, &param);
	}
}

/*****************************************************************************
  FUNCTION: int RecSerMsg(int fd, char *buf, MSG_SPEC *mess, int *msg_list,
               int *num_msg, int *msg_len, int to_before, int to_within)
  PURPOSE: Receive a message via a stream which conforms to a message in
           a given message list
  COMMENTS: (source nccom.c, library: pmx_lib.a)
  PARAMETERS:
      fd - file descriptor of stream
      buf - buffer for received data (must be long enough)
      mess - list of message specifications (MSG_SPEC: see nccom.h)
      msg_list - list of indices of messages from list
      num_msg - num of still active entries in msg_list (may be zero)
      msg_len - length in bytes of received message
      to_before - timeout before reception starts (in seconds)
      to_within - timeout between bytes during reception (in seconds)
  RETURNS: >=0: no. of message as given in list
           <0: one of the following errors (see nccom.h)
             TO_BEFORE: timeout before start of reception
             TO_WITHIN: timeout between bytes during reception
             FRAME_ERROR: frame or read error
             CHECKSUM_ERROR: checksum error
             LENGTH_ERROR: check length error
             CORRUPT_LEN: corrupted length field
             INCONSIST_ERROR: mutually inconsistent message
                            reception conditions
	It is strongly recommended that the list of message specifications be
	verified (using "VerSerMsg()") before first use.
****************************************************************************/
int RecSerMsg(int fd, char *buf, MSG_SPEC *mess, int *msg_list, int *num_msg,
				int *msg_len, int to_before, int to_within)
{
	register	i;
	register	j;
	register	c, k;
	char		ch;
	int			rc;				/* return code */
	int			still_valid;	/* index of next valid message */
	int			no_start_seq;	/* no. of messages without start sequence */
	int			validated;		/* message validated this time round */
	int			stop_start;		/* a message has fulfilled start criterion */
	int			stop_end;		/* a message has fulfilled end criterion */
	int			prend_char;		/* preceding end char (if given) */
	int			end_char;		/* end char (if given) */
	int			checksum;		/* generated checksum byte */
	int			checklen;		/* generated checksum */
	char		checklen_char[6]; /* checksum sequence read */
	int			checksum_start;
	int			trigger_index;
	MSG_SPEC	*msg;

	*msg_len = 0; /* total length of received message */
	/* Set up timeout signal trap for before start of message */
	signal(SIGALRM, SIG_IGN);
	/* Long jump after timeout detected */
	switch (setjmp(env)) {
	case 0:
		break;
	case 1:
		/* Remove timeout signal trap */
		signal(SIGALRM, SIG_IGN);
		alarm(0); /* cancel alarm */
#ifdef TRACE
		trace_out_str(" [timeout before]", 0);
		endtime();
#endif	/* TRACE */
		return(TO_BEFORE);
	case 2:
		/* Remove timeout signal trap */
		signal(SIGALRM, SIG_IGN);
		alarm(0); /* cancel alarm */
#ifdef TRACE
		trace_out_str(" [timeout within]", 0);
		endtime();
#endif	/* TRACE */
		return(TO_WITHIN);
	}
	if (to_before > 0) {
		signal(SIGALRM, alfun1);
		alarm(to_before);
	}

	/* Check start sequences of all messages against each character,
	 * eliminating all those messages which don't pass
	 */
	for (i=0; i<RIDICULOUS_LEN; i++) {
#ifdef TRACE
		if (i == 0)
			trace_out_str("<<<Receive Active<<<", 1);
#endif	/* TRACE */
		trigger_index = -1;
		stop_start = 1;
		/* Check if any remaining messages still
		 * require checking for a start sequence
		 */
		for (j=0; j<*num_msg; j++) {
			msg = mess + msg_list[j];
			if (msg->do_start_check || msg->start_len) {
				stop_start = 0;
				break;
			}
		}
		/* If they all don't, we can skip the rest of the start */
		if (stop_start)
			break;
		/* Continue with checking start sequence */
		buf[i] = '\0';

		rc = ReadChar(fd, &buf[i], i == 0);

		buf[i+1] = '\0'; /* make it a string (useful for analysis) */
		/* We have received a char */
		if (rc == 1) {
			/* Renew timeout within message */
			/* Remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
			if (to_within) {
				signal(SIGALRM, alfun2);
				alarm(to_within);
			}
		}
		else {
			/* Remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
#ifdef TRACE
			endtime();
#endif	/* TRACE */
			return(FRAME_ERROR);
		}
		
		j = 0;
		still_valid = 0;	/* no. of messages still valid */
		no_start_seq = 0;	/* no. of messages without start sequence */
		/* Check all (start sequence of) messages against this character */
		do {
			validated = 0;	/* a message not detected yet */
			/* Compact remaining list of messages */
			msg_list[still_valid] = msg_list[j];
			msg = mess + msg_list[j];
			/* Only if the sequence ends with a specified character */
			if (msg->do_start_check) {
				if (msg->start_char == buf[i]) {
					 /* We end the start sequence, even if
					 * we were still verifying all chars
					 * precisely within the start sequence.
					 */
					stop_start++;
					if (trigger_index == -1)
						/* Remember it was this message */
						trigger_index = still_valid;
					still_valid++;
					validated = 1;
				}
				/* We can only validate this character if we
				 * are beyond the initial precise verification
				 * sequence (of length "start_len"). Otherwise
				 * we have to check the char exactly.
				 */
				else if (i >= msg->start_len) {
					still_valid++;
					validated = 1;
				}
			}
			/* Only if a starting sequence is specified
			 * and we are still within it do we still
			 * check for a precise match.
			 */
			if (msg->start_len && i < msg->start_len) {
				/* Check if characters match
				 * exactly with start sequence
				 */
				if (msg->msg_start[i] == buf[i]) {
					if (!validated)
						still_valid++;
					if (msg->start_len <= (i + 1)) {
						stop_start++;
						if (trigger_index == -1)
							trigger_index = still_valid - 1;
					}
				}
			}
			/* Keep those messages which don't even have
			 * a start sequence
			 */
			if (!msg->do_start_check && msg->start_len == 0) {
				still_valid++;
				no_start_seq++;
			}
			j++;
		} while (j < *num_msg);


		/* If all possible messages have been eliminated
		 * then there is a frame error
		 */
		if (still_valid <= 0) {
			/* Remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
#ifdef TRACE
			trace_out_str(" [frame_error 1]", 0);
			endtime();
#endif	/* TRACE */
			return(FRAME_ERROR);
		}

		*msg_len = i + 1;
		*num_msg = still_valid;

		checksum = 0;
		/* Stop the start sequence check if there are no more
		 * remaining messages with a start sequence. In this case,
		 * we must include all characters in the checksum
		 */
		if (still_valid == no_start_seq)
			stop_start++;

		/* Either there are messages with a valid start sequence, and/or
		 * there are messages without a start sequence. There is a slight
		 * conceptual error here in that any message without a start
		 * sequence must be longer than any possible start sequence for
		 * the selection made above to work.
		 */
		if (stop_start)
			break; /* end of start sequence */
	}

	if (trigger_index >= 0) {
		/* It is possible to specify a start sequence equal in
		 * length to the total length
		 */
		if ((mess+msg_list[trigger_index])->end_len<=*msg_len) {
			/* remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
#ifdef TRACE
			endtime();
#endif	/* TRACE */
			return(msg_list[trigger_index]);
		}
	}

	/* Start index for checksum generation */
	if (still_valid == no_start_seq)
		checksum_start = 0;	/* no start sequence */
	else
		checksum_start = *msg_len;	/* ignore start sequence */

	stop_end = 0;

	/**** data check loop ****
	 * Check reception of data after start sequence
	 */
	for (i=*msg_len; i<RIDICULOUS_LEN; i++) {
		trigger_index = -1;
		buf[i] = '\0';

		rc = ReadChar(fd, &buf[i], !*msg_len);

		buf[i+1] = '\0';
		if (rc != 1) {
			/* Remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
#ifdef TRACE
			endtime();
#endif	/* TRACE */
			return(FRAME_ERROR);
		}
		else {
			/* Remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
			/* Now reactivate timeout to within message */
			if (to_within) {
				signal(SIGALRM, alfun2);
				alarm(to_within);
			}
		}
		j = still_valid = 0;
		/* Check all messages against this character */
		do {
			validated = 0;
			/* compact remaining list of messages */
			msg_list[still_valid] = msg_list[j];
			msg = mess + msg_list[j];
			/* If we are checking for a specific terminating character */
			if (msg->do_end_check) {
				/* Possible end character but one (if given) */
				prend_char = (msg->end_char >> 8) & 0xff;
				/* End char */
				end_char = msg->end_char & 0xff;
				if (end_char == buf[i] &&
						(!prend_char || (i && prend_char == buf[i-1]))) {
					/* if we are doing a length check */
					k = msg->do_checklen;
					if (k) {
						if (k >= i) {
							/* Remove timeout signal trap */
							signal(SIGALRM, SIG_IGN);
							alarm(0); /* cancel alarm */
							msg_list[0] = msg_list[j];
							*num_msg = 1;
#ifdef TRACE
							trace_out_str(" [length error 1]", 0);
							endtime();
#endif	/* TRACE */
							return(LENGTH_ERROR);
						}
						/* Get length from message */
						memcpy(checklen_char, buf+i-k, k);
						checklen_char[k] = '\0';
						if (sscanf(checklen_char, "%d", &checklen) != 1) {
							/* remove timeout signal trap */
							signal(SIGALRM, SIG_IGN);
							alarm(0); /* cancel alarm */
							msg_list[0] = msg_list[j];
							*num_msg = 1;
#ifdef TRACE
							trace_out_str(" [length error 2]", 0);
							endtime();
#endif	/* TRACE */
							return(CORRUPT_LENGTH);
						}
						/* Compare with with received length */
						if (checklen != (i - checksum_start)) {
							/* remove timeout signal trap */
							signal(SIGALRM, SIG_IGN);
							alarm(0); /* cancel alarm */
							msg_list[0] = msg_list[j];
							*num_msg = 1;
#ifdef TRACE
							trace_out_str(" [length error 3]", 0);
							endtime();
#endif	/* TRACE */
							return(LENGTH_ERROR);
						}

					}
					/* If we are doing a checksum verification */
					if (msg->do_checksum) {
						/* Generate checksum from message */
						for (k=checksum_start; k<=i; k++)
							checksum ^= ((int)(buf[k]) & 0xff);
						/* Get received checksum */
						ReadChar(fd, &ch, 0);
						/* Compare checksums */
						c = (int)ch & 0xff;
						if (c != checksum) {
							/* Remove timeout signal trap */
							signal(SIGALRM, SIG_IGN);
							alarm(0); /* cancel alarm */
							msg_list[0] = msg_list[j];
							*num_msg = 1;
#ifdef TRACE
							trace_out_vstr(0,
								" [checksum error 0x%x]", checksum);
							endtime();
#endif	/* TRACE */
							return(CHECKSUM_ERROR);
						}
					}
					/* Note the first message to satisfy reception criterion */
					if (trigger_index < 0)
						trigger_index = j;
					stop_end++;
				}
				still_valid++;
				validated = 1;
			}
			/* Accept characters up to end length */
			if (!validated)
				still_valid++;
			if (msg->end_len <= (i + 1)) {
				if (trigger_index < 0)
					trigger_index = j;
				stop_end++;
			}
			j++;
		} while (j < *num_msg && !stop_end /* a message found */);
		if (still_valid <= 0) {
			/* Remove timeout signal trap */
			signal(SIGALRM, SIG_IGN);
			alarm(0); /* cancel alarm */
#ifdef TRACE
			trace_out_str(" [frame error 2]", 0);
			endtime();
#endif	/* TRACE */
			return(FRAME_ERROR);
		}
		*msg_len = i + 1;
		*num_msg = still_valid;
		if (stop_end)
			break;
	}
	signal(SIGALRM, SIG_IGN);
	alarm(0); /* cancel alarm */
#ifdef TRACE
	endtime();
#endif	/* TRACE */
	return(msg_list[trigger_index]);
}

/*****************************************************************************
  FUNCTION: int SendSerMsg(int fd, char *send_buf,
                             MSG_SPEC *msg, int *msg_len)
  PURPOSE: Transmit message to a stream
  COMMENTS: It is strongly recommended that the message be
            verified (using "VerSerMsg") before first use.
            If the buf value is NULL then only the start message
            defined in the MSG_SPEC is sent.
            (source nccom.c, library: pmx_lib.a)
  PARAMETERS: fd - file descriptor of stream
              send_buf - buf containing data to be sent
              msg - specification of message to be sent
                    (MSG_SPEC: see nccom.h)
              msg_len - length of transmitted message
  RETURNS:
         =0: transmitted OK
         <0: WRITE_ERROR - transmit fail (see nccom.h)
 ****************************************************************************/
int SendSerMsg(int fd, char *send_buf, MSG_SPEC *msg, int *msg_len)
{
	register	len_cnt;
	register	buf_cnt;
	register	chk_cnt;
	int			rc = 0;			/* return code */
	char		ch;
	char		ch0, ch1;
	int			stop_start;		/* a message has fulfilled start criterion */
	int			stop_end;		/* a message has fulfilled end criterion */
	int			prend_char;		/* last-but-one end char (if given) */
	int			end_char;		/* end char (if given) */
	int			checksum;		/* generated checksum byte */
	char		checklen_char[6]; /* checksum sequence read */

#ifdef TRACE
	trace_out_str(">>> ", 1);
#endif	/* TRACE */
	stop_start = stop_end = 0;
	checksum = 0;
	buf_cnt = 0;
	/* Possible end character but one (if given) */
	prend_char = (msg->end_char >> 8) & 0xff;
	/* End char */
	end_char = msg->end_char & 0xff;

	for (len_cnt=0; len_cnt<msg->end_len; len_cnt++) {
		/* Only if a starting sequence is specified */
		if (msg->start_len && !stop_start && !stop_end) {
			/* Take character from given start sequence */
			ch1 = msg->msg_start[len_cnt];
			/* Check if end of start criterion satisfied */
			if (msg->start_len == (len_cnt + 1) ||
					(msg->do_start_check && msg->start_char == ch1))
				stop_start++;
			/* Check for overall length, last character */
			if (msg->end_len  == (len_cnt + 1) ||
					(msg->do_end_check && msg->end_char == ch1))
				stop_end++;
			/* Write character in start sequence to port */
			if (WriteChar(fd, ch1) != 1) {
				rc = WRITE_ERROR;
				goto End_send;
			}
			*msg_len = len_cnt + 1;
			/* If the start sequence and the full sequence have both
			 * finished here then return OK
			 */
			if (stop_end)
				goto End_send;
			if (stop_start) {
				/* MODIFICA per spedire NULL al primo posto del buffer */
				/* if (!send_buf || !send_buf[0]) */
				/* If there is no data to transmit then return here */
				if (!send_buf)
					goto End_send;
			}
			else
				/* Not yet reached end of start sequence */
				continue;
		}

		/*** Only arrive here when sending send buffer ***/
		if (buf_cnt)	/* already chars in buffer */
			ch0 = send_buf[buf_cnt - 1];
		/* Get next char from send buffer */
		ch1 = send_buf[buf_cnt++];
		/* Add this char to checksum */
		checksum ^= ((int)ch1 & 0xff);
		/* Check for total length, last char and last char but one,
		 * if this was specified
		 */
		if (msg->end_len  <= (len_cnt + 1) ||	/* Total length */
			(msg->do_end_check && (end_char == ch1 &&
				(!prend_char || (buf_cnt && (prend_char == ch0)))))) {
			/* Note total length */
			*msg_len = len_cnt + 1;
			/* Write end length */
			if (msg->do_checklen) {
				sprintf(checklen_char, "%.*d",
					msg->do_checklen, buf_cnt);
				for (chk_cnt=0; chk_cnt<msg->do_checklen; chk_cnt++) {
					if (WriteChar(fd, checklen_char[chk_cnt]) != 1) {
						rc = WRITE_ERROR;
						goto End_send;
					}
					/* Add length to checksum */
					checksum ^= ((int)(checklen_char[chk_cnt]) & 0xff);
					/* Length + 1 for each character of length */
					*msg_len++;
				}
			}
			/* Write end char to port */
			if (WriteChar(fd, ch1) != 1) {
				rc = WRITE_ERROR;
				goto End_send;
			}
			/* Length + 1 for final character */
			*msg_len++;
			/* Write checksum after end character */
			if (msg->do_checksum) {
				ch = (char)checksum;
				if (WriteChar(fd, ch) != 1) {
					rc = WRITE_ERROR;
					goto End_send;
				}
				/* Length + 1 for checksum */
				*msg_len++;
			}
			/* OK exit from loop */
			goto End_send;
		}
		/* Write the next char, still waiting for end char */
		if (WriteChar(fd, ch1) != 1) {
			rc = WRITE_ERROR;
			goto End_send;
		}
	}

End_send:
#ifdef TRACE
	endtime();
#endif	/* TRACE */
	return(rc);
}

/******************************************************************************
  FUNCTION: int FlushSer(int fd, int arg)
  PURPOSE: flush a stream
  COMMENTS: (source nccom.c, library: pmx_lib.a)
  PARAMETERS: fd - file descriptor of stream
              arg - one of the following values:
                   0: input
                   1: output
                   2: input and output
  RETURNS:  0: OK
           -1: arg incorrect
******************************************************************************/
int FlushSer(int fd, int arg)
{
	switch (arg)
	{
		case 0:	/* input */
		case 1:	/* output */
		case 2:	/* input and output */
			ioctl(fd, TCFLSH, arg);
		return(0);
	}
	return(-1);
}

/*****************************************************************************
  FUNCTION: int ReadChar (int fd, char *ch, int start)
  PURPOSE: Read a character from pipe/serial line
  PARAMETERS: fd - file descriptor
              ch - returns with char here
              start - !=0: show start in trace
  RETURNS:  1: char read OK
           <0: UNIX errno of read() call
 ****************************************************************************/
int ReadChar(int fd, char *ch, int start)
{
	int		rc;

	rc = read(fd, ch, 1);
#ifdef TRACE
	if (start)
		trace_out_str("<<< ", 1);
#endif	/* TRACE */
	if (rc != 1) {
		rc = -errno;	/* UNIX error number */
#ifdef TRACE
		/* Output with text of UNIX system error */
		if(strerror(-rc)!=(char *)NULL){
			trace_out_vstr(0," [read error: %s]", strerror(-rc));
		}
#endif	/* TRACE */
	}
#ifdef TRACE
	else
		trace_out_ch(*ch);
#endif	/* TRACE */
	return(rc);
}

/*****************************************************************************
  FUNCTION: int WriteChar(int fd, char ch)
  PURPOSE: Write a character from pipe/serial line
  PARAMETERS: fd - file descriptor
              ch - returns with char here
  RETURNS:  1: char written OK
           <0: UNIX errno of write() call
 ****************************************************************************/
int WriteChar(int fd, char ch)
{
	int		rc;

	rc = write(fd, &ch, 1);
	if (rc != 1) {
		rc = -errno;	/* UNIX error number */
#ifdef TRACE
		if(strerror(-rc)!=(char *)NULL){
			trace_out_vstr(0," [write error: %s]", strerror(-rc));
		}
		trace_out_str(sbuf, 0);
#endif	/* TRACE */
	}
#ifdef TRACE
	else
		trace_out_ch(ch);
#endif	/* TRACE */
	return(rc);
}

/*****************************************************************************
  FUNCTION: alfun1()
  PURPOSE: Timeout ALARM CALL function 1 (timeout before)
  COMMENTS: function used only in this module
 ****************************************************************************/
static void alfun1()
{
	longjmp(env, 1);
}

/*****************************************************************************
  FUNCTION: alfun2()
  PURPOSE: Timeout ALARM CALL function 2 (timeout within)
  COMMENTS: function used only in this module
 ****************************************************************************/
static void alfun2()
{
	longjmp(env, 2);
}

/******************************************************************************
  FUNCTION: pipeint()
  PURPOSE: recognizes a SIGPIPE fail
******************************************************************************/
static void pipeint()
{
	exit(-1);
}
