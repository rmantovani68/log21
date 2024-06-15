/*
* Modulo : receive.c
* ------------------
* Modulo per la gestione della ricezione dati da host
*
* Roberto Mantovani
* Copyright A&L 2002
*
* Storia delle modifiche
*
* Data       Descrizione
* ---------- ---------------------------------------------------------------------
* 02-12-2017 Utilizzo di receive-rcs per ricezione dati su impianto mondadori rcs
* ---------- ---------------------------------------------------------------------
*/
#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <glib.h>
#include <gio/gio.h>

#include <picking.h>

#include <fcntl.h>
// #include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef TRACE
	#include "trace.h"
#endif

#include "ep-common.h"
#include "ep-db.h"
#include "shared.h"
#include "pmx_msq.h"

#include "proc_list.h"

#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __module_name__[];
extern char __module_version__[];
extern char __customer__[];
extern char __authors__[];
extern char __copyright__[];
extern char __configuration_file__[];

ep_bool_t bAlone=FALSE;

/*
* Main function
* inizializzazione dati
* lettura dei dati di configurazione
*/
int main(int argc,char **argv)
{
	ep_bool_t bErrore=FALSE;
	char szMsg[128];
#ifdef TRACE
	char szBufTrace[80];
#endif

	int nOpt;

	while ((nOpt = getopt(argc, argv, "hap:")) != -1) {
		switch(nOpt){
			case 'a':	/* Stand Alone */
				bAlone=TRUE;
			break;
			case 'h':	/* help */
				printf("%s : copyright 1995-2017 A&L srl\n",__module_name__);
				printf("Uso: %s [-h] [-a] [-p <pid>]\n",__module_name__);
				printf("\t-h       : help\n");
				printf("\t-a       : run stand alone\n");
				printf("\t-p <pid> : PID\n");
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
		fprintf(stderr,"PID non definito : usato [%d] di default\n",PROC_RECEIVE);
		nPID=PROC_RECEIVE;
	}
	if(bErrore){
		printf("%s : Copyright A&L srl 2003-2021\n",__module_name__);
		printf("Uso: %s [-h] [-a] [-p <pid>]\n",__module_name__);
		printf("\t-h       : help\n");
		printf("\t-a       : run stand alone\n");
		printf("\t-p <pid> : PID\n");
	}



	/*
	* lettura del file di configurazione
	*/
	if(ReadConfiguration(__configuration_file__,TRUE)==FALSE){
		fprintf(stderr, "Cannot read configuration file [%s] ... exiting",__configuration_file__);
		exit(1);
	}

#ifdef TRACE
	/*
	* Apro il file trace 
	*/
	sprintf(szBufTrace,"%s/%s_%d%s",Cfg.szPathTrace,__module_name__,nPID,TRACE_FILE_SUFFIX);
	open_trace(argv[0], szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

	trace_debug(FALSE, TRUE, "Module Name     : %s",__module_name__);
	trace_debug(FALSE, TRUE, "Global Version  : %s",__version__);
	trace_debug(FALSE, TRUE, "Module Version  : %s",__module_version__);
	trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
	trace_debug(FALSE, TRUE, "Authors         : %s",__authors__);
	trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
	trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
	trace_debug(FALSE, TRUE, "Configuration   : %s",__configuration_file__);
	trace_debug(TRUE, TRUE, "Started");
#endif

	if(bAlone==FALSE){
		/* Apro la coda messaggi principale */
		if(OpenProcessMsgQ(Cfg.nMainID)<0){
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Cannot open main messages queue");
#endif
		}
	}
	/* Creo la coda messaggi locale  */
	if(CreateProcessMsgQ(nPID, 1)<0){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Cannot create local messages queue");
#endif
	}

	/*
	* Lettura configurazione tabelle utilizzate
	*/
	InsertTableInfo(&tRicOrd,     Cfg.szPathData,"import_ric_ord");
	InsertTableInfo(&tRicArt,     Cfg.szPathData,"import_ric_art");
	InsertTableInfo(&tCatalogo,   Cfg.szPathData,"import_catalogo");

	ReadTablesInfo();
	/*
	* intercetto le uscite irregolari dal programma
	*/
	signal(SIGINT,SafeIntFunc);
	signal(SIGQUIT,SafeIntFunc);
	signal(SIGILL,SafeIntFunc);
	signal(SIGKILL,SafeIntFunc);
	signal(SIGSYS,SafeIntFunc);
	signal(SIGTERM,SafeIntFunc);

	/* 
	* Connessione al database
	*/
	if(!DBConnect(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName)){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Connection to database [%s:%s] failed.", Cfg.szDBHost,Cfg.szDBName);
#endif
		SafeIntFunc();
		exit(1);
	}

	if(bAlone==FALSE){
		/*
		* avverto MAIN dello start
		*/
		SendMessage(Cfg.nMainID, nPID,  PROGRAM_STARTED, NULL);
	}

	/*
	* controllo e cancello i files di backup a inizio mese
	*/
	DeleteBackupFiles();

	/*
	* Ciclo infinito di controllo e ricezione dati
	*/
	for(;;){
		if(!CheckDatiRicezione()){
			if(bAlone==FALSE){
				sprintf(szMsg,"Errore in ricezione dati");
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			} else {
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Errore in ricezione dati");
#endif
			}

		}
		sleep(Cfg.nDelayRicezione);
		/*
		* Se richiesta uscita esco in modo regolare
		*/
		if(bExitRequest){
			SafeExit();
		}
	}
	return 0;
} 

/*
* Ricava il tipo ubicazione dati data la tipologia ordine
*/
ep_bool_t GetTipoUbicazione(char *szOrdProg,char *szTipoOrdine,char *szTipoUbicazioni)
{
	DBresult *DBRes;
	int nTuples;
	ep_bool_t bOK=TRUE;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select tipoubi from tipo_ubi where ordtipo='%s';",szTipoOrdine);
	/* verifica risultato */
	if((nTuples=DBntuples(DBRes))){
		strcpy(szTipoUbicazioni,DBgetvalue(DBRes,0,0));
	} else {
		bOK=FALSE;
		strcpy(szTipoUbicazioni,"");
#ifdef TRACE
		trace_debug(TRUE, TRUE, "ATTENZIONE ! : Ordine [%s - %s] Errore in attribuzione tipo ubicazione", szOrdProg,szTipoOrdine); 
#endif
	}
	DBclear(DBRes);

	return bOK;
}

/*
* CheckDatiRicezione()
* Ricezione dati da file
*/
ep_bool_t CheckDatiRicezione( void )
{
	FILE *fp;
	char szCommand[256];
	DBSTRUCT db[20];
	char *pszFiles[20];
	int nUpdated[20];
	ep_bool_t bUpdate[20];
	ep_bool_t bDelete[20];
	ep_bool_t bRicezione=FALSE;
	int nIndex=0;
	int nTables=0;
	int nTuples=0;
	int nRighe;
	int nCopie=0;
	int nPeso=0;
	int nRigaIndex=0;
	DBresult *DBRes;
	DBresult *DBResOrdini;
	DBresult *DBResRighe;
	DBresult *DBResUpdate;
	DBresult *DBResCat;
	ep_bool_t bRetValue=TRUE;
	char szOrdProg[128];
	char szRACDPRO[128];
	char szMsgData[256];
	char szMsg[256];
	char szBuffer[256];
	char szTipoOrdini[128];
	char szTipoUbicazioni[128];
	ep_bool_t bOK=TRUE;
	int nNMCED;
	int nNMORD;
	int nNMRGH;
	int nNMCPE;
	int nCedolaKit;
	char szCDKIT[128];
	DBresult *DBResKit;
	int nKits;

	
	/* nomi dei files di import dati */
	char szImportFile[128];
	
	/* semafori associati ai files di import */
	char szImportMonitor[128];


	nTables=0;
	/* Struttura DB */      /* File di import */                        /* Aggiorna */          /* Cancella */
	db[nTables]=tRicOrd;    pszFiles[nTables]=Cfg.szImportSpedizioni;   bUpdate[nTables]=FALSE;  bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tRicArt;    pszFiles[nTables]=Cfg.szImportRighe;        bUpdate[nTables]=FALSE;  bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tCatalogo;  pszFiles[nTables]=Cfg.szImportCatalogo;     bUpdate[nTables]=FALSE;   bDelete[nTables]=FALSE; nTables++; 


	/* prima ricevo il catalogo */
	bRicezione=FALSE;
	sprintf(szImportMonitor,"%s/ricezione-catalogo.mon",Cfg.szPathExport);
	if(FileExists(szImportMonitor)){
		bRicezione=TRUE;
		
		DBRes=DBExecQuery(Cfg.nDebugLevel,"truncate table import_catalogo;"); DBclear(DBRes);

		DBRes=DBExecQuery(Cfg.nDebugLevel,"BEGIN WORK;"); DBclear(DBRes);

#define ID_CATALOGO	2

		sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(pszFiles[ID_CATALOGO]));

		/*
		* Flag di tabella modificata
		*/
		nUpdated[ID_CATALOGO]=0;

		if((fp=fopen(szImportFile,"r"))!=NULL){
			char szBuffer[1024];

#ifdef TRACE
			trace_debug(TRUE, TRUE, "INIZIO RICEZIONE : [%s]",db[ID_CATALOGO].szTableName);
#endif
			if(bAlone==FALSE){
				/*
				* segnalo a video
				*/
				sprintf(szMsg,"Inizio ricezione [%s]",db[ID_CATALOGO].szTableName);
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			} else {
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Inizio ricezione [%s]",db[ID_CATALOGO].szTableName);
#endif
			}

			/* 
			* se e' prevista la cancellazione
			* verifico la presenza di records
			* nel file
			*/
			if(bDelete[ID_CATALOGO]){
				ep_bool_t bRecordPresente=FALSE;

				while(fgets(szBuffer,sizeof(szBuffer)-1,fp)){
					if(IsAsciiRecord(szBuffer,&db[ID_CATALOGO])){
						bRecordPresente=TRUE;
						break;
					}
					memset(szBuffer,(int)0,sizeof(szBuffer));
				}
				if(bRecordPresente){

					/*
					* torno all'inizio del file
					*/
					rewind(fp);

#ifdef TRACE
					trace_debug(TRUE, TRUE, "DELETE : %s",db[ID_CATALOGO].szTableName);
#endif
					/*
					* Cancello la copia della tabella eventualmente presente
					*/
					DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table %s_copy;",db[ID_CATALOGO].szTableName); 
					DBclear(DBRes);
					/*
					* Creo la copia della tabelle e inserisco il contenuto
					*/
					DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table %s_copy as select * from %s;",
						db[ID_CATALOGO].szTableName,db[ID_CATALOGO].szTableName); DBclear(DBRes);

					/*
					* cancello il contenuto della tabella 
					*/
					DBRes=DBExecQuery(Cfg.nDebugLevel,"delete from %s;",db[ID_CATALOGO].szTableName); DBclear(DBRes);
				}
			}


			while(fgets(szBuffer,sizeof(szBuffer)-1,fp)){
				switch(UpdateAsciiRecord(szBuffer,&db[ID_CATALOGO],bUpdate[ID_CATALOGO])){
					case TRUE:
						nUpdated[ID_CATALOGO]++;
					break;
					case DBRES_FATAL_ERROR:
						bOK=FALSE;
					break;
				}
				memset(szBuffer,(int)0,sizeof(szBuffer));
			}
			fclose(fp);



#ifdef TRACE
			trace_debug(TRUE, TRUE, "FINE RICEZIONE : [%s]",db[ID_CATALOGO].szTableName);
#endif
			if(bAlone==FALSE){
				/*
				* segnalo a video
				*/
				if(bOK){
					sprintf(szMsg,"Termine ricezione [%s]",db[ID_CATALOGO].szTableName);
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				} else {
					sprintf(szMsg,"Errore in ricezione [%s]",db[ID_CATALOGO].szTableName);
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				}
			} else {
#ifdef TRACE
				if(bOK){
					trace_debug(TRUE, TRUE, "Termine ricezione [%s]",db[ID_CATALOGO].szTableName);
				} else {
					trace_debug(TRUE, TRUE, "Errore in ricezione [%s]",db[ID_CATALOGO].szTableName);
				}
#endif
			}
		}

		sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(pszFiles[ID_CATALOGO]));
		/* 
		* rinomino il file di import
		*/
		if(FileExists(szImportFile)){
#ifdef TRACE
			trace_debug(TRUE, TRUE, "DATI : Rinomino %s in %s.%s-%s%s",
				szImportFile,
				szImportFile,
				GetDateYYYYMMDD(time((long *)0),szDateBuffer),
				GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");
#endif
			sprintf(szCommand,"mv -f %s %s.%s-%s%s",
				szImportFile,
				szImportFile,
				GetDateYYYYMMDD(time((long *)0),szDateBuffer),
				GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");

			system(szCommand);
		}
#ifdef TRACE
		trace_debug(TRUE, TRUE, "MONITOR : Rinomino %s in %s.%s-%s%s",
			szImportMonitor,
			szImportMonitor,
			GetDateYYYYMMDD(time((long *)0),szDateBuffer),
			GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");
#endif
		sprintf(szCommand,"mv -f %s %s.%s-%s%s",
			szImportMonitor,
			szImportMonitor,
			GetDateYYYYMMDD(time((long *)0),szDateBuffer),
			GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");

		system(szCommand);

		if(nUpdated[ID_CATALOGO]){
			/* CATALOGO */
			// Metto il flag e un identificativo a import_catalogo
			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update import_catalogo set prflric='%c';",IMPORT_CATALOGO_OK);
			sprintf(szMsg,"FILE CATALOGO: %d righe",atoi(DBcmdTuples(DBRes)));
			if(bAlone==FALSE){
				// SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			} else {
				fprintf(stderr,szMsg);
			}
			DBclear(DBRes);

			// Importo in catalogo
			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into catalogo ( PRNMCED, PRCDPRO, PRTPART, PRDSTIT, PRPESGR, PRLUNGH, PRLARGH, PRALTEZ) select  PRNMCED, PRCDPRO, PRTPART, PRDSTIT, PRPESGR, PRLUNGH, PRLARGH, PRALTEZ from import_catalogo where prflric='%c' and prcdpro not in (select prcdpro from catalogo)",IMPORT_CATALOGO_OK);
			sprintf(szMsg,"CATALOGO: importati %d prodotti",atoi(DBcmdTuples(DBRes)));
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			} else {
				fprintf(stderr,szMsg);
			}
			DBclear(DBRes);
		}


		if(bOK){
			DBRes=DBExecQuery(Cfg.nDebugLevel,"commit work;"); DBclear(DBRes);
		} else {
			DBRes=DBExecQuery(Cfg.nDebugLevel,"abort work;"); DBclear(DBRes);
		}
	}




	/* fine gestione catalogo */




	bRicezione=FALSE;
	sprintf(szImportMonitor,"%s/%s",Cfg.szPathExport, StrTrimAll(Cfg.szImportMonitor));
	if(FileExists(szImportMonitor)){
		bRicezione=TRUE;
		
		DBRes=DBExecQuery(Cfg.nDebugLevel,"truncate table import_ric_ord;"); DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel,"truncate table import_ric_art;"); DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel,"truncate table import_catalogo;"); DBclear(DBRes);

		DBRes=DBExecQuery(Cfg.nDebugLevel,"BEGIN WORK;"); DBclear(DBRes);

		for(nIndex=0; nIndex<nTables; nIndex++){

			sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(pszFiles[nIndex]));

			/*
			* Flag di tabella modificata
			*/
			nUpdated[nIndex]=0;

			if((fp=fopen(szImportFile,"r"))!=NULL){
				char szBuffer[1024];

#ifdef TRACE
				trace_debug(TRUE, TRUE, "INIZIO RICEZIONE : [%s]",db[nIndex].szTableName);
#endif
				if(bAlone==FALSE){
					/*
					* segnalo a video
					*/
					sprintf(szMsg,"Inizio ricezione [%s]",db[nIndex].szTableName);
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				} else {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Inizio ricezione [%s]",db[nIndex].szTableName);
#endif
				}

				/* 
				* se e' prevista la cancellazione
				* verifico la presenza di records
				* nel file
				*/
				if(bDelete[nIndex]){
					ep_bool_t bRecordPresente=FALSE;

					while(fgets(szBuffer,sizeof(szBuffer)-1,fp)){
						if(IsAsciiRecord(szBuffer,&db[nIndex])){
							bRecordPresente=TRUE;
							break;
						}
						memset(szBuffer,(int)0,sizeof(szBuffer));
					}
					if(bRecordPresente){

						/*
						* torno all'inizio del file
						*/
						rewind(fp);

#ifdef TRACE
						trace_debug(TRUE, TRUE, "DELETE : %s",db[nIndex].szTableName);
#endif
						/*
						* Cancello la copia della tabella eventualmente presente
						*/
						DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table %s_copy;",db[nIndex].szTableName); 
						DBclear(DBRes);
						/*
						* Creo la copia della tabelle e inserisco il contenuto
						*/
						DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table %s_copy as select * from %s;",
							db[nIndex].szTableName,db[nIndex].szTableName); DBclear(DBRes);

						/*
						* cancello il contenuto della tabella 
						*/
						DBRes=DBExecQuery(Cfg.nDebugLevel,"delete from %s;",db[nIndex].szTableName); DBclear(DBRes);
					}
				}


				while(fgets(szBuffer,sizeof(szBuffer)-1,fp)){
					switch(UpdateAsciiRecord(szBuffer,&db[nIndex],bUpdate[nIndex])){
						case TRUE:
							nUpdated[nIndex]++;
						break;
						case DBRES_FATAL_ERROR:
							bOK=FALSE;
						break;
					}
					memset(szBuffer,(int)0,sizeof(szBuffer));
				}
				fclose(fp);



#ifdef TRACE
				trace_debug(TRUE, TRUE, "FINE RICEZIONE : [%s]",db[nIndex].szTableName);
#endif
				if(bAlone==FALSE){
					/*
					* segnalo a video
					*/
					if(bOK){
						sprintf(szMsg,"Termine ricezione [%s]",db[nIndex].szTableName);
						SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
					} else {
						sprintf(szMsg,"Errore in ricezione [%s]",db[nIndex].szTableName);
						SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
					}
				} else {
#ifdef TRACE
					if(bOK){
						trace_debug(TRUE, TRUE, "Termine ricezione [%s]",db[nIndex].szTableName);
					} else {
						trace_debug(TRUE, TRUE, "Errore in ricezione [%s]",db[nIndex].szTableName);
					}
#endif
				}
			} else {
				/* se non ricevo tutto (ad esclusione del catalogo) do errore */
				if(nIndex!=2) {
					bOK=FALSE;
				}
			}
			if(!bOK){
				break;
			}
		}
		for(nIndex=0; nIndex<nTables; nIndex++){
			sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(pszFiles[nIndex]));
			/* 
			* rinomino il file di import
			*/
			if(FileExists(szImportFile)){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "DATI : Rinomino %s in %s.%s-%s%s",
					szImportFile,
					szImportFile,
					GetDateYYYYMMDD(time((long *)0),szDateBuffer),
					GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");
#endif
				sprintf(szCommand,"mv -f %s %s.%s-%s%s",
					szImportFile,
					szImportFile,
					GetDateYYYYMMDD(time((long *)0),szDateBuffer),
					GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");

				system(szCommand);
			}
		}
		if(FileExists(szImportMonitor)){
#ifdef TRACE
			trace_debug(TRUE, TRUE, "MONITOR : Rinomino %s in %s.%s-%s%s",
				szImportMonitor,
				szImportMonitor,
				GetDateYYYYMMDD(time((long *)0),szDateBuffer),
				GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");
#endif
			sprintf(szCommand,"mv -f %s %s.%s-%s%s",
				szImportMonitor,
				szImportMonitor,
				GetDateYYYYMMDD(time((long *)0),szDateBuffer),
				GetTimeHHMMSS(time((long *)0),szTimeBuffer),bOK?"":"-KO");

			system(szCommand);
		}

		if(bOK){
			DBRes=DBExecQuery(Cfg.nDebugLevel,"commit work;"); DBclear(DBRes);
		} else {
			DBRes=DBExecQuery(Cfg.nDebugLevel,"abort work;"); DBclear(DBRes);
		}

	}

	if(bOK==FALSE){
		return FALSE;
	}
	/*
	* Se ho ricevuto dati eseguo l'elaborazione di post-ricezione
	*/
	if(nUpdated[0]){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "INIZIO POST RICEZIONE ORDINI");
#endif

		sprintf(szMsg,"CHECK FILE IMPORTAZIONE -----> INIZIO");
		if(bAlone==FALSE){
			// SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}

		/* ORDINI */
		// Metto il flag e un identificativo a import_ric_ord
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_ord set roflric='%c';",IMPORT_ORDINE_OK);
		sprintf(szMsg,"FILE ORDINI: %d righe",atoi(DBcmdTuples(DBResOrdini)));
		if(bAlone==FALSE){
			// SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBResOrdini);

		// Trovo ordini gia' presenti in ric_ord
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_ord set roflric='%c' where ordprog in (SELECT ordprog from ric_ord);",IMPORT_ORDINE_DOPPIO);
		sprintf(szMsg,"FILE ORDINI: %d ordini gia' in database",atoi(DBcmdTuples(DBResOrdini)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBResOrdini);

		// Trovo ordini privi di articoli
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_ord set roflric='%c' where ordprog not in (SELECT distinct ordprog from import_ric_art);",IMPORT_ORDINE_SENZA_ARTICOLI);
		sprintf(szMsg,"FILE ORDINI: %d ordini senza articoli associati",atoi(DBcmdTuples(DBResOrdini)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBResOrdini);

		/* RIGHE */
		// Metto il flag e un identificativo a import_ric_art
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_art set raflric='%c';",IMPORT_ARTICOLO_OK);
		sprintf(szMsg,"FILE ARTICOLI: %d righe",atoi(DBcmdTuples(DBResOrdini)));
		if(bAlone==FALSE){
			// SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBResOrdini);

		// Trovo articoli gia' presenti in ric_art
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_art set raflric='%c' where ordprog||racdpro in (SELECT ordprog||racdpro from ric_art);",IMPORT_ARTICOLO_DOPPIO);
		sprintf(szMsg,"FILE ARTICOLI: %d articoli gia' in database",atoi(DBcmdTuples(DBResOrdini)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBResOrdini);

		// Trovo articoli privi di ordini
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_art set raflric='%c' where ordprog not in (SELECT distinct ordprog from import_ric_ord);",IMPORT_ARTICOLO_SENZA_ORDINE);
		sprintf(szMsg,"FILE ARTICOLI: %d articoli senza ordini associati",atoi(DBcmdTuples(DBResOrdini)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBResOrdini);

		// Trovo articoli con ordini gia' presenti in ric_ord 
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update import_ric_art set raflric='%c' where ordprog in (SELECT distinct ordprog from import_ric_ord where roflric='%c');",IMPORT_ARTICOLO_CON_ORDINE_DOPPIO,IMPORT_ORDINE_DOPPIO);
		sprintf(szMsg,"FILE ARTICOLI: %d articoli con ordini gia' in database",atoi(DBcmdTuples(DBRes)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBRes);


		sprintf(szMsg,"CHECK FILE IMPORTAZIONE -----> FINE");
		if(bAlone==FALSE){
			// SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}


		// Importo in ric_ord
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_ord ( RONMCED, ORDPROG, ROCDLIN, RODSLIN, ROCDRID, RODSCLI, ROINCLI, ROLOCLI, ROCPCLI, ROPRCLI, ROFLPAR, ROFLARR, RODSARR, RODTBCD, ROTPBCD, RODSNOT, ROSWCOL, ROFLAMZ , ROPRBOL,ROCDAMZ,ROCDPDL,RODSPDL,ROSGESS,ROCCESS,RONOESS) select RONMCED, ORDPROG, ROCDLIN, RODSLIN, ROCDCLI, RODSCLI1, ROINCLI, ROLOCLI, ROCPCLI, ROPRCLI, ROFLPAR, ROFLARR, RODSARR, RODTBCD, ROTPBCD, RODSNOT, ROSWPLT, ROFLAMZ, ROPRBOL,ROCDAMZ,ROCDPDL,RODSPDL,ROSGESS,ROCCESS,RONOESS from import_ric_ord where roflric='%c'",IMPORT_ORDINE_OK);
		sprintf(szMsg,"TABELLA ORDINI: importati %d ordini",atoi(DBcmdTuples(DBRes)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBRes);

		// Importo in ric_art
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_art (RANMCED,ORDPROG,RACDPRO,RAQTORD) select  RANMCED, ORDPROG, RACDPRO, RAQTORD from import_ric_art where raflric='%c'",IMPORT_ARTICOLO_OK);
		sprintf(szMsg,"TABELLA ARTICOLI: importati %d articoli",atoi(DBcmdTuples(DBRes)));
		if(bAlone==FALSE){
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		} else {
			fprintf(stderr,szMsg);
		}
		DBclear(DBRes);


		/*
		* Ricalcolo numero righe, numero copie e peso dell'ordine ricevuto
		*/
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where rostato IS NULL;");
		nTuples=DBntuples(DBResOrdini);
		if(nTuples){

			for(nIndex=0;nIndex<nTuples;nIndex++){
				bOK=TRUE;
				strcpy(szOrdProg,DBgetvalue(DBResOrdini,nIndex,0));  /* Ordine */

				/* ricalcolo righe e copie */
				DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"select racdpro,raqtord from ric_art where ordprog='%s';",szOrdProg);
				nRighe=DBntuples(DBResRighe);
				nCopie=0;
				nPeso=0;
				for(nRigaIndex=0;nRigaIndex<nRighe;nRigaIndex++){
					strcpy(szRACDPRO,DBgetvalue(DBResRighe,nRigaIndex,0));
					nCopie+=atoi(DBgetvalue(DBResRighe,nRigaIndex,1));
					DBResCat=DBExecQuery(Cfg.nDebugLevel>1,"select prpesgr from catalogo where prcdpro='%s';",szRACDPRO);
					if(DBntuples(DBResCat)){
						nPeso+=atoi(DBgetvalue(DBResCat,0,0))*nCopie;
					} else {
						bOK=FALSE;
#ifdef TRACE
						trace_debug(TRUE, TRUE, "Prodotto [%s] non presente in catalogo",szRACDPRO);
#endif
						sprintf(szMsg,"Prodotto [%s] non presente in catalogo",szRACDPRO);
						if(bAlone==FALSE){
							SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
						} else {
							fprintf(stderr,szMsg);
						}
					}
					DBclear(DBResCat);
				}
				DBclear(DBResRighe);

				/*
				* Ricezione Campo tipo ordine non prevista in questo impianto
				*/
				strcpy(szTipoOrdini,Cfg.szTipoOrdini);

				/* 
				* ricava e assegna il tipo ubicazione di riferimento
				*/
				strcpy(szTipoUbicazioni,Cfg.szTipoUbicazioni);

				/*
				* Eseguo update ordine OK o NON OK
				*/

				DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1, "update ric_ord set ronmcpe=%d,ronmrgh=%d,ropspre=%d,rostato='%c',rotmrcz='now',ordtipo='%s',ubitipo='%s',rotpspe='AM', rocdve1 = '  ', rocdve2 = '  ', rocdve3 = '  ',roflimc='%c'  where ordprog='%s';", 
					nCopie, nRighe, nPeso,
					bOK?ORDINE_ATTESA_RICEZIONE:ORDINE_ERRATO,
					szTipoOrdini,
					szTipoUbicazioni,
					CARICHI_ORDINE_DA_IMPORTARE,
					szOrdProg);
				if (DBresultStatus(DBResUpdate) != DBRES_COMMAND_OK && atoi(DBcmdTuples(DBResUpdate))==0) {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Errore in update ordine [%s]",szOrdProg);
#endif
					sprintf(szMsg,"Errore in update ordine [%s]",szOrdProg);
					if(bAlone==FALSE){
						SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
					} else {
						fprintf(stderr,szMsg);
					}
				}
				DBclear(DBResUpdate);
			}
		}

		DBclear(DBResOrdini);

		bOK=TRUE;

		/* assegnazione dati cedola */
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ronmced ,count(ronmced) ,sum(ronmrgh) ,sum(ronmcpe) from ric_ord where ubitipo='%s' and rostato='%c' group by ronmced order by ronmced;",Cfg.szTipoUbicazioni,ORDINE_ATTESA_RICEZIONE);
		if(DBntuples(DBRes)){
			nNMCED=atoi(DBgetvalue(DBRes,0,0));
			nNMORD=atoi(DBgetvalue(DBRes,0,1));
			nNMRGH=atoi(DBgetvalue(DBRes,0,2));
			nNMCPE=atoi(DBgetvalue(DBRes,0,3));
		} else {
			bOK=FALSE;
		}
		DBclear(DBRes);
		/* inserimento dati cedola - solo se cedola non presente */
		if(bOK){
			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cdnmced from cedole where cdnmced=%d",nNMCED);
			if (DBntuples(DBRes) != 0 ) {
				/* cedola presente */
				bOK=FALSE;
			}
			DBclear(DBRes);

			if(bOK){
				DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into cedole (cdnmced,cdnmord,cdnmrgh,cdnmcpe,cdstato,cdtmulm) values (%d,%d,%d,%d,'%c','now');", nNMCED, nNMORD, nNMRGH, nNMCPE,CEDOLA_RICEVUTA);
				DBclear(DBRes);
			}
		}
		DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1, "update ric_ord set rostato='%c' where rostato='%c';", 
			ORDINE_RICEVUTO,ORDINE_ATTESA_RICEZIONE);
		DBclear(DBResUpdate);


#ifdef TRACE
		trace_debug(TRUE, TRUE, "FINE POST RICEZIONE ORDINI");
#endif

	}


	szMsgData[0]='\0';

	if(nUpdated[0]){
		/* Ordini */
		sprintf(szBuffer,"[%d] Ordini ",nUpdated[0]);
		strcat(szMsgData,szBuffer);
	} 
	if(nUpdated[1]){
		/* Righe */
		sprintf(szBuffer,"[%d] Righe ",nUpdated[1]);
		strcat(szMsgData,szBuffer);
	}
	if(nUpdated[2]){
		/* Catalogo */
		sprintf(szBuffer,"[%d] Titoli di catalogo",nUpdated[2]);
		strcat(szMsgData,szBuffer);
	}

	if(bRicezione){
		if(strlen(szMsgData)){
			sprintf(szMsg,"Ricevuti %s\n",szMsgData);
			/*
			* segnalo a video
			*/
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				SendMessage(Cfg.nMainID, nPID,  REFRESH_DATA, szMsg);
			}
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Ricevuti %s",szMsgData);
#endif
		} else {
			strcpy(szMsg,"Nessun dato ricevuto");
			/*
			* segnalo a video
			*/
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			}
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Nessun dato ricevuto");
#endif
		}
	}
	

  /* azzero i dati di ricezione */
	for(nIndex=0; nIndex<nTables; nIndex++){
		/*
		* Flag di tabella modificata
		*/
		nUpdated[nIndex]=0;
		if(bOK){
			/* vacuum analyze per velocizzare l'accesso alla tabella */
			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"vacuum analyze %s;",db[nIndex].szTableName);
			DBclear(DBRes);
		}
	}

	return bRetValue;
}

/*
* SafeExit()
*
* safe exit from software interrupt 
*/
void SafeExit(void)
{
	DBDisconnect();

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stopped");
#endif
	exit(0);
}


/*
* SafeIntFunc()
*
* safe exit from software interrupt 
*/
void SafeIntFunc()
{
	FreeTablesInfo();
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Got Signal ! ");
#endif
	bExitRequest=TRUE;
}


/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
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
	ReadCfgItems(CFGItems,Cfg.szCfgFileName,nPID);

	/*
	* Lettura dati processi
	*/
	if(bReadProcInfo){
		ReadProcInfo(Cfg.szCfgFileName);
	}
	return TRUE;
}

/* 
* cancello tutti i files relativi al mese scorso
*/
void DeleteBackupFiles(void)
{
	int nMese;
	int nMesePrecedente;
	char szCommand[256];

	nMese=atoi(GetDatef("%m"));
	
	nMesePrecedente=nMese>1?nMese-1:12;

	sprintf(szCommand,"rm -f %s/\?\?\?\?%02d\?\?-*",
					Cfg.szPathExport,
					nMesePrecedente);
#ifdef TRACE
	trace_debug(FALSE, TRUE, "DeleteBackupFiles() : Eseguo Comando : [%s]",szCommand);
#endif
	system(szCommand);
}

void do_elaborazione_kit(char *szCDKIT,int nNMCED)
{
	ep_bool_t bOK=TRUE;
	ep_bool_t bOrdineOK=TRUE;
	DBresult *DBRes;
	DBresult *DBResOrd;
	DBresult *DBResArt;
	DBresult *DBResUpdate;
	DBresult *DBResInsert;
	DBresult *DBResDelete;
	char szOrdprog[128];
	char szCDPRO[128];
	int nCopieKit;
	int nCopieProdotto;
	int nProdottiKit;
	int nNumeroKit;
	char szSetProdottiKit[256];
	int nCopieOrdine;
	int nRigheOrdine;
	int nIndex;
	int nArticoliIndex;
	int nArticoli;
	int nOrdini;
	int nOrdiniModificati=0;
	int nTotaleKit=0;
	int nTotaleRigheKit=0;




	/* controllo presenza cedola/kit */
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ktcdpro from kit where ktcdpro='%s' and ktnmced=%d;",szCDKIT,nNMCED);
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "KIT : Errore in ricerca codice kit [%s]",szCDKIT);
#endif
		bOK=FALSE;
	}
	DBclear(DBRes);


	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select pkcdpro,pknmcpe from prodotti_kit where pkcdkit='%s';",szCDKIT);
	nProdottiKit=DBntuples(DBRes);
	strcpy(szSetProdottiKit,"");
	for(nIndex=0;nIndex<nProdottiKit;nIndex++){
		if(nIndex){
			strcat(szSetProdottiKit,",");
		}
		strcat(szSetProdottiKit,"'");
		strcat(szSetProdottiKit,DBgetvalue(DBRes,nIndex,0));
		strcat(szSetProdottiKit,"'");
	}
	DBclear(DBRes);

	if(!bOK){
		return;
	}


	/*
	* Inizio la transazione
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"begin work;");
	DBclear(DBRes);
	

	DBResOrd=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,ronmrgh,ronmcpe from ric_ord where ronmced='%d' order by ordprog;",nNMCED);
	nOrdini=DBntuples(DBResOrd);

	for(nIndex=0;nIndex<nOrdini;nIndex++){

		bOrdineOK=TRUE;

		strcpy(szOrdprog,DBgetvalue(DBResOrd,nIndex,0));
		nRigheOrdine=atoi(DBgetvalue(DBResOrd,nIndex,1));
		nCopieOrdine=atoi(DBgetvalue(DBResOrd,nIndex,2));

		/*
		* PRIMO GIRO - VERIFICA 
		*/
		/* RM : mi attendo righe gia' raggruppate per codice prodotto !!!! */
		DBResArt=DBExecQuery(Cfg.nDebugLevel>1,"select racdpro,raqtord from ric_art where ordprog='%s' and racdpro in (%s);",szOrdprog,szSetProdottiKit);
		nArticoli=DBntuples(DBResArt);
		if(nArticoli==nProdottiKit){
			nNumeroKit=0;
			for(nArticoliIndex=0;nArticoliIndex<nArticoli;nArticoliIndex++){
				strcpy(szCDPRO,DBgetvalue(DBResArt,nArticoliIndex,0));
				DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select pkcdpro,pknmcpe from prodotti_kit where pkcdkit='%s' and pkcdpro='%s';",
					szCDKIT,szCDPRO);
				if(DBntuples(DBRes)==1){
					nCopieKit=atoi(DBgetvalue(DBRes,0,1));
					nCopieProdotto=atoi(DBgetvalue(DBResArt,nArticoliIndex,1));
					if(nCopieKit>nCopieProdotto){
						/* almeno un prodotto non ha le copie necessarie al kit */
						bOrdineOK=FALSE;
					} else {
						nNumeroKit=min(nNumeroKit?nNumeroKit:nCopieProdotto,nCopieProdotto/nCopieKit);
					}
				} else {
					/* prodotto non in kit */
					bOrdineOK=FALSE;
				}
				DBclear(DBRes);
			}
			
		} else {
			/* almeno un prodotto per kit non presente */
			bOrdineOK=FALSE;
		}
		DBclear(DBResArt);

		/*
		* SECONDO GIRO - ATTUAZIONE 
		*/
		if(bOrdineOK){
			nOrdiniModificati++;
			/* inserisco il record di kit per prodotto */
			DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_art (ranmced,ordprog,racdpro,raqtord) values ('%d','%s','%s',%d);",
				nNMCED,szOrdprog,szCDKIT,nNumeroKit);
			DBclear(DBResInsert);
			/* ricalcolo i valori di righe e copie per ordine */
			nRigheOrdine++;
			nCopieOrdine+=nNumeroKit;
			nTotaleKit+=nNumeroKit;
			nTotaleRigheKit++;



			/* sottraggo le copie ai prodotti che formano il kit */

			DBResArt=DBExecQuery(Cfg.nDebugLevel>1,"select racdpro,raqtord from ric_art where ordprog='%s' and racdpro in (%s);",szOrdprog,szSetProdottiKit);
			nArticoli=DBntuples(DBResArt);
			nNumeroKit=0;
			for(nArticoliIndex=0;nArticoliIndex<nArticoli;nArticoliIndex++){
				strcpy(szCDPRO,DBgetvalue(DBResArt,nArticoliIndex,0));
				DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select pkcdpro,pknmcpe from prodotti_kit where pkcdkit='%s' and pkcdpro='%s';",
					szCDKIT,szCDPRO);
				if(DBntuples(DBRes)==1){
					nCopieKit=atoi(DBgetvalue(DBRes,0,1));
					nCopieProdotto=atoi(DBgetvalue(DBResArt,nArticoliIndex,1));
					if(nCopieKit>nCopieProdotto){
						/* almeno un prodotto non ha le copie necessarie al kit */
						bOrdineOK=FALSE;
					} else {
						nNumeroKit=min(nNumeroKit?nNumeroKit:nCopieProdotto,nCopieProdotto/nCopieKit);
					}
				}
				DBclear(DBRes);

				
				/* cancello le eventuali righe a 0 */
				if(nCopieProdotto==nNumeroKit*nCopieKit){
					/* 
					* se il numero copie prodotto e' uguale al numero copie messe in kit cancello la riga
					*/
					DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_art where ordprog='%s' and racdpro='%s';", szOrdprog,szCDPRO);
					DBclear(DBResDelete);
					/* ricalcolo i valori di righe e copie per ordine */
					nCopieOrdine-=nCopieKit*nNumeroKit;
					nRigheOrdine-=1;
				} else {
					DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update ric_art set raqtord=raqtord-%d where ordprog='%s' and racdpro='%s';",
						nCopieKit*nNumeroKit,szOrdprog,szCDPRO);
					DBclear(DBResUpdate);
					/* ricalcolo i valori di righe e copie per ordine */
					nCopieOrdine-=nCopieKit*nNumeroKit;
				}

			}
				
			DBclear(DBResArt);



			/* aggiorno il numero copie/righe dell'ordine */
			DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set ronmcpe=%d,ronmrgh=%d where ordprog='%s';",nCopieOrdine,nRigheOrdine,szOrdprog);
			DBclear(DBResUpdate);

			/*
			* TODO
			* peso ordine ? (lo ricalcola volum ...)
			*/
		}
	}
	DBclear(DBResOrd);

	if(bOK){
		DBRes=DBExecQuery(Cfg.nDebugLevel,"commit work;"); DBclear(DBRes);
	} else {
		DBRes=DBExecQuery(Cfg.nDebugLevel,"abort work;"); DBclear(DBRes);
	}

}
