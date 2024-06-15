
/*
* @(#) sysdep.h 1.1 Mon Sep 22 09:59:32 MET DST 1997
*
*  Tipi (ed altro) dipendenti dal sistema.
*
* 05/10/94 GG 1.0 Prima stesura.
* 22/09/97 GG 1.1 Un piccolo aggiustamento per compilazione su NT.
*/

#ifndef _SYSDEP_H

#define _SYSDEP_H

#if  defined(_WINDOWS) && !defined(WIN32)
#define FAR_ far
#else
#define FAR_
#endif

#if defined(WIN32)
#define MAXPATHLEN FILENAME_MAX
#endif

#define PTR_ FAR_ *

typedef void PTR_ LPVOID;
typedef char PTR_ LPCHAR;

#endif

