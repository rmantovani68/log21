/* crossmat.c  -  routines for managing message queues with Crossmatic */
#include <string.h>
#include "msq_lib.h"
#include "cross_in.h"

/*****************************************************************************
  FUNCTION: int UnpackCrossmatMsg(char *msg_info,
                     char *crm_info, int len)
  PURPOSE: Unpack a Crossmatic message code from message information
  COMMENTS: (source cross_in.c, library: pmx_lib.a)
  PARAMETERS: msg_info - message information
              crm_info - holds crossmatic information after return
              len - length of crossmatic information
  RETURNS: >=0: value of Crossmatic message code
            <0: error
*****************************************************************************/
int UnpackCrossmatMsg(char *msg_info, char *crm_info, int len)
{
	int		rc;

	rc = GetField(msg_info, CROSSC_LEN);

	if (rc<0 || crm_info==NULL || len<=0) {
		return(rc);
	}

	memcpy(crm_info, msg_info+CROSSC_LEN, len);
	crm_info[len] = '\0';
	return(rc);
}

/*****************************************************************************
  FUNCTION: int SendCrossmatMsg(int msgq_id, int dest, int srce,
                                  int crm_code, char *str)
  PURPOSE: Send a Crossmatic message code via a message queue
  COMMENTS: (source cross_in.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              crm_code - crossmatic message code
              str - information to be sent
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendCrossmatMsg(int msgq_id, int dest, int srce, int crm_code, char *str)
{
	char	buf[INFO_LEN];

	PackInt(buf, crm_code, CROSSC_LEN);
	if (str)
		strcpy(buf+CROSSC_LEN, str);

	return(SendSpecificMsg(msgq_id, dest, srce, MSG_CROSSMAT, buf));
}

/*****************************************************************************
  FUNCTION: int SendCrossmatStatus(int msgq_id, int dest, int srce,
                                  int crm_code, int stat)
  PURPOSE: Send a Crossmatic message code + status via a message queue
  COMMENTS: (source cross_in.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              crm_code - crossmatic message code
              stat - crossmatic status
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendCrossmatStatus(int msgq_id, int dest, int srce, int crm_code, int stat)
{
	char	info[CROSSC_LEN+1], msg[INFO_LEN];
	int		len;

	len = PackInt(info, crm_code, CROSSC_LEN);
	len = PackMsg(msg, dest, srce,
			0, stat, SPECIFIC_CLASS, MSG_CROSSMAT, info, len);

	return(SendMsg(msgq_id, msg, len));
}
