/* keybd_in.h - header file for keybd_in.c */
#ifndef _KEYBD_IN_H
#define _KEYBD_IN_H

#define KEYC_LEN	4

int UnpackKeybdMsg(char *);
int SendKeybdMsg(int, int, int, int);

#endif /* _KEYBD_IN_H */
