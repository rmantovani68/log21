/* cross_in.h - header file for crossmat_in.c */
#ifndef _CROSS_IN_H
#define _CROSS_IN_H

#define CROSSC_LEN	1

int UnpackCrossmatMsg(char *, char *, int);
int SendCrossmatMsg(int, int, int, int, char *);
int SendCrossmatStatus(int, int, int, int, int);

#endif	/* _CROSS_IN_H */
