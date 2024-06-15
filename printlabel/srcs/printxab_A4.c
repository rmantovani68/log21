/*
* printxab.c : Stampa XAB
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli
*
* Copyright A&L srl 2003-2021
*
* 02-01-2002 rm+st : ATTENZIONE : i prezzi in euro sono interi (in centesimi)
*                    usare la funzione NotazioneConVirgola() per la normalizzazione
* 11-02-2002 rm    : riscrittura (da CodeBase a PostgreSQL)
*
* 25-02-2002 rm : manca update ordine al termine della stampa XAB (prxab,dtxab ...)
* 09-10-2003 rm : Stampa xab linea con discriminante per CEDOLA
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
#include <picking.h>
#include <ep-db.h>
#include <proc_list.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"
#include	"linklist.h"
#include	"stampa.h"

/* 
********************************************************************************************
* Funzioni per la stampa della XAB
********************************************************************************************
*/
//
//ep_bool_t IsXAB(char *szStato)
//{
//	switch(szStato[0]){
//		case ORDINE_EVASO:
//		case ORDINE_STAMPATA_XAB:
//		case ORDINE_STAMPATA_DISTINTA:
//		case ORDINE_SPEDITO_HOST:
//			return TRUE;
//		break;
//	}
//	return FALSE;
//}
//
//
///*
//* Converte una data dal formato YYYYMMDD al formato DD/MM/YYYY
/* */
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY)
//{
//	char szYYYY[40];
//	char szMM[40];
//	char szDD[40];
//
//	sprintf(szYYYY,"%4.4s",szDateYYYYMMDD);
//	sprintf(szMM,"%2.2s",szDateYYYYMMDD+4);
//	sprintf(szDD,"%2.2s",szDateYYYYMMDD+6);
//	sprintf(szDateDDMMYYYY,"%s/%s/%s",szDD,szMM,szYYYY);
//
//	return (szDateDDMMYYYY);
//}
//
//
///*
//* Converte una data dal formato YYYYMMDD al formato DD/MM/YY
/* */
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY)
//{
//	char szYY[40];
//	char szMM[40];
//	char szDD[40];
//
//	sprintf(szYY,"%2.2s",szDateYYYYMMDD+2);
//	sprintf(szMM,"%2.2s",szDateYYYYMMDD+4);
//	sprintf(szDD,"%2.2s",szDateYYYYMMDD+6);
//	sprintf(szDateDDMMYY,"%s/%s/%s",szDD,szMM,szYY);
//
//	return (szDateDDMMYY);
//}


/*
* InserisciDatiEveryPageXAB()
* Funzione per la gestione della intestazione e del fondo pagine di tutte le pagine XAB
* 02-01-2002 rm+st : Gestione EURO
*/
ep_bool_t InserisciDatiEveryPageXAB_A4(PDATIORDINE pDatiOrdine)
{
	char szAnno[20];
	int nAnno;
	char szProg[20];
	char szDataBAMC[20];
	char szDataBAML[20];
	char szDataStampa[20];
	char szAssegno[40];
	char szAspetto[40];
	char szDSCST[80];
	char szString[80];
	ep_bool_t bOK=TRUE;
	ep_bool_t bEuro;
	DBresult *DBRes;

	/*
	* Decido se usare EURO o LIRE
	*/
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);

	DateDDMMYY(pDatiOrdine->szRODTBAM,szDataBAMC);
	DateDDMMYYYY(pDatiOrdine->szRODTBAM,szDataBAML);
	GetDate(szDataStampa);
	/*
	* Stampa della causale del trasporto (da gestire decodifica da CDMOV)
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select  ELCDSCST from ttcs where elccdsoc='%s' and elccdmag='%s' and elccdcst='%s';", 
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		pDatiOrdine->szROCDMOV);
	if(DBntuples(DBRes)){
		strcpy(szDSCST,DBgetvalue(DBRes,0,0));
	} else {
		strcpy(szDSCST,"");
	}
	DBclear(DBRes);
	/*
	* 02-01-2002 rm+st : valore in EURO o LIRE
	*/
	if(bEuro){
		/* il prezzo in euro e' in centesimi */
		sprintf(szAssegno,"%.2f",(double)pDatiOrdine->nROVLCOE/(double)100);
	} else {
		sprintf(szAssegno,"%d",pDatiOrdine->nROVLCON);
	}
	switch (pDatiOrdine->szROSWCOL[0]){
		case '1':
			strcpy(szAspetto,"COLLI");
		break;
		default:
			strcpy(szAspetto,"PANCALI");
		break;
	}

	/* Anno */
	nAnno=atoi(LeftStr(GetDateYYYYMMDD(szDateBuffer),4));


	sprintf(szAnno,"/%02d",nAnno%100);
	sprintf(szProg,"%07d",pDatiOrdine->nROPRXAB);
		
	InitData("EveryPage",pDatiOrdine->szROCDCLA, XABPos_A4[F_DESTINATARIO0].nSR,     XABPos_A4[F_DESTINATARIO0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPA, XABPos_A4[F_DESTINATARIO1].nSR,     XABPos_A4[F_DESTINATARIO1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPC, XABPos_A4[F_DESTINATARIO2].nSR,     XABPos_A4[F_DESTINATARIO2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLA, XABPos_A4[F_DESTINATARIO3].nSR,     XABPos_A4[F_DESTINATARIO3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLA, XABPos_A4[F_DESTINATARIO4].nSR,     XABPos_A4[F_DESTINATARIO4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLA, XABPos_A4[F_DESTINATARIO5].nSR,     XABPos_A4[F_DESTINATARIO5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLA, XABPos_A4[F_DESTINATARIO6].nSR,     XABPos_A4[F_DESTINATARIO6].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLA, XABPos_A4[F_DESTINATARIO7].nSR,     XABPos_A4[F_DESTINATARIO7].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLA, XABPos_A4[F_DESTINATARIO8].nSR,     XABPos_A4[F_DESTINATARIO8].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, XABPos_A4[F_NUMEROINALTO].nSR,      XABPos_A4[F_NUMEROINALTO].nSC,      FALSE);
	InitData("EveryPage",szDataBAML,             XABPos_A4[F_DATAXAB].nSR,           XABPos_A4[F_DATAXAB].nSC,           FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDMOV, XABPos_A4[F_DOCUMENTOINTERNO0].nSR, XABPos_A4[F_DOCUMENTOINTERNO0].nSC, FALSE);
	//InitData("EveryPage",szDataBAMC,             XABPos_A4[F_DOCUMENTOINTERNO1].nSR, XABPos_A4[F_DOCUMENTOINTERNO1].nSC, FALSE);
	InitData("EveryPage",szDataBAML,             XABPos_A4[F_DOCUMENTOINTERNO1].nSR, XABPos_A4[F_DOCUMENTOINTERNO1].nSC, FALSE);
	//InitData("EveryPage",szDataStampa,           XABPos_A4[F_DOCUMENTOINTERNO2].nSR, XABPos_A4[F_DOCUMENTOINTERNO2].nSC, FALSE);
	InitData("EveryPage",szProg,                 XABPos_A4[F_NUMEROINBASSO].nSR,     XABPos_A4[F_NUMEROINBASSO].nSC,     FALSE);
	InitData("EveryPage",szAnno,                 XABPos_A4[F_NUMEROINBASSO].nSR,     XABPos_A4[F_NUMEROINBASSO].nSC+7,   FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, XABPos_A4[F_SPEDIZIONE].nSR,        XABPos_A4[F_SPEDIZIONE].nSC,        FALSE);
	InitData("EveryPage",LeftStr(szDSCST,20),    XABPos_A4[F_CAUSALE].nSR,           XABPos_A4[F_CAUSALE].nSC,           FALSE);
	InitData("EveryPage",Cfg.szContenuto,        XABPos_A4[F_CONTENUTO].nSR,         XABPos_A4[F_CONTENUTO].nSC,         FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDRID, XABPos_A4[F_DESTINAZIONE0].nSR,     XABPos_A4[F_DESTINAZIONE0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLI, XABPos_A4[F_DESTINAZIONE1].nSR,     XABPos_A4[F_DESTINAZIONE1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLI, XABPos_A4[F_DESTINAZIONE2].nSR,     XABPos_A4[F_DESTINAZIONE2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLI, XABPos_A4[F_DESTINAZIONE3].nSR,     XABPos_A4[F_DESTINAZIONE3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLI, XABPos_A4[F_DESTINAZIONE4].nSR,     XABPos_A4[F_DESTINAZIONE4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLI, XABPos_A4[F_DESTINAZIONE5].nSR,     XABPos_A4[F_DESTINAZIONE5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLI, XABPos_A4[F_DESTINAZIONE6].nSR,     XABPos_A4[F_DESTINAZIONE6].nSC,     FALSE);
	InitData("EveryPage",szAssegno,              XABPos_A4[F_ASSEGNO].nSR,           XABPos_A4[F_ASSEGNO].nSC,           FALSE);
	InitData("EveryPage",szAspetto,              XABPos_A4[F_ASPETTOBENI].nSR,       XABPos_A4[F_ASPETTOBENI].nSC,       FALSE);

	sprintf(szString,"PACKING LIST ORDINE %s",pDatiOrdine->szROPRDOC);
	InitData("EveryPage",szString,Cfg.nPrimaRigaXAB_A4-3,XABPos_A4[F_AUTOREXAB].nSC,FALSE);
	sprintf(szString,"%s @@",Cfg.szFoglio);
	InitData("EveryPage",szString,Cfg.nPrimaRigaXAB_A4-2,XABPos_A4[F_AUTOREXAB].nSC,FALSE);
	//sprintf(szString,"Nr. Collo");
	//InitData("EveryPage",szString,Cfg.nPrimaRigaXAB_A4-2,XABPos_A4[F_SPEDIZIONE].nSC,FALSE);
	
	return bOK;
}


/*
* InserisciDatiOldPageXAB()
* Funzione per la gestione della pagina precedente in caso di salto pagina
*/
void InserisciDatiOldPageXAB_A4(void)
{
	InitData("OldPage",Cfg.szSegue,Cfg.nUltimaRigaXAB_A4+1,XABPos_A4[F_AUTOREXAB].nSC,FALSE);
}


/*
* InserisciDatiNewPageXAB()
* Funzione per la gestione della pagina attuale in caso di salto pagina
*/
void InserisciDatiNewPageXAB_A4(PDATIORDINE pDatiOrdine)
{
}

/*
* InserisciDatiLastPageXAB()
* Funzione per la gestione dell'ultima pagina 
* 02-01-2002 rm+st : Gestione EURO
*/
ep_bool_t InserisciDatiLastPageXAB_A4(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot)
{
	ep_bool_t bOK=TRUE;
	ep_bool_t bFound=FALSE;
	ep_bool_t bEuro=FALSE;
	DBresult *DBRes;

	char szDSTSP[80];
	char szString[80];
	char szBuffer[80];
	char szFLNOT[80];

	/* 
	* Ultima riga 
	*/
	sprintf(szString,"  %s @@ FOGLI",Cfg.szFineBolla);
	InitData("LastPage",szString,Cfg.nUltimaRigaXAB_A4+1,XABPos_A4[F_AUTOREXAB].nSC,FALSE);
	/* 
	* Quantita' 
	*/
	sprintf(szString,"%6d",nQtaTot);
	InitData("LastPage",szString,XABPos_A4[F_QUANTITAXAB].nSR,XABPos_A4[F_QUANTITAXAB].nSC,FALSE);
	/* 
	* Peso in Kg. 
	*/
	if (pDatiOrdine->nROPSREA){
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSREA/(float)1000));
	} else {
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSPRE/(float)1000));
	}

	InitData("LastPage",szString,XABPos_A4[F_PESO].nSR,XABPos_A4[F_PESO].nSC,FALSE);
	/* 
	* Numero colli 
	*/
	sprintf(szString,"%6d",pDatiOrdine->nRONMCLL);
	InitData("LastPage",szString,XABPos_A4[F_NUMCOLLI].nSR,XABPos_A4[F_NUMCOLLI].nSC,FALSE);

	/* decido se usare EURO o LIRE */
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);

	if(bEuro){
		NotazioneConVirgola(nPrzTot,szString);
	} else {
		NotazioneConPunti(nPrzTot,szString);
	}

	InitData("LastPage",szString,XABPos_A4[F_VARIAZDEST].nSR,XABPos_A4[F_VARIAZDEST].nSC,FALSE);
	/* -- Vettori -- */

	if (!DatiVettore_A4(pDatiOrdine,pDatiOrdine->szROCDVE1,F_VETTORI0,F_VETTORI1,F_VETTORI2)) bOK=FALSE;
	if (!DatiVettore_A4(pDatiOrdine,pDatiOrdine->szROCDVE2,F_VETTORI3,F_VETTORI4,F_VETTORI5)) bOK=FALSE;

//	/*
//	* rm 17-03-2004 : gestione descrizione linea diviso per cedola 
//	*/
//	/*
//	* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
//	*/
//	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='%s';", 
//		pDatiOrdine->szROCDSOC, 
//		pDatiOrdine->szROCDMAG, 
//		pDatiOrdine->szRONMCED, 
//		pDatiOrdine->szROTPSPE,
//		pDatiOrdine->szROCDLIN);
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
//			pDatiOrdine->szROCDSOC, 
//			pDatiOrdine->szROCDMAG, 
//			pDatiOrdine->szRONMCED, 
//			pDatiOrdine->szROTPSPE);
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
		pDatiOrdine->szORDPROG);
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
		/*
		* Chiave per la tabella ttts:
		* 	societa'+magazzino+tipo spedizione
		*/
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttts where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s';",
			pDatiOrdine->szROCDSOC,
			pDatiOrdine->szROCDMAG,
			pDatiOrdine->szROTPSPE);
		if(DBntuples(DBRes)){
			strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
			bFound=TRUE;
		} else {
	#ifdef TRACE
			trace_debug(FALSE, TRUE, "Chiave %s non presente nella tabella ttts",pDatiOrdine->szROTPSPE);
	#endif
			bOK=FALSE;
			bFound=FALSE;
		}
		DBclear(DBRes);
	}
	if(bFound){
		InitData("LastPage",szDSTSP,XABPos_A4[F_ANNOTAZIONI].nSR,XABPos_A4[F_ANNOTAZIONI].nSC,FALSE);
	}

	return bOK;
}


ep_bool_t DatiVettore_A4(PDATIORDINE pDatiOrdine,char *szCDVET,int nPos1,int nPos2,int nPos3)
{
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdsvet,elcinvet,elclovet from ttve where elccdsoc='%s' and elccdmag='%s' and elccdvet='%s';",
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		szCDVET);
	if(DBntuples(DBRes)){
		InitData("LastPage",DBgetvalue(DBRes,0,0),XABPos_A4[nPos1].nSR,XABPos_A4[nPos1].nSC,FALSE);
		InitData("LastPage",DBgetvalue(DBRes,0,1),XABPos_A4[nPos2].nSR,XABPos_A4[nPos2].nSC,FALSE);
		InitData("LastPage",DBgetvalue(DBRes,0,2),XABPos_A4[nPos3].nSR,XABPos_A4[nPos3].nSC,FALSE);
	} else {
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Dati vettore non trovati nella tabella ttve. Chiave %s",szCDVET);
#endif
		bOK=FALSE;
	}
	DBclear(DBRes);

	return bOK;
}



/*
* StampaXAB(PDATIORDINE pDatiOrdine)
*
* Stampa l'XAB dell'ordine in oggetto
*
* Parametri di ingresso:
* 
* pDatiOrdine : Dati Ordine da stampare
*  
* Ritorna:
*  in caso di errore:   FALSE
*  in caso di successo: TRUE 
*
* rm 13-07-1999 : gestione storico
* 02-01-2002 : Gestione EURO - I PREZZI IN EURO SONO IN CENTESIMI !
*/
ep_bool_t StampaXAB_A4(PDATIORDINE pDatiOrdine,ep_bool_t bStorico)
{
	DBresult *DBRes=NULL;
	DBresult *DBResTot=NULL;
	DBresult *DBResCat=NULL;
	DBresult *DBResRA=NULL;
	DBresult *DBResCollo=NULL;
	ep_bool_t bOK=TRUE;
	char szTmpBuf[80];
	char szBuffer[128];
	// char szRiga[MAX_COLONNE_XAB_A4];
	char szRiga[256];
	char szValueField[80];
	char szRPCDPRO[80];
	int nRDBTSPE;
	int nRDBTSPETot;
	int nRPNMCOL;
	char szPRDSTIT[128];
	char szPRDSAUT[128];
	char szRACDIVA[128];
	int nRAPZPRO;
	int nValoreOrdine;
	int nNumeroPezzi;
	int nRigheProdotto;
	int nAnno;
	int nTuples;
	int nTotTuples;
	int nIndex=0;
	int nTotIndex=0;
	ep_bool_t bEuro=FALSE;
	ep_bool_t bRistampa=TRUE;
	char szRicOrd[128];
	char szRicArt[128];
	char szRicNote[128];
	char szRigProd[128];
	char szColProd[128];
	int nColli;
	int nColloIndex=0;


#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa XAB [%s]",pDatiOrdine->szORDPROG);
#endif
	/* 
	* Decido se usare EURO o LIRE 
	*/
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);

	if(bEuro){
		/* euro  - centesimi */
		strcpy(szValueField,"rapzpre");
	} else {
		/* lirette */
		strcpy(szValueField,"rapzpro");
	}

	if(bStorico){
		strcpy(szRicOrd,  "ric_ord_stor");
		strcpy(szRicArt,  "ric_art_stor");
		strcpy(szRicNote, "ric_note_stor");
		strcpy(szRigProd, "rig_prod_stor");
		strcpy(szColProd, "col_prod_stor");
	} else {
		strcpy(szRicOrd, "ric_ord");
		strcpy(szRicArt, "ric_art");
		strcpy(szRicNote,"ric_note");
		strcpy(szRigProd,"rig_prod");
		strcpy(szColProd,"col_prod");
	}

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select sum(rp.rpqtspe),sum(rp.rpqtspe * ra.%s) from %s rp,%s ra where rp.rpcdpro=ra.racdpro and ra.ordprog=rp.ordprog and rp.ordprog='%s';",szValueField,szRigProd,szRicArt,pDatiOrdine->szORDPROG);
	if(DBntuples(DBRes)){
		nNumeroPezzi=atoi(DBgetvalue(DBRes,0,0));
		nValoreOrdine=atoi(DBgetvalue(DBRes,0,1));
	} else {
		nValoreOrdine=0;
		nNumeroPezzi=0;
		bOK=FALSE;
	}
	DBclear(DBRes);

	if(!bOK){
		return FALSE;
	}

	/* 
	* se non si tratta di una ristampa 
	* incremento il progressivo XAB
	*/
	if(!pDatiOrdine->nROPRXAB){
		nAnno=atoi(LeftStr(GetDateYYYYMMDD(szDateBuffer),4));
		/* 
		* Progressivo XAB 
		* se si tratta della prima volta che si stampa l'XAB di questo ordine
		* allora si calcolano e assegnano i valori e le sequenze relative
		*/
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select nextval('sequence_xab');");
		if(DBntuples(DBRes)){
			pDatiOrdine->nROPRXAB=atoi(DBgetvalue(DBRes,0,0));
			pDatiOrdine->nROAAXAB=nAnno%100;
			strcpy(pDatiOrdine->szRODTXAB,GetDateYYYYMMDD(szDateBuffer));
		} else {
			bOK=FALSE;
		}
		DBclear(DBRes);
		bRistampa=FALSE;
	} else {
		bRistampa=TRUE;
	}

	/* pulizia pagine */
	DeleteData("FirstPage");
	DeleteData("LastPage");
	DeleteData("OldPage");
	DeleteData("EveryPage");
	DeleteData("NewPage");

	InserisciDatiEveryPageXAB_A4(pDatiOrdine);
	InserisciDatiNewPageXAB_A4(pDatiOrdine);
	InserisciDatiOldPageXAB_A4();
	InserisciDatiLastPageXAB_A4(pDatiOrdine,nNumeroPezzi,nValoreOrdine);


	/* righe di produzione */
	/* 
	*
	* Righe di prod non raggruppate
	* Esistono 2 tipi di stampa:
	*
	* 2.ordinate per collo e rpprrig
	* 1.ordinate per codice prodotto, con relativo collo in cui sono presenti
	*/


	switch(Cfg.nTipoOrdinamentoStampaXAB){
		case 1:
		{
			DBResTot=DBExecQuery(Cfg.nDebugLevel>2,"select rpcdpro,count(rpcdpro),sum(rpqtspe) from %s where ordprog='%s' group by rpcdpro order by rpcdpro;",szRigProd,pDatiOrdine->szORDPROG);
			if((nTotTuples=DBntuples(DBResTot))){
				for(nTotIndex=0;nTotIndex<nTotTuples;nTotIndex++){
					strcpy(szRPCDPRO,DBgetvalue(DBResTot,nTotIndex,0));
					nRigheProdotto=atoi(DBgetvalue(DBResTot,nTotIndex,1));
					nRDBTSPETot=atoi(DBgetvalue(DBResTot,nTotIndex,2));


					DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rpqtspe,rpnmcol from %s where ordprog='%s' and rpcdpro='%s' order by rpnmcol;",szRigProd,pDatiOrdine->szORDPROG,szRPCDPRO);
					if((nTuples=DBntuples(DBRes))){
						for(nIndex=0;nIndex<nTuples;nIndex++){
							nRDBTSPE=atoi(DBgetvalue(DBRes,nIndex,0));
							nRPNMCOL=atoi(DBgetvalue(DBRes,nIndex,1));

							/* catalogo */
							DBResCat=DBExecQuery(Cfg.nDebugLevel>2,"select prdstit,prdsaut from catalogo where prcdpro='%s';",szRPCDPRO);
							if(DBntuples(DBResCat)){
								strcpy(szPRDSTIT,DBgetvalue(DBResCat,0,0));
								strcpy(szPRDSAUT,DBgetvalue(DBResCat,0,1));
							} else {
								strcpy(szPRDSTIT,"ERRORE");
								strcpy(szPRDSAUT,"ERRORE");
							}
							DBclear(DBResCat);

							/* righe ricevute */
							DBResRA=DBExecQuery(Cfg.nDebugLevel>2,"select racdiva,%s from %s where ordprog='%s' and racdpro='%s';",szValueField,szRicArt,pDatiOrdine->szORDPROG,szRPCDPRO);
							if(DBntuples(DBResRA)){
								strcpy(szRACDIVA,DBgetvalue(DBResRA,0,0));
								nRAPZPRO=atoi(DBgetvalue(DBResRA,0,1));
							} else {
								strcpy(szRACDIVA,"ERRORE");
								nRAPZPRO=0;
							}
							DBclear(DBResRA);

							sprintf(szBuffer," %-9.9s   ",    RightStr(szRPCDPRO,9)); strcpy(szRiga,szBuffer);
							sprintf(szBuffer,"%-10.10s ",   szPRDSAUT); strcat(szRiga,szBuffer);
							sprintf(szBuffer,"%-29.29s ",  szPRDSTIT); strcat(szRiga,szBuffer);
							sprintf(szBuffer,"%10d  ",    nRDBTSPE); strcat(szRiga,szBuffer);
							/*
							* 02-08-2005 LG : tolta la quantità in lettere
							*/
							//sprintf(szBuffer,"%7.7s    ", NumberToLetter(nRDBTSPE,szTmpBuf)); strcat(szRiga,szBuffer);
							/*
							* 02-01-2002 rm+st : Gestione EURO
							*/
							if(bEuro){
								sprintf(szBuffer," %9.9s ",    NotazioneConVirgola(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
							} else {
								sprintf(szBuffer,"%9.9s ",    NotazioneConPunti(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
							}
							sprintf(szBuffer,"%3.3s",     szRACDIVA); strcat(szRiga,szBuffer);

							/* stampo il collo nella colonna note */
							sprintf(szBuffer," Collo : %3d", nRPNMCOL); strcat(szRiga,szBuffer);
							PrintRow(szRiga,1,TRUE);
						}
					} else {
						bOK=FALSE;
					}
					DBclear(DBRes);

					if (nRigheProdotto>1){
						sprintf(szBuffer,"                                                      TOT: %5d", nRDBTSPETot); strcpy(szRiga,szBuffer);
						PrintRow(szRiga,1,TRUE);
					} 
					//sprintf(szBuffer,"     "); strcpy(szRiga,szBuffer);
					//PrintRow(szRiga,1,TRUE);
				}
			}
			DBclear(DBResTot);
		}
		break;
		case 2:
		{
			DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rpcdpro,rpqtspe,rpnmcol from %s where ordprog='%s' order by rpcdpro,rpnmcol;",szRigProd,pDatiOrdine->szORDPROG);
			if((nTuples=DBntuples(DBRes))){
				for(nIndex=0;nIndex<nTuples;nIndex++){
					strcpy(szRPCDPRO,DBgetvalue(DBRes,nIndex,0));
					nRDBTSPE=atoi(DBgetvalue(DBRes,nIndex,1));
					nRPNMCOL=atoi(DBgetvalue(DBRes,nIndex,2));

					/* catalogo */
					DBResCat=DBExecQuery(Cfg.nDebugLevel>2,"select prdstit,prdsaut from catalogo where prcdpro='%s';",szRPCDPRO);
					if(DBntuples(DBResCat)){
						strcpy(szPRDSTIT,DBgetvalue(DBResCat,0,0));
						strcpy(szPRDSAUT,DBgetvalue(DBResCat,0,1));
					} else {
						strcpy(szPRDSTIT,"ERRORE");
						strcpy(szPRDSAUT,"ERRORE");
					}
					DBclear(DBResCat);

					/* righe ricevute */
					DBResRA=DBExecQuery(Cfg.nDebugLevel>2,"select racdiva,%s from %s where ordprog='%s' and racdpro='%s';",szValueField,szRicArt,pDatiOrdine->szORDPROG,szRPCDPRO);
					if(DBntuples(DBResRA)){
						strcpy(szRACDIVA,DBgetvalue(DBResRA,0,0));
						nRAPZPRO=atoi(DBgetvalue(DBResRA,0,1));
					} else {
						strcpy(szRACDIVA,"ERRORE");
						nRAPZPRO=0;
					}
					DBclear(DBResRA);

					sprintf(szBuffer," %-9.9s   ",    RightStr(szRPCDPRO,9)); strcpy(szRiga,szBuffer);
					sprintf(szBuffer,"%-10.10s ",   szPRDSAUT); strcat(szRiga,szBuffer);
					sprintf(szBuffer,"%-29.29s ",  szPRDSTIT); strcat(szRiga,szBuffer);
					sprintf(szBuffer,"%10d  ",    nRDBTSPE); strcat(szRiga,szBuffer);
					/*
					* 02-08-2005 LG : tolta la quantità in lettere
					*/
					//sprintf(szBuffer,"%7.7s    ", NumberToLetter(nRDBTSPE,szTmpBuf)); strcat(szRiga,szBuffer);
					/*
					* 02-01-2002 rm+st : Gestione EURO
					*/
					if(bEuro){
						sprintf(szBuffer," %9.9s ",    NotazioneConVirgola(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
					} else {
						sprintf(szBuffer,"%9.9s ",    NotazioneConPunti(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
					}
					sprintf(szBuffer,"%3.3s",     szRACDIVA); strcat(szRiga,szBuffer);

					/* stampo il collo nella colonna note */
					sprintf(szBuffer," Collo : %3d", nRPNMCOL); strcat(szRiga,szBuffer);
					PrintRow(szRiga,1,TRUE);
				}
			} else {
				bOK=FALSE;
			}
			DBclear(DBRes);
		}
		break;
		case 3:
		{
			
			DBResCollo=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmcol from %s where ordprog='%s' order by cpnmcol;",szColProd,pDatiOrdine->szORDPROG);

			if((nColli=DBntuples(DBResCollo))){

				for(nColloIndex=0;nColloIndex<nColli;nColloIndex++){


					DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rpcdpro,rpqtspe from %s where ordprog='%s' and rpnmcol=%d order by rpprrig;",szRigProd,pDatiOrdine->szORDPROG,nColloIndex+1);
					if((nTuples=DBntuples(DBRes))){
						for(nIndex=0;nIndex<nTuples;nIndex++){
							strcpy(szRPCDPRO,DBgetvalue(DBRes,nIndex,0));
							nRDBTSPE=atoi(DBgetvalue(DBRes,nIndex,1));

							/* catalogo */
							DBResCat=DBExecQuery(Cfg.nDebugLevel>2,"select prdstit,prdsaut from catalogo where prcdpro='%s';",szRPCDPRO);
							if(DBntuples(DBResCat)){
								strcpy(szPRDSTIT,DBgetvalue(DBResCat,0,0));
								strcpy(szPRDSAUT,DBgetvalue(DBResCat,0,1));
							} else {
								strcpy(szPRDSTIT,"ERRORE");
								strcpy(szPRDSAUT,"ERRORE");
							}
							DBclear(DBResCat);

							/* righe ricevute */
							DBResRA=DBExecQuery(Cfg.nDebugLevel>2,"select racdiva,%s from %s where ordprog='%s' and racdpro='%s';",szValueField,szRicArt,pDatiOrdine->szORDPROG,szRPCDPRO);
							if(DBntuples(DBResRA)){
								strcpy(szRACDIVA,DBgetvalue(DBResRA,0,0));
								nRAPZPRO=atoi(DBgetvalue(DBResRA,0,1));
							} else {
								strcpy(szRACDIVA,"ERRORE");
								nRAPZPRO=0;
							}
							DBclear(DBResRA);

							sprintf(szBuffer," %-9.9s   ",    RightStr(szRPCDPRO,9)); strcpy(szRiga,szBuffer);
							sprintf(szBuffer,"%-10.10s ",   szPRDSAUT); strcat(szRiga,szBuffer);
							sprintf(szBuffer,"%-29.29s ",  szPRDSTIT); strcat(szRiga,szBuffer);
							sprintf(szBuffer,"%10d  ",    nRDBTSPE); strcat(szRiga,szBuffer);
							/*
							* 02-08-2005 LG : tolta la quantità in lettere
							*/
							//sprintf(szBuffer,"%7.7s    ", NumberToLetter(nRDBTSPE,szTmpBuf)); strcat(szRiga,szBuffer);
							/*
							* 02-01-2002 rm+st : Gestione EURO
							*/
							if(bEuro){
								sprintf(szBuffer," %9.9s ",    NotazioneConVirgola(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
							} else {
								sprintf(szBuffer,"%9.9s ",    NotazioneConPunti(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
							}
							sprintf(szBuffer,"%3.3s",     szRACDIVA); strcat(szRiga,szBuffer);

							/* stampo il collo nella colonna note */
							if(nIndex==0){
								sprintf(szBuffer," Collo : %3d",     nColloIndex+1); strcat(szRiga,szBuffer);
							}
							PrintRow(szRiga,1,TRUE);
						}
					} else {
						bOK=FALSE;
					}
					DBclear(DBRes);

				}
			}
			DBclear(DBResCollo);
		}
		break;
		case 4:
		{
			DBResTot=DBExecQuery(Cfg.nDebugLevel>2,"select rpcdpro,count(rpcdpro),sum(rpqtspe) from %s where ordprog='%s' group by rpcdpro order by rpcdpro;",szRigProd,pDatiOrdine->szORDPROG);
			if((nTotTuples=DBntuples(DBResTot))){
				for(nTotIndex=0;nTotIndex<nTotTuples;nTotIndex++){
					strcpy(szRPCDPRO,DBgetvalue(DBResTot,nTotIndex,0));
					nRigheProdotto=atoi(DBgetvalue(DBResTot,nTotIndex,1));
					nRDBTSPE=atoi(DBgetvalue(DBResTot,nTotIndex,2));

					/* catalogo */
					DBResCat=DBExecQuery(Cfg.nDebugLevel>2,"select prdstit,prdsaut from catalogo where prcdpro='%s';",szRPCDPRO);
					if(DBntuples(DBResCat)){
						strcpy(szPRDSTIT,DBgetvalue(DBResCat,0,0));
						strcpy(szPRDSAUT,DBgetvalue(DBResCat,0,1));
					} else {
						strcpy(szPRDSTIT,"ERRORE");
						strcpy(szPRDSAUT,"ERRORE");
					}
					DBclear(DBResCat);

					/* righe ricevute */
					DBResRA=DBExecQuery(Cfg.nDebugLevel>2,"select racdiva,%s from %s where ordprog='%s' and racdpro='%s';",szValueField,szRicArt,pDatiOrdine->szORDPROG,szRPCDPRO);
					if(DBntuples(DBResRA)){
						strcpy(szRACDIVA,DBgetvalue(DBResRA,0,0));
						nRAPZPRO=atoi(DBgetvalue(DBResRA,0,1));
					} else {
						strcpy(szRACDIVA,"ERRORE");
						nRAPZPRO=0;
					}
					DBclear(DBResRA);

					sprintf(szBuffer," %-9.9s   ",    RightStr(szRPCDPRO,9)); strcpy(szRiga,szBuffer);
					sprintf(szBuffer,"%-10.10s ",   szPRDSAUT); strcat(szRiga,szBuffer);
					sprintf(szBuffer,"%-29.29s ",  szPRDSTIT); strcat(szRiga,szBuffer);
					sprintf(szBuffer,"%10d  ",    nRDBTSPE); strcat(szRiga,szBuffer);
					/*
					* 02-08-2005 LG : tolta la quantità in lettere
					*/
					//sprintf(szBuffer,"%7.7s    ", NumberToLetter(nRDBTSPE,szTmpBuf)); strcat(szRiga,szBuffer);
					/*
					* 02-01-2002 rm+st : Gestione EURO
					*/
					if(bEuro){
						sprintf(szBuffer," %9.9s ",    NotazioneConVirgola(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
					} else {
						sprintf(szBuffer,"%9.9s ",    NotazioneConPunti(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
					}
					sprintf(szBuffer,"%3.3s",     szRACDIVA); strcat(szRiga,szBuffer);

					PrintRow(szRiga,1,TRUE);
				}
			}
			DBclear(DBResTot);
		}
		break;
	}



	/*
	* Ciclo di stampa delle note.
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select RNFRTXT from %s where ordprog='%s' and rntptxt='%s' order by rnnmtxt;",szRicNote, pDatiOrdine->szORDPROG,Cfg.szTipoNotaXAB);
	if((nTuples=DBntuples(DBRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			sprintf(szRiga,"        %s",DBgetvalue(DBRes,nIndex,0));
			PrintRow(szRiga,2,TRUE);
		}
	}
	DBclear(DBRes);

	/* 
	* aggiorno il progressivo XAB
	* rm 30-09-2005 : eliminato il settaggio dello stato XAB 
	* in quanto stampato all'inizio del prelievo
	*/
	if(bOK && !bRistampa){
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"update %s set roprxab=%d,roaaxab=%d,rodtxab='%s' where ordprog='%s';",
			szRicOrd,
			pDatiOrdine->nROPRXAB,
			pDatiOrdine->nROAAXAB,
			pDatiOrdine->szRODTXAB,
			pDatiOrdine->szORDPROG);
		if((DBresultStatus(DBRes)!=DBRES_COMMAND_OK)){
			bOK=FALSE;
		}
		DBclear(DBRes);
	}

	return bOK;
}

/*
* Funzione di stampa della XAB per una singola spedizione
* Parametri di ingresso:
*  pszPrinterName: nome della stampante
*  szOrdProg:      chiave
* Ritorno:
*  TRUE in caso di successo
*	 FALSE in caso di insuccesso
*/
ep_bool_t CreateXAB_A4(char *pszPrinterName,char *szOrdProg,char *szFile,ep_bool_t bStorico)
{
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	int nPagine=0;
	ep_bool_t bOK=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa XAB Spedizione [%s] Storico : %d",szOrdProg,bStorico);
#endif

	sprintf(szPrintFile,"%s/%s",Cfg.szPathStampe,szFile);

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
		/*
		* La XAB viene stampata se si trova negli stati:
		*  (E) Evaso
		*  (X) XAB
		*  (D) Distinta
		*  (H) Storicizzato
		*/
		if(1 || IsXAB(DatiOrdine.szROSTATO)){
			/* Creazione del file di stampa */
			InitPrint(szPrintFile,MAX_RIGHE_XAB_A4,MAX_COLONNE_XAB_A4,Cfg.nPrimaRigaXAB_A4,Cfg.nUltimaRigaXAB_A4);

			if(!StampaXAB_A4(&DatiOrdine,bStorico)){
				bOK=FALSE;
			}
			/*
			* Chiudo il file di stampa
			*/
			nPagine=EndPrint(TRUE);

		} else {
			/*
			* La spedizione non e' in uno stato utile per la stampa XAB
			*/
			bOK=FALSE;
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Ordine [%s] in stato non corretto",szOrdProg);
#endif
		}
	} else {
		bOK=FALSE;
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Ordine [%s] non presente",szOrdProg);
#endif
	}


	return bOK;
}



/*
* Funzione di stampa della XAB per una singola spedizione
* Parametri di ingresso:
*  pszPrinterName: nome della stampante
*  szOrdProg:      chiave
* Ritorno:
*  TRUE in caso di successo
*	 FALSE in caso di insuccesso
*/
ep_bool_t StampaXABSpedizione_A4(char *pszPrinterName,char *szOrdProg,ep_bool_t bStorico)
{
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	int nPagine=0;
	ep_bool_t bOK=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa XAB Spedizione [%s] Storico : %d",szOrdProg,bStorico);
#endif

	sprintf(szPrintFile,"%s/%s.xab_A4",Cfg.szPathStampe,pszPrinterName);

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
		/*
		* La XAB viene stampata se si trova negli stati:
		*  (E) Evaso
		*  (X) XAB
		*  (D) Distinta
		*  (H) Storicizzato
		*/
		if(1 || IsXAB(DatiOrdine.szROSTATO)){
			/* Creazione del file di stampa */
			InitPrint(szPrintFile,MAX_RIGHE_XAB_A4,MAX_COLONNE_XAB_A4,Cfg.nPrimaRigaXAB_A4,Cfg.nUltimaRigaXAB_A4);

			if(!StampaXAB_A4(&DatiOrdine,bStorico)){
				bOK=FALSE;
			}
			/*
			* Chiudo il file di stampa
			*/
			nPagine=EndPrint(TRUE);

		} else {
			/*
			* La spedizione non e' in uno stato utile per la stampa XAB
			*/
			bOK=FALSE;
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Ordine [%s] in stato non corretto",szOrdProg);
#endif
		}
	} else {
		bOK=FALSE;
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Ordine [%s] non presente",szOrdProg);
#endif
	}
	if(bOK){
		PrintFileRaw(szPrintFile,pszPrinterName);
	}


	return bOK;
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
ep_bool_t StampaPackingListLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,ep_bool_t bStorico)
{
	DBresult *DBRes;
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	char szOrdProg[128];
	int nPagine=0;
	int nTuples;
	int nIndex;
	ep_bool_t bOK=TRUE;
	ep_bool_t bStampa=FALSE;
	char szRicOrd[128];
	char szRicArt[128];
	char szRicNote[128];
	char szRigProd[128];
	char szColProd[128];

	if(bStorico){
		strcpy(szRicOrd,  "ric_ord_stor");
		strcpy(szRicArt,  "ric_art_stor");
		strcpy(szRicNote, "ric_note_stor");
		strcpy(szRigProd, "rig_prod_stor");
		strcpy(szColProd, "col_prod_stor");
	} else {
		strcpy(szRicOrd, "ric_ord");
		strcpy(szRicArt, "ric_art");
		strcpy(szRicNote,"ric_note");
		strcpy(szRigProd,"rig_prod");
		strcpy(szColProd,"col_prod");
	}

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa Packing List Linea [%s - %s - %s - %s]",szNMCED,szTPSPE,szCDLIN,szCDVET);
#endif
	sprintf(szPrintFile,"%s/%s.PL_Linea_A4",Cfg.szPathStampe,pszPrinterName);

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from %s where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s'  and rocdve2='%s'  order by ordprog;",szRicOrd,Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET);
	if((nTuples=DBntuples(DBRes))){
		/* Creazione del file di stampa */
		InitPrint(szPrintFile,MAX_RIGHE_XAB_A4,MAX_COLONNE_XAB_A4,Cfg.nPrimaRigaXAB_A4,Cfg.nUltimaRigaXAB_A4);

		for(nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szOrdProg,DBgetvalue(DBRes,nIndex,0));
			if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
				/*
				* La XAB viene stampata se si trova negli stati:
				*  E si tratta di una stampa 
				*     - valorizzazione campo ROPRXAB)
				*     - aggiornamento stato)
				*  X,D si tratta di una ristampa
				*/
				/* rm : modifica temporanea */
				if(1 || IsXAB(DatiOrdine.szROSTATO)){

					if(!StampaXAB_A4(&DatiOrdine,bStorico)){
						bOK=FALSE;
					} else {
						/*
						* DA_FARE 
						* Se non si tratta di una ristampa aggiorno il progressivo XAB
						*/
						/* almeno un XAB da stampare */
						bStampa=TRUE;
					}

				} else {
					/*
					* La spedizione non e' in uno stato utile per la stampa XAB
					*/
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
			if(nIndex+1<nTuples){
				/*
				* C'e' un'altra spedizione da stampare
				* Eseguo un salto pagina senza azzerare la numerazione delle pagine
				*/
				EnableOldPage(FALSE);
				EnableNewPage(FALSE);
				SkipPage(TRUE,TRUE,TRUE);
				EnableOldPage(TRUE);
				EnableNewPage(TRUE);
			}
		}
		/*
		* Chiudo il file di stampa
		*/
		nPagine=EndPrint(TRUE);
		if(bStampa){
			PrintFileRaw(szPrintFile,pszPrinterName);
		}
	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);

	return bOK;
}

