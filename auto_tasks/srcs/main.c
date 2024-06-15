/*
* Modulo : receive.c
* ------------------
* Modulo per la gestione della ricezione dati da host
*
* Progetto Mondadori - Delta Trading Srl - Gestione Ordini
*
* Roberto Mantovani - Stefano Tarroni
* Copyright A&L 1995-2005
*
* Storia delle modifiche
*
* Data       Descrizione
* ---------- -------------------------------------------
* 31-01-2002 Prima Stesura - derivato da momo-3.4.26
* ---------- -------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <glib.h>
#include <gio/gio.h>

#include <fcntl.h>
// #include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef TRACE
	#include "trace.h"
#endif

#include <msq_lib.h>
#include <shared.h>

#include <proc_list.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __authors__[];
extern char __copyright__[];

ep_bool_t bAlone=FALSE;

void print_help(char *name)
{
    printf("%s : copyright 2000-2021 A&L srl\n",name);
    printf("Uso: %s [-h] [-a] [-p <pid>]\n",name);
    printf("\t-h       : help\n");
    printf("\t-a       : run stand alone\n");
    printf("\t-p <pid> : PID\n");
}

/*
* Main function
* inizializzazione dati
* lettura dei dati di configurazione
*/
int main(int argc,char **argv)
{
	ep_bool_t bErrore=FALSE;
	//char szMsg[128];
#ifdef TRACE
	char szBufTrace[80];
#endif
	time_t lActualTime=0, lStartTime=0;
	int nDiffSeconds=0;
	int nCheckCambioStatoTimer=0;
	struct tm *lt;
	static ep_bool_t bDatiSpediti=FALSE;
	static ep_bool_t bReindexEseguito=FALSE;
	char szParagraph[128];
	ep_bool_t bOK;
	int nIndex;
	int nCodTask=0;
	ATASK Task;

	int nOpt;

	while ((nOpt = getopt(argc, argv, "hap:")) != -1) {
		switch(nOpt){
			case 'a':	/* Stand Alone */
				bAlone=TRUE;
			break;
			case 'h':	/* help */
                print_help(argv[0]);
			break;
			case 'p':	/* Process ID */
				nPID=atoi(optarg);
			break;
			default:
				bErrore=TRUE;
			break;
		}
	}
	if(nPID==0){
		fprintf(stderr,"PID non definito : utilizzo %d\n",PROC_AUTO_TASKS);
		nPID=PROC_AUTO_TASKS;
	}
	if(bErrore){
        print_help(argv[0]);
	}


#ifdef TRACE
	/*
	* Apro il file trace 
	*/
	sprintf(szBufTrace,"%s/%s%s",Cfg.szPathTrace, g_path_get_basename (argv[0]),TRACE_FILE_SUFFIX);
	open_trace(argv[0], szBufTrace, TRACE_FILE_MAXSIZE);

	trace_debug(FALSE, TRUE, "Module Name     : %s",argv[0]);
	trace_debug(FALSE, TRUE, "Version         : %s",__version__);
	trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
	trace_debug(FALSE, TRUE, "Authors         : %s",__authors__);
	trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
	trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
	trace_debug(TRUE, TRUE, "Started");
#endif

	/*
	* lettura del file di configurazione
	*/
	ReadConfiguration();

	/* Apro la coda messaggi principale */
	if(OpenProcessMsgQ(Cfg.nMainID)<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Apertura coda messaggi principale fallita");
#endif
	}
	/* Apro la coda messaggi locale */
	if(CreateProcessMsgQ( nPID, 1)<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Apertura coda messaggi locale fallita");
#endif
	}

	/*
	* Lettura configurazione tabelle utilizzate
	*/
	/*
	InsertTableInfo(&tCorriere,       Cfg.szPathData,"corriere");
	InsertTableInfo(&tCorriereAgg,    Cfg.szPathData,"corr_agg");
	InsertTableInfo(&tCorrSDA,        Cfg.szPathData,"corr_sda");
	InsertTableInfo(&tQuadProd,       Cfg.szPathData,"quad_prod");
	InsertTableInfo(&tFlussoCedola,   Cfg.szPathData,"flusso_cedola");
	InsertTableInfo(&tUdcRestituitiExport,   Cfg.szPathData,"udc_restituiti_export");
	*/

	/*
	* nuova gestione mondadori-rcs
	*/
	InsertTableInfo(&tOrdProd,         Cfg.szPathData,"ord_prod");
	InsertTableInfo(&tColProd,         Cfg.szPathData,"col_prod");
	InsertTableInfo(&tRigProd,         Cfg.szPathData,"rig_prod");
	InsertTableInfo(&tEvasi,           Cfg.szPathData,"evasi");
	InsertTableInfo(&tContenutoColli,  Cfg.szPathData,"contenuto_colli");
	InsertTableInfo(&tFlussoCedola,    Cfg.szPathData,"flusso_cedola");
	InsertTableInfo(&tFlussoCedola,    Cfg.szPathData,"flusso_cedola");
	ReadTablesInfo();

	/*
	* intercetto le uscite irregolari dal programma
	*/
	signal(SIGINT,SafeIntFunc);
	signal(SIGILL,SafeIntFunc);
	signal(SIGTERM,SafeIntFunc);
#ifdef __linux__
	signal(SIGQUIT,SafeIntFunc);
	signal(SIGKILL,SafeIntFunc);
	signal(SIGSYS,SafeIntFunc);
#endif

	/* 
	* Connessione al database
	*/
	bOK=FALSE;
	nIndex=0;
	while(!bOK && nIndex<5){
		if(FALSE==DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword)){
			sleep(1);
		} else {
			bOK=TRUE;
		}
		nIndex++;
	}

		

	if(bAlone==FALSE){
		/*
		* avverto MAIN dello start
		*/
		SendMessage(Cfg.nMainID, nPID,  PROGRAM_STARTED, NULL);
	}

	// Ricavo il tempo di start prima dell'inizio del ciclo infinito
	lStartTime = time(NULL);
	/*
	* Ciclo infinito di controllo e ricezione dati
	*/
	for(;;){
		DBTestConnection(TRUE);

		// Ricavo il tempo attuale
		lActualTime = time(NULL);
		// Ricavo i secondi di differenza tra lo start del programma e il tempo attuale
		nDiffSeconds=lActualTime-lStartTime;
		// Riempio la struttura per ricavare il tempo reale in ora, giorno, min ...
		lt = localtime(&lActualTime);

		/*
		* TASK con innesco da database
		*/
		// Ricavo l'innesco di tasks dal db
		nCodTask=GetNewTasksFromDB(&Task);
		switch(nCodTask){
			case COD_TASK_GENERICO:
			break;
			default:
			break;
		}


		/*
		* Primo task:
		*/

		/*
		* controllo cambio stato linea per creazione file distinta pdf e xab pdf
		*/
		if (nDiffSeconds>=nCheckCambioStatoTimer){
			nCheckCambioStatoTimer=nDiffSeconds+Cfg.nDelayCheckCambioStato;
			if(!CheckCambioStatoLinea('E',TRUE)){
			}
		}

		/*
		* 17-06-2013 rm+lg : porto invio periodico e reindex periodico in autotasks per poi 'staccare' autotasks 
		*/
		/* rinfresco i dati di invio e reindex periodico */
		strcpy(szParagraph,"General Settings");
		Cfg.nInvioPeriodico        = GetFileInt(szParagraph,      "InvioPeriodico",          0, Cfg.szCfgFileName,NULL);
		Cfg.nOraInvioPeriodico     = GetFileInt(szParagraph,      "OraInvioPeriodico",       0, Cfg.szCfgFileName,NULL);
		Cfg.nReindexPeriodico      = GetFileInt(szParagraph,      "ReindexPeriodico",        0, Cfg.szCfgFileName,NULL);
		Cfg.nOraReindexPeriodico   = GetFileInt(szParagraph,      "OraReindexPeriodico",     0, Cfg.szCfgFileName,NULL);



		/*
		* rm 05-07-2002 gestione invio periodico 
		*/
		if(FALSE && Cfg.nInvioPeriodico){
			struct tm *ltime;

			ltime = localtime(&lActualTime);

			/*
			* rm 25-09-2008 : azzero i dati spediti 
			*/
			if(ltime->tm_hour!=Cfg.nOraInvioPeriodico && bDatiSpediti){
				bDatiSpediti=FALSE;
			}
				
		}

		/*
		* rm 04-10-2002 gestione reindex periodico 
		*/
		/* -------------------------------------------------------------------- */
		/*                     MOMENTANEMENTE SOSPESO                           */
		/* -------------------------------------------------------------------- */
		if(FALSE && Cfg.nReindexPeriodico){
			struct tm *ltime;

			ltime = localtime(&lActualTime);

			if(ltime->tm_hour!=Cfg.nOraReindexPeriodico && bReindexEseguito){
				bReindexEseguito=FALSE;
			}
				
			if(ltime->tm_hour==Cfg.nOraReindexPeriodico && !bReindexEseguito){
				if(!bReindexEseguito){

#ifdef TRACE
					trace_debug(TRUE, TRUE, "Aggiornamento periodico iniziato");
#endif
					/* aggiornamento */
					do_aggiorna_db();

#ifdef TRACE
					trace_debug(TRUE, TRUE, "Aggiornamento periodico terminato");
#endif
					bReindexEseguito=TRUE;

				}
			} else {
				bReindexEseguito=FALSE;
			}
		}



		/********************************************************************************
		* Task finali di servizio:
		*********************************************************************************/
		/*
		* Faccio comunque una sleep di 1 secondo, tanto non ho processi
		* che richiedono una spazzolata ogni sempre
		*/
		sleep(1);
		/*
		* Se richiesta uscita esco in modo regolare
		*/
		if(bExitRequest){
			SafeExit();
		}
	}
	return 0;
} 

