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
#include <termio.h>
#include <sys/stat.h>
#include <glib.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <proc_list.h>
#include <picking.h>
#include <ep-common.h>
#include <ep-db.h>

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

BOOL IsXAB(char *szStato)
{
	switch(szStato[0]){
		case ORDINE_EVASO:
		case ORDINE_STAMPATA_XAB:
		case ORDINE_STAMPATA_DISTINTA:
		case ORDINE_FILE_INVIATO:
		case ORDINE_SPEDITO_HOST:
			return TRUE;
		break;
	}
	return FALSE;
}


/*
* Converte una data dal formato YYYYMMDD al formato DD/MM/YYYY
*/
char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY)
{
	char szYYYY[40];
	char szMM[40];
	char szDD[40];

	sprintf(szYYYY,"%4.4s",szDateYYYYMMDD);
	sprintf(szMM,"%2.2s",szDateYYYYMMDD+4);
	sprintf(szDD,"%2.2s",szDateYYYYMMDD+6);
	sprintf(szDateDDMMYYYY,"%s/%s/%s",szDD,szMM,szYYYY);

	return (szDateDDMMYYYY);
}


/*
* Converte una data dal formato YYYYMMDD al formato DD/MM/YY
*/
char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY)
{
	char szYY[40];
	char szMM[40];
	char szDD[40];

	sprintf(szYY,"%2.2s",szDateYYYYMMDD+2);
	sprintf(szMM,"%2.2s",szDateYYYYMMDD+4);
	sprintf(szDD,"%2.2s",szDateYYYYMMDD+6);
	sprintf(szDateDDMMYY,"%s/%s/%s",szDD,szMM,szYY);

	return (szDateDDMMYY);
}


/*
* InserisciDatiEveryPageXAB()
* Funzione per la gestione della intestazione e del fondo pagine di tutte le pagine XAB
* 02-01-2002 rm+st : Gestione EURO
*/
BOOL InserisciDatiEveryPageXAB(PDATIORDINE pDatiOrdine)
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
	BOOL bOK=TRUE;
	BOOL bEuro;
	DBresult *PGRes;

	/*
	* Decido se usare EURO o LIRE
	*/
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);

	DateDDMMYY(pDatiOrdine->szRODTBAM,szDataBAMC);
	DateDDMMYYYY(pDatiOrdine->szRODTBAM,szDataBAML);
	GetDate(time((long *)0),szDataStampa);
	/*
	* Stampa della causale del trasporto (da gestire decodifica da CDMOV)
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select  ELCDSCST from ttcs where elccdsoc='%s' and elccdmag='%s' and elccdcst='%s';", 
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		pDatiOrdine->szROCDMOV);
	if(DBntuples(PGRes)){
		strcpy(szDSCST,DBgetvalue(PGRes,0,0));
	} else {
		strcpy(szDSCST,"");
	}
	DBclear(PGRes);
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
	nAnno=atoi(LeftStr(GetDateYYYYMMDD(time((long *)0),szDateBuffer),4));


	sprintf(szAnno,"/%02d",nAnno%100);
	sprintf(szProg,"%07d",pDatiOrdine->nROPRXAB);
		
	InitData("EveryPage",pDatiOrdine->szROCDCLA, XABPos[F_DESTINATARIO0].nSR,     XABPos[F_DESTINATARIO0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPA, XABPos[F_DESTINATARIO1].nSR,     XABPos[F_DESTINATARIO1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPC, XABPos[F_DESTINATARIO2].nSR,     XABPos[F_DESTINATARIO2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLA, XABPos[F_DESTINATARIO3].nSR,     XABPos[F_DESTINATARIO3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLA, XABPos[F_DESTINATARIO4].nSR,     XABPos[F_DESTINATARIO4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLA, XABPos[F_DESTINATARIO5].nSR,     XABPos[F_DESTINATARIO5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLA, XABPos[F_DESTINATARIO6].nSR,     XABPos[F_DESTINATARIO6].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLA, XABPos[F_DESTINATARIO7].nSR,     XABPos[F_DESTINATARIO7].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLA, XABPos[F_DESTINATARIO8].nSR,     XABPos[F_DESTINATARIO8].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, XABPos[F_NUMEROINALTO].nSR,      XABPos[F_NUMEROINALTO].nSC,      FALSE);
	InitData("EveryPage",szDataBAML,             XABPos[F_DATAXAB].nSR,           XABPos[F_DATAXAB].nSC,           FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDMOV, XABPos[F_DOCUMENTOINTERNO0].nSR, XABPos[F_DOCUMENTOINTERNO0].nSC, FALSE);
	InitData("EveryPage",szDataBAMC,             XABPos[F_DOCUMENTOINTERNO1].nSR, XABPos[F_DOCUMENTOINTERNO1].nSC, FALSE);
	InitData("EveryPage",szDataStampa,           XABPos[F_DOCUMENTOINTERNO2].nSR, XABPos[F_DOCUMENTOINTERNO2].nSC, FALSE);
	InitData("EveryPage",szProg,                 XABPos[F_NUMEROINBASSO].nSR,     XABPos[F_NUMEROINBASSO].nSC,     FALSE);
	InitData("EveryPage",szAnno,                 XABPos[F_NUMEROINBASSO].nSR,     XABPos[F_NUMEROINBASSO].nSC+7,   FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, XABPos[F_SPEDIZIONE].nSR,        XABPos[F_SPEDIZIONE].nSC,        FALSE);
	InitData("EveryPage",LeftStr(szDSCST,20),    XABPos[F_CAUSALE].nSR,           XABPos[F_CAUSALE].nSC,           FALSE);
	InitData("EveryPage",Cfg.szContenuto,        XABPos[F_CONTENUTO].nSR,         XABPos[F_CONTENUTO].nSC,         FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDRID, XABPos[F_DESTINAZIONE0].nSR,     XABPos[F_DESTINAZIONE0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLI, XABPos[F_DESTINAZIONE1].nSR,     XABPos[F_DESTINAZIONE1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLI, XABPos[F_DESTINAZIONE2].nSR,     XABPos[F_DESTINAZIONE2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLI, XABPos[F_DESTINAZIONE3].nSR,     XABPos[F_DESTINAZIONE3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLI, XABPos[F_DESTINAZIONE4].nSR,     XABPos[F_DESTINAZIONE4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLI, XABPos[F_DESTINAZIONE5].nSR,     XABPos[F_DESTINAZIONE5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLI, XABPos[F_DESTINAZIONE6].nSR,     XABPos[F_DESTINAZIONE6].nSC,     FALSE);
	InitData("EveryPage",szAssegno,              XABPos[F_ASSEGNO].nSR,           XABPos[F_ASSEGNO].nSC,           FALSE);
	InitData("EveryPage",szAspetto,              XABPos[F_ASPETTOBENI].nSR,       XABPos[F_ASPETTOBENI].nSC,       FALSE);

	return bOK;
}


/*
* InserisciDatiOldPageXAB()
* Funzione per la gestione della pagina precedente in caso di salto pagina
*/
void InserisciDatiOldPageXAB(void)
{
	InitData("OldPage",Cfg.szSegue,Cfg.nUltimaRigaXAB+1,XABPos[F_AUTOREXAB].nSC,FALSE);
}


/*
* InserisciDatiNewPageXAB()
* Funzione per la gestione della pagina attuale in caso di salto pagina
*/
void InserisciDatiNewPageXAB(PDATIORDINE pDatiOrdine)
{
	char szString[127];
	char szDataBAM[80];

	DateDDMMYY(pDatiOrdine->szRODTBAM,szDataBAM);
	sprintf(szString,"%s %s DEL %s",Cfg.szSegueBAM,pDatiOrdine->szROPRDOC,szDataBAM);
	InitData("NewPage",szString,Cfg.nPrimaRigaXAB-3,XABPos[F_AUTOREXAB].nSC,FALSE);
	sprintf(szString,"%s @@",Cfg.szFoglio);
	InitData("NewPage",szString,Cfg.nPrimaRigaXAB-2,XABPos[F_AUTOREXAB].nSC,FALSE);
}

/*
* InserisciDatiLastPageXAB()
* Funzione per la gestione dell'ultima pagina 
* 02-01-2002 rm+st : Gestione EURO
*/
BOOL InserisciDatiLastPageXAB(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot)
{
	BOOL bOK=TRUE;
	BOOL bFound=FALSE;
	BOOL bEuro=FALSE;
	DBresult *PGRes;

	char szDSTSP[80];
	char szString[80];
	char szBuffer[80];
	char szFLNOT[80];

	/* 
	* Ultima riga 
	*/
	sprintf(szString,"%s @@ XAB",Cfg.szFineBolla);
	InitData("LastPage",szString,Cfg.nUltimaRigaXAB+1,XABPos[F_AUTOREXAB].nSC,FALSE);
	/* 
	* Quantita' 
	*/
	sprintf(szString,"%6d",nQtaTot);
	InitData("LastPage",szString,XABPos[F_QUANTITAXAB].nSR,XABPos[F_QUANTITAXAB].nSC,FALSE);
	NumberToLetter(nQtaTot,szBuffer);
	sprintf(szString,"%6s",szBuffer);
	InitData("LastPage",szString,XABPos[F_QUANTITAXAB].nSR+1,XABPos[F_QUANTITAXAB].nSC,FALSE);
	/* 
	* Peso in Kg. 
	*/
	if (pDatiOrdine->nROPSREA){
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSREA/(float)1000));
	} else {
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSPRE/(float)1000));
	}

	InitData("LastPage",szString,XABPos[F_PESO].nSR,XABPos[F_PESO].nSC,FALSE);
	/* 
	* Numero colli 
	*/
	sprintf(szString,"%6d",pDatiOrdine->nRONMCLL);
	InitData("LastPage",szString,XABPos[F_NUMCOLLI].nSR,XABPos[F_NUMCOLLI].nSC,FALSE);

	/* decido se usare EURO o LIRE */
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);

	if(bEuro){
		NotazioneConVirgola(nPrzTot,szString);
	} else {
		NotazioneConPunti(nPrzTot,szString);
	}

	InitData("LastPage",szString,XABPos[F_VARIAZDEST].nSR,XABPos[F_VARIAZDEST].nSC,FALSE);
	/* -- Vettori -- */

	if (!DatiVettore(pDatiOrdine,pDatiOrdine->szROCDVE1,F_VETTORI0,F_VETTORI1,F_VETTORI2)) bOK=FALSE;
	if (!DatiVettore(pDatiOrdine,pDatiOrdine->szROCDVE2,F_VETTORI3,F_VETTORI4,F_VETTORI5)) bOK=FALSE;

//	/*
//	* rm 17-03-2004 : gestione descrizione linea diviso per cedola 
//	*/
//	/*
//	* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
//	*/
//	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='%s';", 
//		pDatiOrdine->szROCDSOC, 
//		pDatiOrdine->szROCDMAG, 
//		pDatiOrdine->szRONMCED, 
//		pDatiOrdine->szROTPSPE,
//		pDatiOrdine->szROCDLIN);
//	if(DBntuples(PGRes)){
//		strcpy(szDSTSP,DBgetvalue(PGRes,0,0));
//		bFound=TRUE;
//	} else {
//		bFound=FALSE;
//	}
//	DBclear(PGRes);
//
//	if(!bFound){
//		/* rm 27-09-2004 : primo giro bis: cerco con cdlin = '' */
//		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='';", 
//			pDatiOrdine->szROCDSOC, 
//			pDatiOrdine->szROCDMAG, 
//			pDatiOrdine->szRONMCED, 
//			pDatiOrdine->szROTPSPE);
//		if(DBntuples(PGRes)){
//			strcpy(szDSTSP,DBgetvalue(PGRes,0,0));
//			bFound=TRUE;
//		} else {
//			bFound=FALSE;
//		}
//		DBclear(PGRes);
//	}


	/*
	* lg 27-02-2012 : ora il dato deve essere preso da ric_ord. Il dato deve essere precedentemente settato
	*                 in fase di lancio ordine o stampa etichette
	*                 altrimenti lo ricalcolo?
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select rodstsp,roflnot from ric_ord where ordprog='%s';", 
		pDatiOrdine->szORDPROG);
	if(DBntuples(PGRes)){
		strcpy(szDSTSP,DBgetvalue(PGRes,0,0));
		strcpy(szFLNOT,DBgetvalue(PGRes,0,1));
		if (szFLNOT[0]==NOTA_CORRIERE_SETTATA) {
			bFound=TRUE;
		} else {
			bFound=FALSE;
		}
	} else {
	}
	DBclear(PGRes);



	if(!bFound){
		/*
		* Chiave per la tabella ttts:
		* 	societa'+magazzino+tipo spedizione
		*/
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdstsp from ttts where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s';",
			pDatiOrdine->szROCDSOC,
			pDatiOrdine->szROCDMAG,
			pDatiOrdine->szROTPSPE);
		if(DBntuples(PGRes)){
			strcpy(szDSTSP,DBgetvalue(PGRes,0,0));
			bFound=TRUE;
		} else {
	#ifdef TRACE
			trace_out_vstr(1,"Chiave %s non presente nella tabella ttts",pDatiOrdine->szROTPSPE);
	#endif
			bOK=FALSE;
			bFound=FALSE;
		}
		DBclear(PGRes);
	}
	if(bFound){
		InitData("LastPage",szDSTSP,XABPos[F_ANNOTAZIONI].nSR,XABPos[F_ANNOTAZIONI].nSC,FALSE);
	}

	return bOK;
}


BOOL DatiVettore(PDATIORDINE pDatiOrdine,char *szCDVET,int nPos1,int nPos2,int nPos3)
{
	BOOL bOK=TRUE;
	DBresult *PGRes;

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdsvet,elcinvet,elclovet from ttve where elccdsoc='%s' and elccdmag='%s' and elccdvet='%s';",
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		szCDVET);
	if(DBntuples(PGRes)){
		InitData("LastPage",DBgetvalue(PGRes,0,0),XABPos[nPos1].nSR,XABPos[nPos1].nSC,FALSE);
		InitData("LastPage",DBgetvalue(PGRes,0,1),XABPos[nPos2].nSR,XABPos[nPos2].nSC,FALSE);
		InitData("LastPage",DBgetvalue(PGRes,0,2),XABPos[nPos3].nSR,XABPos[nPos3].nSC,FALSE);
	} else {
#ifdef TRACE
		trace_out_vstr(1,"Dati vettore non trovati nella tabella ttve. Chiave %s",szCDVET);
#endif
		bOK=FALSE;
	}
	DBclear(PGRes);

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
BOOL StampaXAB(PDATIORDINE pDatiOrdine,BOOL bStorico)
{
	DBresult *PGRes=NULL;
	DBresult *PGResCat=NULL;
	DBresult *PGResRA=NULL;
	BOOL bOK=TRUE;
	char szTmpBuf[80];
	char szBuffer[128];
	char szRiga[MAX_COLONNE_XAB];
	char szValueField[80];
	char szRPCDPRO[80];
	int nRPQTSPE;
	char szPRDSTIT[128];
	char szPRDSAUT[128];
	char szRACDIVA[128];
	int nRAPZPRO;
	int nValoreOrdine;
	int nNumeroPezzi;
	int nAnno;
	int nTuples;
	int nIndex;
	BOOL bEuro=FALSE;
	BOOL bRistampa=TRUE;
	char szRicOrd[128];
	char szRicArt[128];
	char szRicNote[128];
	char szRigProd[128];
	char szColProd[128];

#ifdef TRACE
	trace_out_vstr_date(1,"Stampa XAB [%s]",pDatiOrdine->szORDPROG);
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

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select sum(rp.rpqtspe),sum(rp.rpqtspe * ra.%s) from %s rp,%s ra where rp.rpcdpro=ra.racdpro and ra.ordprog=rp.ordprog and rp.ordprog='%s';",szValueField,szRigProd,szRicArt,pDatiOrdine->szORDPROG);
	if(DBntuples(PGRes)){
		nNumeroPezzi=atoi(DBgetvalue(PGRes,0,0));
		nValoreOrdine=atoi(DBgetvalue(PGRes,0,1));
	} else {
		nValoreOrdine=0;
		nNumeroPezzi=0;
		bOK=FALSE;
	}
	DBclear(PGRes);

	if(!bOK){
		return FALSE;
	}

	/* 
	* se non si tratta di una ristampa 
	* incremento il progressivo XAB
	*/
	if(!pDatiOrdine->nROPRXAB){
		nAnno=atoi(LeftStr(GetDateYYYYMMDD(time((long *)0),szDateBuffer),4));
		/* 
		* Progressivo XAB 
		* se si tratta della prima volta che si stampa l'XAB di questo ordine
		* allora si calcolano e assegnano i valori e le sequenze relative
		*/
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select nextval('sequence_xab');");
		if(DBntuples(PGRes)){
			pDatiOrdine->nROPRXAB=atoi(DBgetvalue(PGRes,0,0));
			pDatiOrdine->nROAAXAB=nAnno%100;
			strcpy(pDatiOrdine->szRODTXAB,GetDateYYYYMMDD(time((long *)0),szDateBuffer));
		} else {
			bOK=FALSE;
		}
		DBclear(PGRes);
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

	InserisciDatiEveryPageXAB(pDatiOrdine);
	InserisciDatiNewPageXAB(pDatiOrdine);
	InserisciDatiOldPageXAB();
	InserisciDatiLastPageXAB(pDatiOrdine,nNumeroPezzi,nValoreOrdine);


	/* righe di produzione */
	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select rpcdpro,sum(rpqtspe) from %s where ordprog='%s' group by rpcdpro;",szRigProd,pDatiOrdine->szORDPROG);
	if((nTuples=DBntuples(PGRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szRPCDPRO,DBgetvalue(PGRes,nIndex,0));
			nRPQTSPE=atoi(DBgetvalue(PGRes,nIndex,1));

			/* catalogo */
			PGResCat=DBExecQuery(Cfg.nDebugVersion>2,"select prdstit,prdsaut from catalogo where prcdpro='%s';",szRPCDPRO);
			if(DBntuples(PGResCat)){
				strcpy(szPRDSTIT,DBgetvalue(PGResCat,0,0));
				strcpy(szPRDSAUT,DBgetvalue(PGResCat,0,1));
			} else {
				strcpy(szPRDSTIT,"ERRORE");
				strcpy(szPRDSAUT,"ERRORE");
			}
			DBclear(PGResCat);

			/* righe ricevute */
			PGResRA=DBExecQuery(Cfg.nDebugVersion>2,"select racdiva,%s from %s where ordprog='%s' and racdpro='%s';",szValueField,szRicArt,pDatiOrdine->szORDPROG,szRPCDPRO);
			if(DBntuples(PGResRA)){
				strcpy(szRACDIVA,DBgetvalue(PGResRA,0,0));
				nRAPZPRO=atoi(DBgetvalue(PGResRA,0,1));
			} else {
				strcpy(szRACDIVA,"ERRORE");
				nRAPZPRO=0;
			}
			DBclear(PGResRA);

			sprintf(szBuffer,"%9.9s ",    RightStr(szRPCDPRO,9)); strcpy(szRiga,szBuffer);
			sprintf(szBuffer,"%11.11s",   szPRDSAUT); strcat(szRiga,szBuffer);
			sprintf(szBuffer,"%24.24s ",  szPRDSTIT); strcat(szRiga,szBuffer);
			sprintf(szBuffer,"%7d   ",    nRPQTSPE); strcat(szRiga,szBuffer);
			sprintf(szBuffer,"%7.7s    ", NumberToLetter(nRPQTSPE,szTmpBuf)); strcat(szRiga,szBuffer);
			/*
			* 02-01-2002 rm+st : Gestione EURO
			*/
			if(bEuro){
				sprintf(szBuffer,"%9.9s ",    NotazioneConVirgola(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
			} else {
				sprintf(szBuffer,"%9.9s ",    NotazioneConPunti(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
			}
			sprintf(szBuffer,"%3.3s",     szRACDIVA); strcat(szRiga,szBuffer);

			PrintRow(szRiga,1,FALSE);
		}
	} else {
		bOK=FALSE;
	}
	DBclear(PGRes);


	/*
	* Ciclo di stampa delle note.
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select RNFRTXT from %s where ordprog='%s' and rntptxt='%s' order by rnnmtxt;",szRicNote, pDatiOrdine->szORDPROG,Cfg.szTipoNotaXAB);
	if((nTuples=DBntuples(PGRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			sprintf(szRiga,"        %s",DBgetvalue(PGRes,nIndex,0));
			PrintRow(szRiga,2,FALSE);
		}
	}
	DBclear(PGRes);

	/* 
	* aggiorno il progressivo XAB
	*/
	if(bOK && !bRistampa){
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"update %s set roprxab=%d,roaaxab=%d,rodtxab='%s',rostato='%c' where ordprog='%s';",
			szRicOrd,
			pDatiOrdine->nROPRXAB,
			pDatiOrdine->nROAAXAB,
			pDatiOrdine->szRODTXAB,
			ORDINE_STAMPATA_XAB,
			pDatiOrdine->szORDPROG);
		if((DBresultStatus(PGRes)!=PGRES_COMMAND_OK)){
			bOK=FALSE;
		}
		DBclear(PGRes);
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
BOOL StampaXABSpedizione(char *pszPrinterName,char *szOrdProg,BOOL bStorico)
{
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	int nPagine=0;
	BOOL bOK=TRUE;

#ifdef TRACE
	trace_out_vstr_date(1,"Stampa XAB Spedizione [%s] Storico : %d",szOrdProg,bStorico);
#endif

	sprintf(szPrintFile,"%s/%s.xab",Cfg.szPathStampe,pszPrinterName);

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
		/*
		* La XAB viene stampata se si trova negli stati:
		*  (E) Evaso
		*  (X) XAB
		*  (D) Distinta
		*  (H) Storicizzato
		*/
		if(IsXAB(DatiOrdine.szROSTATO)){
			/* Creazione del file di stampa */
			InitPrint(szPrintFile,MAX_RIGHE_XAB,MAX_COLONNE_XAB,Cfg.nPrimaRigaXAB,Cfg.nUltimaRigaXAB);

			if(!StampaXAB(&DatiOrdine,bStorico)){
				bOK=FALSE;
			}
			/*
			* Chiudo il file di stampa
			*/
			nPagine=EndPrint(FALSE);

		} else {
			/*
			* La spedizione non e' in uno stato utile per la stampa XAB
			*/
			bOK=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] in stato non corretto",szOrdProg);
#endif
		}
	} else {
		bOK=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] non presente",szOrdProg);
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
BOOL StampaXABLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,BOOL bStorico)
{
	DBresult *PGRes;
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	char szOrdProg[128];
	int nPagine=0;
	int nTuples;
	int nIndex;
	BOOL bOK=TRUE;
	BOOL bStampa=FALSE;
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
	trace_out_vstr_date(1,"Stampa XAB Linea [%s - %s - %s - %s]",szNMCED,szTPSPE,szCDLIN, szCDVET);
#endif
	sprintf(szPrintFile,"%s/%s.xab",Cfg.szPathStampe,pszPrinterName);

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from %s where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s' order by ordprog;",szRicOrd,Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET);
	if((nTuples=DBntuples(PGRes))){
		/* Creazione del file di stampa */
		InitPrint(szPrintFile,MAX_RIGHE_XAB,MAX_COLONNE_XAB,Cfg.nPrimaRigaXAB,Cfg.nUltimaRigaXAB);

		for(nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szOrdProg,DBgetvalue(PGRes,nIndex,0));
			if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
				/*
				* La XAB viene stampata se si trova negli stati:
				*  E si tratta di una stampa 
				*     - valorizzazione campo ROPRXAB)
				*     - aggiornamento stato)
				*  X,D si tratta di una ristampa
				*/
				if(IsXAB(DatiOrdine.szROSTATO)){

					if(!StampaXAB(&DatiOrdine,bStorico)){
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
				SkipPage(TRUE,TRUE,FALSE);
				EnableOldPage(TRUE);
				EnableNewPage(TRUE);
			}
		}
		/*
		* Chiudo il file di stampa
		*/
		nPagine=EndPrint(FALSE);
		if(bStampa){
			PrintFileRaw(szPrintFile,pszPrinterName);
		}
	} else {
		bOK=FALSE;
	}

	return bOK;
}

