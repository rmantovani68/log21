/* shared.c - shared memory message routines */
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include <stdio.h>
#include "msq_lib.h"
#include "shared.h"

#ifdef __linux__
#define SHMKEY_LEN        4
#define SHMSIZE_LEN        6

typedef struct {
    char shm_key[SHMKEY_LEN];
    char shm_size[SHMSIZE_LEN];
} SHM_MSG;

/******************************************************************************
  FUNCTION: int SendShmMsg(int dest_qid, int dest, int srce,
               int code, int stat, int shm_key, int shm_size)
  PURPOSE: Pack and send a SHARED_CLASS message with a
           zero-terminated string as the message info
  PARAMETERS: dest_qid - queue id of msg queue to send message to
              dest - gid of destination
              srce - gid of source
              code - message code
              stat - message status:
                 0 - message contains shared memory key to attach
                 1 - access last attached shared memory segment
                     (shared memory key not used here)
                 2 - detach last attached shared memory segment
                     (shared memory key not used here)
                shm_key - shared memory segment key
                shm_size - shared memory segment size in bytes
  RETURNS:
       =0: OK,
       <0: - UNIX system error, EIDRM, EACCES, EAGAIN, EFAULT or EINVAL
             (see /usr/include/sys/errno.h)
******************************************************************************/
int SendShmMsg(int dest_qid, int dest, int srce, int code, int stat,
                int shm_key, int shm_size)
{
    char    buf[INFO_LEN];
    char    msg[INFO_LEN];
    int        len;

    len = PackShmMsg(buf, shm_key, shm_size);
    len = PackMsg(msg, dest, srce, 0, stat, SHARED_CLASS, code, buf, len);
    return(SendMsg(dest_qid, msg, len));
}

/*****************************************************************************
  FUNCTION: int PackShmMsg(char *ms_info, int shm_key, int shm_size)
  PURPOSE: Pack shared memory information
  PARAMETERS: ms_info - buffer to hold packed message information
              shm_key - key of shared memory segment
              shm_size - size of shared memory segment
  RETURNS: length of message information to be sent
*****************************************************************************/
int PackShmMsg(char *ms_info, int shm_key, int shm_size)
{
    SHM_MSG    *ptr = (SHM_MSG *)ms_info;

    /* Insert shared memory key */
    PutField(ptr->shm_key, shm_key, SHMKEY_LEN);

    /* Insert shared memory size */
    PutField(ptr->shm_size, shm_size, SHMSIZE_LEN);

    /* Length of message information to be sent */
    return(SHMKEY_LEN + SHMSIZE_LEN);
}

/*****************************************************************************
  FUNCTION: UnpackShmMsg(char *msg_info, int *shm_key, int *shm_size)
  PURPOSE: Unpack shared memory information from message information
  PARAMETERS: ms_info - message information to unpack
              shm_key - -> returned shared memory segment key
              shm_size - -> returned shared memory segment size
  RETURNS: none
*****************************************************************************/
void UnpackShmMsg(char *ms_info, int *shm_key, int *shm_size)
{
    SHM_MSG    *ptr = (SHM_MSG *)ms_info;

    /* Insert shared memory key */
    *shm_key = GetField(ptr->shm_key, SHMKEY_LEN);

    /* Insert shared memory size */
    *shm_size = GetField(ptr->shm_size, SHMSIZE_LEN);
}

/***************************************************************************
  FUNCTION: int SetupShm(int shm_key, char *shm_size)
  PURPOSE : set up shared memory segment
  PARAMETERS: shm_key - key of shared memory segment
              shm_size - size of shared memory segment
  RETURNS: !NULL - pointer to shared memory segment
            NULL - error
 **************************************************************************/
char *SetupShm(int shm_key, int shm_size)
{
    int        shm_id;
    char    *shm_ptr;

    if ((shm_id = shmget(shm_key, shm_size, 0666 | IPC_CREAT)) < 0)
        return(NULL);
    if ((shm_ptr = (char *)shmat(shm_id, 0, 0)) == (char *)-1)
        return(NULL);
    return(shm_ptr);
}

/***************************************************************************
  FUNCTION: int GetShm(int shm_key, char *shm_size)
  PURPOSE : Get pointer to shared memory segment
  PARAMETERS: shm_key - key of shared memory segment
              shm_size - size of shared memory segment
  RETURNS: !NULL - pointer to shared memory segment
             NULL - error
 **************************************************************************/
char *GetShm(int shm_key, int shm_size)
{
    int        shm_id;
    char    *shm_ptr;

    if ((shm_id = shmget(shm_key, shm_size, 0666)) < 0)
        return(NULL);
    if ((shm_ptr = (char *)shmat(shm_id, 0, 0)) == (char *)-1)
        return(NULL);
    return(shm_ptr);
}

/***************************************************************************
  FUNCTION: int RemoveShm(int shm_key, char *shm_size)
  PURPOSE : Remove Shared Memory Segment
  PARAMETERS: shm_key - key of shared memory segment
              shm_size - size of shared memory segment
  RETURNS: !NULL - id of shared memory removed
             NULL - error
 **************************************************************************/
int RemoveShm(int shm_key, int shm_size)
{
    int        shm_id;

    if ((shm_id = shmget(shm_key, shm_size, 0666)) < 0)
        return(0);
    if (shmctl(shm_id, IPC_RMID, 0))
        return(0);
    return(shm_id);
}

/***************************************************************************
  FUNCTION: void DetachShm(char *shm_ptr)
  PURPOSE : Detach shared memory segment
  COMMENTS: On detaching the last reference to a shared memory segment,
            the segemnt is removed from the system.
  PARAMETER: shm_ptr - pointer to shared memory segment
 **************************************************************************/
void DetachShm(char *shm_ptr)
{
    shmdt(shm_ptr);
}

int CheckShm(int shm_key, int shm_size)
{
    int        shm_id;

    if ((shm_id = shmget(shm_key, shm_size, 0666)) < 0)
        return(0);

    return shm_id;
}
#else

char *SetupShm(int shm_key, int shm_size)
{
    return(NULL);
}

char *GetShm(int shm_key, int shm_size)
{
    return(NULL);
}

int RemoveShm(int shm_key, int shm_size)
{
    return(-1);
}

void DetachShm(char *shm_ptr)
{
}

int CheckShm(int shm_key, int shm_size)
{
    return -1;
}

#endif