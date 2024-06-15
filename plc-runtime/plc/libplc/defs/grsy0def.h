/****************************************************************************
* Fichier     : grsy0def.h
* Description : ISaGRAF WDS
* Creation    : LIS 20/01/93
*****************************************************************************
* Modif       : 25/07/94 stdio.h for VxWorks
*             : 03/08/94 DEF_CHILD, DEF_ERR, DEF_TMR 
*             : 30/08/94 MIN_SLEEP, LOOP_SLEEP 
*             : 10/10/94 DEF_SIGNAL
*             : 24/11/94 LynxOS (GG)
*****************************************************************************/

/************ Operating system is DOS ********************/

#ifdef DOS

#define INTEL
#undef  MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MODIF
#define DEF_MSG
#define DEF_PRINTF
#define DEF_SAV
#undef  DEF_SHELL
#define DEF_SYSHELL
#define DEF_TMR
#undef  DEF_THREAD
#define DEF_USF
#endif

/************ Operating system is Windows ********************/

#ifdef WNDOS

#define INTEL
#undef  MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#undef  DEF_MDB
#undef  DEF_MODIF
#define DEF_MSG
#undef  DEF_PRINTF
#undef  DEF_SAV
#undef  DEF_SHELL
#undef  DEF_SYSHELL
#define DEF_TMR
#undef  DEF_THREAD
#define DEF_USF

#define printf trace
#define malloc ssa_give

#endif

/************ Operating system is OS9 ********************/

#ifdef OS9

#undef  INTEL
#define MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MODIF
#define DEF_MSG  
#define DEF_PRINTF
#define DEF_SAV
#undef  DEF_SIGNAL
#define DEF_SHELL
#define DEF_SYSHELL
#define DEF_TMR
#undef  DEF_THREAD
#define DEF_USF
#endif

/************ Operating system is MOMUX ********************/

#ifdef MOMUX

#undef  INTEL
#define MOTOROLA
#define DEF_BKP
#define DEF_CHILD 
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MSG  
#define DEF_SAV
#define DEF_TMR
#define DEF_USF
#define DEF_THREAD
#endif

/************ Operating system is VMEX ********************/

#ifdef VMEX

#undef  INTEL
#define MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MODIF
#define DEF_MSG  
#define DEF_PRINTF
#define DEF_SAV
#undef  DEF_SHELL
#undef  DEF_SYSHELL
#define DEF_TMR
#define DEF_THREAD
#define DEF_USF
#endif

/************ Operating system is VXWORKS ********************/

#ifdef VXWORKS

#undef  INTEL
#define MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MODIF
#define DEF_MSG  
#define DEF_PRINTF
#define DEF_SAV
#undef  DEF_SHELL
#undef  DEF_SYSHELL
#define DEF_TMR
#define DEF_THREAD
#define DEF_USF
#endif

/************ Operating system is COMPACT MOTOROLA ********************/

#ifdef CP_MOTO

#undef  INTEL
#define DEF_COMPACT
#define MOTOROLA
#endif

/************ Operating system is COMPACT INTEL ********************/

#ifdef CP_INTEL

#define INTEL
#define DEF_COMPACT
#undef  MOTOROLA
#endif

/************ Operating system is LynxOS ********************/

#ifdef Lynx

#define INTEL
#undef MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MODIF
#define DEF_MSG  
#define DEF_PRINTF
#undef DEF_SAV
#define DEF_SIGNAL
#define DEF_SHELL
#define DEF_SYSHELL
#define DEF_TMR

/* Necessario per evitare la definizione automatica di "main". */
#define DEF_THREAD

#define DEF_USF
#endif


/************ Operating system is SUN Solaris ********************/

#ifdef Solaris

#define INTEL
#undef MOTOROLA
#define DEF_BKP
#define DEF_CHILD
#define DEF_ERR
#define DEF_FBLOCK
#define DEF_FLOAT
#define DEF_IL
#define DEF_ILDBG
#define DEF_INITVAL
#define DEF_LOCK
#define DEF_MDB
#define DEF_MODIF
#define DEF_MSG  
#define DEF_PRINTF
#undef DEF_SAV
#define DEF_SIGNAL
#define DEF_SHELL
#define DEF_SYSHELL
#define DEF_TMR

/* Necessario per evitare la definizione automatica di "main". */
#define DEF_THREAD

#define DEF_USF
#endif


/******* Please define your operating system *******************/

#ifndef DOS
#ifndef OS9
#ifndef VMEX
#ifndef VXWORKS
#ifndef WNDOS
#ifndef CP_MOTO
#ifndef CP_INTEL
#ifndef MOMUX
#ifndef Lynx
#ifndef Solaris

#error "********* Define operating system: DOS, OS9, VMEX, VXWORKS,... ******"

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

/******* Function return values *******************/

#define TRUE   1
#define FALSE  0
#define CR_OK  0
#define CR_BAD -1

/****** Bytes copy *************************/

#define BT(a)     (*((unsigned char*)a))
#define MOVS(d,s) BT(d)=BT(s);BT(d+1)=BT(s+1)
#define MOVL(d,s) BT(d)=BT(s);BT(d+1)=BT(s+1);BT(d+2)=BT(s+2);BT(d+3)=BT(s+3)

#ifdef INTEL
#define COPYS(d,s) BT(d)=BT(s+1);BT(d+1)=BT(s)
#define COPYL(d,s) BT(d)=BT(s+3);BT(d+1)=BT(s+2);BT(d+2)=BT(s+1);BT(d+3)=BT(s)
#else
#define COPYS(d,s) BT(d)=BT(s);BT(d+1)=BT(s+1)
#define COPYL(d,s) BT(d)=BT(s);BT(d+1)=BT(s+1);BT(d+2)=BT(s+2);BT(d+3)=BT(s+3)
#endif

/******* Loop macro *****************************/

#define FOR(count,max)  for((count)=0;(count)<(max);(count)++)

/******* Basic types ****************************/

typedef unsigned char  byte;
typedef unsigned short word;

typedef void (*UFP)();                        /* user function pointer */
typedef UFP  (*UFP_LIST)();                   /* array of usf pointers */

typedef word (*IBP)(word);                    /* Init fbl Pointer */
typedef void (*ABP)(word,void *,void *);      /* Activate fbl Pointer */
typedef void (*RBP)(word,void *,word,void *); /* Read fbl Pointer */
typedef ABP  (*FBL_LIST)();                   /* aray of fbl pointers */

/******* ANSI Prototypes *************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#ifndef VMEX
#include <stdlib.h>
#endif

/******* ISaGRAF definitions ********************/

#include <grsy0df2.h>
#include <grsy0pro.h>

#define MAX_USF    256        /* maximum number of user function */
#define MAX_FBL    256        /* maximum number of function block */ 
#define MIN_SLEEP    1        /* minimum sleep duration (ms) for waittim */
#define LOOP_SLEEP  10        /* sleep duration (ms) for polling loop */

/**************** COMPACT HOST DEFINITION ***********/

#ifdef DEF_COMPACT 

#define BASE_RAM   (0x6000L)    /* base address for ISaGRAF tables */ 
#define MAX_KER    (0x800L)     /* 2K max data base kernel */
#define MAX_APPLI  (0x800L)     /* 2K max appli data base */

#endif

/*************** Error management *********************/

#ifndef DEF_ERR
#define sys_err(x,y)     /* nothing to save code */
#endif

/* eof */

