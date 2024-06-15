/*
* Modulo : recutil.c
* --------------------
* Funzioni di varia utilita'
*
*
* Data creazione 17/05/2001
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <malloc.h>
#include <signal.h>

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef TRACE
	#include <trace.h>
#endif

#include <ep.h>
#include <shared.h>
#include <pmx_msq.h>

#include <proc_list.h>


#include <dbfun.h>
#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"







