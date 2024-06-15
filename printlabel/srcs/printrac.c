/*
* printrac.c : Stampa Moduli di raccolta
* 
* Progetto Mondadori Picking
*
* Copyright A&L srl 2003-2021
*
* 02-01-2002 rm+st : ATTENZIONE : i prezzi in euro sono interi (in centesimi)
*                    usare la funzione NotazioneConVirgola() per la normalizzazione
* 05-02-2002 rm    : riscrittura (da CodeBase a PostgreSQL)
* 09-10-2003 rm    : Stampa RAC linea con discriminante per CEDOLA
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <proc_list.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"
#include	"linklist.h"
#include	"stampa.h"


/*
* ReadPrintFieldsInfo()
*
* Printer Fields structure inizialization
*/
int ReadPrintFieldsInfo(char *szCfgFile)
{
	char szBuffer[256];
	char szParagraph[80];
	char szField[80];
	char *pPtr;
	char *szSeparator=" ,\t\n";
	int nIndex;

	strcpy(szParagraph,"Etichetta");
	nIndex=0;
	sprintf(szField,"Field_%02d",nIndex);
	while(GetFileString(szParagraph,szField,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL)){
		if(strlen(szBuffer)){
			/* ID Campo */
			if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
				strcpy(LabelFields[nIndex].szFieldID,pPtr);
			} else {return(-1);}
			/* Tipo Campo */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				strcpy(LabelFields[nIndex].szFieldType,pPtr);
			} else {return(-1);}
			/* Tabella Campo */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				strcpy(LabelFields[nIndex].szTableName,pPtr);
			} else {return(-1);}
			/* Nome Campo in Tabella */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				strcpy(LabelFields[nIndex].szFieldName,pPtr);
			} else {return(-1);}
			/* descrizione Campo */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				strcpy(LabelFields[nIndex].szFieldDescr,pPtr);
			} else {return(-1);}
		}
		nIndex++;
		sprintf(szField,"Field_%02d",nIndex);
	}
	LabelFields[nIndex].szFieldName[0]='\0';

	return nIndex;
}

/*
* Funzione di stampa della XAB per linea di spedizione
* Parametri di ingresso:
* pszPrinterName : nome della stampante
* szNMCED        : Numero Cedola
* szTPSPE        : Tipo Spedizione
* szCDLIN        : Codice Linea di Spedizione
* Ritorno:
*/
ep_bool_t StampaRACLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char * szCDVET,int nGID)
{
	DBresult *DBRes;
	char szPrintFile[128];
	char szOrdProg[128];
	char szMsg[128];
	int nTuples;
	int nIndex;
	ep_bool_t bOK=TRUE;
	FILE *fp;


#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa RAC Linea [%s - %s - %s - %s]",szNMCED,szTPSPE,szCDLIN,szCDVET);
#endif
	sprintf(szPrintFile,"%s/%s.rac_linea",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szPrintFile,"w"))!=(FILE *)NULL) {

		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s'  and rocdve2='%s' order by ordprog;",Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET);
		if((nTuples=DBntuples(DBRes))){

			for(nIndex=0;nIndex<nTuples;nIndex++){
				strcpy(szOrdProg,DBgetvalue(DBRes,nIndex,0));
				if(StampaEtichetteOrdine(fp,szOrdProg,pszPrinterName,nGID)){
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Stampa RAC OK [%s]",szOrdProg);
#endif
					SendMessage(Cfg.nMainID, nPID, PRINTLABEL_RAC_STAMPATO,szOrdProg);
				} else {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Stampa RAC KO [%s]",szOrdProg);
#endif
					sprintf(szMsg,"Errore in stampa RAC ordine [%s]\n",szOrdProg);
					SendMessage(Cfg.nMainID, nPID, DISPLAY_MSG, szMsg);
				}
			}
		}
		DBclear(DBRes);
	
		fclose(fp);

		PrintFileRaw(szPrintFile,pszPrinterName);
	}

	return bOK;
}

/*
* Funzione di stampa della RAC totale 
* Parametri di ingresso:
* pszPrinterName : nome della stampante
* Ritorno:
*/
ep_bool_t StampaAllRAC(char *pszPrinterName,int nGID)
{
	DBresult *DBRes;
	char szPrintFile[128];
	char szOrdProg[128];
	char szMsg[128];
	int nTuples;
	int nIndex;
	ep_bool_t bOK=TRUE;
	FILE *fp;


#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa ALL RAC");
#endif
	sprintf(szPrintFile,"%s/%s.all_rac",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szPrintFile,"w"))!=(FILE *)NULL) {
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' order by roprgln;",
			ORDINE_ELABORATO,Cfg.szTipoOrdini);
		if((nTuples=DBntuples(DBRes))){

			for(nIndex=0;nIndex<nTuples;nIndex++){
				strcpy(szOrdProg,DBgetvalue(DBRes,nIndex,0));
				if(StampaEtichetteOrdine(fp,szOrdProg,pszPrinterName,nGID)){
					SendMessage(Cfg.nMainID, nPID, PRINTLABEL_RAC_STAMPATO,szOrdProg);
				} else {
					sprintf(szMsg,"Errore in stampa RAC ordine [%s]\n",szOrdProg);
					SendMessage(Cfg.nMainID, nPID, DISPLAY_MSG, szMsg);
				}
			}
		}
		DBclear(DBRes);

		fclose(fp);

		PrintFileRaw(szPrintFile,pszPrinterName);
	}

	return bOK;
}

/*
* StampaEtichetteOrdineStorico(FILE *fp,char *pszOrdProg,char *pszPrinterName,ep_bool_t bSendMsg,int nGID)
* Descrizione
*  Funzione che stampa le etichette di tutti i colli dell'ordine specificato
*	Parametri di ingresso
*  -pszOrdProg : progressivo ordine
*  -pszPrinterName: nome della stampante
*/
ep_bool_t StampaEtichetteOrdineStorico(FILE *fp,char *pszOrdProg,char *pszPrinterName,int nGID)
{
	char szFileName[128];
	FILE *fpEtichetta;
	char szSQLCmd[1024];
	ep_bool_t bRetVal=FALSE;
	ep_bool_t bRC;

	if(fp==(FILE *)NULL){
		sprintf(szFileName,"%s/O%s",Cfg.szPathStampe,pszOrdProg);
		if ((fpEtichetta=fopen(szFileName,"w"))==(FILE *)NULL) {
			return FALSE;
		}
	} else {
		fpEtichetta=fp;
	}

#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaEtichetteOrdineStorico(%s,%s)",pszOrdProg,pszPrinterName);
#endif

	ReadPrintFieldsInfo(Cfg.szPrintCfg);
	/*
	* Apertura del file per la stampa
	*/
	sprintf(szSQLCmd,"select * from ric_ord_stor o,col_prod_stor c where o.ordprog=c.ordprog and c.ordprog='%s' order by cpnmcol;",pszOrdProg);
	if((bRC=StampaEtichetteStorico(szSQLCmd,fpEtichetta,pszPrinterName))){
		bRetVal=TRUE;
	}
	if(fp==(FILE *)NULL){
		fclose(fpEtichetta);

		PrintFileRaw(szFileName,pszPrinterName);
	}

	return bRetVal;
}

/*
* StampaEtichettaCollo(char *pszOrdProg,int nCollo,char *pszPrinterName)
* Descrizione
*  Funzione che stampa l'etichetta di un collo dell'ordine specificato
*	Parametri di ingresso
*  -pszOrdProg : Progressivo Ordine
*  -nCollo     : Numero Collo
*  -pszPrinterName: nome della stampante
*/
ep_bool_t StampaEtichettaColloStorico(char *pszOrdProg,int nCollo,char *pszPrinterName,int nGID)
{
	char szFileName[128];
	FILE *fpEtichetta;
	char szSQLCmd[1024];
	ep_bool_t bRetVal=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaEtichettaColloStorico(%s,%d,%s)",pszOrdProg,nCollo,pszPrinterName);
#endif

	ReadPrintFieldsInfo(Cfg.szPrintCfg);

	/*
	* Apertura del file per la stampa
	*/
	sprintf(szFileName,"%s/C%s%03d",Cfg.szPathStampe,pszOrdProg,nCollo);
	if ((fpEtichetta=fopen(szFileName,"w"))!=(FILE *)NULL) {

		sprintf(szSQLCmd,"select * from ric_ord_stor o,col_prod_stor c where o.ordprog=c.ordprog and c.ordprog='%s' and c.cpnmcol=%d order by cpnmcol;",pszOrdProg,nCollo);
		StampaEtichetteStorico(szSQLCmd,fpEtichetta,pszPrinterName);

		fclose(fpEtichetta);

		PrintFileRaw(szFileName,pszPrinterName);
	} else {
		bRetVal=FALSE;
		SendMessage(nGID, nPID, PRINTLABEL_RAC_ERROR, pszOrdProg);
	}

	return bRetVal;
}



/*
* StampaEtichetteOrdine(FILE *fp,char *pszOrdProg,char *pszPrinterName,ep_bool_t bSendMsg,int nGID)
* Descrizione
*  Funzione che stampa le etichette di tutti i colli dell'ordine specificato
*	Parametri di ingresso
*  -pszOrdProg : progressivo ordine
*  -pszPrinterName: nome della stampante
*/
ep_bool_t StampaEtichetteOrdine(FILE *fp,char *pszOrdProg,char *pszPrinterName,int nGID)
{
	char szFileName[128];
	FILE *fpEtichetta;
	char szSQLCmd[1024];
	ep_bool_t bRetVal=FALSE;
	ep_bool_t bRC;

	if(fp==(FILE *)NULL){
		sprintf(szFileName,"%s/O%s",Cfg.szPathStampe,pszOrdProg);
		if ((fpEtichetta=fopen(szFileName,"w"))==(FILE *)NULL) {
			return FALSE;
		}
	} else {
		fpEtichetta=fp;
	}

#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaEtichetteOrdine(%s,%s)",pszOrdProg,pszPrinterName);
#endif

	ReadPrintFieldsInfo(Cfg.szPrintCfg);
	/*
	* Apertura del file per la stampa
	*/
	sprintf(szSQLCmd,"select * from ric_ord o,col_prod c where o.ordprog=c.ordprog and c.ordprog='%s' order by cpnmcol;",pszOrdProg);
	if((bRC=StampaEtichette(szSQLCmd,fpEtichetta,pszPrinterName))){
		bRetVal=TRUE;
	}
	if(fp==(FILE *)NULL){
		fclose(fpEtichetta);

		PrintFileRaw(szFileName,pszPrinterName);
	}

	return bRetVal;
}

/*
* StampaEtichettaCollo(char *pszOrdProg,int nCollo,char *pszPrinterName)
* Descrizione
*  Funzione che stampa l'etichetta di un collo dell'ordine specificato
*	Parametri di ingresso
*  -pszOrdProg : Progressivo Ordine
*  -nCollo     : Numero Collo
*  -pszPrinterName: nome della stampante
*/
ep_bool_t StampaEtichettaCollo(char *pszOrdProg,int nCollo,char *pszPrinterName,int nGID)
{
	char szFileName[128];
	FILE *fpEtichetta;
	char szSQLCmd[1024];
	ep_bool_t bRetVal=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaEtichettaCollo(%s,%d,%s)",pszOrdProg,nCollo,pszPrinterName);
#endif

	ReadPrintFieldsInfo(Cfg.szPrintCfg);

	/*
	* Apertura del file per la stampa
	*/
	sprintf(szFileName,"%s/C%s%03d",Cfg.szPathStampe,pszOrdProg,nCollo);
	if ((fpEtichetta=fopen(szFileName,"w"))!=(FILE *)NULL) {

		sprintf(szSQLCmd,"select * from ric_ord o,col_prod c where o.ordprog=c.ordprog and c.ordprog='%s' and c.cpnmcol=%d order by cpnmcol;",pszOrdProg,nCollo);
		StampaEtichette(szSQLCmd,fpEtichetta,pszPrinterName);

		fclose(fpEtichetta);

		PrintFileRaw(szFileName,pszPrinterName);
	} else {
		bRetVal=FALSE;
		SendMessage(nGID, nPID, PRINTLABEL_RAC_ERROR, pszOrdProg);
	}

	return bRetVal;
}

#define MAX_RIGHE_ETICHETTA 18

/*
* Descrizione
* Stampa etichetta
* Parametri di ingresso
*  fp : file sul quale si produce la stampa
*/
ep_bool_t StampaEtichette(char *szSQLCmd,FILE *fp,char *szLabelPrinterName)
{
	FILE *fpBase;
	char szBuffer[1024];
	char szDestination[1024];
	char *pPtr;
	int nIndex;
	int nColloIndex;
	int nFieldIndex;
	ep_bool_t bFound;
	ep_bool_t bSaltaRiga=FALSE;
	char szLabelBaseFileName[256];
	char szOrdProg[256];
	char szDSTSP[256];
	char szTestoDDT[256];
	char szDSLDV[256];
	int nCPNMCOL;
	int nCPCDLDV;
	int nORDPROG;
	int nROVLCOE;
	int nRONMCED;
	int nROCDVE2;
	int nROCDRID;
	int nROCOCLI;
	int nRODTBAM;
	int nRONMBAM;
	int nRODTSDA;
	int nROFLAMZ;
	int nROCDLIN;
	int nROSGESS;
	int nROCCESS;
	int nRONOESS;
	int nCollo;
	int nRigaIndex=0;
	int nPagine=0;
	int nRighe;
	int nRigheCollo;
	int nCedola;
	char szCdRid[128];
	char szCdVe2[128];
	char szCoCli[128];
	char szPresso[128];
	char szPONum[128];
	char szDTBAM[128];
	char szNMBAM[128];
	char szNMCOL[128];
	char szDTSDA[128];
	char szDataCons[128];
	char szLDV[128];
	char szCGSLDV[128];
	char szROCDLIN[128];
	char szROSGESS[128];
	char szROCCESS[128];
	char szRONOESS[128];
	int nLayoutEtichetta=-1;
	DBresult *DBRes;
	DBresult *DBResEti;
	DBresult *DBResLDV;
	DBresult *DBResRighe;
	DBresult *DBResContrassegno;
	ep_bool_t bFirstUBIC=TRUE;
	ep_bool_t bFirstCDPRO=TRUE;
	ep_bool_t bFirstTITLE=TRUE;
	ep_bool_t bFirstNMCPE=TRUE;
	ep_bool_t bFirstPREZZO=TRUE;
	int nValoreContrassegno=0;
	int nAmazon=0;
	int nGiornoSDA;
	int nMeseSDA;
	int nAnnoSDA;
	int nMeseCorrente;



#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaEtichette(%s)",szSQLCmd);
#endif

	DBRes=DBExecQuery(Cfg.nDebugLevel,szSQLCmd);
	/*
	* Correzione volante causa problemi di compatibilita' tra versioni di postgresql devel libs
	* rm +lg 16-04-2013
	*
	*/
	/*
	if(DBntuples(DBRes)){

		if((nCPNMCOL=DBfnumber(DBRes,"cpnmcol"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
		if((nORDPROG=DBfnumber(DBRes,"ordprog"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
		if((nRONMCED=DBfnumber(DBRes,"ronmced"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
		if((nROCDRID=DBfnumber(DBRes,"rocdrid"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
		if((nROCOCLI=DBfnumber(DBRes,"rococli"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
	}

	DBclear(DBRes);
	*/

	nORDPROG=0;	  /* pqfnumber() */
	nROCDRID=1;	  /* pqfnumber() */
	nROVLCOE=40;	/* pqfnumber() */
	nRONMCED=41;	/* pqfnumber() */
	nROCDVE2=43;	/* pqfnumber() */
	nROCOCLI=6;	  /* pqfnumber() */
	nRODTBAM=23;	/* pqfnumber() */
	nRONMBAM=22;	/* pqfnumber() */
	nRODTSDA=75;	/* pqfnumber() */
	nROFLAMZ=91;	/* pqfnumber() */
	nROCDLIN=28;	/* pqfnumber() */
	nROSGESS=95;	/* pqfnumber() */
	nROCCESS=96;	/* pqfnumber() */
	nRONOESS=97;	/* pqfnumber() */
	nCPNMCOL=99;	/* pqfnumber() */
	nCPCDLDV=116;	/* pqfnumber() */










































/*
#  nota : si fa riferimento alla seguente lista campi
# OGNI VOLTA CHE SI AGGIUNGONO CAMPI ALLE TABELLE DI PRODUZIONE
# QUESTA LISTA CAMBIA !!! RICORDARSI DI VARIARLA IN ACCORDO ALLA SITUAZIONE REALE
# DEL DATABASE !!!
# --------------------------------------------------
#  0   ordprog     | 000871392
#  1   rocdrid     | RZ5104499 
#  2   rocdspc     | 
#  3   rocdare     | 
#  4   rocdven     | 
#  5   rodscli     | AMAZON EU S.A.R.L MXP5  PO 26VBPSYP
#  6   rococli     | 
#  7   roincli     | STRADA DOGANA PO, 2 T         *    
#  8   rolocli     | CASTEL SAN GIOVANNI           
#  9   roprcli     | PC 
# 10   rocpcli     | 29015
# 11   ronzcli     | 
# 12   rocdcla     | 
# 13   rocdspa     | 
# 14   rodscla     | 
# 15   roincla     | 
# 16   rococla     | 
# 17   rolocla     | 
# 18   roprcla     | 
# 19   rocpcla     | 
# 20   ronzcla     | 
# 21   roswspf     | 
# 22   ronmbam     | 
# 23   rodtbam     | 
# 24   rotptar     | 
# 25   roswcol     | 0
# 26   rovlcon     | 0
# 27   rotpspe     | AM
# 28   rocdlin     | MIL
# 29   rocdsca     | 
# 30   rocdssc     | 
# 31   rocdsot     | 
# 32   rocdmov     | 
# 33   rotpdoc     | 
# 34   rodteva     | 
# 35   roswfin     | 
# 36   rocdfpg     | 
# 37   rotpbut     | 
# 38   roorins     | 
# 39   rocdval     | 
# 40   rovlcoe     | 0
# 41   ronmced     | 25
# 42   rocdve1     |   
# 43   rocdve2     |   
# 44   rocdve3     |   
# 45   roidvet     | 
# 46   ronmrgh     | 1
# 47   ronmrgp     | 6
# 48   ronmcpe     | 6
# 49   ronmcpp     | 6
# 50   ronmcll     | 6
# 51   ronmcla     | 0
# 52   rocdbtc     | 
# 53   ropspre     | 7638
# 54   ropsrea     | 0
# 55   rotmrcz     | 2018-06-28 17:36:54.559742
# 56   rotmini     | 
# 57   rotmeva     | 
# 58   roaaxab     | 0
# 59   roprxab     | 0
# 60   rodtxab     | 
# 61   ronmdis     | 0
# 62   rodtdis     | 
# 63   roprgln     | 96214
# 64   rocdflg     |  
# 65   rostato     | A
# 66   ordtipo     | N
# 67   ubitipo     | N
# 68   roflimc     |  
# 69   rocdveo     | 
# 70   rofliem     | 
# 71   roprgem     | 0
# 72   rodsnot     | TASSATIVA IL 30/12/2017 - PO 92VBPSYP                       
# 73   rodstsp     | 
# 74   roflnot     | 
# 75   rodtsda     | 
# 76   roflied     | 
# 77   rofliex     | 
# 78   roflifl     | 
# 79   rodslin     | DMA TRASPORTI                 
# 80   roflpar     | MI
# 81   roflarr     | PC
# 82   rodsarr     |                          
# 83   rodtbcd     | AME[SSSSSSSSS]                
# 84   rotpbcd     | 0001
# 85   rosgbnc     | 0
# 86   insert_user | roberto
# 87   insert_time | 2018-06-28 17:36:40.800004
# 88   update_user | roberto
# 89   update_time | 2018-06-28 17:44:58.956433
# 90   roprbol     | 8654199  
# 91   roflamz     | 1
# 92   rocdamz     | 190519827           
# 93   rocdpdl     |    
# 94   rodspdl     |                     
# 95   rosgess     | - NUOVI CAMPI ESSELUNGA
# 96   roccess     | - NUOVI CAMPI ESSELUNGA
# 97   ronoess     | - NUOVI CAMPI ESSELUNGA
# 98   ordprog     | 000871392
# 99   cpnmcol     | 1
# 100  cpnmppt     | 0
# 101  cppspre     | 1273
# 102  cppsrea     | 0
# 103  cpvlaff     | 0
# 104  cptpfor     | F
# 105  cpswfps     | 
# 106  cpswchm     | 
# 107  cpvlafe     | 0
# 108  cpswlin     | 1
# 109  cpnmrgh     | 1
# 110  cpnmcpe     | 1
# 111  cpbrcde     | 01087139200012
# 112  cptmeva     | 
# 113  cpcdflg     |  
# 114  cpcntsc     | 0
# 115  cpstato     | A
# 116  cpcdldv     | 
# 117  cpldvmd     | 
# 118  cpprgcl     | 10767353
# 119  insert_user | roberto
# 120  insert_time | 2018-06-28 17:44:58.962626
# 121  update_user | 
# 122  update_time | 
# --------------------------------------------------
*/















	/*
	* Rilancio la select dei colli da stampare
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel,szSQLCmd);
	if(DBntuples(DBRes)){
		for(nColloIndex=0;nColloIndex<DBntuples(DBRes);nColloIndex++){
			strcpy(szOrdProg,DBgetvalue(DBRes,nColloIndex,nORDPROG));
			nCollo=atoi(DBgetvalue(DBRes,nColloIndex,nCPNMCOL));
			strcpy(szLDV,DBgetvalue(DBRes,nColloIndex,nCPCDLDV));
			GetDSTSP(szOrdProg,szDSTSP,FALSE);
			GetTestoDDT(szOrdProg,nCollo,szTestoDDT);

			/* 
			* rm 23-03-2018 : progressivo FF multicollo 
			* se si tratta di un FF multi-collo
			* devo calcolare il numero collo del set di colli associati ad un unico prodotto
			* 
			*/

			/* amazon */
			nAmazon=atoi(DBgetvalue(DBRes,nColloIndex,nROFLAMZ));

			nCedola=atoi(DBgetvalue(DBRes,nColloIndex,nRONMCED));
			nValoreContrassegno=atoi(DBgetvalue(DBRes,nColloIndex,nROVLCOE));
			strcpy(szCdRid,DBgetvalue(DBRes,nColloIndex,nROCDRID));
			strcpy(szCoCli,DBgetvalue(DBRes,nColloIndex,nROCOCLI));
			/* rm 19-09-2014 : PONUM diventa : data bam (ggmmaa) - spedizione (ssssss) - num collo (cccccc) */
			// strcpy(szPONum,LeftStr(szCoCli,8));

			strcpy(szDTBAM,DBgetvalue(DBRes,nColloIndex,nRODTBAM));
			strcpy(szNMBAM,DBgetvalue(DBRes,nColloIndex,nRONMBAM));
			strcpy(szDTSDA,DBgetvalue(DBRes,nColloIndex,nRODTSDA));
			nGiornoSDA=atoi(SubStr(szDTSDA,1,2));
			nMeseSDA=atoi(SubStr(szDTSDA,3,2));
			nAnnoSDA=atoi(GetYear(szBuffer));

			nMeseCorrente=atoi(GetMonth(szBuffer));

			if(nMeseSDA<nMeseCorrente){
				/* si tratta - presumibilmente ... - di una data del prossimo anno */
				nAnnoSDA++;
			}


			if(nGiornoSDA==0){
				/* caso di data consegna non definita */
				sprintf(szDataCons,"IMMEDIATA");
				strcpy(szDTSDA,"");
				strcpy(szCGSLDV,szLDV);
			} else {
				sprintf(szDataCons,"%2d/%2d/%4d",nGiornoSDA,nMeseSDA,nAnnoSDA);
				strcpy(szCGSLDV,"CGS");
				strcat(szCGSLDV,szLDV);
			}

			/* nuova versione ponum per amazon */
			strcpy(szPONum,szDTBAM+6); szPONum[2]='\0'; /* giorno gg */
			strcat(szPONum,szDTBAM+4); szPONum[4]='\0'; /* mese mm */
			strcat(szPONum,szDTBAM+2); szPONum[6]='\0'; /* anno aa */
			strcat(szPONum,szNMBAM);
			sprintf(szNMCOL,"%06d",nCollo);
			strcat(szPONum,szNMCOL);

			/* versione precedente - se commentata vale la nuova */
			//strcpy(szPONum,LeftStr(szCoCli,8));

			

			strcpy(szPresso,RightStr(szCoCli,9));
			strcpy(szCdVe2,DBgetvalue(DBRes,nColloIndex,nROCDVE2));

			/* 14-03-2011 rm : ricavo il layout dell'etichetta da stampare */
			DBResEti=DBExecQuery(Cfg.nDebugLevel,"select cdlyeti from cedole where cdnmced=%d;",nCedola);
			if(DBntuples(DBResEti)){
				nLayoutEtichetta=atoi(DBgetvalue(DBResEti,0,0));
			}
			DBclear(DBResEti);


			if(nLayoutEtichetta < 1 || nLayoutEtichetta > 3 ){
				/* 'normalizzo' il layout etichetta, se indice fuori dai limiti 1-3 allora -> 1 : normale */
				nLayoutEtichetta=1;
			}

			
			/* 31-05-2012 rm : se cliente AMAZON allora utilizzo layout numero 10 */
			if(!strcmp(szCdRid,"5104499")){
				nLayoutEtichetta=10;
			}

			/*
			* rm + lg 23-06-2014
			* gestione etichetta SDA
			* se CDVE2=503100 o CDVE2=503101 allora uso layout numero 11
			*/
			/* disabilitato */
			if(Cfg.bAbilitaSDA && (!strcmp(szCdVe2,"503100") || !strcmp(szCdVe2,"503101")))
			/* abilitato */
			//if((!strcmp(szCdVe2,"503100") || !strcmp(szCdVe2,"503101")))
			{
				nLayoutEtichetta=11;
			}


			/* 26-06-2018 rm : se linea poste italiane (PDL) uso layout numero 12 */
			strcpy(szROCDLIN,DBgetvalue(DBRes,nColloIndex,nROCDLIN));
			if(!strcmp(szROCDLIN,"PDL")){
				nLayoutEtichetta=12;
			}



			/* primo file - etichetta vera e propria */
			sprintf(szLabelBaseFileName,"%s/%s.%02d.1",Cfg.szPathData,Cfg.szLabelBaseName,nLayoutEtichetta);
			if((fpBase=fopen(szLabelBaseFileName,"r"))==(FILE *)NULL){
				/* non trovato... provo con lo standard */
				sprintf(szLabelBaseFileName,"%s/%s.1",Cfg.szPathData,Cfg.szLabelBaseName);
			} else {
				fclose(fpBase);
			}

			if((fpBase=fopen(szLabelBaseFileName,"r"))!=(FILE *)NULL){
				while(fgets(szBuffer,sizeof(szBuffer)-1,fpBase)){
					/*
					* prevedo piu' campi su una riga
					*/
					bFound=TRUE;
					while(bFound){
						/*
						* cerco l'ID campo all'interno della riga
						*/
						nIndex=0;
						bFound=FALSE;
						while(!bFound && strlen(LabelFields[nIndex].szFieldID)){
							if((pPtr=strstr(szBuffer,LabelFields[nIndex].szFieldID))!=NULL){
								bFound=TRUE;
								continue;
							}
							nIndex++;
						}
						strcpy(szDestination,szBuffer);
						if(bFound){
							szDestination[pPtr-szBuffer]='\0';

							if(!strcmp((LabelFields[nIndex].szFieldID),"<ORDPROG>")){
								strcat(szDestination,SubStr(szOrdProg,11,6));
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<RODSNOT>")){
								/* note spedizione */
								strcat(szDestination,szDSTSP);
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<TESTODDT>")){
								/* Testo DDT */
								strcat(szDestination,szTestoDDT);
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDDATE>")){
								/* Data */
								strcat(szDestination, GetDate(szDateBuffer)); 
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTIME>")){
								/* Ora */
								strcat(szDestination, GetTime(szTimeBuffer)); 
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PRESSO>")){
								/* Presso per Amazon */
								strcat(szDestination, szPresso); 
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PONUM>")){
								/* PO Num per Amazon */
								strcat(szDestination, szPONum); 
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<DATACONS>")){
								/* data consegna (da rodtsda) */
								strcat(szDestination, szDataCons); 
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<DTSDA>")){
								/* DTSDA */
								/* modifica per evitare che venga stampato un barcode anche in caso di DTSDA vuoto */
								if(nGiornoSDA!=0){
									strcat(szDestination, "^FD>:"); 
									strcat(szDestination, szDTSDA); 
									strcat(szDestination, "^FS"); 
								}
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<CGSLDV>")){
								/* Consegna Giorno Stabilito o Immediata */
								strcat(szDestination, szCGSLDV); 
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<CONTRASS>")){
								/* CONTRASSEGNO*/
								if(nValoreContrassegno){
									strcat(szDestination, "CONTRASSEGNO"); 
								} else {
									strcat(szDestination, " "); 
								}
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTPBCVET>")){
								/* 
								* Tipo Barcode Vettore 
								*/
								/*
								* 30-07-2008 rm : se vettore barcode vuoto non scrivo la riga
								*/
								if(0==strlen(StrTrimAll(VettoreBarcode(DBRes,nColloIndex)))){
									bSaltaRiga=TRUE;
								}
								strcat(szDestination,StrTrimAll(TipoVettoreBarcode(DBRes,nColloIndex)));
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDBCSC>")){
								/* 
								* Barcode Segnacollo per amazon 
								*/
								if(0==strlen(StrTrimAll(BarcodeSegnacollo(DBRes,nColloIndex)))){
									bSaltaRiga=TRUE;
								}
								if(nAmazon==0){
									bSaltaRiga=TRUE;
								}

								// funzione da attivare per gestione segnacollo amazon NUOVA (luglio 2018)
								// strcat(szDestination,StrTrimAll(BarcodeSegnacollo(DBRes,nColloIndex)));

								// strcat(szDestination,StrTrimAll(BarcodeSegnacollo_OLD(DBRes,nColloIndex)));
								strcat(szDestination,StrTrimAll(BarcodeSegnacollo(DBRes,nColloIndex)));
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDBCVET>")){
								/* 
								* Barcode Vettore 
								*/
								/*
								* 30-07-2008 rm : se vettore barcode vuoto non scrivo la riga
								*/
								if(0==strlen(StrTrimAll(VettoreBarcode(DBRes,nColloIndex)))){
									bSaltaRiga=TRUE;
								}
								strcat(szDestination,StrTrimAll(VettoreBarcode(DBRes,nColloIndex)));
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<DSLDV>")){
								/* rm+lg 23-06-2014 : Codice a barre 2D DataMatrix per SDA */
#ifdef DOCUMENTAZIONE_LDV
								select
									cpcdldv,											/* LDV */
									ordprog,											/* ordine */
									cpnmcol												/* collo */,
									rodscli												/* rag soc */,
									rodscla,											/* ref dest */ 
									roincla,											/* indirizzo  */ 
									rocpcla,											/* CAP */ 
									rolocla,											/* Loc. */ 
									roprcla,											/* Prov. */ 
									ronzcl,												/* Nazione */
									ronmcll,											/* Num Colli */
									'A.Mondadori Editore',				/* Rag Soc Mitt */,
									'Boxline',										/* Ref Mitt */
									'Via Montelungo,1',						/* Ind Mitt */,
									'37131',											/* CAP Mitt */,
									'Verona',											/* Loc Mitt */,
									'VR',													/* PRV Mitt */,
									'ITA', 												/* Naz Mitt */ 
									'EXT',												/* codice servizio */
									'',														/* Codice tipo pagamento */
									'',														/* importo contrassegno */
									'',														/* importo assicurata */
									'',														/* valore dichiarato */
									'',														/* dest. cellulare */
									'',														/* dest. email */
									'',														/* barcode servizio */
									''														/* peso colli */
#endif
								DBResLDV=DBExecQuery(Cfg.nDebugLevel," select \
									substr(c.cpcdldv,1,22) || '|' || \
									substr(r.ordprog,1,25) || '|' || \
									c.cpbrcde || '|' || \
									substr(r.rodscli,1,40) || '|' || \
									substr(r.rodscla,1,20) || '|' || \
									substr(r.roincla,1,45) || '|' || \
									substr(r.rocpcla,1,9) || '|' || \
									substr(r.rolocla,1,30) || '|' || \
									substr(r.roprcla,1,2) || '|' || \
									substr(r.ronzcla,1,3) || '|' || \
									r.ronmcll || '|' || \
									'A.Mondadori Editore' || '|' || \
									'Boxline' || '|' || \
									'Via Montelungo,1' || '|' || \
									'37131' || '|' || \
									'Verona' || '|' || \
									'VR' || '|' || \
									'ITA' || '|' || \
									'EXT' || '|' || \
									'' || '|' || \
									'' || '|' || \
									'' || '|' || \
									'' || '|' || \
									'' || '|' || \
									'' || '|' || \
									'T12' || '|' || \
									cppspre/1000::int from ric_ord r,col_prod c where c.ordprog=r.ordprog and c.ordprog='%s' and c.cpnmcol=%d;", szOrdProg,nCollo);

								if(DBntuples(DBResLDV)){
									strcpy(szDSLDV,DBgetvalue(DBResLDV,0,0));
								} else {
									strcpy(szDSLDV,"");
								}

								DBclear(DBResLDV);
									
								strcat(szDestination, szDSLDV); 
							} else {
								/* normale campo db - stampo com'e' */
								/*
								* problema con pqfnumber !!!
								* rm+lg 16-04-2013
								*/
								/*
								if((nFieldIndex=DBfnumber(DBRes,LabelFields[nIndex].szFieldName))==-1)
								*/
								if((nFieldIndex=atoi(LabelFields[nIndex].szFieldName))==-1)
								{
									strcat(szDestination,"");
								} else {
									strcat(szDestination,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nFieldIndex)));
								}
							}

							strcat(szDestination,pPtr+strlen(LabelFields[nIndex].szFieldID));
							/*
							* riporto le modifiche 
							*/
							strcpy(szBuffer,szDestination);
						}
					}
					if(!bSaltaRiga){
						fputs(szDestination,fp);
					}
					bSaltaRiga=FALSE;
				}
				fclose(fpBase);
			}

			/* stampa eventuale etichetta esselunga */

			strcpy(szROSGESS,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nROSGESS)));
			strcpy(szROCCESS,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nROCCESS)));
			strcpy(szRONOESS,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nRONOESS)));
			if(strlen(szROSGESS)){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Stampa ETICHETTA ESSELUNGA [%s - %s - %s - %s]",szOrdProg,szRONOESS,szROSGESS,szROCCESS);
#endif

				/* primo file - etichetta vera e propria */
				sprintf(szLabelBaseFileName,"%s/%s.esselunga",Cfg.szPathData,Cfg.szLabelBaseName);
				if((fpBase=fopen(szLabelBaseFileName,"r"))!=(FILE *)NULL){
					while(fgets(szBuffer,sizeof(szBuffer)-1,fpBase)){
						/*
						* prevedo piu' campi su una riga
						*/
						bFound=TRUE;
						while(bFound){
							/*
							* cerco l'ID campo all'interno della riga
							*/
							nIndex=0;
							bFound=FALSE;
							while(!bFound && strlen(LabelFields[nIndex].szFieldID)){
								if((pPtr=strstr(szBuffer,LabelFields[nIndex].szFieldID))!=NULL){
									bFound=TRUE;
									continue;
								}
								nIndex++;
							}
							strcpy(szDestination,szBuffer);
							if(bFound){
								szDestination[pPtr-szBuffer]='\0';

								if(!strcmp((LabelFields[nIndex].szFieldID),"<ORDPROG>")){
									strcat(szDestination,SubStr(szOrdProg,11,6));
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<RODSNOT>")){
									/* note spedizione */
									strcat(szDestination,szDSTSP);
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<TESTODDT>")){
									/* Testo DDT */
									strcat(szDestination,szTestoDDT);
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDDATE>")){
									/* Data */
									strcat(szDestination, GetDate(szDateBuffer)); 
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTIME>")){
									/* Ora */
									strcat(szDestination, GetTime(szTimeBuffer)); 
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PRESSO>")){
									/* Presso per Amazon */
									strcat(szDestination, szPresso); 
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PONUM>")){
									/* PO Num per Amazon */
									strcat(szDestination, szPONum); 
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<DATACONS>")){
									/* data consegna (da rodtsda) */
									strcat(szDestination, szDataCons); 
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<DTSDA>")){
									/* DTSDA */
									/* modifica per evitare che venga stampato un barcode anche in caso di DTSDA vuoto */
									if(nGiornoSDA!=0){
										strcat(szDestination, "^FD>:"); 
										strcat(szDestination, szDTSDA); 
										strcat(szDestination, "^FS"); 
									}
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<CGSLDV>")){
									/* Consegna Giorno Stabilito o Immediata */
									strcat(szDestination, szCGSLDV); 
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<CONTRASS>")){
									/* CONTRASSEGNO*/
									if(nValoreContrassegno){
										strcat(szDestination, "CONTRASSEGNO"); 
									} else {
										strcat(szDestination, " "); 
									}
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTPBCVET>")){
									/* 
									* Tipo Barcode Vettore 
									*/
									/*
									* 30-07-2008 rm : se vettore barcode vuoto non scrivo la riga
									*/
									if(0==strlen(StrTrimAll(VettoreBarcode(DBRes,nColloIndex)))){
										bSaltaRiga=TRUE;
									}
									strcat(szDestination,StrTrimAll(TipoVettoreBarcode(DBRes,nColloIndex)));
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDBCSC>")){
									/* 
									* Barcode Segnacollo per amazon 
									*/
									if(0==strlen(StrTrimAll(BarcodeSegnacollo(DBRes,nColloIndex)))){
										bSaltaRiga=TRUE;
									}
									if(nAmazon==0){
										bSaltaRiga=TRUE;
									}

									// funzione da attivare per gestione segnacollo amazon NUOVA (luglio 2018)
									// strcat(szDestination,StrTrimAll(BarcodeSegnacollo(DBRes,nColloIndex)));

									// strcat(szDestination,StrTrimAll(BarcodeSegnacollo_OLD(DBRes,nColloIndex)));
									strcat(szDestination,StrTrimAll(BarcodeSegnacollo(DBRes,nColloIndex)));
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDBCVET>")){
									/* 
									* Barcode Vettore 
									*/
									/*
									* 30-07-2008 rm : se vettore barcode vuoto non scrivo la riga
									*/
									if(0==strlen(StrTrimAll(VettoreBarcode(DBRes,nColloIndex)))){
										bSaltaRiga=TRUE;
									}
									strcat(szDestination,StrTrimAll(VettoreBarcode(DBRes,nColloIndex)));
								} else if(!strcmp((LabelFields[nIndex].szFieldID),"<DSLDV>")){
									/* rm+lg 23-06-2014 : Codice a barre 2D DataMatrix per SDA */
	#ifdef DOCUMENTAZIONE_LDV
									select
										cpcdldv,											/* LDV */
										ordprog,											/* ordine */
										cpnmcol												/* collo */,
										rodscli												/* rag soc */,
										rodscla,											/* ref dest */ 
										roincla,											/* indirizzo  */ 
										rocpcla,											/* CAP */ 
										rolocla,											/* Loc. */ 
										roprcla,											/* Prov. */ 
										ronzcl,												/* Nazione */
										ronmcll,											/* Num Colli */
										'A.Mondadori Editore',				/* Rag Soc Mitt */,
										'Boxline',										/* Ref Mitt */
										'Via Montelungo,1',						/* Ind Mitt */,
										'37131',											/* CAP Mitt */,
										'Verona',											/* Loc Mitt */,
										'VR',													/* PRV Mitt */,
										'ITA', 												/* Naz Mitt */ 
										'EXT',												/* codice servizio */
										'',														/* Codice tipo pagamento */
										'',														/* importo contrassegno */
										'',														/* importo assicurata */
										'',														/* valore dichiarato */
										'',														/* dest. cellulare */
										'',														/* dest. email */
										'',														/* barcode servizio */
										''														/* peso colli */
	#endif
									DBResLDV=DBExecQuery(Cfg.nDebugLevel," select \
										substr(c.cpcdldv,1,22) || '|' || \
										substr(r.ordprog,1,25) || '|' || \
										c.cpbrcde || '|' || \
										substr(r.rodscli,1,40) || '|' || \
										substr(r.rodscla,1,20) || '|' || \
										substr(r.roincla,1,45) || '|' || \
										substr(r.rocpcla,1,9) || '|' || \
										substr(r.rolocla,1,30) || '|' || \
										substr(r.roprcla,1,2) || '|' || \
										substr(r.ronzcla,1,3) || '|' || \
										r.ronmcll || '|' || \
										'A.Mondadori Editore' || '|' || \
										'Boxline' || '|' || \
										'Via Montelungo,1' || '|' || \
										'37131' || '|' || \
										'Verona' || '|' || \
										'VR' || '|' || \
										'ITA' || '|' || \
										'EXT' || '|' || \
										'' || '|' || \
										'' || '|' || \
										'' || '|' || \
										'' || '|' || \
										'' || '|' || \
										'' || '|' || \
										'T12' || '|' || \
										cppspre/1000::int from ric_ord r,col_prod c where c.ordprog=r.ordprog and c.ordprog='%s' and c.cpnmcol=%d;", szOrdProg,nCollo);

									if(DBntuples(DBResLDV)){
										strcpy(szDSLDV,DBgetvalue(DBResLDV,0,0));
									} else {
										strcpy(szDSLDV,"");
									}

									DBclear(DBResLDV);
										
									strcat(szDestination, szDSLDV); 
								} else {
									/* normale campo db - stampo com'e' */
									/*
									* problema con pqfnumber !!!
									* rm+lg 16-04-2013
									*/
									/*
									if((nFieldIndex=DBfnumber(DBRes,LabelFields[nIndex].szFieldName))==-1)
									*/
									if((nFieldIndex=atoi(LabelFields[nIndex].szFieldName))==-1)
									{
										strcat(szDestination,"");
									} else {
										strcat(szDestination,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nFieldIndex)));
									}
								}

								strcat(szDestination,pPtr+strlen(LabelFields[nIndex].szFieldID));
								/*
								* riporto le modifiche 
								*/
								strcpy(szBuffer,szDestination);
							}
						}
						if(!bSaltaRiga){
							fputs(szDestination,fp);
						}
						bSaltaRiga=FALSE;
					}
					fclose(fpBase);
				}
			}


			/* stampa packing list su collo */


			DBResRighe=DBExecQuery(Cfg.nDebugLevel,"select ubicazione_ridotta(rpcdubi),codprod_ridotto(rpcdpro),prdstit,to_char(rppzpre::float/100::float,'990D99'),lpad(rpqtord::text,5) from rig_prod r,catalogo c where r.rpcdpro=c.prcdpro and r.ordprog='%s' and r.rpnmcol=%d order by r.rpprrig;",szOrdProg,nCollo);
			nRigheCollo=DBntuples(DBResRighe);
			DBclear(DBResRighe);

			nRigaIndex=0;
			nPagine=0;
			while(nRigheCollo>nPagine*MAX_RIGHE_ETICHETTA){

				bFirstUBIC=TRUE;
				bFirstCDPRO=TRUE;
				bFirstTITLE=TRUE;
				bFirstNMCPE=TRUE;
				bFirstPREZZO=TRUE;

				/* secondo file - lista articoli */
				sprintf(szLabelBaseFileName,"%s/%s.%02d.2",Cfg.szPathData,Cfg.szLabelBaseName,nLayoutEtichetta);
				if((fpBase=fopen(szLabelBaseFileName,"r"))==(FILE *)NULL){
					/* non trovato... provo con lo standard */
					sprintf(szLabelBaseFileName,"%s/%s.2",Cfg.szPathData,Cfg.szLabelBaseName);
				} else {
					fclose(fpBase);
				}

				if((fpBase=fopen(szLabelBaseFileName,"r"))!=(FILE *)NULL){

					DBResRighe=DBExecQuery(Cfg.nDebugLevel,"select ubicazione_ridotta(rpcdubi),codprod_ridotto(rpcdpro),prdstit,to_char(rppzpre::float/100::float,'990D99'),lpad(rpqtord::text,5) from rig_prod r,catalogo c where r.rpcdpro=c.prcdpro and r.ordprog='%s' and r.rpnmcol=%d order by r.rpprrig;",szOrdProg,nCollo);

					if((nRighe=DBntuples(DBResRighe))){

						while(fgets(szBuffer,sizeof(szBuffer)-1,fpBase)){
							/*
							* prevedo piu' campi su una riga
							*/
							bFound=TRUE;
							while(bFound){
								/*
								* cerco l'ID campo all'interno della riga
								*/
								nIndex=0;
								bFound=FALSE;
								while(!bFound && strlen(LabelFields[nIndex].szFieldID)){
									if((pPtr=strstr(szBuffer,LabelFields[nIndex].szFieldID))!=NULL){
										bFound=TRUE;
										continue;
									}
									nIndex++;
								}
								strcpy(szDestination,szBuffer);
								if(bFound){
									szDestination[pPtr-szBuffer]='\0';

									if(!strcmp((LabelFields[nIndex].szFieldID),"<IDUB>")){
										if(bFirstUBIC) {
											bFirstUBIC=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,0)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDCDPRO>")){
										if(bFirstCDPRO) {
											bFirstCDPRO=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,1)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTITLE>")){
										if(bFirstTITLE) {
											bFirstTITLE=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,2)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PREZZO>")){
										if(bFirstPREZZO) {
											bFirstPREZZO=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,3)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDNMCPE>")){
										if(bFirstNMCPE) {
											bFirstNMCPE=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,4)); else strcat(szDestination,"");
										nRigaIndex++;
									} else {
										/* normale campo db - stampo com'e' */
										/*
										* problema con pqfnumber !!!
										* rm+lg 16-04-2013
										*/
										/*
										if((nFieldIndex=DBfnumber(DBRes,LabelFields[nIndex].szFieldName))==-1)
										*/
										if((nFieldIndex=atoi(LabelFields[nIndex].szFieldName))==-1)
										{
											strcat(szDestination,"");
										} else {
											strcat(szDestination,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nFieldIndex)));
										}
									}

									strcat(szDestination,pPtr+strlen(LabelFields[nIndex].szFieldID));
									/*
									* riporto le modifiche 
									*/
									strcpy(szBuffer,szDestination);

								}
							}
							if(!bSaltaRiga){
								fputs(szDestination,fp);
							}
							bSaltaRiga=FALSE;
						}
					}
					DBclear(DBResRighe);

					fclose(fpBase);
				}
				nPagine++;
			}
		}
	}
	DBclear(DBRes);

	return TRUE;
}

/*
* Descrizione
* Stampa etichetta
* Parametri di ingresso
*  fp : file sul quale si produce la stampa
*/
ep_bool_t StampaEtichetteStorico(char *szSQLCmd,FILE *fp,char *szLabelPrinterName)
{
	FILE *fpBase;
	char szBuffer[256];
	char szDestination[256];
	char *pPtr;
	int nIndex;
	int nColloIndex;
	int nFieldIndex;
	ep_bool_t bFound;
	ep_bool_t bSaltaRiga=FALSE;
	char szLabelBaseFileName[256];
	char szOrdProg[256];
	char szDSTSP[256];
	char szTestoDDT[256];
	int nCPNMCOL;
	int nORDPROG;
	int nRONMCED;
	int nCollo;
	int nRigaIndex=0;
	int nPagine=0;
	int nRighe;
	int nRigheCollo;
	int nCedola;
	int nLayoutEtichetta=-1;
	DBresult *DBRes;
	DBresult *DBResEti;
	DBresult *DBResRighe;
	ep_bool_t bFirstUBIC=TRUE;
	ep_bool_t bFirstCDPRO=TRUE;
	ep_bool_t bFirstTITLE=TRUE;
	ep_bool_t bFirstNMCPE=TRUE;
	ep_bool_t bFirstPREZZO=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaEtichette(%s)",szSQLCmd);
#endif

	/*
	*
	* rm+lg : problema con pqfnumber !!!

	DBRes=DBExecQuery(Cfg.nDebugLevel,szSQLCmd);
	if(DBntuples(DBRes)){

		if((nCPNMCOL=DBfnumber(DBRes,"cpnmcol"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
		if((nORDPROG=DBfnumber(DBRes,"ordprog"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
		if((nRONMCED=DBfnumber(DBRes,"ronmced"))==-1){
			DBclear(DBRes);
			return FALSE;
		}
	}
	DBclear(DBRes);
	*/

	nCPNMCOL=76;   /* pqfnumber() */
	nORDPROG=0;    /* pqfnumber() */
	nRONMCED=41;   /* pqfnumber() */

	/*
	* Rilancio la select dei colli da stampare
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel,szSQLCmd);
	if(DBntuples(DBRes)){
		for(nColloIndex=0;nColloIndex<DBntuples(DBRes);nColloIndex++){
			strcpy(szOrdProg,DBgetvalue(DBRes,nColloIndex,nORDPROG));
			nCollo=atoi(DBgetvalue(DBRes,nColloIndex,nCPNMCOL));
			GetDSTSP(szOrdProg,szDSTSP,TRUE);
			GetTestoDDT(szOrdProg,nCollo,szTestoDDT);

			nCedola=atoi(DBgetvalue(DBRes,nColloIndex,nRONMCED));

			/* 14-03-2011 rm : ricavo il layout dell'etichetta da stampare */
			DBResEti=DBExecQuery(Cfg.nDebugLevel,"select cdlyeti from cedole_stor where cdnmced=%d;",nCedola);
			if(DBntuples(DBResEti)){
				nLayoutEtichetta=atoi(DBgetvalue(DBResEti,0,0));
			}
			DBclear(DBResEti);

			if(nLayoutEtichetta < 1 || nLayoutEtichetta > 3 ){
				/* 'normalizzo' il layout etichetta, se indice fuori dai limiti 1-3 allora -> 1 : normale */
				nLayoutEtichetta=1;
			}

			/* primo file - etichetta vera e propria */
			sprintf(szLabelBaseFileName,"%s/%s.%02d.1",Cfg.szPathData,Cfg.szLabelBaseName,nLayoutEtichetta);
			if((fpBase=fopen(szLabelBaseFileName,"r"))==(FILE *)NULL){
				/* non trovato... provo con lo standard */
				sprintf(szLabelBaseFileName,"%s/%s.1",Cfg.szPathData,Cfg.szLabelBaseName);
			} else {
				fclose(fpBase);
			}

			if((fpBase=fopen(szLabelBaseFileName,"r"))!=(FILE *)NULL){
				while(fgets(szBuffer,sizeof(szBuffer)-1,fpBase)){
					/*
					* prevedo piu' campi su una riga
					*/
					bFound=TRUE;
					while(bFound){
						/*
						* cerco l'ID campo all'interno della riga
						*/
						nIndex=0;
						bFound=FALSE;
						while(!bFound && strlen(LabelFields[nIndex].szFieldID)){
							if((pPtr=strstr(szBuffer,LabelFields[nIndex].szFieldID))!=NULL){
								bFound=TRUE;
								continue;
							}
							nIndex++;
						}
						strcpy(szDestination,szBuffer);
						if(bFound){
							szDestination[pPtr-szBuffer]='\0';

							if(!strcmp((LabelFields[nIndex].szFieldID),"<ORDPROG>")){
								strcat(szDestination,SubStr(szOrdProg,11,6));
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<RODSNOT>")){
								/* note spedizione */
								strcat(szDestination,szDSTSP);
							} else if(!strcmp((LabelFields[nIndex].szFieldID),"<TESTODDT>")){
								/* Testo DDT */
								strcat(szDestination,szTestoDDT);
							} else {
								/* normale campo db - stampo com'e' */
								/*
								* problema con pqfnumber !!!
								* rm+lg 16-04-2013
								*/
								/*
								if((nFieldIndex=DBfnumber(DBRes,LabelFields[nIndex].szFieldName))==-1)
								*/
								if((nFieldIndex=atoi(LabelFields[nIndex].szFieldName))==-1)
								{
									strcat(szDestination,"");
								} else {
									strcat(szDestination,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nFieldIndex)));
								}
							}

							strcat(szDestination,pPtr+strlen(LabelFields[nIndex].szFieldID));
							/*
							* riporto le modifiche 
							*/
							strcpy(szBuffer,szDestination);
						}
					}
					if(!bSaltaRiga){
						fputs(szDestination,fp);
					}
					bSaltaRiga=FALSE;
				}
				fclose(fpBase);
			}


			DBResRighe=DBExecQuery(Cfg.nDebugLevel,"select ubicazione_ridotta(rpcdubi),codprod_ridotto(rpcdpro),prdstit,to_char(rppzpre::float/100::float,'990D99'),lpad(rpqtord::text,5) from rig_prod_stor r,catalogo c where r.rpcdpro=c.prcdpro and r.ordprog='%s' and r.rpnmcol=%d order by r.rpprrig;",szOrdProg,nCollo);
			nRigheCollo=DBntuples(DBResRighe);
			DBclear(DBResRighe);

			nRigaIndex=0;
			nPagine=0;
			while(nRigheCollo>nPagine*MAX_RIGHE_ETICHETTA){

				bFirstUBIC=TRUE;
				bFirstCDPRO=TRUE;
				bFirstTITLE=TRUE;
				bFirstNMCPE=TRUE;
				bFirstPREZZO=TRUE;

				/* primo file - etichetta vera e propria */
				sprintf(szLabelBaseFileName,"%s/%s.%02d.2",Cfg.szPathData,Cfg.szLabelBaseName,nLayoutEtichetta);
				if((fpBase=fopen(szLabelBaseFileName,"r"))==(FILE *)NULL){
					/* non trovato... provo con lo standard */
					sprintf(szLabelBaseFileName,"%s/%s.2",Cfg.szPathData,Cfg.szLabelBaseName);
				} else {
					fclose(fpBase);
				}

				if((fpBase=fopen(szLabelBaseFileName,"r"))!=(FILE *)NULL){

					DBResRighe=DBExecQuery(Cfg.nDebugLevel,"select ubicazione_ridotta(rpcdubi),codprod_ridotto(rpcdpro),prdstit,to_char(rppzpre::float/100::float,'990D99'),lpad(rpqtord::text,5) from rig_prod_stor r,catalogo c where r.rpcdpro=c.prcdpro and r.ordprog='%s' and r.rpnmcol=%d order by r.rpprrig;",szOrdProg,nCollo);

					if((nRighe=DBntuples(DBResRighe))){

						while(fgets(szBuffer,sizeof(szBuffer)-1,fpBase)){
							/*
							* prevedo piu' campi su una riga
							*/
							bFound=TRUE;
							while(bFound){
								/*
								* cerco l'ID campo all'interno della riga
								*/
								nIndex=0;
								bFound=FALSE;
								while(!bFound && strlen(LabelFields[nIndex].szFieldID)){
									if((pPtr=strstr(szBuffer,LabelFields[nIndex].szFieldID))!=NULL){
										bFound=TRUE;
										continue;
									}
									nIndex++;
								}
								strcpy(szDestination,szBuffer);
								if(bFound){
									szDestination[pPtr-szBuffer]='\0';

									if(!strcmp((LabelFields[nIndex].szFieldID),"<IDUB>")){
										if(bFirstUBIC) {
											bFirstUBIC=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,0)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDCDPRO>")){
										if(bFirstCDPRO) {
											bFirstCDPRO=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,1)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTITLE>")){
										if(bFirstTITLE) {
											bFirstTITLE=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,2)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PREZZO>")){
										if(bFirstPREZZO) {
											bFirstPREZZO=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,3)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDNMCPE>")){
										if(bFirstNMCPE) {
											bFirstNMCPE=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, DBgetvalue(DBResRighe,nRigaIndex,4)); else strcat(szDestination,"");
										nRigaIndex++;
									} else {
										/* normale campo db - stampo com'e' */
										/*
										* problema con pqfnumber !!!
										* rm+lg 16-04-2013
										*/
										/*
										if((nFieldIndex=DBfnumber(DBRes,LabelFields[nIndex].szFieldName))==-1)
										*/
										if((nFieldIndex=atoi(LabelFields[nIndex].szFieldName))==-1)
										{
											strcat(szDestination,"");
										} else {
											strcat(szDestination,StrTrimAll(DBgetvalue(DBRes,nColloIndex,nFieldIndex)));
										}
									}

									strcat(szDestination,pPtr+strlen(LabelFields[nIndex].szFieldID));
									/*
									* riporto le modifiche 
									*/
									strcpy(szBuffer,szDestination);

								}
							}
							if(!bSaltaRiga){
								fputs(szDestination,fp);
							}
							bSaltaRiga=FALSE;
						}
					}
					DBclear(DBResRighe);

					fclose(fpBase);
				}
				nPagine++;
			}
		}
	}
	DBclear(DBRes);

	return TRUE;
}


char *StampaListaCampo(char *szNomeCampo,char *szOrdProg,int nCollo)
{
	static char szLista[1024];
	DBresult *DBRes;
	int nIndex;
	char szBuffer[256];

	strcpy(szLista,"");

	DBRes=DBExecQuery(Cfg.nDebugLevel,"select %s from rig_prod where ordprog='%s' and rpnmcol=%d;",szNomeCampo,szOrdProg,nCollo);
	if(DBntuples(DBRes)){
		for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
			sprintf(szBuffer,"%s\r\n",DBgetvalue(DBRes,nIndex,0));
			strcat(szLista,szBuffer);
		}
	}
	DBclear(DBRes);

	return szLista;
}

char *StampaListaUbicazioni(char *szOrdProg,int nCollo)
{
	static char szLista[1024];
	DBresult *DBRes;
	int nIndex;
	char szBuffer[256];

	strcpy(szLista,"");

	DBRes=DBExecQuery(Cfg.nDebugLevel,"select ubicazione_ridotta(rpcdubi) from rig_prod where ordprog='%s' and rpnmcol=%d;",szOrdProg,nCollo);
	if(DBntuples(DBRes)){
		for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
			sprintf(szBuffer,"%s\r\n",DBgetvalue(DBRes,nIndex,0));
			strcat(szLista,szBuffer);
		}
	}
	DBclear(DBRes);

	return szLista;
}

char *StampaListaTitoli(char *szOrdProg,int nCollo)
{
	static char szLista[1024];
	DBresult *DBRes;
	int nIndex;
	char szBuffer[256];

	strcpy(szLista,"");

	DBRes=DBExecQuery(Cfg.nDebugLevel,"select prdstit,rpcdpro from rig_prod r,catalogo c where r.ordprog='%s' and r.rpnmcol=%d and r.rpcdpro=c.prcdpro;",szOrdProg,nCollo);
	if(DBntuples(DBRes)){
		for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
			sprintf(szBuffer,"%s\r\n",DBgetvalue(DBRes,nIndex,0));
			strcat(szLista,szBuffer);
		}
	}
	DBclear(DBRes);

	return szLista;
}

ep_bool_t GetDSTSP(char *szOrdProg,char *szDSTSP,ep_bool_t bStorico)
{
	ep_bool_t bFound=TRUE;
	DBresult *DBRes;
	DATIORDINE DatiOrdine;
	char szFLNOT[80];
	

	if(!GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
		return FALSE;
	}
	/*
	* rm 17-03-2004 : gestione descrizione linea diviso per cedola 
	*/
	/*
	* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
	*/
//	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='%s';", 
//		DatiOrdine.szROCDSOC, 
//		DatiOrdine.szROCDMAG, 
//		DatiOrdine.szRONMCED, 
//		DatiOrdine.szROTPSPE,
//		DatiOrdine.szROCDLIN);
//	if(DBntuples(DBRes)){
//		strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
//		bFound=TRUE;
//	} else {
//		bFound=FALSE;
//	}
//	DBclear(DBRes);
//
//	if(!bFound){
//		/* rm 27-09-2004 : primo giro bis: cerco con cdlin = '' */
//		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='';", 
//			DatiOrdine.szROCDSOC, 
//			DatiOrdine.szROCDMAG, 
//			DatiOrdine.szRONMCED, 
//			DatiOrdine.szROTPSPE);
//		if(DBntuples(DBRes)){
//			strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
//			bFound=TRUE;
//		} else {
//			bFound=FALSE;
//		}
//		DBclear(DBRes);
//	}

	/*
	* lg 27-02-2012 : ora il dato deve essere preso da ric_ord. Il dato deve essere precedentemente settato
	*                 in fase di lancio ordine o stampa etichette
	*                 altrimenti lo ricalcolo?
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rodstsp,roflnot from ric_ord where ordprog='%s';", 
		szOrdProg);
	if(DBntuples(DBRes)){
		strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
		strcpy(szFLNOT,DBgetvalue(DBRes,0,1));
		if (szFLNOT[0]==NOTA_CORRIERE_SETTATA) {
			bFound=TRUE;
		} else {
			bFound=FALSE;
		}
	} else {
	}
	DBclear(DBRes);



	if(!bFound){
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttts where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s';", 
			DatiOrdine.szROCDSOC, 
			DatiOrdine.szROCDMAG, 
			DatiOrdine.szROTPSPE);

		if(DBntuples(DBRes)){
			strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
		} else {
			strcpy(szDSTSP,"ERR");
		}
		DBclear(DBRes);
	}
	return TRUE;
}

ep_bool_t GetTestoDDT(char *szOrdProg,int nCollo,char *szTestoDDT)
{
	ep_bool_t bFound=FALSE;
	DBresult *DBRes;
	DBresult *DBResPCF;


	strcpy(szTestoDDT,"");

	/*
	* rm 13-10-2005 : cerco il primo collo di linea dell'ordine
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmcol from col_prod where ordprog='%s' and cpswlin='%d' order by cpnmcol limit 1;",szOrdProg,COLLO_AUTOMATICO);
	if(DBntuples(DBRes)){
		/* rm 03-10-2014 : sono comunque presenti colli di linea */
		bFound=TRUE;
		if(nCollo==atoi(DBgetvalue(DBRes,0,0))){
			strcpy(szTestoDDT,"CONTIENE D.D.T.");
		}
	}
	DBclear(DBRes);

	if(!bFound){
		/*
		* rm 01-03-2006 : se non trovo colli di linea cerco colli PCF
		*/
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmcol from col_prod where ordprog='%s' and cptpfor='%c' order by cpnmcol limit 1;",szOrdProg,COLLO_PRECONFEZIONATO);
		if(DBntuples(DBRes)){
			/* rm 03-10-2014 : sono comunque presenti colli di preconfezionato */
			bFound=TRUE;
			if(nCollo==atoi(DBgetvalue(DBRes,0,0))){
				strcpy(szTestoDDT,"CONTIENE D.D.T.");
			}
		}
		DBclear(DBRes);
	}

	if(!bFound){
		/*
		* rm 04-10-2006 : se non trovo colli di linea o PCF cerco colli FF
		*/
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmcol from col_prod where ordprog='%s' and cptpfor='%c' order by cpnmcol limit 1;",szOrdProg,COLLO_FUORI_FORMATO);
		if(DBntuples(DBRes)){
			/* rm 03-10-2014 : ci sono almeno colli preconfezionati */
			bFound=TRUE;
			if(nCollo==atoi(DBgetvalue(DBRes,0,0))){
				strcpy(szTestoDDT,"CONTIENE D.D.T.");
			}
		}
		DBclear(DBRes);
	}

	return bFound;
}
