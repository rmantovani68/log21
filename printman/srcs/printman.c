/*
* printman.c
* 
* Easy Picking 4.0
*
* Autore : Roberto Mantovani / Luca Guerrini
*
* Copyright A&L srl 2003-2021
*/
#include <glib.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>
#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>

#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"

extern char rcsid[];
extern char __version__[];
extern char __module_name__[];
extern char __module_version__[];
extern char __customer__[];
extern char __authors__[];
extern char __copyright__[];
extern char __configuration_file__[];

/*
* main()
*/
int main(int argc,char** argv)
{
	int nOpt;
	int nId;
	int nCod;
	int nStampe;
	int nStampeIndex;
	char szStampante[256];
	char szParametri[256];
	DBresult *DBRes;
	DBresult *DBResUpdate;

	/*
	* Lettura della configurazione corrente
	*/
	if(!ReadConfiguration(__configuration_file__)){
		fprintf(stderr, "Cannot read configuration file [%s]", __configuration_file__);
		exit(1);
	}

#ifdef TRACE
	/* Apro il file trace */
	sprintf(szBufTrace,"%s/%s%s",Cfg.szPathTrace,__module_name__,TRACE_FILE_SUFFIX);
	open_trace(__module_name__, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

	trace_debug(TRUE, TRUE, "Started");
	trace_debug(FALSE, TRUE, "Module Name     : %s",__module_name__);
	trace_debug(FALSE, TRUE, "Version         : %s",__version__);
	trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
	trace_debug(FALSE, TRUE, "Authors         : %s",__authors__);
	trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
	trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
	trace_debug(TRUE, TRUE, "Started");

	TraceCfgItems(CFGItems);
#endif

	/*
	* Connetto a DB Postgres
	*/
	if(!DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName,Cfg.szDBUser,Cfg.szDBPassword)){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Connection to database [%s:%s %s] failed.", Cfg.szDBHost,Cfg.szDBName,Cfg.szDBUser);
#endif
		exit(1);
	}

	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, SafeIntFunc);
	signal(SIGINT,SafeIntFunc);
	signal(SIGILL,SafeIntFunc);
#ifdef __linux__
	signal(SIGQUIT,SafeIntFunc);
	signal(SIGKILL,SafeIntFunc);
#endif

	/*
	* gestione program loop
	*/
	for(;;){

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select id_stampa,codice_tipo_stampa,nome_stampante,stringa_parametri from stampe where stato='R' order by id_stampa,priorita desc;");
		nStampe=0;
		if((nStampe=DBntuples(DBRes))){
			for(nStampeIndex=0;nStampeIndex<nStampe;nStampeIndex++){

				nId=atoi(DBgetvalue(DBRes,nStampeIndex,0));
				nCod=atoi(DBgetvalue(DBRes,nStampeIndex,1));
				strcpy(szStampante,DBgetvalue(DBRes,nStampeIndex,2));
				strcpy(szParametri,DBgetvalue(DBRes,nStampeIndex,3));

				switch(nCod){
					/*
					* Messaggi gestiti dall'applicazione
					*/
					case COD_STAMPA_FRONTESPIZIO_BANCALE:
					{
						char *pPtr;
						char *szSeparator=",\t\n";
						char szKey[128];

						/* numero bancale */ if((pPtr=strtok(szParametri,szSeparator))!=NULL){ strcpy(szKey,pPtr); }

#ifdef TRACE
						trace_debug(TRUE, TRUE, "COD_STAMPA_FRONTESPIZIO_BANCALE [%s] on [%s]",szKey ,szStampante);
#endif
						StampaFrontespizioBancale(szKey,szStampante);
#ifdef TRACE
						trace_debug(TRUE, TRUE, "COD_STAMPA_FRONTESPIZIO_BANCALE [%s] on [%s] - FINE",szKey,szStampante);
#endif
					}
					break;

					case COD_STAMPA_SPEDIZIONE:
					{
						char *pPtr;
						char *szSeparator=",\t\n";
						char szKey[128];

						/* codice spedizione */ if((pPtr=strtok(szParametri,szSeparator))!=NULL){ strcpy(szKey,pPtr); }

#ifdef TRACE
						trace_debug(TRUE, TRUE, "COD_STAMPA_SPEDIZIONE [%s] on [%s]",szKey ,szStampante);
#endif
						StampaSpedizione(szKey,szStampante);
#ifdef TRACE
						trace_debug(TRUE, TRUE, "COD_STAMPA_SPEDIZIONE [%s] on [%s] - FINE",szKey,szStampante);
#endif
					}
					break;

					case COD_STAMPA_DISTINTA_LINEA:
					{
						char *pPtr;
						char *szSeparator=",\t\n";
						char szKey[128];

						/* numero linea */ if((pPtr=strtok(szParametri,szSeparator))!=NULL){ strcpy(szKey,pPtr); }

#ifdef TRACE
						trace_debug(TRUE, TRUE, "COD_STAMPA_DISTINTA_LINEA [%s] on [%s]",szKey ,szStampante);
#endif
						StampaDistintaLinea(szKey,szStampante);
#ifdef TRACE
						trace_debug(TRUE, TRUE, "COD_STAMPA_DISTINTA_LINEA [%s] on [%s] - FINE",szKey,szStampante);
#endif
					}
					break;
				}

				/* setto il flag da ' ' a 'S' */
				DBResUpdate=DBExecQuery(Cfg.nDebugLevel>2,"update stampe set stato='S' where id_stampa=%d;",nId);
				DBclear(DBResUpdate);

			}
		} else {
			sleep(3);
		}
		DBclear(DBRes);
	}

	SafeIntFunc();
}	/* End of main() */

