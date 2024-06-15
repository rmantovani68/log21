/* shared.h - header file for shared.c */
#ifndef _SHARED_H
#define _SHARED_H

int SendShmMsg(int, int, int, int, int, int, int);
int PackShmMsg(char *, int, int);
void UnpackShmMsg(char *, int *, int *);
char *SetupShm(int, int);
char *GetShm(int, int);
int RemoveShm(int shm_key, int shm_size);
void DetachShm(char *);
int CheckShm(int shm_key, int shm_size);

#endif	/* _SHARED_H */
