/*
* Modulo : rffun.c
* -----------------
* Funzioni utilizzate da tutti i moduli componenti il processo RF
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19/06/2000
* 28/03/2001 Porting CodBase -> postgreSQL
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 2000-2001
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <ep-common.h>
#include <picking.h>
#include <proc_list.h>
#include <ep-db.h>

#include <ncurses.h>
#include <panel.h>
#include <form.h>
#include <menu.h>

#include <dbfun-deltacarichi.h>

#include "rf.h"
#include "rfstruct.h"
#include "rfext.h"
#include "rffun.h"

/*
* ReadConfiguration()
* lettura della configurazione
*/
ep_bool_t ReadConfiguration(char *szCfgFileName,ep_bool_t bReadProcInfo)
{
	char szCurrentDirectory[128];

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	strcpy(Cfg.szCfgFileName,szCurrentDirectory);
	strcat(Cfg.szCfgFileName,"/");
	strcat(Cfg.szCfgFileName,szCfgFileName);

	if(!FileExists(Cfg.szCfgFileName)){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Read Configuration : file %s not exists",szCfgFileName);
#endif
		return FALSE;
	}

	/* CFGItems viene dichiarato e inizializzato in mainvar.c */
	ReadCfgItems(CFGItems,Cfg.szCfgFileName,0);

	/*
	* Lettura dati processi
	*/
	if(bReadProcInfo){
		ReadProcInfo(szCfgFileName);
	}
	return TRUE;
}
