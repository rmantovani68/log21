#ifndef __ansidecl_lynx_h
#define __ansidecl_lynx_h
/************************************************************
(C) Copyright 1993-1994
Lynx Real-Time Systems, Inc.,  Los Gatos, CA
All rights reserved.

 File: ansidecl_lynx.h
$Date: 94/08/18 10:23:28 $
$Revision: 1.5 $
************************************************************/


#define i386 1
#define I386 1
#define __I386__ 1

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This is a handy little trick for making ANSI C function
 * prototypes that will not bust the normal, K&R C compiler.
 */

/* external reference declarations */
/* use: extern int foo _AP((int x, char c, long d)); */
#if defined(__NO_AP) 	/* Command line override option */
#define _AP(args)    ()
#else
#if defined(__STDC__) || defined(__cplusplus)
#define _AP(args)    args
#else
#define _AP(args)    ()
#endif
#endif /* __NO_AP */

#ifdef __cplusplus
}
#endif

#endif /* __ansidecl_lynx_h */
