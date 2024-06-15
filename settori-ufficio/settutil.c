/*
* settutil.c : gestione settori di magazzino : funzioni di utilita'
* 
* Progetto Easy Picking 3.0 : wella
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <dbfun.h>
#include <proc_list.h>
#include <picking.h>

#include	"linklist.h"
#include	"settstruct.h"
#include	"settext.h"
#include	"settfun.h"

