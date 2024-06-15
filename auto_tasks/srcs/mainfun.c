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
// #include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>

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


int GetNewTasksFromDB(PATASK pTask)
{
	int nId;
	//int nStampe;
	char szParametri[256];
	DBresult *DBRes;
	DBresult *DBResUpdate;
	int nCod = -1;

	DBRes=DBExecQuery(Cfg.nDebugLevel>5,"select id_task,codice_tipo_task,stringa_parametri from tasks where stato=' ' order by id_task limit 1;");
	//nStampe=0;
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)!=1){
		// errore ? 
	} else {
		nId=atoi(DBgetvalue(DBRes,0,0));
		nCod=atoi(DBgetvalue(DBRes,0,1));
		strcpy(szParametri,DBgetvalue(DBRes,0,2));
		pTask->nId=nId;
		pTask->nCodice=nCod;
		strcpy(pTask->szParametri,szParametri);

		DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update tasks set stato='S' where id_task=%d;",nId);
		DBclear(DBResUpdate);
	}
	DBclear(DBRes);
	return nCod;
}




/*
* Gestione spedizione xab e distinta
* all'atto del cambio stato a S di tutta una linea
*/
ep_bool_t CheckCambioStatoLinea(char cStato, ep_bool_t bCambiaStato)
{
	int nIndex=0;
	int nDistribuzioni=0;
	int nNumeroOrdini;
	int nNumeroRighe;
	int nIndexOrdini;
	int nIndexRighe;
	//int nDistinta;
	DBresult *DBRes;
	//DBresult *DBResSupport;
	DBresult *DBResUpdate;
	//DBresult *DBResFlg;
	DBresult *DBResOrdini;
	DBresult *DBResRighe;
	DBresult *DBResCedola;
	DBresult *DBResUpdateCedola;
	ep_bool_t bRetValue=TRUE;
	ep_bool_t bOK=TRUE;
	//char szText[1024];
	char szNMCED[128];
	char szCDLIN[128];
	char szOrdProg[128];
	//char szExportFileAgg[128];
	//char szCorriereFile[256];
	//char szCorriereFileAgg[256];
	//char szMonitor[128];
	char szCommand[2048];
	//char szAttachment[256];
	//int nProgrMail=0;
	//ep_bool_t bFound=FALSE;
	//ep_bool_t bInviaEMAIL=FALSE;
	ep_bool_t bERRORE=TRUE;
	//GError *pGError;
	FILE *fpExport;
	FILE *fpExportOrdini;
	FILE *fpExportRighe;
	char szExportFile[2048];

	char szExportOrdini[256];
	char szExportRighe[256];



	/*
	* cerco tutti i raggruppamenti ronmced, rocdlin (linee di distribuzione) che contengono ordini in stato 'C'
	*/


	/* CASO NUMERO 1 : FLUSSI */
	/* 01-10-2013 rm+lg : gestione flussi */


	nDistribuzioni=0;
	DBRes=DBExecQuery(Cfg.nDebugLevel>3, "select ronmced, rocdlin from ric_ord where rostato='%c' group by ronmced, rocdlin except select ronmced, rocdlin from ric_ord where rostato not in ('%c') group by ronmced, rotpspe, rocdlin, rocdve2;" , cStato , cStato);

	nDistribuzioni=DBntuples(DBRes);
	if(nDistribuzioni){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Trovate %d linee da spedire - FLUSSO ",nDistribuzioni);
#endif

		sprintf(szExportFile,"%s/%s.part",Cfg.szPathExportFlussoCedola,StrTrimAll(Cfg.szExportFlussoCedola));
		if((fpExport=fopen(szExportFile,"w"))!=NULL){
			for (nIndex=0;nIndex<nDistribuzioni;nIndex++){
				int nLineaIndex,nLinee;
				int nProgressivo;

				strcpy(szNMCED,DBgetvalue(DBRes,nIndex,0)); StrTrimAll(szNMCED);
				strcpy(szCDLIN,DBgetvalue(DBRes,nIndex,1)); StrTrimAll(szCDLIN);

				DBResCedola=DBExecQuery(Cfg.nDebugLevel,"insert into flusso_cedola ( FCNMCED, FCTPSPE, FCCDLIN, FCDSLIN, FCNMCPE, FCNMCLL, FCNMORD, FCPSTOT, FCSTATO) \
					select ronmced,'2',rocdlin,rodslin,sum(ronmcpe),sum(ronmcll),count(ordprog),sum(ropspre),' ' \
					from ric_ord ro where ronmced='%s' and rocdlin='%s' group by ronmced, rocdlin;",szNMCED,szCDLIN);
				DBclear(DBResCedola);

				DBResCedola=DBExecQuery(Cfg.nDebugLevel,"select FCPROGR, FCNMCED, FCTPSPE, FCCDLIN, FCDSLIN, FCNMCPE, FCNMCLL, FCNMORD, FCPSTOT, FCSTATO from flusso_cedola where fcstato=' ';");

				if((nLinee=DBntuples(DBResCedola))){
					for(nLineaIndex=0;nLineaIndex<nLinee;nLineaIndex++){

						nProgressivo=atoi(DBgetvalue(DBResCedola,nLineaIndex,0));

						WriteAsciiRecord(fpExport, &tFlussoCedola,DBResCedola,nLineaIndex,TRUE,FALSE);

						DBResUpdateCedola=DBExecQuery(Cfg.nDebugLevel,"update flusso_cedola set fcstato='S' where FCPROGR=%d;",nProgressivo);
						DBclear(DBResUpdateCedola);

					}
				}

				DBclear(DBResCedola);

				/* aggiorno il flag roflifl */
				DBResUpdate=DBExecQuery(Cfg.nDebugLevel>2,"update ric_ord set roflifl='%c' where ronmced='%s' and rocdlin='%s';",
					FLUSSO_INVIATO, szNMCED, szCDLIN);
				if((DBresultStatus(DBResUpdate)!=DBRES_COMMAND_OK)){
					bOK=FALSE;
				}
				DBclear(DBResUpdate);
			}

			fclose(fpExport); 
			/* rinomino .part per semaforizzare ... */
			sprintf(szCommand,"mv %s/%s.part %s/flusso-cedola-%s-%s.txt",Cfg.szPathExportFlussoCedola,Cfg.szExportFlussoCedola,Cfg.szPathExportFlussoCedola,GetDateYYYYMMDD( szDateBuffer), GetTimeHHMMSS( szTimeBuffer) );
			system(szCommand);


		} else {
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Errore in apertura file [%s] FLUSSO",szExportFile);
#endif
		}

#ifdef TRACE
		trace_debug(TRUE, TRUE, "Trovate %d linee da spedire - CONSUNTIVO ",nDistribuzioni);
#endif

		sprintf(szExportOrdini,"%s/%s.part",Cfg.szPathExport,g_strstrip(Cfg.szExportSpedizioni));
		sprintf(szExportRighe,"%s/%s.part",Cfg.szPathExport,g_strstrip(Cfg.szExportRighe));
		/* se riesco ad aprire i 2 files in scrittura ... */
		if((fpExportOrdini=fopen(szExportOrdini,"w"))!=NULL && (fpExportRighe=fopen(szExportRighe,"w"))!=NULL ){
			for (nIndex=0;nIndex<nDistribuzioni;nIndex++){

				strcpy(szNMCED,DBgetvalue(DBRes,nIndex,0)); StrTrimAll(szNMCED);
				strcpy(szCDLIN,DBgetvalue(DBRes,nIndex,1)); StrTrimAll(szCDLIN);


				/*
				* seleziono gli ordini relativi a cedola e linea
				*/
				DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select r.ronmced,r.ordprog,r.ronmcll,c.cpprgcl from ric_ord r,col_prod c where r.ordprog=s.ordprog and r.ordprog=c.ordprog and c.cpnmcol=1 and ronmced='%s' and rocdlin='%s' order by r.ordprog;", szNMCED, szCDLIN);

				if((nNumeroOrdini=DBntuples(DBResOrdini))){

#ifdef TRACE
					trace_debug(TRUE, TRUE, "Lanciata Spedizione Dati - C:%s L:%s (%d:Ordini) a Host",szNMCED,szCDLIN, nNumeroOrdini);
#endif

					for (nIndexOrdini=0;nIndexOrdini<nNumeroOrdini;nIndexOrdini++){
						strcpy(szOrdProg,DBgetvalue(DBResOrdini,nIndexOrdini,1));
						/*
						* spedizione dell'ordine a Host
						*/
						/*
						* rm 07-02-2007 : controllo segnacollo > 0 
						*/
						if(atoi(DBgetvalue(DBResOrdini,nIndexOrdini,3))==0){
	#ifdef TRACE
							trace_debug(TRUE, TRUE, "ERRORE IN SPEDIZIONE ORDINE [%s] SEGNACOLLO NON PRESENTE !",DBgetvalue(DBResOrdini,nIndexOrdini,1));
	#endif
							bERRORE=TRUE;
						}
						if(!bERRORE){
							WriteAsciiRecord(fpExport, &tEvasi,DBResOrdini,nIndexOrdini,TRUE,FALSE);
							
							if(bCambiaStato){
								/*
								* cambio lo stato dell'ordine ricevuto (E -> H)
								*/
								DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c' where ordprog='%s';", ORDINE_SPEDITO_HOST, szOrdProg);DBclear(DBResUpdate);

								/*
								* cambio lo stato dei colli relativi all'ordine(E -> H)
								*/
								DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c' where ordprog='%s';", COLLO_SPEDITO_HOST, szOrdProg); DBclear(DBResUpdate);

								/*
								* cambio lo stato delle righe relative all'ordine(E -> H)
								*/
								DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update rig_prod set rpstato='%c' where ordprog='%s';", RIGA_SPEDITA_HOST, szOrdProg); DBclear(DBResUpdate);

								/*
								* RIGHE - seleziono le righe dell'ordine 
								*/
								DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"select r.RONMCED, r.ORDPROG, p.RPNMCOL, p.RPCDUBI, p.RPCDPRO, p.RPPRIOR, p.RPPRRIG, p.RPSWFFO, p.RPSWPCF, p.RPQTSPE, to_char(p.RPTMPRE,'YYYYMMDDHH24MISS'), p.RPCDOPR, to_char(c.CPTMEVA,'YYYYMMDDHH24MISS'), c.CPPSPRE, c.CPPSREA, c.CPTPFOR from ric_ord r,col_prod c,rig_prod p where r.ordprog=s.ordprog and r.ordprog=c.ordprog and p.ordprog=r.ordprog and p.rpnmcol=c.cpnmcol and r.ordprog='%s' order by r.ordprog,c.cpnmcol,p.rpcdpro;",szOrdProg);
								if((nNumeroRighe=DBntuples(DBResRighe))){
									for (nIndexRighe=0;nIndexRighe<nNumeroRighe;nIndexRighe++){
										/*
										* spedizione del contenuto ordine a Host (RIGHE)
										*/
										WriteAsciiRecord(fpExportRighe, &tContenutoColli,DBResRighe,nIndexRighe,TRUE,FALSE);
									}
								}
								DBclear(DBResRighe);


							}
						}
					}

				} else {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Spedizione Dati - non ci sono ordini da spedire - C:%s L:%s",szNMCED,szCDLIN);
#endif
				}

				DBclear(DBResOrdini);
			}

			if(fpExportOrdini) fclose(fpExportOrdini); 
			if(fpExportRighe) fclose(fpExportRighe); 

			/* rinomino .part per semaforizzare ... */
			sprintf(szCommand,"mv %s/%s.part %s/%s",Cfg.szPathExport,Cfg.szExportSpedizioni,Cfg.szPathExport,Cfg.szExportSpedizioni);
			system(szCommand);

			sprintf(szCommand,"mv %s/%s.part %s/%s",Cfg.szPathExport,Cfg.szExportRighe,Cfg.szPathExport,Cfg.szExportRighe);
			system(szCommand);

			sprintf(szExportOrdini,"%s/%s",Cfg.szPathExport,g_strstrip(Cfg.szExportSpedizioni));
			sprintf(szExportRighe,"%s/%s",Cfg.szPathExport,g_strstrip(Cfg.szExportRighe));

			if(!bERRORE){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Spediti a Host %d ordini",nNumeroOrdini);
#endif

				/* 
				* 07-02-2007 rm : copia di sicurezza 
				*/	
				sprintf(szCommand,"cp %s %s.%s-%s",
								szExportOrdini,
								szExportOrdini,
								GetDateYYYYMMDD(szDateBuffer),
								GetTimeHHMMSS(szTimeBuffer));
				system(szCommand);

				/* 
				* 07-02-2007 rm : copia di sicurezza 
				*/	
				sprintf(szCommand,"cp %s %s.%s-%s",
								szExportRighe,
								szExportRighe,
								GetDateYYYYMMDD(szDateBuffer),
								GetTimeHHMMSS(szTimeBuffer));
				system(szCommand);
			} else {
#ifdef TRACE
				trace_debug(TRUE, TRUE, "SPEDIZIONE ANNULLATA");
#endif
				/* 
				* 07-02-2007 rm : copia di sicurezza 
				*/	
				sprintf(szCommand,"mv %s %s.%s-%s-KO",
								szExportOrdini,
								szExportOrdini,
								GetDateYYYYMMDD(szDateBuffer),
								GetTimeHHMMSS(szTimeBuffer));
				system(szCommand);

				sprintf(szCommand,"mv %s %s.%s-%s-KO",
								szExportRighe,
								szExportRighe,
								GetDateYYYYMMDD(szDateBuffer),
								GetTimeHHMMSS(szTimeBuffer));
				system(szCommand);
			}

		} else {
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Errore in apertura files [%s][%s] Spedizione",szExportOrdini,szExportRighe);
#endif
		}
	}
	DBclear(DBRes);


	return bRetValue;
}


ep_bool_t CheckMailError( void )
{
	DBresult *DBResMail;
	ep_bool_t bRetValue=TRUE;

	/* 
	* elimino le mail con errori di ricezione e piu' vecchie di una settimana 
	*/
	DBResMail=DBExecQuery(Cfg.nDebugLevel>3,"update mail_queue set mqstato='%c' where mqstato='%c' and mqtmins<CURRENT_TIMESTAMP-interval '1 week';",EMAIL_STORNATA,EMAIL_ERRORE);
	DBclear(DBResMail);

	/* 
	* Risetto le mail con errore e data piu' recente 
	*/
	DBResMail=DBExecQuery(Cfg.nDebugLevel>3,"update mail_queue set mqstato='%c' where mqstato='%c';",EMAIL_IN_CODA,EMAIL_ERRORE);
	DBclear(DBResMail);

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
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Got Signal ! ");
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
	
    extern char __configuration_file__[];
	strcpy(Cfg.szCfgFileName,szCurrentDirectory);
	strcat(Cfg.szCfgFileName,"/");
	strcat(Cfg.szCfgFileName, __configuration_file__);
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	GetFileString(szParagraph,"TipoOrdini",                 "C",                    Cfg.szTipoOrdini,               80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"tmp_suffix",                 "tmp",                  Cfg.szTmpSuffix,                80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathData",                   "../data",              Cfg.szPathData,                 80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathTrace",                  "../trace",             Cfg.szPathTrace,                80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExport",                 "../export",            Cfg.szPathExport,               80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathStorico",                "../storico",           Cfg.szPathStorico,              80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathCorriereExport",         "../export",            Cfg.szPathCorriereExport,       80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathCorriereExportBackup",   "../export",            Cfg.szPathCorriereExportBackup, 80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathPDFExport",              "../export/pdf",        Cfg.szPathPDFExport,            80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathPDFExportBackup",        "../export/pdf-backup", Cfg.szPathPDFExportBackup,      80,Cfg.szCfgFileName,NULL);

	GetFileString(szParagraph,"PathExportFlussoCedola",     "../export/flusso-cedola", Cfg.szPathExportFlussoCedola, 80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportFlussoCedola",         "flusso-cedola.txt",       Cfg.szExportFlussoCedola, 80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExportUdcRestituiti",     "../export/flusso-udc-restituiti", Cfg.szPathExportFlussoUdcRestituiti, 80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportUdcRestituiti",         "flusso-udc-restituiti.txt",       Cfg.szExportFlussoUdcRestituiti, 80,Cfg.szCfgFileName,NULL);

	GetFileString(szParagraph,"ExportMonitorStorico",       "export.mon",   Cfg.szExportMonitorStorico,  80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportSpedizioniStorico",    "export.txt",   Cfg.szExportSpedizioniStorico,  80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ImportMonitor",              "import.mon",   Cfg.szImportMonitor,     80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ImportSpedizioni",           "import.txt",   Cfg.szImportSpedizioni,  80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ImportCatalogo",             "import.txt",   Cfg.szImportCatalogo,    80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ImportTabelle",              "import.txt",   Cfg.szImportTabelle,     80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportSpedizioni",           "export.txt",   Cfg.szExportSpedizioni,  80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportRighe",                "righe.txt",    Cfg.szExportRighe,       80,Cfg.szCfgFileName,NULL);

	GetFileString(szParagraph,"PDFDistintaFileName",        "distinta.xml",   Cfg.szPDFDistintaFileName,  80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"PDFXABFileName",             "xab.xml",        Cfg.szPDFXABFileName,       80,Cfg.szCfgFileName,NULL);


	GetFileString(szParagraph,"ExportMonitor",              "export.mon",   Cfg.szExportMonitor,     80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportCorriere",             "corriere.txt", Cfg.szExportCorriere,    80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportCorrAgg",              "corr_agg.txt", Cfg.szExportCorriereAgg, 80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"ExportCorrSDA",              "corr_sda.txt", Cfg.szExportCorrSDA,     80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"NotaCorriere",               "NOTA CORRIERE",Cfg.szNotaCorriere,      80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"TPSPE",                      "",             Cfg.szTPSPE,             80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"DSTSP",                      "",             Cfg.szDSTSP,             80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"CDSOC",                      "AM",           Cfg.szCDSOC,             80,Cfg.szCfgFileName,NULL);
	GetFileString(szParagraph,"CDMAG",                      "0000015",      Cfg.szCDMAG,             80,Cfg.szCfgFileName,NULL);


	/*
	* lettura configurazione DataBase System
	*/
	strcpy(szParagraph,"DataBase Settings");
	GetFileString(szParagraph,"DBHost",     "localhost", Cfg.szDBHost,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBPort",     "5432",      Cfg.szDBPort,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBName",     "momo",      Cfg.szDBName,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBUser",     "user",      Cfg.szDBUser,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBPassword", "pwd",       Cfg.szDBPassword, 80,Cfg.szCfgFileName,NULL); 

	/*
	* lettura configurazione mail
	*/
	strcpy(szParagraph,"Mail Settings");
	GetFileString(szParagraph,"MailAddress",    "localhost", Cfg.szMailAddress,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailHost",    "localhost", Cfg.szMailHost,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailUser",    "localhost", Cfg.szMailUser,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailPassword",    "localhost", Cfg.szMailPassword,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailAddressDDTNovita",    "localhost", Cfg.szMailAddressDDTNovita,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailHostDDTNovita",    "localhost", Cfg.szMailHostDDTNovita,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailUserDDTNovita",    "localhost", Cfg.szMailUserDDTNovita,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailPasswordDDTNovita",    "localhost", Cfg.szMailPasswordDDTNovita,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailAddressFlussoDati",    "localhost", Cfg.szMailAddressFlussoDati,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailHostFlussoDati",    "localhost", Cfg.szMailHostFlussoDati,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailUserFlussoDati",    "localhost", Cfg.szMailUserFlussoDati,     80,Cfg.szCfgFileName,NULL); 
	GetFileString(szParagraph,"MailPasswordFlussoDati",    "localhost", Cfg.szMailPasswordFlussoDati,     80,Cfg.szCfgFileName,NULL); 

	/*
	* Lettura dati processi
	*/
	ReadProcInfo(Cfg.szCfgFileName);

	/* 
	* PROC_AUTO_TASKS 
	*/
	strcpy(szParagraph,"auto_tasks");
	Cfg.nDelaySendFileCorriere         = GetFileInt(szParagraph,"DelaySendFileCorriere",          100,Cfg.szCfgFileName,NULL);
	Cfg.nDelaySendMailCorriere         = GetFileInt(szParagraph,"DelaySendMailCorriere",          100,Cfg.szCfgFileName,NULL);
	Cfg.nDelayCheckMail                = GetFileInt(szParagraph,"DelayCheckMail",                 100,Cfg.szCfgFileName,NULL);
	Cfg.nDelayCheckMailError           = GetFileInt(szParagraph,"DelayCheckMailError",           3000,Cfg.szCfgFileName,NULL);
	Cfg.nDelayCheckCambioStato         = GetFileInt(szParagraph,"DelayCheckCambioStato",          100,Cfg.szCfgFileName,NULL);
	Cfg.nDelayCheckFlussoUdcRestituiti = GetFileInt(szParagraph,"DelayCheckFlussoUdcRestituiti",  600,Cfg.szCfgFileName,NULL);
	Cfg.nDebugLevel                  = GetFileInt(szParagraph,"DebugLevel",                     0,Cfg.szCfgFileName,NULL);

	/*
	* Codice PID main process di riferimento (a cui mandare i messaggi )
	*/
	Cfg.nMainID = GetFileInt(ProcessGetName(nPID),"MainID",   0, Cfg.szCfgFileName,NULL);

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Read Configuration");
	trace_debug(FALSE, TRUE, "-------------------------------------------");
	trace_debug(FALSE, TRUE, "PathData                : %s",Cfg.szPathData);
	trace_debug(FALSE, TRUE, "PathExport              : %s",Cfg.szPathExport);
	trace_debug(FALSE, TRUE, "DebugLevel              : %d",Cfg.nDebugLevel);
	trace_debug(FALSE, TRUE, "DBHost                  : %s",Cfg.szDBHost);
	trace_debug(FALSE, TRUE, "DBPort                  : %s",Cfg.szDBPort);
	trace_debug(FALSE, TRUE, "DBName                  : %s",Cfg.szDBName);
	trace_debug(FALSE, TRUE, "-------------------------------------------");
#endif
}

/*
* void on_dlg_aggiorna_archivi_pb_ok_clicked (gpointer user_data, GtkButton *button)
* Effettua un aggiornamento degli indici del BD
*/
void do_aggiorna_db (void)
{
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Lanciato aggiornamento indici database");
#endif

	do_vacuum_table("col_prod");
	do_vacuum_table("rig_prod");
	do_vacuum_table("ric_ord");
	do_vacuum_table("ric_art");
	do_vacuum_table("ric_note");
	do_vacuum_table("catalogo");
	do_vacuum_table("ubicazioni");

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Terminato aggiornamento indici database");
#endif
}

ep_bool_t do_vacuum_table(char *szTable)
{
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;

	DBRes=DBExecQuery(FALSE,"vacuum analyze %s;",szTable);
	if (DBresultStatus(DBRes) == DBRES_COMMAND_OK ) {
	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);

	return bOK;
}
