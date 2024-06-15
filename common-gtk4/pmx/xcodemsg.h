/* xcodemsg.h - header file for xcodemsg.c */
#ifndef _XCODEMSG_H
#define _XCODEMSG_H

#define XCODE_LEN	2

int UnpackXcodeMsg(char *, char *, int);
int UnpackXcodeMsgL(char *, char *, int *);
int SendXcodeMsg(int, int, int, int, char *);
int SendXcodeMsgL(int, int, int, int, char *, int);
int SendXcodeStatus(int, int, int, int, int);

#endif	/* _XCODEMSG_H */
