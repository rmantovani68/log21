/* 
* msq_lib.c -- message queue functions 
* HISTORY OF MODIFICATIONS
* ---------------------------------------------------------
*    12.05.93 -MC-    Corretta pulizia coda nella CreateMsgQ
*    10.10.94 -RM-    Aumentata la lunghezza massima dei messaggi
*    21.09.95 -RM-    Aggiunta funzione ReceiveMsgNoWait()
*    28.01.99 -RM-    Aggiunta funzione SendMsgNoWait()
*    28.01.99 -RM-    Aggiunta funzione SendSpecificMsgNoWait()
*    13.02.99 -RM-    Aggiunti parametri per timeout e retry alla SendSpecificMsgNoWait()
* ---------------------------------------------------------
*/

#ifdef __linux__
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include "msq_lib.h"

/* Current send serial number (initialized to zero on load) */
static int sndserial = 0;

#ifdef __linux__

/* Long jump environment */
static jmp_buf env;

/* Module-local function prototypes */
static void AlarmFunc();    /* need empty argument list for signal() */

/******************************************************************************
  FUNCTION: int CreateMsgQ(int key, int clear)
  PURPOSE: Create a message queue / open an already existing one
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: key - key of message queue to be created
              clear - clear message queue after opening
  RETURNS: msg id of message queue (error: -1 + errno set)
******************************************************************************/
int CreateMsgQ(int key, int clear)
{
    int        msgq_id;
    char    rbuf[256];

    /* We want to let everyone read and write this message queue,
     * hence the 0666 permissions
     */
    if ((msgq_id = msgget(key, IPC_CREAT | 0666)) < 0)
        return(msgq_id);
    if (clear)
        /* Clear all messages by reading them */
        while (msgrcv(msgq_id, &rbuf, 256, 0, IPC_NOWAIT)>=0)
            ;
    return(msgq_id);
}

/******************************************************************************
  FUNCTION: void DeleteMsgQ(int msq_id)
  PURPOSE: Delete a message queue
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msq_id - id of message queue
  RETURNS: -1 if an error occurred (error then in errno),
           otherwise total number of bytes received
******************************************************************************/
void DeleteMsgQ(int msq_id)
{
    struct    msqid_ds    buf[1];

    msgctl(msq_id, IPC_RMID, buf);
}

/******************************************************************************
  FUNCTION: int OpenMsgQ(int key)
  PURPOSE: Open an already existing message queue
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETER: key - message queue key
  RETURNS: msg id of message queue (error: -1 + errno set)
******************************************************************************/
int OpenMsgQ(int key)
{
    /* We want to let everyone read and write this message queue,
     * hence the 0666 permissions
     */
    return(msgget(key, 0666));
}

/******************************************************************************
  FUNCTION: int SendMsg(int msgq_id, char *msg, int len_msg)
  PURPOSE: Send a message to a message queue
  COMMENTS: NB: do not send a message with a destination field
            of 0. As this field is used to set the mtype of the
            message, this causes an error (EINVAL: 22), a
            value of zero not being permitted for sending.
            (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to be sent to
              msg - buffer containing message to be sent
              len_msg - length of message in this buffer
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendMsg(int msgq_id, char *msg, int len_msg)
{
    MSG        *ptr = (MSG *)msg;
    MSGBUF    mbuf;
    int        rc;

    /* Set type to destination */
    mbuf.mtype = GetField(ptr->dest, ADR_LEN);

    /* Insert (global) current send message serial number */
    PutField(ptr->serial, sndserial, SER_LEN);
    /* Increment message serial number */
    sndserial = (sndserial + 1) % MAX_SERIAL;

    /* Put message into buffer */
    memcpy(mbuf.mtext, msg, len_msg);
    rc = msgsnd(msgq_id, &mbuf, len_msg, 0);
    if (rc < 0)
        /* Minus UNIX system error number */
        rc = -errno;
    else
        rc = 0;

    return(rc);
}

/******************************************************************************
  FUNCTION: int SendMsgNoWait(int msgq_id, char *msg, int len_msg)
  PURPOSE: Send a message to a message queue without waiting (nonblocking write)
  COMMENTS: NB: do not send a message with a destination field
            of 0. As this field is used to set the mtype of the
            message, this causes an error (EINVAL: 22), a
            value of zero not being permitted for sending.
            (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to be sent to
              msg - buffer containing message to be sent
              len_msg - length of message in this buffer
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendMsgNoWait(int msgq_id, char *msg, int len_msg)
{
    MSG        *ptr = (MSG *)msg;
    MSGBUF    mbuf;
    int        rc;

    /* Set type to destination */
    mbuf.mtype = GetField(ptr->dest, ADR_LEN);

    /* Insert (global) current send message serial number */
    PutField(ptr->serial, sndserial, SER_LEN);
    /* Increment message serial number */
    sndserial = (sndserial + 1) % MAX_SERIAL;

    /* Put message into buffer */
    memcpy(mbuf.mtext, msg, len_msg);
    rc = msgsnd(msgq_id, &mbuf, len_msg, IPC_NOWAIT);
    if (rc < 0)
        /* Minus UNIX system error number */
        rc = -errno;
    else
        rc = 0;

    return(rc);
}


/******************************************************************************
  FUNCTION: int ReceiveMsgNoWait(int msgq_id, int *mtype, char *msg)
  PURPOSE: Receive a message from a message queue without waiting (nonblocking read)
  COMMENTS: The message type determines the type of message
            which can be received. 0 = all messages.
            (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to receive from
              mtype - type of message expected (0 = all messages)
              msg - buffer returned with received message
  RETURNS:
        >0: OK, number of bytes received in message
        =0: should never happen
        <0: - UNIX system error, EIDRM, E2BIG, EACCES, EFAULT, EINVAL,
              or ENOMSG (msg queue), EINTR (timeout, if used)
              (see /usr/include/sys/errno.h)
******************************************************************************/
int ReceiveMsgNoWait(int msgq_id, int *mtype, char *msg)
{
    MSGBUF    rbuf;
    int        len;

    if ((len = msgrcv(msgq_id, &rbuf, INFO_LEN, *mtype, IPC_NOWAIT)) < 0)
        /* Minus UNIX system error number */
        return(-errno);

    *mtype = rbuf.mtype;
    memcpy(msg, rbuf.mtext, len);
    msg[len] = '\0';

    return(len);
}


/******************************************************************************
  FUNCTION: int ReceiveMsg(int msgq_id, int *mtype, char *msg)
  PURPOSE: Receive a message from a message queue
  COMMENTS: The message type determines the type of message
            which can be received. 0 = all messages.
            (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to receive from
              mtype - type of message expected (0 = all messages)
              msg - buffer returned with received message
  RETURNS:
        >0: OK, number of bytes received in message
        =0: should never happen
        <0: - UNIX system error, EIDRM, E2BIG, EACCES, EFAULT, EINVAL,
              or ENOMSG (msg queue), EINTR (timeout, if used)
              (see /usr/include/sys/errno.h)
******************************************************************************/
int ReceiveMsg(int msgq_id, int *mtype, char *msg)
{
    MSGBUF    rbuf;
    int        len;

    if ((len = msgrcv(msgq_id, &rbuf, INFO_LEN, *mtype, 0)) < 0)
        /* Minus UNIX system error number */
        return(-errno);

    *mtype = rbuf.mtype;
    memcpy(msg, rbuf.mtext, len);
    msg[len] = '\0';

    return(len);
}

/******************************************************************************
  FUNCTION: int ReceiveMsgTimed(int msgq_id, int *mtype,
                     char *msg, int *timeout)
  PURPOSE: Receive a message from a message queue with a timeout
  COMMENTS: The message type determines the type of message
            which can be received. 0 = all messages.
            The timeout parameter is used to set the timeout
            for the call. On returning, this parameter is set to
            the time remaining (0 in the case of a timeout)
            If the parameter is zero then there is no timeout active.
            (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msgq_id - id of message queue to receive from
              mtype - type of message expected (0 = all messages)
              msg - buffer returned with received message
              timeout - initial timeout in seconds. The timeout remaining
                        is returned in this parameter, if a message is
                        received before the timeout elapses.
  RETURNS: >0: OK, number of bytes received in message
           =0: message timed out or error in timeout (<= 0)
           <0: - UNIX system error:
                       EIDRM, E2BIG, EACCES, EFAULT, EINVAL, or ENOMSG
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int ReceiveMsgTimed(int msgq_id, int *mtype, char *msg, int *timeout)
{
    MSGBUF    rbuf;
    int        len;
    long    start_timeout;

    /* Check timeout is > 0 */
    if (*timeout > 0) {
        /* Long jump entry point */
        switch (setjmp(env)) {
        case 0:    /* first time round */
            /* set up timeout signal trap */
            signal(SIGALRM, AlarmFunc);    /* insert alarm signal trap */
            alarm(*timeout);            /* enable alarm */
            time(&start_timeout);        /* note start of timeout */
            break;
        case 1:    /* timeout elapsed */
            signal(SIGALRM, SIG_IGN);    /* remove alarm signal trap */
            alarm(0);                     /* disable alarm */
            *timeout = 0;                /* time remaining */
            return(0);
        }
    }
    /* Receive message via UNIX call */
    len = msgrcv(msgq_id, &rbuf, INFO_LEN, *mtype, 0);
    if (*timeout > 0) {
        signal(SIGALRM, SIG_IGN);        /* remove alarm signal trap */
        alarm(0);                         /* disable alarm */
        *timeout -= time(NULL) - start_timeout;    /* note time remaining */
    }
    if (len < 0) {    /* error */
        /* Minus UNIX system error number */
        return(-errno);
    }
    *mtype = rbuf.mtype;
    memcpy(msg, rbuf.mtext, len);
    msg[len] = '\0';

    return(len);
}


/******************************************************************************
  FUNCTION: int ExecModule (char *progname, ...)
  PURPOSE: Fork and exec a module
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: progname - name of module to be started
              ... - NULL-terminated list of parameters for the module
  RETURNS: Process id of module
******************************************************************************/
int ExecModule(char *progname, ...)
{
    va_list    ap;
    int        i;
    int        client_pid;
    char    *ptr;
    char    *params[10];    /* allow up to 9 parameters in list */


    va_start(ap, progname);
    client_pid = fork();

    if (client_pid == 0) {    /* child */
        /* Create parameter list */
        for (i=0; ;i++) {
            if (i == 9 || (ptr = va_arg(ap, char *)) == NULL) {
                params[i] = NULL;
                break;
            }
            params[i] = ptr;
        }
        va_end(ap);
        /* exec with path and argument list */
        execv(progname, params); 
        /*
        * fprintf(stderr, "Could not execv() process %s\n", progname);
        */
        _exit(-1); 
    }
    va_end(ap);
    return(client_pid);
}

/******************************************************************************
  FUNCTION: static void AlarmFunc()
  PURPOSE: Module-local function to manage timeout alarm
  COMMENTS: Used in conjunction with a signal() call.
******************************************************************************/
static void AlarmFunc()
{
    longjmp(env, 1);
}

#else

int CreateMsgQ(int key, int clear)
{
    return(-1);
}

void DeleteMsgQ(int msq_id)
{
}

int OpenMsgQ(int key)
{
    return(-1);
}

int SendMsg(int msgq_id, char *msg, int len_msg)
{
    return(-1);
}

int SendMsgNoWait(int msgq_id, char *msg, int len_msg)
{
    return(-1);
}

int ReceiveMsgNoWait(int msgq_id, int *mtype, char *msg)
{
    return(-1);
}

int ReceiveMsg(int msgq_id, int *mtype, char *msg)
{
    return(-1);
}

int ReceiveMsgTimed(int msgq_id, int *mtype, char *msg, int *timeout)
{
    return(-1);
}

int ExecModule(char *progname, ...)
{
    return(-1);
}

#endif

/******************************************************************************
  FUNCTION: int PackMsg(char *msg, int dest, int srce, int ack, int stat,
                   int class, int code, char *info, int info_len)
  PURPOSE: Pack information into a message structure
           in preparation for sending
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msg - buffer to receive packed message
              dest - gid of destination
              srce - gid of source
              ack - >0: acknowledge expected
              stat - status
              class - class of message
              code - code of message
              info - buffer containing message information
              info_len - length of information in this buffer
  RETURNS: Total length of message
******************************************************************************/
int PackMsg(char *msg, int dest, int srce, int ack, int stat, int class,int code, char *info, int info_len)
{
    MSG        *ptr = (MSG *)msg;
    int        len = HDR_LEN;

    /* Insert destination */
    PutField(ptr->dest, dest, ADR_LEN);

    /* Insert source */
    PutField(ptr->srce, srce, ADR_LEN);

    /* Insert acknowledge request */
    PutField(ptr->ack, ack, ACK_LEN);

    /* Insert status request */
    PutField(ptr->stat, stat, STAT_LEN);

    /* Insert message class */
    PutField(ptr->class, class, CLASS_LEN);

    /* Insert message code */
    PutField(ptr->code, code, CODE_LEN);

    /* Serial number inserted just before transmission */

    /* Insert information */
    if (info_len > 0 && info != NULL) {
        memcpy(ptr->info, info, info_len);
        len += info_len;
        ptr->info[info_len] = '\0';    /* zero-terminate it */
    }
    return(len);
}

/******************************************************************************
  FUNCTION: int PackStructMsg(char *msg, MSGHDRINT *msghdr,
                          char *info, int info_len)
  PURPOSE: Pack information structure into a message structure
           in preparation for sending
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msg - buffer to receive packed message
              msghdr - prefilled structure conatining header information
              info - buffer containing message information
              info_len - length of information in this buffer
  RETURNS: Total length of message
******************************************************************************/
int PackStructMsg(char *msg, MSGHDRINT *msghdr, char *info, int info_len)
{
    MSG        *ptr = (MSG *)msg;
    int        len = HDR_LEN;

    /* Insert destination */
    PutField(ptr->dest, msghdr->dest, ADR_LEN);

    /* Insert source */
    PutField(ptr->srce, msghdr->srce, ADR_LEN);

    /* Insert acknowledge request */
    PutField(ptr->ack, msghdr->ack, ACK_LEN);

    /* Insert status request */
    PutField(ptr->stat, msghdr->stat, STAT_LEN);

    /* Insert message class */
    PutField(ptr->class, msghdr->class, CLASS_LEN);

    /* Insert message code */
    PutField(ptr->code, msghdr->code, CODE_LEN);

    /* Send message serial number inserted just before transmission */

    /* Insert information */
    if (info_len > 0 && info != NULL) {
        memcpy(ptr->info, info, info_len);
        len += info_len;
    }
    return(len);
}

/******************************************************************************
  FUNCTION: UnpackMsg
  FUNCTION: int UnpackMsg(char *msg, int *dest, int *srce,
                 int *ack, int *stat, int *class, int *code,
                 int *rcvserial, char *info, int len)
  PURPOSE: Unpack information from a message structure
  COMMENTS: >0: Returns length of additional information
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msg - buffer to receive packed message
              dest - gid of destination
              srce - gid of source
              ack - >0: acknowledge expected
              stat - status
              class - class of message
              code - code of message
              rcvserial - serial number of received message
              info - buffer containing message information
              info_len - length of information in this buffer
  RETURNS: >=0: length of additional information
******************************************************************************/
int UnpackMsg(char *msg, int *dest, int *srce, int *ack, int *stat, int *class,
                int *code, int *rcvserial, char *info, int len)
{
    MSG        *ptr = (MSG *)msg;
    int        hdr_len = HDR_LEN, info_len;

    /* Insert destination */
    *dest = GetField(ptr->dest, ADR_LEN);

    /* Insert source */
    *srce = GetField(ptr->srce, ADR_LEN);

    /* Insert acknowledge request */
    *ack = GetField(ptr->ack, ACK_LEN);

    /* Insert status request */
    *stat = GetField(ptr->stat, STAT_LEN);

    /* Insert message class */
    *class = GetField(ptr->class, CLASS_LEN);

    /* Insert message code */
    *code = GetField(ptr->code, CODE_LEN);

    /* Insert received serial number */
    *rcvserial = GetField(ptr->serial, SER_LEN);

    /* Insert information */
    info_len = len - hdr_len;
    if (info_len > 0)
        memcpy(info, ptr->info, info_len);
    else
        info_len = 0;
    info[info_len] = '\0';

    return(info_len);
}

/******************************************************************************
  FUNCTION: int UnpackStructMsg(char *msg, MSGHDRINT *msghdr,
                                   char *info, int len)
  PURPOSE: Unpack information structure from a message
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: msg - buffer containing message
              msghdr - structure returned with header information
              info - buffer returned with additional information
              len - length of message in msg
  RETURNS: >=0: length of additional information
******************************************************************************/
int UnpackStructMsg(char *msg, MSGHDRINT *msghdr, char *info, int len)
{
    MSG        *ptr = (MSG *)msg;
    int        hdr_len = HDR_LEN;
    int        info_len;

    /* Insert destination */
    msghdr->dest = GetField(ptr->dest, ADR_LEN);

    /* Insert source */
    msghdr->srce = GetField(ptr->srce, ADR_LEN);

    /* Insert acknowledge request */
    msghdr->ack = GetField(ptr->ack, ACK_LEN);

    /* Insert status request */
    msghdr->stat = GetField(ptr->stat, STAT_LEN);

    /* Insert message class */
    msghdr->class = GetField(ptr->class, CLASS_LEN);

    /* Insert message code */
    msghdr->code = GetField(ptr->code, CODE_LEN);

    /* Insert received serial number */
    msghdr->serial = GetField(ptr->serial, SER_LEN);

    /* Insert information */
    info_len = len - hdr_len;
    if (info_len > 0)
        memcpy(info, ptr->info, info_len);
    else
        info_len = 0;
    info[info_len] = '\0';

    return(info_len);
}

/******************************************************************************
  FUNCTION: int SendSpecificMsg(int dest_qid, int dest, int srce,
                                      int code, char *str)
  PURPOSE: Pack and send a simple SPECIFIC_CLASS message with a
           zero-terminated string as the message info
  PARAMETERS: dest_qid - message queue to be sent to
              dest - gid of destination module
              srce - gid of destination module
              code - message code to be sent
              str - zero-terminated string containing additional
                    information to be sent.
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendSpecificMsg(int dest_qid, int dest, int srce, int code, char *str)
{
    char    msg[INFO_LEN];
    int        len;

    len = (str && str[0]) ? strlen(str) : 0;
    len = PackMsg(msg, dest, srce, 0, 0, SPECIFIC_CLASS, code, str, len);
    return(SendMsg(dest_qid, msg, len));
}

/******************************************************************************
  FUNCTION: int SendSpecificMsgNoWait(int dest_qid, int dest, int srce,
                                      int code, char *str,int nTimeOut)
  PURPOSE: Pack and send a simple SPECIFIC_CLASS message with a
           zero-terminated string as the message info
                     without waiting (non blocking write)
  PARAMETERS: dest_qid - message queue to be sent to
              dest - gid of destination module
              srce - gid of destination module
              code - message code to be sent
              str - zero-terminated string containing additional
                    information to be sent.
                            nTimeOut : TimeOut in msecs to wait for ...
                            nRetries : Numero di volte
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendSpecificMsgNoWait(int dest_qid, int dest, int srce, int code, char *str,int nTimeOut,int nRetries)
{
    char    msg[INFO_LEN];
    int        len;
    int nIndex=0;
    int nRC;

    len = (str && str[0]) ? strlen(str) : 0;
    len = PackMsg(msg, dest, srce, 0, 0, SPECIFIC_CLASS, code, str, len);
    /*
    * Provo a spedire il messaggio per 'nRetries' volte 
    */
    while(nIndex<nRetries && (nRC=SendMsgNoWait(dest_qid, msg, len))!=0){
        /*
        * Attendo il tempo specificato in nTimeOut
        */
        usleep(nTimeOut*1000);
        nIndex++;
    }
    return nRC;
}

/******************************************************************************
  FUNCTION: int SendSpecificMsgL(int dest_qid, int dest, int srce,
                                      int code, char *buf, int len)
  PURPOSE: Pack and send a simple SPECIFIC_CLASS message with a
           buffer of len characters as the message info
  PARAMETERS: dest_qid - message queue to be sent to
              dest - gid of destination module
              srce - gid of destination module
              code - message code to be sent
              buf  - buffer containing additional information to be sent.
              len  - buf length
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendSpecificMsgL(int dest_qid,int dest,int srce,int code,char *buf,int len)
{
    char    msg[INFO_LEN];
    int        tot_len;

    tot_len = PackMsg(msg, dest, srce, 0, 0, SPECIFIC_CLASS, code, buf, len);
    return(SendMsg(dest_qid, msg, tot_len));
}

/******************************************************************************
  FUNCTION: int SendSpecificMsgA(int dest_qid, int dest, int srce,
                                      int code, char *str, ...)
  PURPOSE: Pack and send a simple SPECIFIC_CLASS message with a
           zero-terminated string as the message info
  PARAMETERS: dest_qid - message queue to be sent to
              dest - gid of destination module
              srce - gid of destination module
              code - message code to be sent
              str - zero-terminated string containing format string
              ... - arguments
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendSpecificMsgA(int dest_qid,int dest,int srce,int code,char *format, ...)
{
    char    msg[INFO_LEN];
    char    str[INFO_LEN];
    int        len;
    va_list    arg_pointer;
    int        retcode=0;

    va_start(arg_pointer,format);
    if (format!=NULL)
    {
        vsprintf(str,format,arg_pointer);
        len = strlen(str);
        len = PackMsg(msg, dest, srce, 0, 0, SPECIFIC_CLASS, code, str, len);
        retcode=SendMsg(dest_qid, msg, len);
    }
    va_end(arg_pointer);
    return(retcode);
}

/******************************************************************************
  FUNCTION: int SendFileMsg(int dest_qid, int dest, int srce,
                                      int code, char *str)
  PURPOSE: Pack and send a simple FILE_CLASS message with a
           zero-terminated file name string as the message info
  PARAMETERS: dest_qid - message queue to be sent to
              dest - gid of destination module
              srce - gid of destination module
              code - message code to be sent
              str - zero-terminated string containing file name
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  RETURNS: =0: OK,
           <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                 (see /usr/include/sys/errno.h)
******************************************************************************/
int SendFileMsg(int dest_qid, int dest, int srce, int code, char *str)
{
    char    msg[INFO_LEN];
    int        len;

    len = (str && str[0]) ? strlen(str) : 0;
    len = PackMsg(msg, dest, srce, 0, 0, FILE_CLASS, code, str, len);
    return(SendMsg(dest_qid, msg, len));
}

/******************************************************************************
  FUNCTION: int SendAckMsg(int dest_qid, int dest, int srce, int serial)
  PURPOSE: Send an acknowledge message with a returned serial number
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: dest_qid - id of message queue to be sent to
              dest - gid of destination module
              srce - gid of source module
              serial - serial number to be sent (see parameter
                       'rcvserial' in UnpackMsg())
  RETURNS: =0: OK,
            <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
                  (see /usr/include/sys/errno.h)
******************************************************************************/
int SendAckMsg(int dest_qid, int dest, int srce, int serial)
{
    char    buf[SER_LEN+1];

    /* Send acknowledge message */
    PackInt(buf, serial, SER_LEN);
    return(SendSpecificMsg(dest_qid, dest, srce, MSG_ACK, buf));
}

/******************************************************************************
  FUNCTION: int GetSerial()
  PURPOSE: Get current value of message send serial number
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  RETURNS: current value of message send serial number
******************************************************************************/
int GetSerial()
{
    return(sndserial);
}

/******************************************************************************
  FUNCTION: void PutField(char *to, int from, int len)
  PURPOSE: Put an integer value into a field of a given length
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: to - address of field
              from - integer value
              len - length of field
******************************************************************************/
void PutField(char *to, int from, int len)
{
    char    buf[20];

    sprintf(buf, "%*.*x", len, len, from);
    memcpy(to, buf, len);
}

/******************************************************************************
  FUNCTION: int GetField (char *from, int len)
  PURPOSE: Get a hexadecimal integer value from a field of a given length
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: from - address of field
              len - length of field
  RETURNS: hexadecimal integer value
******************************************************************************/
int GetField(char *from, int len)
{
    char    buf[20];
    int        i;

    memcpy(buf, from, len);
    buf[len] = '\0';
    sscanf(buf, "%x", &i);
    return(i);
}

/******************************************************************************
  FUNCTION: int PackInt(char *to, int from, int len)
  PURPOSE: Put a integer value as a hexadecimal string into a field,
           terminated with zero
  COMMENTS: (source msq_lib.c, library: pmx_lib.a)
  PARAMETERS: to - address of field
              from - integer value
              len - length of field
  RETURNS: length of field
******************************************************************************/
int PackInt(char *to, int from, int len)
{
    PutField(to, from, len);
    to[len] = '\0';
    return(len);
}

