/* mscr_out.h - header file for mscreen_out.c */
#ifndef _MSCR_OUT_H
#define _MSCR_OUT_H

/* Lengths of multiscreen write fields */
#define SCR_LEN			2
#define POS_LEN			2
#define CLR_LEN			2
#define SCRHDR_LEN		(SCR_LEN + 2 * POS_LEN + 2 * CLR_LEN)
#define STR_LEN			256	/* max. length of string to screen */

extern void SetMscreenName(char *);
extern int OpenMscreen(int, char *, int);
extern int CloseMscreen(int);
extern int ClearMscreen(int);
extern int WriteMscreen(int, int, int, int, int, char *);
extern int WriteMscreenStr(int, char *);
extern int WriteMscreenColour(int, int, int);
extern int WriteMscreenMsg(char *);
extern int PackMscreenMsg(char *, int, int, int, int, int, char *);

#endif	/* _MSCR_OUT_H */
