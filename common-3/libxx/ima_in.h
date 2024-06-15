/* ima_in.h - header file for ima_in.c */
#ifndef _IMA_IN_H
#define _IMA_IN_H

#define IMAC_LEN	1

int UnpackImatronMsg(char *, char *, int);
int SendImatronMsg(int, int, int, int, char *);
int SendImatronStatus(int, int, int, int, int);

#endif	/* _IMA_IN_H */
