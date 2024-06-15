/* ima_in.c  -  routines for managing message queues with Imatronic */
#include <string.h>
#include "msq_lib.h"
#include "ima_in.h"

/*****************************************************************************
  FUNCTION: int UnpackImatronMsg(char *msg_info,
                     char *ima_info, int len)
  PURPOSE: Unpack an Imatronic message code from message information
  COMMENTS: (source ima_in.c, library: pmx_lib.a)
  PARAMETERS: msg_info - message information
              ima_info - holds Imatronic information after return
              len - length of Imatronic information
  RETURNS: >=0: value of Imatronic message code
            <0: error
*****************************************************************************/
int UnpackImatronMsg(char *msg_info, char *ima_info, int len)
{
	int rc;

	/* Get IMATRONIC message code at start of message */
	rc = GetField(msg_info, IMAC_LEN);

	if (rc < 0 || ima_info == NULL || len <= 0)
		return(rc);	/* error or no info transmitted */

	/* Place information into buffer */
	memcpy(ima_info, msg_info+IMAC_LEN, len);
	ima_info[len] = '\0';
	return(rc);
}

/*****************************************************************************
  FUNCTION: int SendImatronMsg(int msgq_id, int dest, int srce,
                                  int ima_code, char *str)
  PURPOSE: Send Imatronic message code via a message queue
  COMMENTS: (source ima_in.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              ima_code - Imatronic message code
              str - information to be sent
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendImatronMsg(int msgq_id, int dest, int srce, int ima_code, char *str)
{
	char buf[INFO_LEN];

	PackInt(buf, ima_code, IMAC_LEN);
	if (str)
		strcpy(buf+IMAC_LEN, str);

	return(SendSpecificMsg(msgq_id, dest, srce, MSG_IMATRON, buf));
}

/*****************************************************************************
  FUNCTION: int SendImatronStatus(int msgq_id, int dest, int srce,
                                  int ima_code, int stat)
  PURPOSE: Send a Imatronic message code + status via a message queue
  COMMENTS: (source ima_in.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              ima_code - Imatronic message code
              stat - Imatronic status
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendImatronStatus(int msgq_id, int dest, int srce, int ima_code, int stat)
{
	char	info[IMAC_LEN+1], msg[INFO_LEN];
	int		len;

	len = PackInt(info, ima_code, IMAC_LEN);
	len = PackMsg(msg, dest, srce,
			0, stat, SPECIFIC_CLASS, MSG_IMATRON, info, len);

	return(SendMsg(msgq_id, msg, len));
}
