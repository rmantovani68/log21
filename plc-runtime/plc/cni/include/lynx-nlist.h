#ifndef __nlist_h
#define __nlist_h
/************************************************************
(C) Copyright 1987-1994
Lynx Real-Time Systems, Inc.
Los Gatos, CA
All rights reserved.

 File: nlist.h
$Date: 94/06/17 21:14:46 $
$Revision: 5.3 $
************************************************************/

#define I386
#define __I386__
#ifdef __cplusplus
extern "C" {
#endif

/*
	There is a small problem with the nlist structure.  It would be
	very nice if there weren't a union for n_name so that a program
	could have an initialized array of nlist structures ready for a
	call to nlist() to fill in the neat values.  Also, there is a
	compatability problem with 4.2.  They defined nlist two different
	ways depending upon which include file is used.  Lynxos does the same
	thing, although in a slightly more manageable fashion.
*/

/*
	format of the symbol table entries
*/
struct nlist {
#ifdef _FROM_AOUT
  union {
    char       *n_name;			/* in-core pointer to name */
    long        n_strx;			/* index into file string table */
  } n_un;
#else
  char         *n_name;			/* if nlist.h included directly */
#endif
  unsigned char n_type;			/* symbol type --- see below */
  char          n_other;		/* filler */
  short         n_desc;			/* description for a stab entry */
  unsigned long n_value;		/* value (offset) of the symbol */
};

/*
	Values for the n_type field
*/
#define N_UNDF 0x00            /* undefined */
#define N_ABS  0x02            /* Absolute */
#define N_TEXT 0x04            /* In text segment */
#define N_DATA 0x06            /* In data segment */
#define N_BSS  0x08            /* In bss segment */
#define N_COMM 0x12            /* In blank common */

#define N_EXT  0x01            /* or'ed in if external */
#define N_TYPE 0x1e            /* mask for all bits */

#define N_FN   0x1f            /* file name symbol */
#define N_STAB 0xe0            /* mask for stab entry */

#ifdef __cplusplus
}
#endif

#endif /* __nlist_h */
