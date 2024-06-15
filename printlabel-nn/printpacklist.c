/*
* printxab.c : Stampa PL
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
* 25-02-2002 rm : manca update ordine al termine della stampa PL (prxab,dtxab ...)
* 09-10-2003 rm : Stampa PL linea con discriminante per CEDOLA
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
* Funzioni per la stampa della PL
********************************************************************************************
*/
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
* InserisciDatiEveryPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo)
* Funzione per la gestione della intestazione e del fondo pagine di tutte le pagine XAB
* 02-01-2002 rm+st : Gestione EURO
*/
BOOL InserisciDatiEveryPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo)
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
		
	InitData("EveryPage",pDatiOrdine->szROCDCLA, PLPos_A4[F_DESTINATARIO0].nSR,     PLPos_A4[F_DESTINATARIO0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPA, PLPos_A4[F_DESTINATARIO1].nSR,     PLPos_A4[F_DESTINATARIO1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPC, PLPos_A4[F_DESTINATARIO2].nSR,     PLPos_A4[F_DESTINATARIO2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLA, PLPos_A4[F_DESTINATARIO3].nSR,     PLPos_A4[F_DESTINATARIO3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLA, PLPos_A4[F_DESTINATARIO4].nSR,     PLPos_A4[F_DESTINATARIO4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLA, PLPos_A4[F_DESTINATARIO5].nSR,     PLPos_A4[F_DESTINATARIO5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLA, PLPos_A4[F_DESTINATARIO6].nSR,     PLPos_A4[F_DESTINATARIO6].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLA, PLPos_A4[F_DESTINATARIO7].nSR,     PLPos_A4[F_DESTINATARIO7].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLA, PLPos_A4[F_DESTINATARIO8].nSR,     PLPos_A4[F_DESTINATARIO8].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, PLPos_A4[F_NUMEROINALTO].nSR,      PLPos_A4[F_NUMEROINALTO].nSC,      FALSE);
	InitData("EveryPage",szDataBAML,             PLPos_A4[F_DATAXAB].nSR,           PLPos_A4[F_DATAXAB].nSC,           FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDMOV, PLPos_A4[F_DOCUMENTOINTERNO0].nSR, PLPos_A4[F_DOCUMENTOINTERNO0].nSC, FALSE);
	InitData("EveryPage",szDataBAMC,             PLPos_A4[F_DOCUMENTOINTERNO1].nSR, PLPos_A4[F_DOCUMENTOINTERNO1].nSC, FALSE);
	InitData("EveryPage",szDataStampa,           PLPos_A4[F_DOCUMENTOINTERNO2].nSR, PLPos_A4[F_DOCUMENTOINTERNO2].nSC, FALSE);
	InitData("EveryPage",szProg,                 PLPos_A4[F_NUMEROINBASSO].nSR,     PLPos_A4[F_NUMEROINBASSO].nSC,     FALSE);
	InitData("EveryPage",szAnno,                 PLPos_A4[F_NUMEROINBASSO].nSR,     PLPos_A4[F_NUMEROINBASSO].nSC+7,   FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, PLPos_A4[F_SPEDIZIONE].nSR,        PLPos_A4[F_SPEDIZIONE].nSC,        FALSE);
	InitData("EveryPage",LeftStr(szDSCST,20),    PLPos_A4[F_CAUSALE].nSR,           PLPos_A4[F_CAUSALE].nSC,           FALSE);
	InitData("EveryPage",Cfg.szContenuto,        PLPos_A4[F_CONTENUTO].nSR,         PLPos_A4[F_CONTENUTO].nSC,         FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDRID, PLPos_A4[F_DESTINAZIONE0].nSR,     PLPos_A4[F_DESTINAZIONE0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLI, PLPos_A4[F_DESTINAZIONE1].nSR,     PLPos_A4[F_DESTINAZIONE1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLI, PLPos_A4[F_DESTINAZIONE2].nSR,     PLPos_A4[F_DESTINAZIONE2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLI, PLPos_A4[F_DESTINAZIONE3].nSR,     PLPos_A4[F_DESTINAZIONE3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLI, PLPos_A4[F_DESTINAZIONE4].nSR,     PLPos_A4[F_DESTINAZIONE4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLI, PLPos_A4[F_DESTINAZIONE5].nSR,     PLPos_A4[F_DESTINAZIONE5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLI, PLPos_A4[F_DESTINAZIONE6].nSR,     PLPos_A4[F_DESTINAZIONE6].nSC,     FALSE);
	InitData("EveryPage",szAssegno,              PLPos_A4[F_ASSEGNO].nSR,           PLPos_A4[F_ASSEGNO].nSC,           FALSE);
	InitData("EveryPage",szAspetto,              PLPos_A4[F_ASPETTOBENI].nSR,       PLPos_A4[F_ASPETTOBENI].nSC,       FALSE);
	
	sprintf(szString,"PACKING LIST ORDINE %s COLLO %d",pDatiOrdine->szROPRDOC,pDatiCollo->nCPNMCOL);
	InitData("EveryPage",szString,Cfg.nPrimaRigaPL_A4-3,PLPos_A4[F_AUTOREXAB].nSC,FALSE);
	sprintf(szString,"%s @@",Cfg.szFoglio);
	InitData("EveryPage",szString,Cfg.nPrimaRigaPL_A4-2,PLPos_A4[F_AUTOREXAB].nSC,FALSE);

	return bOK;
}


/*
* InserisciDatiOldPagePL_PF()
* Funzione per la gestione della pagina precedente in caso di salto pagina
*/
void InserisciDatiOldPagePL_PF(void)
{
	InitData("OldPage",Cfg.szSegue,Cfg.nUltimaRigaPL_A4+1,PLPos_A4[F_AUTOREXAB].nSC,FALSE);
}


/*
* InserisciDatiNewPagePL_PF(PDATIORDINE pDatiOrdine)
* Funzione per la gestione della pagina attuale in caso di salto pagina
*/
void InserisciDatiNewPagePL_PF(PDATIORDINE pDatiOrdine)
{
}

/*
* InserisciDatiLastPagePL(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,int nQtaTot,int nPrzTot)
* Funzione per la gestione dell'ultima pagina 
* 02-01-2002 rm+st : Gestione EURO
*/
BOOL InserisciDatiLastPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,int nQtaTot,int nPrzTot)
{
	BOOL bOK=TRUE;
	BOOL bFound=FALSE;
	BOOL bEuro=FALSE;
	DBresult *PGRes;

	char szDSTSP[80];
	char szString[80];
	char szFLNOT[80];

	/* 
	* Ultima riga
	* LG: Non serve nella packing list, lascio per il futuro
	*/
	//sprintf(szString,"  %s @@ PL",Cfg.szFineBolla);
	//InitData("LastPage",szString,Cfg.nUltimaRigaXAB_A4+1,PLPos_A4[F_AUTOREXAB].nSC,FALSE);
	/* 
	* Quantita' 
	*/
	sprintf(szString,"%6d",nQtaTot);
	InitData("LastPage",szString,PLPos_A4[F_QUANTITAXAB].nSR+1,PLPos_A4[F_QUANTITAXAB].nSC,FALSE);
	/* 
	* Peso in Kg. 
	*/
	if (pDatiCollo->nCPPSREA){
		sprintf(szString,"%8.3f",(float)(pDatiCollo->nCPPSREA/(float)1000));
	} else {
		sprintf(szString,"%8.3f",(float)(pDatiCollo->nCPPSPRE/(float)1000));
	}
	InitData("LastPage",szString,PLPos_A4[F_PESO].nSR,PLPos_A4[F_PESO].nSC,FALSE);
	/* 
	* Numero colli e' sempre 1 nella packing list collo!!!
	*/
	sprintf(szString,"     1");
	InitData("LastPage",szString,PLPos_A4[F_NUMCOLLI].nSR,PLPos_A4[F_NUMCOLLI].nSC,FALSE);

	/* decido se usare EURO o LIRE */
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);
	if(bEuro){
		NotazioneConVirgola(nPrzTot,szString);
	} else {
		NotazioneConPunti(nPrzTot,szString);
	}
	InitData("LastPage",szString,PLPos_A4[F_VARIAZDEST].nSR,PLPos_A4[F_VARIAZDEST].nSC,FALSE);
	
	/* -- Vettori -- */
	if (!DatiVettorePL_PF(pDatiOrdine,pDatiOrdine->szROCDVE1,F_VETTORI0,F_VETTORI1,F_VETTORI2)) bOK=FALSE;
	if (!DatiVettorePL_PF(pDatiOrdine,pDatiOrdine->szROCDVE2,F_VETTORI3,F_VETTORI4,F_VETTORI5)) bOK=FALSE;

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
		InitData("LastPage",szDSTSP,PLPos_A4[F_ANNOTAZIONI].nSR,PLPos_A4[F_ANNOTAZIONI].nSC,FALSE);
	}

	return bOK;
}


BOOL DatiVettorePL_PF(PDATIORDINE pDatiOrdine,char *szCDVET,int nPos1,int nPos2,int nPos3)
{
	BOOL bOK=TRUE;
	DBresult *PGRes;

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdsvet,elcinvet,elclovet from ttve where elccdsoc='%s' and elccdmag='%s' and elccdvet='%s';",
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		szCDVET);
	if(DBntuples(PGRes)){
		InitData("LastPage",DBgetvalue(PGRes,0,0),PLPos_A4[nPos1].nSR,PLPos_A4[nPos1].nSC,FALSE);
		InitData("LastPage",DBgetvalue(PGRes,0,1),PLPos_A4[nPos2].nSR,PLPos_A4[nPos2].nSC,FALSE);
		InitData("LastPage",DBgetvalue(PGRes,0,2),PLPos_A4[nPos3].nSR,PLPos_A4[nPos3].nSC,FALSE);
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
*
* StampaPL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,BOOL bStorico)
*
* Stampa la packing list del collo in oggetto
*
* Parametri di ingresso:
* 
* pDatiOrdine : Dati Ordine da stampare
* pDatiCollo :  Dati Collo da stampare
*  
* Ritorna:
*  in caso di errore:   FALSE
*  in caso di successo: TRUE 
*
*/
BOOL StampaPL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,BOOL bStorico)
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
	int nTuples;
	int nIndex;
	BOOL bEuro=FALSE;
	char szRicOrd[128];
	char szRicArt[128];
	char szRicNote[128];
	char szRigProd[128];
	char szColProd[128];

	/* Decido se usare EURO o LIRE */
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

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select sum(rp.rpqtspe),sum(rp.rpqtspe * ra.%s) from %s rp,%s ra where rp.rpcdpro=ra.racdpro and ra.ordprog=rp.ordprog and rp.ordprog='%s' and rp.rpnmcol=%d;",szValueField,szRigProd,szRicArt,pDatiOrdine->szORDPROG,pDatiCollo->nCPNMCOL);
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

	/* pulizia pagine */
	DeleteData("FirstPage");
	DeleteData("LastPage");
	DeleteData("OldPage");
	DeleteData("EveryPage");
	DeleteData("NewPage");

	InserisciDatiEveryPagePL_PF(pDatiOrdine,pDatiCollo);
	/* LG: Non serve ma la lascio per il futuro, casomai */
	//InserisciDatiNewPagePL_PF(pDatiOrdine);
	InserisciDatiOldPagePL_PF();
	InserisciDatiLastPagePL_PF(pDatiOrdine,pDatiCollo,nNumeroPezzi,nValoreOrdine);

	/* righe di produzione */
	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select rpcdpro,sum(rpqtspe) from %s where ordprog='%s' and rpnmcol=%d group by rpcdpro;",szRigProd,pDatiOrdine->szORDPROG,pDatiCollo->nCPNMCOL);
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
			/* Formattazione della riga */
			sprintf(szBuffer,"   %-9.9s   ",RightStr(szRPCDPRO,9)); strcpy(szRiga,szBuffer);
			sprintf(szBuffer,"%-10.10s ",szPRDSAUT); strcat(szRiga,szBuffer);
			sprintf(szBuffer,"%-29.29s ",szPRDSTIT); strcat(szRiga,szBuffer);
			sprintf(szBuffer,"%10d   ",nRPQTSPE); strcat(szRiga,szBuffer);
			/* Gestione EURO */
			if(bEuro){
				sprintf(szBuffer,"%9.9s ",NotazioneConVirgola(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
			} else {
				sprintf(szBuffer,"%9.9s ",NotazioneConPunti(nRAPZPRO,szTmpBuf)); strcat(szRiga,szBuffer);
			}
			sprintf(szBuffer,"%3.3s",szRACDIVA); strcat(szRiga,szBuffer);
			/* Inserimento della riga */
			PrintRow(szRiga,1,TRUE);
		}
	} else {
		bOK=FALSE;
	}
	DBclear(PGRes);

	return bOK;
}

/*
* Funzione di stampa della packing_list per un singolo collo
* Parametri di ingresso:
*  pszPrinterName:        nome della stampante
*  szOrdProg+nCollo:      chiave
* Ritorno:
*  TRUE in caso di successo
*	 FALSE in caso di insuccesso
*/
int CreatePackingListColloPrefincato(char *pszPrinterName,char *szOrdProg,int nCollo,char *szFile,BOOL bStorico)
{
	DATIORDINE DatiOrdine;
	DATICOLLO DatiCollo;
	char szPrintFile[128];
	int nPagine=0;
	int nRC=0;

#ifdef TRACE
	trace_out_vstr_date(1,"Stampa Packing List Prefincato [%s,%d] Storico : %d",szOrdProg,nCollo,bStorico);
#endif

	sprintf(szPrintFile,"%s/%s",Cfg.szPathStampe,szFile);

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico) && GetDatiCollo (szOrdProg, nCollo,&DatiCollo, bStorico) ){
		/* Creazione del file di stampa */
		InitPrint(szPrintFile,MAX_RIGHE_XAB_A4,MAX_COLONNE_XAB_A4,Cfg.nPrimaRigaPL_A4,Cfg.nUltimaRigaPL_A4);
		/* Stampa del file*/
		if(!StampaPL_PF(&DatiOrdine,&DatiCollo,bStorico)){
			nRC=1;
		}
		/* Chiudo il file di stampa	*/
		nPagine=EndPrint(TRUE);
	} else {
		nRC=1;
#ifdef TRACE
		trace_out_vstr_date(1,"Collo [%s,%d] non presente",szOrdProg,nCollo);
#endif
	}
	return nRC;
}

/*
* Funzione di stampa della packing_list per un singolo collo
* Parametri di ingresso:
*  pszPrinterName:        nome della stampante
*  szOrdProg+nCollo:      chiave
* Ritorno:
*  TRUE in caso di successo
*	 FALSE in caso di insuccesso
*/
BOOL StampaPackingListColloPrefincato(char *pszPrinterName,char *szOrdProg,int nCollo,BOOL bStorico)
{
	DATIORDINE DatiOrdine;
	DATICOLLO DatiCollo;
	char szPrintFile[128];
	int nPagine=0;
	BOOL bOK=TRUE;

#ifdef TRACE
	trace_out_vstr_date(1,"Stampa Packing List Prefincato [%s,%d] Storico : %d",szOrdProg,nCollo,bStorico);
#endif

	sprintf(szPrintFile,"%s/%s.plc_pf_%s_%03d",Cfg.szPathStampe,pszPrinterName,szOrdProg,nCollo);

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico) && GetDatiCollo (szOrdProg, nCollo,&DatiCollo, bStorico) ){
		/* Creazione del file di stampa */
		InitPrint(szPrintFile,MAX_RIGHE_XAB_A4,MAX_COLONNE_XAB_A4,Cfg.nPrimaRigaPL_A4,Cfg.nUltimaRigaPL_A4);
		/* Stampa del file*/
		if(!StampaPL_PF(&DatiOrdine,&DatiCollo,bStorico)){
			bOK=FALSE;
		}
		/* Chiudo il file di stampa	*/
		nPagine=EndPrint(TRUE);
	} else {
		bOK=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Collo [%s,%d] non presente",szOrdProg,nCollo);
#endif
	}
	if(bOK){
		local_PrintFile(szPrintFile,pszPrinterName,NULL,FALSE,0);
	}
	return bOK;
}

BOOL StampaPackingListBollaPrefincato(char *pszPrinterName,char *szOrdProg)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC=TRUE;
	int nIndex;
	int nTuples;
	BOOL bRetVal=FALSE;
	DBresult *PGResColli;
	DBresult *PGRes;
	BOOL bStorico=FALSE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];

	strcpy(szRicOrd,"ric_ord");
	strcpy(szOrdProd,"ord_prod");
	strcpy(szColProd,"col_prod");
	strcpy(szRigProd,"rig_prod");

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from ric_ord where ordprog='%s';",szOrdProg);
	if(DBntuples(PGRes)==0){
		DBclear(PGRes);
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from ric_ord_stor where ordprog='%s';",szOrdProg);
		if(DBntuples(PGRes)){
			bStorico=TRUE;
			strcpy(szRicOrd,"ric_ord_stor");
			strcpy(szOrdProd,"ord_prod_stor");
			strcpy(szColProd,"col_prod_stor");
			strcpy(szRigProd,"rig_prod_stor");
		}
	}
	DBclear(PGRes);

	/*
	* Apertura del file per la stampa
	*/
	//sprintf(szFileName,"%s/pl_pf_%s",Cfg.szPathStampe,szOrdProg);
	//if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {

	PGResColli=DBExecQuery(Cfg.nDebugVersion>2,"select cpnmcol from %s where ordprog='%s';",szColProd,szOrdProg);
	if(DBresultStatus(PGResColli)==DBRES_TUPLES_OK){
		if((nTuples=DBntuples(PGResColli))){
			for(nIndex=0;nIndex<nTuples;nIndex++){
				StampaPackingListColloPrefincato(pszPrinterName,szOrdProg,nIndex+1,bStorico);
			}
		} else {
		}
		DBclear(PGResColli);
	}
		/*
		* Salto pagina
		*/
		//fprintf(fp,"");

		//fclose(fp);
	//if(nRC){
	//	sprintf(szTitle,"AeL - Stampa Packing List Bolla");
	//	local_PrintFile(szPrintFile,pszPrinterName,NULL,FALSE,0);
	//	unlink(szFileName);

	bRetVal=TRUE;
	//}
	//}
	return bRetVal;
}
