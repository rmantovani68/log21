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

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef TRACE
	#include "trace.h"
#endif

#include <msq_lib.h>
#include <shared.h>
#include <libpq-fe.h>

#include <dbfun.h>
#include <proc_list.h>
#include <picking.h>

#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __authors__[];
extern char __copyright__[];

BOOL bAlone=FALSE;

/*
* Main function
* inizializzazione dati
* lettura dei dati di configurazione
*/
int main(int argc,char **argv)
{
	BOOL bErrore=FALSE;
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
				printf("%s : Copyright A&L srl 2003-2021\n",argv[0]);
				printf("Uso: %s [-h] [-a] [-p <pid>]\n",argv[0]);
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
		fprintf(stderr,"PID non definito : utilizzo %d\n",PROC_RECEIVE);
		nPID=PROC_RECEIVE;
	}
	if(bErrore){
		printf("%s : Copyright A&L srl 2003-2021\n",argv[0]);
		printf("Uso: %s [-h] [-a] [-p <pid>]\n",argv[0]);
		printf("\t-h       : help\n");
		printf("\t-a       : run stand alone\n");
		printf("\t-p <pid> : PID\n");
	}


#ifdef TRACE
	/*
	* Apro il file trace 
	*/
	sprintf(szBufTrace,"%s%s",argv[0],TRACE_FILE_SUFFIX);
	open_trace(argv[0], szBufTrace, TRACE_FILE_MAXSIZE);

	trace_out_vstr(1,"Module Name     : %s",argv[0]);
	trace_out_vstr(1,"Version         : %s",__version__);
	trace_out_vstr(1,"Customer Name   : %s",__customer__);
	trace_out_vstr(1,"Authors         : %s",__authors__);
	trace_out_vstr(1,"Copyright       : %s",__copyright__);
	trace_out_vstr(1,"RCSID String    : %s",rcsid);
	trace_out_vstr_date(1,"Started");
#endif

	/*
	* lettura del file di configurazione
	*/
	ReadConfiguration();

	if(bAlone==FALSE){
		/* Apro la coda messaggi principale */
		if((ProcList[Cfg.nMainID].nQNumber = OpenMsgQ(ProcList[Cfg.nMainID].nQKey))<0){
#ifdef TRACE
			trace_out_vstr(1, "Apertura coda messaggi principale fallita");
#endif
		}
	}

	/*
	* Lettura configurazione tabelle utilizzate
	*/
	InsertTableInfo(&tRicOrd,     Cfg.szPathData,"ric_ord");
	InsertTableInfo(&tRicArt,     Cfg.szPathData,"ric_art");
	InsertTableInfo(&tRicNote,    Cfg.szPathData,"ric_note");
	InsertTableInfo(&tRicQuad,    Cfg.szPathData,"ric_quad");
	InsertTableInfo(&tCatalogo,   Cfg.szPathData,"catalogo");
	InsertTableInfo(&tTTCS,       Cfg.szPathData,"ttcs");
	InsertTableInfo(&tTTFC,       Cfg.szPathData,"ttfc");
	InsertTableInfo(&tTTFP,       Cfg.szPathData,"ttfp");
	InsertTableInfo(&tTTGC,       Cfg.szPathData,"ttgc");
	InsertTableInfo(&tTTLS,       Cfg.szPathData,"ttls");
	InsertTableInfo(&tTTLV,       Cfg.szPathData,"ttlv");
	InsertTableInfo(&tTTRC,       Cfg.szPathData,"ttrc");
	InsertTableInfo(&tTTTS,       Cfg.szPathData,"ttts");
	InsertTableInfo(&tTTVE,       Cfg.szPathData,"ttve");
	InsertTableInfo(&tTTEC,       Cfg.szPathData,"ttec");

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
	ConnectDataBase(Cfg.szPGHost, Cfg.szPGPort, Cfg.szPGDataBase);

	if(bAlone==FALSE){
		/*
		* avverto MAIN dello start
		*/
		SendMessage(Cfg.nMainID, nPID,  PROGRAM_STARTED, NULL);
	}

	/*
	* Ciclo infinito di controllo e ricezione dati
	*/
	for(;;){
		if(!CheckDatiRicezione()){
			if(bAlone==FALSE){
				sprintf(szMsg,"Errore in ricezione dati\n");
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			} else {
#ifdef TRACE
				trace_out_vstr_date(1, "Errore in ricezione dati");
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
* CheckDatiRicezione()
* Ricezione dati da file
*/
BOOL CheckDatiRicezione( void )
{
	FILE *fp;
	char szCommand[256];
	char szSQLCmd[256];
	DBSTRUCT db[20];
	char *pszFiles[20];
	int nUpdated[20];
	BOOL bUpdate[20];
	BOOL bDelete[20];
	BOOL bRicezione=FALSE;
	BOOL bOK;
	int nIndex=0;
	int nTables=0;
	int nTuples=0;
	int nRighe;
	int nCopie=0;
	int nPeso=0;
	int nColli=0;
	int nRigaIndex=0;
	PGresult *PGRes;
	PGresult *PGResOrdini;
	PGresult *PGResRighe;
	BOOL bRetValue=TRUE;
	char szOrdProg[128];
	char szCodProv[128];
	char szMsgData[256];
	char szMsg[256];
	char szBuffer[256];
	DATI_DISTRIBUZIONE DatiDistribuzione;
	int nCedola=0;
	int nOrdiniRicevuti=0,nRigheRicevute=0,nCopieRicevute=0;
	int nOrdiniQuadratura=0,nRigheQuadratura=0,nCopieQuadratura=0;
	int nNMCED;
	int nNMORD;
	int nNMRGH;
	int nNMCPE;
	char szDTEVA[128];
	
	/* nomi dei files di import dati */
	char szImportFile[128];
	
	/* semafori associati ai files di import */
	char szImportMonitor[128];


	nTables=0;
	/* Struttura DB */      /* File di import */                        /* Aggiorna */           /* Cancella */
	db[nTables]=tRicOrd;    pszFiles[nTables]=Cfg.szImportSpedizioni;   bUpdate[nTables]=FALSE;  bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tRicArt;    pszFiles[nTables]=Cfg.szImportSpedizioni;   bUpdate[nTables]=FALSE;  bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tRicNote;   pszFiles[nTables]=Cfg.szImportSpedizioni;   bUpdate[nTables]=FALSE;  bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tRicQuad;   pszFiles[nTables]=Cfg.szImportSpedizioni;   bUpdate[nTables]=FALSE;  bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tCatalogo;  pszFiles[nTables]=Cfg.szImportCatalogo;     bUpdate[nTables]=TRUE;   bDelete[nTables]=FALSE; nTables++; 
	db[nTables]=tTTCS;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTFC;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++;  
	db[nTables]=tTTFP;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTGC;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTLS;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTLV;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTRC;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTTS;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTVE;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 
	db[nTables]=tTTEC;      pszFiles[nTables]=Cfg.szImportTabelle;      bUpdate[nTables]=TRUE;   bDelete[nTables]=TRUE;  nTables++; 

	bRicezione=FALSE;
	sprintf(szImportMonitor,"%s/%s",Cfg.szPathExport, StrTrimAll(Cfg.szImportMonitor));
	if(FileExists(szImportMonitor)){
		bRicezione=TRUE;



		for(nIndex=0; nIndex<nTables; nIndex++){

			sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(pszFiles[nIndex]));

			/*
			* Flag di tabella modificata
			*/
			nUpdated[nIndex]=0;

			if((fp=fopen(szImportFile,"r"))!=NULL){
				char szBuffer[1024];

#ifdef TRACE
				trace_out_vstr_date(1,"INIZIO RICEZIONE : [%s]",db[nIndex].szTableName);
#endif
				if(bAlone==FALSE){
					/*
					* segnalo a video
					*/
					sprintf(szMsg,"Inizio ricezione [%s]\n",db[nIndex].szTableName);
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				} else {
#ifdef TRACE
					trace_out_vstr_date(1, "Inizio ricezione [%s]",db[nIndex].szTableName);
#endif
				}

				/* 
				* se e' prevista la cancellazione
				* verifico la presenza di records
				* nel file
				*/
				if(bDelete[nIndex]){
					BOOL bRecordPresente=FALSE;

					while(fgets(szBuffer,sizeof(szBuffer)-1,fp)){
						if(IsAsciiRecord(szBuffer,&db[nIndex])){
							bRecordPresente=TRUE;
							break;
						}
						memset(szBuffer,(int)NULL,sizeof(szBuffer));
					}
					if(bRecordPresente){

						/*
						* torno all'inizio del file
						*/
						rewind(fp);

#ifdef TRACE
						trace_out_vstr_date(1,"DELETE : %s",db[nIndex].szTableName);
#endif
						/*
						* Cancello la copia della tabella eventualmente presente
						*/
						PGRes=PGExecSQL(Cfg.nDebugVersion>1,"drop table %s_copy;",db[nIndex].szTableName); 
						DBclear(PGRes);
						/*
						* Creo la copia della tabelle e inserisco il contenuto
						*/
						PGRes=PGExecSQL(Cfg.nDebugVersion>1,"create table %s_copy as select * from %s;",
							db[nIndex].szTableName,db[nIndex].szTableName); DBclear(PGRes);

						/*
						* cancello il contenuto della tabella 
						*/
						PGRes=PGExecSQL(Cfg.nDebugVersion,"delete from %s;",db[nIndex].szTableName); DBclear(PGRes);
						PGRes=PGExecSQL(Cfg.nDebugVersion,"vacuum %s;",db[nIndex].szTableName); DBclear(PGRes);
					}
				}

				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"BEGIN WORK;");
				DBclear(PGRes);

				while(fgets(szBuffer,sizeof(szBuffer)-1,fp)){
					if(UpdateAsciiRecord(szBuffer,&db[nIndex],bUpdate[nIndex])){
						nUpdated[nIndex]++;
					}
					memset(szBuffer,(int)NULL,sizeof(szBuffer));
				}
				fclose(fp);

				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"COMMIT WORK;");
				DBclear(PGRes);

				/* vacuum analyze per velocizzare l'accesso alla tabella */
				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"vacuum analyze %s;",db[nIndex].szTableName);
				DBclear(PGRes);

#ifdef TRACE
				trace_out_vstr_date(1,"FINE RICEZIONE : [%s]",db[nIndex].szTableName);
#endif
				if(bAlone==FALSE){
					/*
					* segnalo a video
					*/
					sprintf(szMsg,"Termine ricezione [%s]\n",db[nIndex].szTableName);
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				} else {
#ifdef TRACE
					trace_out_vstr_date(1, "Termine ricezione [%s]",db[nIndex].szTableName);
#endif
				}
			}
		}
		for(nIndex=0; nIndex<nTables; nIndex++){
			sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(pszFiles[nIndex]));
			/* 
			* rinomino il file di import
			*/
			if(FileExists(szImportFile)){
#ifdef TRACE
				trace_out_vstr_date(1,"DATI : Rinomino %s in %s.%s-%s",
					szImportFile,
					szImportFile,
					GetDateYYYYMMDD(time((long *)0),szDateBuffer),
					GetTimeHHMMSS(time((long *)0),szTimeBuffer));
#endif
				sprintf(szCommand,"mv -f %s %s.%s-%s",
					szImportFile,
					szImportFile,
					GetDateYYYYMMDD(time((long *)0),szDateBuffer),
					GetTimeHHMMSS(time((long *)0),szTimeBuffer));

				system(szCommand);
			}
		}
		if(FileExists(szImportMonitor)){
#ifdef TRACE
			trace_out_vstr_date(1,"MONITOR : Rinomino %s in %s.%s-%s",
				szImportMonitor,
				szImportMonitor,
				GetDateYYYYMMDD(time((long *)0),szDateBuffer),
				GetTimeHHMMSS(time((long *)0),szTimeBuffer));
#endif
			sprintf(szCommand,"mv -f %s %s.%s-%s",
				szImportMonitor,
				szImportMonitor,
				GetDateYYYYMMDD(time((long *)0),szDateBuffer),
				GetTimeHHMMSS(time((long *)0),szTimeBuffer));

			system(szCommand);
		}

	}


/*
* Messo su script
*/

//	// Da ripetere per tutti i tre file di ricezione
//	sprintf(szImportFile,"%s/%s",Cfg.szPathExport, StrTrimAll(Cfg.szImportCatalogo));
//	if(FileExists(szImportFile)){
//		// Se gia' esiste non faccio nulla, aspetto che qualcuno lo riceva
//	} else {
//		// Provo a spostare il file piu' vecchio di quelli importati e gli cambio nome
//		// in quello univoco che posso riconoscere, oltre a cambiargli directory
//		sprintf(szCommand,"mv -f $(ls -rt %s/`basename %s .TXT`* |head -1) %s/%s",
//			Cfg.szPathFtpImport,
//			StrTrimAll(Cfg.szImportCatalogo),
//			Cfg.szPathExport,
//			StrTrimAll(Cfg.szImportCatalogo));
//
//		system(szCommand);
//	}
	

	/*
	* Se ho ricevuto dati eseguo l'elaborazione di post-ricezione
	*/
	if(nUpdated[0]){

#ifdef TRACE
		trace_out_vstr_date(1,"INIZIO CALCOLO INSTRADAMENTO ORDINI");
#endif
		if(bAlone==FALSE){
			/*
			* segnalo a video
			*/
			sprintf(szMsg,"Inizio Determinazione Instradamenti\n");
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		}

		/* Ordini */
		PGRes=PGExecSQL(FALSE,"BEGIN WORK;"); DBclear(PGRes);

		/*
		* rm 18-09-2003 : si evita di incrementare qui il numero cedola
		*                 operazione eseguita in main (vedi dlg_numero_cedola)
		*/
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select last_value from sequence_cedola;"); 
		if(PQntuples(PGRes)){
			nCedola=atoi(PQgetvalue(PGRes,0,0));
		} 
		DBclear(PGRes);
		/*
		* Ricalcolo numero righe, numero copie e peso dell'ordine ricevuto
		* assegno codice vettore e dati accessori
		*/
		PGResOrdini=PGExecSQL(Cfg.nDebugVersion>1,"select ordprog,roprcli from ric_ord where ronmced=' ' or ronmced is null;");
		nTuples=PQntuples(PGResOrdini);
		if(nTuples){

			for(nIndex=0;nIndex<nTuples;nIndex++){
				strcpy(szOrdProg,PQgetvalue(PGResOrdini,nIndex,0));  /* Ordine */
				strcpy(szCodProv,PQgetvalue(PGResOrdini,nIndex,1));  /* Provincia */

				/* ricalcolo righe e copie */
				PGResRighe=PGExecSQL(Cfg.nDebugVersion>1,"select r.raqtord,r.raqtpes from ric_art as r where ordprog='%s';",szOrdProg);
				nRighe=PQntuples(PGResRighe);
				nCopie=0;
				nPeso=0;
				for(nRigaIndex=0;nRigaIndex<nRighe;nRigaIndex++){
					nCopie+=atoi(PQgetvalue(PGResRighe,nRigaIndex,0));
					nPeso+=atoi(PQgetvalue(PGResRighe,nRigaIndex,1));
				}
				DBclear(PGResRighe);

				/* calcolo distribuzione */

				/* default a 10 */
				nColli=10; 

				RicavaDistribuzione(&DatiDistribuzione,szOrdProg,nPeso,nColli);

				/*
				* Eseguo update ordine
				*
				* LG: 26-12-2011 aggiunto il campo rocdveo per contenere il rocdve2 originale
				*/
				PGResRighe=PGExecSQL(Cfg.nDebugVersion>1,"update ric_ord set roswcol='%s', rotpspe='%s', rocdlin='%s', rocdsca='%s', rocdssc='%s', roidvet='%s', rocdve1='%s', rocdve2='%s', ronmcpe=%d, ronmrgh=%d,ropspre=%d,rostato='%c',ordtipo='%s',rotmrcz='now',ronmced='%d',rocdveo='%s' where ordprog='%s';", 
					DatiDistribuzione.szSWCOL,
					DatiDistribuzione.szTPSPE,
					DatiDistribuzione.szCDLIN,
					DatiDistribuzione.szCDSCA,
					DatiDistribuzione.szCDSSC,
					DatiDistribuzione.szIDVET,
					DatiDistribuzione.szCDVE1,
					DatiDistribuzione.szCDVE2,
					nCopie,
					nRighe,
					nPeso,
					ORDINE_CONTROLLO_QUADRATURA,
					Cfg.szTipoOrdini,
					nCedola,
					DatiDistribuzione.szCDVE2,
					szOrdProg);
				if (PQresultStatus(PGResRighe) != PGRES_COMMAND_OK && atoi(PQcmdTuples(PGResRighe))==0) {
#ifdef TRACE
					trace_out_vstr_date(1,"Errore in update ordine [%s]",szOrdProg);
#endif
					bRetValue=FALSE;
				}
				DBclear(PGResRighe);
			}
		}

		DBclear(PGResOrdini);


		PGRes=PGExecSQL(FALSE,"COMMIT WORK;"); DBclear(PGRes);

#ifdef TRACE
		trace_out_vstr_date(1,"FINE CALCOLO INSTRADAMENTO ORDINI");
#endif

		if(bAlone==FALSE){
			sprintf(szMsg,"Fine Determinazione Instradamenti\n");
			SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
		}

		/* 
		* creo la cedola associata agli ordini ricevuti 
		*/
		bOK=TRUE;

		/* 
		* rm 21-01-2008 : gestione multicedola 
		*/
		/* assegnazione dati cedola */
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select ronmced ,count(ronmced) ,sum(ronmrgh) ,sum(ronmcpe), min(to_timestamp(rodteva, 'YYYYMMDD')) from ric_ord where rostato='%c' and ronmced='%d' group by ronmced order by ronmced;",ORDINE_CONTROLLO_QUADRATURA,nCedola);
		if(PQntuples(PGRes)){
			nNMCED=atoi(PQgetvalue(PGRes,0,0));
			nNMORD=atoi(PQgetvalue(PGRes,0,1));
			nNMRGH=atoi(PQgetvalue(PGRes,0,2));
			nNMCPE=atoi(PQgetvalue(PGRes,0,3));
			strcpy(szDTEVA,PQgetvalue(PGRes,0,4));
		} else {
			bOK=FALSE;
		}
		DBclear(PGRes);
		/* inserimento dati cedola - solo se cedola non presente */
		if(bOK){
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select cdnmced from cedole where cdnmced=%d",nNMCED);
			if (PQntuples(PGRes) != 0 ) {
				/* cedola presente */
				bOK=FALSE;
			}
			DBclear(PGRes);

			if(bOK){
				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"insert into cedole (cdnmced,cdnmord,cdnmrgh,cdnmcpe,cdstato,cdtmulm,cdtmrcz) values (%d,%d,%d,%d,'%c','now','%s');", nNMCED, nNMORD, nNMRGH, nNMCPE,CEDOLA_RICEVUTA,szDTEVA);
				DBclear(PGRes);
			} else {
				if(bAlone==FALSE){
					sprintf(szMsg,"Errore in inserimento cedola %d - cedola gia' presente\n",nNMCED);
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				}
			}
			/*
			 * Dico a Main di riassegnare le note della cedola
			 */
			sprintf(szMsg,"%d",nNMCED);
			SendMessage(Cfg.nMainID, nPID,  RIASSEGNA_NOTE_ORDINE_CEDOLA, szMsg);
		}


		/*
		* rm 09-10-2003 : Controllo quadratura
		*/
		PGResOrdini=PGExecSQL(Cfg.nDebugVersion>1,"select count(ordprog),sum(ronmrgh),sum(ronmcpe) from ric_ord where rostato = '%c' and ronmced='%d' group by ronmced;",ORDINE_CONTROLLO_QUADRATURA,nCedola);
		if(PQntuples(PGResOrdini)){
			nOrdiniRicevuti = atoi(PQgetvalue(PGResOrdini,0,0));
			nRigheRicevute  = atoi(PQgetvalue(PGResOrdini,0,1));
			nCopieRicevute  = atoi(PQgetvalue(PGResOrdini,0,2));
		}
		DBclear(PGResOrdini);
		
		/* rimetto a posto */
		PGResOrdini=PGExecSQL(Cfg.nDebugVersion>1,"update ric_ord set rostato='%c' where rostato='%c' and ronmced='%d';", ORDINE_RICEVUTO,ORDINE_CONTROLLO_QUADRATURA,nCedola);
		DBclear(PGResOrdini);
	}

	/* quadratura */
	if(nUpdated[3]){
		BOOL bOK=TRUE;
		PGresult *PGResQuad;

#ifdef TRACE
		trace_out_vstr_date(1,"CONTROLLO QUADRATURA");
#endif

		PGResQuad=PGExecSQL(Cfg.nDebugVersion>1,"select rqnmtes,rqnmrig,rqqttor from ric_quad where rqstato is NULL or rqstato = ' ';");
		if(PQntuples(PGResQuad)){
			nOrdiniQuadratura = atoi(PQgetvalue(PGResQuad,0,0));
			nRigheQuadratura  = atoi(PQgetvalue(PGResQuad,0,1));
			nCopieQuadratura  = atoi(PQgetvalue(PGResQuad,0,2));
		} else {
			bOK=FALSE;
			sprintf(szMsg,"Errore in quadratura ordini - mancata ricezione quadratura\n");
			/*
			* segnalo a video
			*/
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			}
#ifdef TRACE
			trace_out_vstr_date(1,"Errore in quadratura ordini - mancata ricezione quadratura\n");
#endif
		}
		DBclear(PGResQuad);

		/* controllo solo se ho trovato qualcosa */
		if(nOrdiniRicevuti){
			if(nOrdiniRicevuti!=nOrdiniQuadratura){
				bOK=FALSE;
				sprintf(szMsg,"Errore in quadratura ordini [ricevuti : %d - quadratura : %d]\n",nOrdiniRicevuti,nOrdiniQuadratura);
				/*
				* segnalo a video
				*/
				if(bAlone==FALSE){
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				}
#ifdef TRACE
				trace_out_vstr_date(1,"Errore in quadratura ordini [ricevuti : %d - quadratura : %d]",nOrdiniRicevuti,nOrdiniQuadratura);
#endif
			} else if(nRigheRicevute!=nRigheQuadratura){
				bOK=FALSE;
				sprintf(szMsg,"Errore in quadratura righe [ricevute : %d - quadratura : %d]\n",nRigheRicevute,nRigheQuadratura);
				/*
				* segnalo a video
				*/
				if(bAlone==FALSE){
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				}
#ifdef TRACE
				trace_out_vstr_date(1,"Errore in quadratura righe [ricevute : %d - quadratura : %d]",nRigheRicevute,nRigheQuadratura);
#endif
			} else if(nCopieRicevute!=nCopieQuadratura){
				bOK=FALSE;
				sprintf(szMsg,"Errore in quadratura copie [ricevute : %d - quadratura : %d]\n",nCopieRicevute,nCopieQuadratura);
				/*
				* segnalo a video
				*/
				if(bAlone==FALSE){
					SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
				}
#ifdef TRACE
				trace_out_vstr_date(1,"Errore in quadratura copie [ricevute : %d - quadratura : %d]",nCopieRicevute,nCopieQuadratura);
#endif
			
			}
		} else {
			bOK=FALSE;
			/* Segnalo problemi di ricezione */
			sprintf(szMsg,"Errore in ricezione dati\n");
			/*
			* segnalo a video
			*/
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			}
#ifdef TRACE
			trace_out_vstr_date(1,"Errore in ricezione dati");
#endif
		}
		PGResQuad=PGExecSQL(Cfg.nDebugVersion>1,"update ric_quad set rqstato='%c' where rqstato is NULL or rqstato=' ';",bOK?QUADRATURA_OK:QUADRATURA_KO);
		DBclear(PGResQuad);

	} else {
		/* se ho ricevuto ordini ma non quadratura segnalo */
		if(nUpdated[0]){
			sprintf(szMsg,"Errore in quadratura ordini - mancata ricezione quadratura\n");
			/*
			* segnalo a video
			*/
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			}
#ifdef TRACE
			trace_out_vstr_date(1,"Errore in quadratura ordini - mancata ricezione quadratura");
#endif
		}
	}

	/* ricevuto catalogo */
	if(nUpdated[4]){
		char szCDPRO[128];
		int nSWFFO;
		int nPCRIC;
		int nQTFFO;
		int nTuples;
		int nIndex;
		PGresult *PGResCat;
		PGresult *PGResUpdate;

		/* rm 21-01-2008 : assegno cedola al catalogo  - multicedola */
		PGResCat=PGExecSQL(Cfg.nDebugVersion>1,"update catalogo set prnmced = %d where  prnmced = 0;",nCedola);
		DBclear(PGResCat);

		PGResCat=PGExecSQL(Cfg.nDebugVersion>1,"select prcdpro,prswffo,prpcric,prqtffo from catalogo where prswffo > 1;");
		nTuples=PQntuples(PGResCat);
		for(nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szCDPRO,PQgetvalue(PGResCat,nIndex,0));
			nSWFFO=atoi(PQgetvalue(PGResCat,nIndex,1));
			nPCRIC=atoi(PQgetvalue(PGResCat,nIndex,2));
			nQTFFO=atoi(PQgetvalue(PGResCat,nIndex,3));

			/* gestione fuori formato */
			switch (nSWFFO){
				case 2:
					/* con % di riempimento - normalizzo */
					nQTFFO=max(1,100/nPCRIC);
					sprintf(szSQLCmd,"update catalogo set prqtffo=%d,prswffo=1 where prcdpro='%s';",nQTFFO,szCDPRO);
				break;
				default:
					/* diverso da 2 (e > di 1) - non gestisco - metto a 0 */
					sprintf(szSQLCmd,"update catalogo set prqtffo=0,prswffo=0 where prcdpro='%s';",szCDPRO);
				break;
			}
			PGResUpdate=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
			if (PQresultStatus(PGResUpdate) != PGRES_COMMAND_OK && atoi(PQcmdTuples(PGResUpdate))==0) {
#ifdef TRACE
				trace_out_vstr_date(1,"Errore in update articolo [%s]",szCDPRO);
#endif
				bRetValue=FALSE;
			}
			DBclear(PGResUpdate);
		}
		DBclear(PGResCat);
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
		/* Note */
		sprintf(szBuffer,"[%d] Note ",nUpdated[2]);
		strcat(szMsgData,szBuffer);
	}
	if(nUpdated[4]){
		/* Catalogo  */
		sprintf(szBuffer,"[%d] Articoli di Catalogo ",nUpdated[4]);
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
			}
#ifdef TRACE
			trace_out_vstr_date(1,"Ricevuti %s",szMsgData);
#endif
		} else {
			strcpy(szMsg,"Nessun dato ricevuto\n");
			/*
			* segnalo a video
			*/
			if(bAlone==FALSE){
				SendMessage(Cfg.nMainID, nPID,  DISPLAY_MSG, szMsg);
			}
#ifdef TRACE
			trace_out_vstr_date(1,"Nessun dato ricevuto");
#endif
		}
	}
	

  /* azzero i dati di ricezione */
	for(nIndex=0; nIndex<nTables; nIndex++){
		/*
		* Flag di tabella modificata
		*/
		nUpdated[nIndex]=0;
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
	DisconnectDataBase();

#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
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
#ifdef TRACE
	trace_out_vstr_date(1,"Got Signal ! ");
#endif
	bExitRequest=TRUE;
}

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(void)
{
	char szParagraph[128];
	char szCurrentDirectory[128];

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(Cfg.szCniCfg,szCurrentDirectory);
	strcat(Cfg.szCniCfg,"/cni.cfg");
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	GetFileString(szParagraph,"TipoOrdini",           "C",          Cfg.szTipoOrdini,       80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathData",       "../data",          Cfg.szPathData,         80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathExport",     "../export",        Cfg.szPathExport,       80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathFtpImport",     "../export/ftp-mondadori",        Cfg.szPathFtpImport,       80,Cfg.szCniCfg,NULL);

	GetFileString(szParagraph,"ImportMonitor",    "import.mon",     Cfg.szImportMonitor,    80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"ImportSpedizioni", "import.txt",     Cfg.szImportSpedizioni, 80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"ImportCatalogo",   "import.txt",     Cfg.szImportCatalogo,   80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"ImportTabelle",    "import.txt",     Cfg.szImportTabelle,    80,Cfg.szCniCfg,NULL);

	Cfg.nDebugVersion    = GetFileInt(szParagraph,"DebugVersion",      0, Cfg.szCniCfg,NULL);

	/*
	* lettura configurazione DataBase System
	*/
	strcpy(szParagraph,"DataBase Settings");
	GetFileString(szParagraph,"PGHost",    "localhost", Cfg.szPGHost,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"PGPort",    "5432",      Cfg.szPGPort,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"PGDataBase","momo",      Cfg.szPGDataBase, 80,Cfg.szCniCfg,NULL); 

	/*
	* Lettura dati processi
	*/
	ReadProcInfo(Cfg.szCniCfg);

	/* 
	* PROC_RECEIVE 
	*/
	Cfg.nDelayRicezione = GetFileInt(ProcList[nPID].szProcName,"DelayRicezione",         30,Cfg.szCniCfg,NULL);
	/*
	* Codice PID main process di riferimento (a cui mandare i messaggi )
	*/
	Cfg.nMainID = GetFileInt(ProcList[nPID].szProcName,"MainID",   0, Cfg.szCniCfg,NULL);

#ifdef TRACE
	trace_out_vstr_date(1,"Read Configuration");
	trace_out_vstr(1,"-------------------------------------------");
	trace_out_vstr(1,"PathData                : %s",Cfg.szPathData);
	trace_out_vstr(1,"PathExport              : %s",Cfg.szPathExport);
	trace_out_vstr(1,"DebugVersion            : %d",Cfg.nDebugVersion);
	trace_out_vstr(1,"PGHost                  : %s",Cfg.szPGHost);
	trace_out_vstr(1,"PGPort                  : %s",Cfg.szPGPort);
	trace_out_vstr(1,"PGDataBase              : %s",Cfg.szPGDataBase);
	trace_out_vstr(1,"-------------------------------------------");
#endif
}

/*
* ReadProcInfo()
* Lettura dei parametri relativi ai processi 
*/
void ReadProcInfo(char *szFileConfig)
{
	char szProcIndex[128];
	char szProcBuffer[128];
	int nIndex;

	for (nIndex=0; nIndex<NUM_PROC; nIndex++) {
		sprintf(szProcIndex,"Proc_%02d",nIndex);
		GetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szFileConfig,NULL);
		sscanf(szProcBuffer, "%[^,],%d,%d,%d",
				ProcList[nIndex].szProcName,
				&ProcList[nIndex].nQKey,
				&ProcList[nIndex].nPriority,
				&ProcList[nIndex].bExecute);
		/* 
		* rm 16-01-2002 : Elimino gli spazi in testa e in coda per 
		*                 problemi con la RunSimpleProcess()  e la lettura della condifgurazione
		*/
		StrTrimAll(ProcList[nIndex].szProcName);

		ProcList[nIndex].nGId=nIndex+1;

		/*
		* resetto il numero coda messaggi del processo
		*/
		ProcList[nIndex].nQNumber=0;
	}
}


/*
* RicavaDistribuzione()
* ---------------------
* Ricavo i seguenti dati:
*  -  Peso Lordo Spedizione
*  -  Switch Tipo Imballo
*  -  Tipo Spedizione
*  -  Linea
*  -  Scalo
*  -  Sottoscalo
* ritorna : 
*    TRUE  : tutto OK
*    FALSE : qualche problema
*/
BOOL RicavaDistribuzione(PDATI_DISTRIBUZIONE pDati,char *szOrdProg,int nPeso,int nColli)
{
	PGresult *PGRes;
	BOOL bFoundFC=FALSE;
	BOOL bFoundGC=FALSE;
	BOOL bFoundLS=FALSE;
	BOOL bFoundFP=FALSE;
	BOOL bRetValue=TRUE;
	char szCDRID[128];
	char szCDSOC[128];
	char szCDMAG[128];
	char szCPCLI[128];
	char szCDFPG[128];
	char szCPRAG[128];
	char szFPRAG[128];
	char szTPNAZ[128];

	strcpy(szCDFPG,"");
	strcpy(szCPRAG,"");
	strcpy(szFPRAG,"");

	strcpy(pDati->szTPSPE,"99");
	strcpy(pDati->szCDLIN,"99");
	strcpy(pDati->szCDSCA,"99");
	strcpy(pDati->szCDSSC,"99");
	strcpy(pDati->szIDVET,"99");
	strcpy(pDati->szCDVE1,"999999");
	strcpy(pDati->szCDVE2,"999999");

	/* ricavare peso e cap spedizione */
	/* rm 01-06-2011 - aggiungo tpspe */
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select rocdrid,rocpcli,rocdfpg,rotpspe from ric_ord where ordprog='%s';",szOrdProg);
	if(PQntuples(PGRes)){
		strcpy(szCDRID,PQgetvalue(PGRes,0,0));
		strcpy(szCPCLI,PQgetvalue(PGRes,0,1));
		strcpy(szCDFPG,PQgetvalue(PGRes,0,2));
		strcpy(pDati->szTPSPE,PQgetvalue(PGRes,0,3));

		strcpy(szCDSOC,SubStr(szOrdProg,0,2));
		strcpy(szCDMAG,SubStr(szOrdProg,2,7));
	}
	DBclear(PGRes);

	/* inizializzo il tipo scatole a 1 (colli) */
	strcpy(pDati->szSWCOL,"1");


	/* 
	* rm 08-09-2011 : cerco in tabella forzatura clienti, e' stata richiesta
	*                 da Mondadori (Avesani / Paiusco)
	*/ 

	/* cerco in tabella forzatura clienti */
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcswcol,elctpspe, elccdlin, elccdsca, elccdssc from ttfc where elccdsoc='%s' and elccdmag='%s' and elccdrid='%s' and elctptpd='G' order by elccdsoc,elccdmag,elccdrid;",
		szCDSOC, szCDMAG, szCDRID);
	if(PQntuples(PGRes)){
		bFoundFC=TRUE;
		
		strcpy(pDati->szSWCOL,PQgetvalue(PGRes,0,0));
		strcpy(pDati->szTPSPE,PQgetvalue(PGRes,0,1));
		strcpy(pDati->szCDLIN,PQgetvalue(PGRes,0,2));
		strcpy(pDati->szCDSCA,PQgetvalue(PGRes,0,3));
		strcpy(pDati->szCDSSC,PQgetvalue(PGRes,0,4));

	}
	DBclear(PGRes);

	/*
	* rm 01-06-2011 : se tpspe e' valorizzata 'SF' salto tutta la ricerca su ttfc/ttgc e ricavo la nazionalita'
	*                 della spedizione per poi passare a ttlv 
	*/
	if(!bFoundFC && !strcmp(pDati->szTPSPE,"SF")){
		/* Ricavo nazionalita' da ttgc - TPNAZ */
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elctpnaz from ttgc where elccdsoc='%s' and elccdmag='%s' and elccdcap='%s' order by elccdsoc,elccdmag,elccdcap;",
			szCDSOC, szCDMAG, szCPCLI);
		if(PQntuples(PGRes)){
			/* Ricavo ELCTPNAZ */
			bFoundGC=TRUE;
			strcpy(szTPNAZ,PQgetvalue(PGRes,0,0));
			DBclear(PGRes);
		} else {
			DBclear(PGRes);
			/*
			* Metto a zero gli ultimi 2 caratteri del CAP
			*/
			strcpy(szCPCLI+(strlen(szCPCLI)-2),"00");
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elctpnaz from ttgc where elccdsoc='%s' and elccdmag='%s' and elccdcap='%s' order by elccdsoc,elccdmag,elccdcap;",
				szCDSOC, szCDMAG, szCPCLI);
			if(PQntuples(PGRes)){
				/* Ricavo ELCTPNAZ */
				bFoundGC=TRUE;
				strcpy(szTPNAZ,PQgetvalue(PGRes,0,0));
			}
			DBclear(PGRes);
		}
		if(!bFoundGC){
			bRetValue=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttgc non trovata [CPCLI:%s]",szOrdProg,szCPCLI);
#endif
		} else {
			/* rm 01-06-2011 : valorizzo i campi cdlin,cdsca,cdssc */
			switch(szTPNAZ[0]){
				case 'I' : /* italia */
					strcpy(pDati->szCDLIN,"IT");
				break;
				case 'E' : /* italia */
					strcpy(pDati->szCDLIN,"ES");
				break;
			}
			strcpy(pDati->szCDSCA,"00");
			strcpy(pDati->szCDSSC,"00");
		}
	} else {




#ifdef ELIMINATO_FC
		/* cerco in tabella forzatura clienti */
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcswcol,elctpspe, elccdlin, elccdsca, elccdssc from ttfc where elccdsoc='%s' and elccdmag='%s' and elccdrid='%s' and elctptpd='G' order by elccdsoc,elccdmag,elccdrid;",
			szCDSOC, szCDMAG, szCDRID);
		if(PQntuples(PGRes)){
			bFoundFC=TRUE;
			
			strcpy(pDati->szSWCOL,PQgetvalue(PGRes,0,0));
			strcpy(pDati->szTPSPE,PQgetvalue(PGRes,0,1));
			strcpy(pDati->szCDLIN,PQgetvalue(PGRes,0,2));
			strcpy(pDati->szCDSCA,PQgetvalue(PGRes,0,3));
			strcpy(pDati->szCDSSC,PQgetvalue(PGRes,0,4));

		}
		DBclear(PGRes);
#endif /* ELIMINATO_FC */

		if(!bFoundFC){

			/* CAP */
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elccprag from ttgc where elccdsoc='%s' and elccdmag='%s' and elccdcap='%s' order by elccdsoc,elccdmag,elccdcap;",
				szCDSOC, szCDMAG, szCPCLI);
			if(PQntuples(PGRes)){
				/* Ricavo ELCCPRAG */
				bFoundGC=TRUE;
				strcpy(szCPRAG,PQgetvalue(PGRes,0,0));
				DBclear(PGRes);
			} else {
				DBclear(PGRes);
				/*
				* Metto a zero gli ultimi 2 caratteri del CAP
				*/
				strcpy(szCPCLI+(strlen(szCPCLI)-2),"00");
				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elccprag from ttgc where elccdsoc='%s' and elccdmag='%s' and elccdcap='%s' order by elccdsoc,elccdmag,elccdcap;",
					szCDSOC, szCDMAG, szCPCLI);
				if(PQntuples(PGRes)){
					/* Ricavo ELCCPRAG */
					bFoundGC=TRUE;
					strcpy(szCPRAG,PQgetvalue(PGRes,0,0));
				}
				DBclear(PGRes);
			}

			if(!bFoundGC){
				bRetValue=FALSE;
#ifdef TRACE
				trace_out_vstr_date(1,"Ordine [%s] chiave ttgc non trovata [CPCLI:%s]",szOrdProg,szCPCLI);
#endif
			}

			if(bRetValue){
				/* Forma di pagamento */
				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcfprag from ttfp where elccdsoc='%s' and elccdmag='%s' and elccdfpg='%s' order by elccdsoc,elccdmag,elccdfpg;",
					szCDSOC, szCDMAG, szCDFPG);
				if(PQntuples(PGRes)){
					/* Ricavo ELCFPRAG */
					bFoundFP=TRUE;
					strcpy(szFPRAG,PQgetvalue(PGRes,0,0));
					DBclear(PGRes);
				} else {
					DBclear(PGRes);
					/*
					* Provo con '***'
					*/
					strcpy(szCDFPG,"***");
					PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcfprag from ttfp where elccdsoc='%s' and elccdmag='%s' and elccdfpg='%s' order by elccdsoc,elccdmag,elccdfpg;",
						szCDSOC, szCDMAG, szCDFPG);
					if(PQntuples(PGRes)){
						/* Ricavo ELCFPRAG */
						bFoundFP=TRUE;
						strcpy(szFPRAG,PQgetvalue(PGRes,0,0));
					}
					DBclear(PGRes);
				}
			}


			if(!bFoundFP){
#ifdef TRACE
				trace_out_vstr_date(1,"Ordine [%s] chiave ttfp non trovata [CDFPG:%s]",szOrdProg,szCDFPG);
#endif
				bRetValue=FALSE;
			}
		}

		if(bRetValue){
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elctpspe,elccdlin,elccdsca,elccdssc from ttls where elccdsoc='%s' and elccdmag='%s' and elccprag='%s' and elcfprag='%s' and elctptpd='G' and elcmaxps > %d and elcmaxcl > %d order by elccdsoc,elccdmag,elccprag,elcfprag,elcmaxps,elcmaxcl;",
					szCDSOC, szCDMAG, szCPRAG,szFPRAG,nPeso,nColli);
			if(PQntuples(PGRes)){
				bFoundLS=TRUE;
				strcpy(pDati->szTPSPE,PQgetvalue(PGRes,0,0));
				strcpy(pDati->szCDLIN,PQgetvalue(PGRes,0,1));
				strcpy(pDati->szCDSCA,PQgetvalue(PGRes,0,2));
				strcpy(pDati->szCDSSC,PQgetvalue(PGRes,0,3));
			}
			DBclear(PGRes);
		}

		if(!bFoundLS){
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttls non trovata [CPRAG:%s FPRAG:%s]",szOrdProg,szCPRAG,szFPRAG);
#endif
			bRetValue=FALSE;
		}
	}

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcidvet,elccdve1,elccdve2 from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='%s' and elccdssc='%s';",
		szCDSOC,szCDMAG,pDati->szTPSPE,pDati->szCDLIN,pDati->szCDSCA,pDati->szCDSSC);
	if(PQntuples(PGRes)){
		strcpy(pDati->szIDVET,PQgetvalue(PGRes,0,0));
		strcpy(pDati->szCDVE1,PQgetvalue(PGRes,0,1));
		strcpy(pDati->szCDVE2,PQgetvalue(PGRes,0,2));
	} else {
#ifdef TRACE
		trace_out_vstr_date(1,"Ordine [%s] chiave ttlv non trovata [TPSPE:%s CDLIN:%s CDSCA:%s CDSSC:%s] provo con **",szOrdProg, pDati->szTPSPE,pDati->szCDLIN,pDati->szCDSCA,pDati->szCDSSC);
#endif
		DBclear(PGRes);
		/*
		* rm 8-01-2008 - Provo con '**'
		*/
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcidvet,elccdve1,elccdve2 from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='%s' and elccdssc='%s';",
			szCDSOC,szCDMAG,pDati->szTPSPE,pDati->szCDLIN,"**","**");
		if(PQntuples(PGRes)){
			strcpy(pDati->szIDVET,PQgetvalue(PGRes,0,0));
			strcpy(pDati->szCDVE1,PQgetvalue(PGRes,0,1));
			strcpy(pDati->szCDVE2,PQgetvalue(PGRes,0,2));
		} else {
			bRetValue=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttlv non trovata [TPSPE:%s CDLIN:%s CDSCA:%s CDSSC:%s]",szOrdProg, pDati->szTPSPE,pDati->szCDLIN,"**","**");
#endif
		}
	}
	DBclear(PGRes);

	return(bRetValue);
}

