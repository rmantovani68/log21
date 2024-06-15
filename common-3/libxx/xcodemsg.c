/***
* 	xcodemsg.c 
* 	routines per la gestione di messaggi con codice extra oltre l'header
* 	standard.
***/
#include <string.h>
#include "msq_lib.h"
#include "xcodemsg.h"

/*****************************************************************************
  FUNCTION: int UnpackXcodeMsg(char *msg_info,char *xcode_info, int len)
  PURPOSE: Unpack a message code from message information
  COMMENTS: (source xcodemsg.c, library: pmx_lib.a)
  PARAMETERS: msg_info - message information
              xcode_info - holds information after return (string)
              len - length of information
  RETURNS: >=0: value of extra message code
            <0: error
*****************************************************************************/
int UnpackXcodeMsg(char *msg_info, char *xcode_info, int len)
{
	int rc;
	
	if (xcode_info!=NULL)	xcode_info[0]='\0';
	len=len-XCODE_LEN;

	/* Get extra message code at start of message */
	rc = GetField(msg_info, XCODE_LEN);

	if (rc < 0 || xcode_info == NULL || len <= 0)
		return(rc);	/* error or no info transmitted */

	/* Place information into buffer */
	memcpy(xcode_info, msg_info+XCODE_LEN, len);
	xcode_info[len] = '\0';
	return(rc);
}

/*****************************************************************************
  FUNCTION: int UnpackXcodeMsgL(char *msg_info,char *xcode_info, int *len)
  PURPOSE: Unpack a message code from message information
  COMMENTS: (source xcodemsg.c, library: pmx_lib.a)
  PARAMETERS: msg_info - message information
              xcode_info - holds information after return
              len - length of input information and output information on return
  RETURNS: >=0: value of extra message code
            <0: error
*****************************************************************************/
int UnpackXcodeMsgL(char *msg_info, char *xcode_info, int *len)
{
	int rc;
	
	*len=*len-XCODE_LEN;
	
	/* Get extra message code at start of message */
	rc = GetField(msg_info, XCODE_LEN);

	if (rc < 0 || xcode_info == NULL || *len <= 0)
		return(rc);	/* error or no info transmitted */

	/* Place information into buffer */
	memcpy(xcode_info, msg_info+XCODE_LEN,*len);
	return(rc);
}

/*****************************************************************************
  FUNCTION: int SendXcodeMsg(int msgq_id, int dest, int srce,
                                  int xcode_code, char *str)
  PURPOSE: Send a message code via a message queue
  COMMENTS: (source xcodemsg.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              xcode_code - extra message code
              str - information to be sent (string)
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendXcodeMsg(int msgq_id, int dest, int srce, int xcode_code, char *str)
{
	char buf[INFO_LEN];

	PackInt(buf, xcode_code, XCODE_LEN);
	if (str)
		strcpy(buf+XCODE_LEN, str);

	return(SendSpecificMsg(msgq_id, dest, srce, MSG_XCODE, buf));
}

/*****************************************************************************
  FUNCTION: int SendXcodeMsgL(int msgq_id, int dest, int srce,
                                  int xcode_code, char *info,int len)
  PURPOSE: Send a message code via a message queue
  COMMENTS: (source xcodemsg.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              xcode_code - extra message code
              info - information to be sent
			  len  - length of information
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendXcodeMsgL(int msgq_id, int dest, int srce, int xcode_code, 
					char *info,int len)
{
	char buf[INFO_LEN];

	PackInt(buf, xcode_code, XCODE_LEN);
	if (len>0)
		memcpy(buf+XCODE_LEN, info,len);
	else
		len=0;
	
	len=len+XCODE_LEN;
	
	return(SendSpecificMsgL(msgq_id, dest, srce, MSG_XCODE, buf, len));
}

/*****************************************************************************
  FUNCTION: int SendXcodeStatus(int msgq_id, int dest, int srce,
                                  int xcode_code, int stat)
  PURPOSE: Send a message code + status via a message queue
  COMMENTS: (source xcodemsg.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination
              srce - gid of source
              xcode_code - extra message code
              stat - status
  RETURNS: return value of SendMsg()
           >=0: OK
            <0: error
*****************************************************************************/
int SendXcodeStatus(int msgq_id, int dest, int srce, int xcode_code, int stat)
{
	char	info[XCODE_LEN+1], msg[INFO_LEN];
	int		len;

	len = PackInt(info, xcode_code, XCODE_LEN);
	len = PackMsg(msg, dest, srce,
			0, stat, SPECIFIC_CLASS, MSG_XCODE, info, len);

	return(SendMsg(msgq_id, msg, len));
}
