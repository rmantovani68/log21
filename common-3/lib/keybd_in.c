/* keybd_in.c  -  routines for managing message queues with keyboard */
#include <string.h>
#include "msq_lib.h"
#include "keybd_in.h"

/*****************************************************************************
  FUNCTION: int UnpackKeybdMsg(char *msg_info)
  PURPOSE: Unpack a key code from message information
  COMMENTS: (source keybd_in.c, library: pmx_lib.a)
  PARAMETERS: msg_info - message information containing key code
  RETURNS: >=0: value of key code,
            <0: error
*****************************************************************************/
int UnpackKeybdMsg(char *msg_info)
{
	return(GetField(msg_info, KEYC_LEN));
}

/*****************************************************************************
  FUNCTION: int SendKeybdMsg(int msgq_id, int dest, int srce, int keycode)
  PURPOSE: Send a key code via a message queue
  COMMENTS: (source keybd_in.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to send to
              dest - gid of destination module
              srce - gid of sourc module
              keycode - code of key to send
  RETURNS: >=0: value of key code,
            <0: error
*****************************************************************************/
int SendKeybdMsg(int msgq_id, int dest, int srce, int keycode)
{
	char	buf[10];

	PackInt(buf, keycode, KEYC_LEN);
	return(SendSpecificMsg(msgq_id, dest, srce, MSG_KEYBD, buf));
}
