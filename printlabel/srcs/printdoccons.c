/*
* printdoccons.c : Stampa Documento di consegna
* 
* Progetto Mondadori Picking
*
* Autore : Luca Guerrini
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
#include <trace.h>
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
********************************************************************************************
* Funzioni per la stampa della DC
********************************************************************************************
*/

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
* InserisciDatiEveryPagePL()
* Funzione per la gestione della intestazione e del fondo pagine di tutte le pagine PL
* 02-01-2002 rm+st : Gestione EURO
*/
ep_bool_t InserisciDatiEveryPageDC(PDATIORDINE pDatiOrdine,int nNumeroPezzi,int nValoreOrdine)
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
	ep_bool_t bFound=FALSE;
	char szDSTSP[80];
	char szBuffer[80];
	char szFLNOT[80];

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
		
	InitData("EveryPage",pDatiOrdine->szROCDCLA, DCPos[F_DESTINATARIO0].nSR,     DCPos[F_DESTINATARIO0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPA, DCPos[F_DESTINATARIO1].nSR,     DCPos[F_DESTINATARIO1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDSPC, DCPos[F_DESTINATARIO2].nSR,     DCPos[F_DESTINATARIO2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLA, DCPos[F_DESTINATARIO3].nSR,     DCPos[F_DESTINATARIO3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLA, DCPos[F_DESTINATARIO4].nSR,     DCPos[F_DESTINATARIO4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLA, DCPos[F_DESTINATARIO5].nSR,     DCPos[F_DESTINATARIO5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLA, DCPos[F_DESTINATARIO6].nSR,     DCPos[F_DESTINATARIO6].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLA, DCPos[F_DESTINATARIO7].nSR,     DCPos[F_DESTINATARIO7].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLA, DCPos[F_DESTINATARIO8].nSR,     DCPos[F_DESTINATARIO8].nSC,     FALSE);
	//InitData("EveryPage",pDatiOrdine->szROPRDOC, DCPos[F_NUMEROINALTO].nSR,      DCPos[F_NUMEROINALTO].nSC,      FALSE);
	//InitData("EveryPage",szDataBAML,             DCPos[F_DATAXAB].nSR,           DCPos[F_DATAXAB].nSC,           FALSE);
	//InitData("EveryPage",pDatiOrdine->szROCDMOV, DCPos[F_DOCUMENTOINTERNO0].nSR, DCPos[F_DOCUMENTOINTERNO0].nSC, FALSE);
	//InitData("EveryPage",szDataBAMC,             DCPos[F_DOCUMENTOINTERNO1].nSR, DCPos[F_DOCUMENTOINTERNO1].nSC, FALSE);
	InitData("EveryPage",szDataStampa,           DCPos[F_DOCUMENTOINTERNO2].nSR, DCPos[F_DOCUMENTOINTERNO2].nSC, FALSE);
	//InitData("EveryPage",szProg,                 DCPos[F_NUMEROINBASSO].nSR,     DCPos[F_NUMEROINBASSO].nSC,     FALSE);
	//InitData("EveryPage",szAnno,                 DCPos[F_NUMEROINBASSO].nSR,     DCPos[F_NUMEROINBASSO].nSC+7,   FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRDOC, DCPos[F_SPEDIZIONE].nSR,        DCPos[F_SPEDIZIONE].nSC,        FALSE);
	InitData("EveryPage",LeftStr(szDSCST,20),    DCPos[F_CAUSALE].nSR,           DCPos[F_CAUSALE].nSC,           FALSE);
	InitData("EveryPage",Cfg.szContenuto,        DCPos[F_CONTENUTO].nSR,         DCPos[F_CONTENUTO].nSC,         FALSE);
	InitData("EveryPage",pDatiOrdine->szROCDRID, DCPos[F_DESTINAZIONE0].nSR,     DCPos[F_DESTINAZIONE0].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szRODSCLI, DCPos[F_DESTINAZIONE1].nSR,     DCPos[F_DESTINAZIONE1].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCOCLI, DCPos[F_DESTINAZIONE2].nSR,     DCPos[F_DESTINAZIONE2].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROINCLI, DCPos[F_DESTINAZIONE3].nSR,     DCPos[F_DESTINAZIONE3].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROCPCLI, DCPos[F_DESTINAZIONE4].nSR,     DCPos[F_DESTINAZIONE4].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROLOCLI, DCPos[F_DESTINAZIONE5].nSR,     DCPos[F_DESTINAZIONE5].nSC,     FALSE);
	InitData("EveryPage",pDatiOrdine->szROPRCLI, DCPos[F_DESTINAZIONE6].nSR,     DCPos[F_DESTINAZIONE6].nSC,     FALSE);
	InitData("EveryPage",szAssegno,              DCPos[F_ASSEGNO].nSR,           DCPos[F_ASSEGNO].nSC,           FALSE);
	InitData("EveryPage",szAspetto,              DCPos[F_ASPETTOBENI].nSR,       DCPos[F_ASPETTOBENI].nSC,       FALSE);

	/* 
	* Peso in Kg. 
	*/
	if (pDatiOrdine->nROPSREA){
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSREA/(float)1000));
	} else {
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSPRE/(float)1000));
	}
	InitData("EveryPage",szString,DCPos[F_PESO].nSR,DCPos[F_PESO].nSC,FALSE);

	
	/* 
	* Quantita' 
	*/
	sprintf(szString,"%6d",nNumeroPezzi);
	InitData("EveryPage",szString,DCPos[F_QUANTITAXAB].nSR,DCPos[F_QUANTITAXAB].nSC,FALSE);
	/* 
	* Numero Colli 
	*/
	sprintf(szString,"%5d",pDatiOrdine->nRONMCLL);
	InitData("EveryPage",szString,DCPos[F_NUMCOLLI].nSR,DCPos[F_NUMCOLLI].nSC,FALSE);
	/* 
	* Valore 
	*/
	/* decido se usare EURO o LIRE */
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);
	if(bEuro){
		NotazioneConVirgola(nValoreOrdine,szString);
	} else {
		NotazioneConPunti(nValoreOrdine,szString);
	}
	InitData("EveryPage",szString,DCPos[F_VARIAZDEST].nSR,DCPos[F_VARIAZDEST].nSC,FALSE);

	/*
	* rm 17-03-2004 : gestione descrizione linea diviso per cedola 
	*/
	/*
	* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
	*/
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
		InitData("EveryPage",szDSTSP,DCPos[F_ANNOTAZIONI].nSR,DCPos[F_ANNOTAZIONI].nSC,FALSE);
	}

  if (!DatiVettoreDC(pDatiOrdine,pDatiOrdine->szROCDVE2,F_VETTORI3,F_VETTORI4,F_VETTORI5)){
		if (!DatiVettoreDC(pDatiOrdine,pDatiOrdine->szROCDVE1,F_VETTORI0,F_VETTORI1,F_VETTORI2)){
			trace_debug(FALSE, TRUE, "Mancano i dati del vettore [%s]",pDatiOrdine->szROTPSPE);
			bOK=FALSE;
		}
	}
	
	return bOK;
}


/*
* InserisciDatiOldPageDC()
* Funzione per la gestione della pagina precedente in caso di salto pagina
*/
void InserisciDatiOldPageDC(void)
{
	InitData("OldPage",Cfg.szSegue,Cfg.nUltimaRigaDC+1,DCPos[F_AUTOREXAB].nSC,FALSE);
}


/*
* InserisciDatiNewPageDC()
* Funzione per la gestione della pagina attuale in caso di salto pagina
*/
void InserisciDatiNewPageDC(PDATIORDINE pDatiOrdine)
{
	char szString[127];
	char szDataBAM[80];

	DateDDMMYY(pDatiOrdine->szRODTBAM,szDataBAM);
	sprintf(szString,"%s %s DEL %s",Cfg.szSegueBAM,pDatiOrdine->szROPRDOC,szDataBAM);
	InitData("NewPage",szString,Cfg.nPrimaRigaDC-3,DCPos[F_AUTOREXAB].nSC,FALSE);
	sprintf(szString,"%s @@",Cfg.szFoglio);
	InitData("NewPage",szString,Cfg.nPrimaRigaDC-2,DCPos[F_AUTOREXAB].nSC,FALSE);
}

/*
* InserisciDatiLastPageDC()
* Funzione per la gestione dell'ultima pagina 
* 02-01-2002 rm+st : Gestione EURO
*/
ep_bool_t InserisciDatiLastPageDC(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot)
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
	sprintf(szString,"%s @@ XAB",Cfg.szFineBolla);
	InitData("LastPage",szString,Cfg.nUltimaRigaDC+1,DCPos[F_AUTOREXAB].nSC,FALSE);
	/* 
	* Quantita' 
	*/
	sprintf(szString,"%6d",nQtaTot);
	InitData("LastPage",szString,DCPos[F_QUANTITAXAB].nSR,DCPos[F_QUANTITAXAB].nSC,FALSE);
	NumberToLetter(nQtaTot,szBuffer);
	sprintf(szString,"%6s",szBuffer);
	InitData("LastPage",szString,DCPos[F_QUANTITAXAB].nSR+1,DCPos[F_QUANTITAXAB].nSC,FALSE);
	/* 
	* Peso in Kg. 
	*/
	if (pDatiOrdine->nROPSREA){
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSREA/(float)1000));
	} else {
		sprintf(szString,"%8.3f",(float)(pDatiOrdine->nROPSPRE/(float)1000));
	}

	InitData("LastPage",szString,DCPos[F_PESO].nSR,DCPos[F_PESO].nSC,FALSE);
	/* 
	* Numero colli 
	*/
	sprintf(szString,"%6d",pDatiOrdine->nRONMCLL);
	InitData("LastPage",szString,DCPos[F_NUMCOLLI].nSR,DCPos[F_NUMCOLLI].nSC,FALSE);

	/* decido se usare EURO o LIRE */
	bEuro=!strncasecmp(pDatiOrdine->szROCDVAL,"EUR",3);

	if(bEuro){
		NotazioneConVirgola(nPrzTot,szString);
	} else {
		NotazioneConPunti(nPrzTot,szString);
	}

	InitData("LastPage",szString,DCPos[F_VARIAZDEST].nSR,DCPos[F_VARIAZDEST].nSC,FALSE);
	/* -- Vettori -- */

	if (!DatiVettoreDC(pDatiOrdine,pDatiOrdine->szROCDVE1,F_VETTORI0,F_VETTORI1,F_VETTORI2)) bOK=FALSE;
	if (!DatiVettoreDC(pDatiOrdine,pDatiOrdine->szROCDVE2,F_VETTORI3,F_VETTORI4,F_VETTORI5)) bOK=FALSE;

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
		InitData("LastPage",szDSTSP,DCPos[F_ANNOTAZIONI].nSR,DCPos[F_ANNOTAZIONI].nSC,FALSE);
	}

	return bOK;
}


ep_bool_t DatiVettoreDC(PDATIORDINE pDatiOrdine,char *szCDVET,int nPos1,int nPos2,int nPos3)
{
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdsvet,elcinvet,elclovet from ttve where elccdsoc='%s' and elccdmag='%s' and elccdvet='%s';",
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		szCDVET);
	if(DBntuples(DBRes)){
		InitData("EveryPage",DBgetvalue(DBRes,0,0),DCPos[nPos1].nSR,DCPos[nPos1].nSC,FALSE);
		InitData("EveryPage",DBgetvalue(DBRes,0,1),DCPos[nPos2].nSR,DCPos[nPos2].nSC,FALSE);
		InitData("EveryPage",DBgetvalue(DBRes,0,2),DCPos[nPos3].nSR,DCPos[nPos3].nSC,FALSE);
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
* StampaDC(PDATIORDINE pDatiOrdine,ep_bool_t bStorico)
*
* Stampa l'DC dell'ordine in oggetto
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
ep_bool_t StampaDC(PDATIORDINE pDatiOrdine,ep_bool_t bStorico)
{
	DBresult *DBRes=NULL;
	ep_bool_t bOK=TRUE;
	char szValueField[80];
	int nValoreOrdine;
	int nNumeroPezzi;
	ep_bool_t bEuro=FALSE;
	char szRicOrd[128];
	char szRicArt[128];
	char szRicNote[128];
	char szRigProd[128];
	char szColProd[128];
	ep_bool_t bRistampa=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa DC [%s]",pDatiOrdine->szORDPROG);
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

	/* pulizia pagine */
	DeleteData("FirstPage");
	DeleteData("LastPage");
	DeleteData("OldPage");
	DeleteData("EveryPage");
	DeleteData("NewPage");

	InserisciDatiEveryPageDC(pDatiOrdine,nNumeroPezzi,nValoreOrdine);
	/* LG: non servono ma li lascio, casomai tornino utili in futuro */
	//InserisciDatiNewPageDC(pDatiOrdine);
	//InserisciDatiOldPageDC();
	//InserisciDatiLastPageDC(pDatiOrdine,nNumeroPezzi,nValoreOrdine);

	/*
	* LG: Cosa ci va nelle note del documento di trasporto?
	*     Sta roba qui sotto o le note estratte da ttds e ttts
	*/
	/* 
	* aggiorno il progressivo XAB
	*/
	if(bOK && !bStorico){
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"update ric_ord set rostato='%c' where ordprog='%s';",
			ORDINE_STAMPATA_XAB,
			pDatiOrdine->szORDPROG);
		if((DBresultStatus(DBRes)!=DBRES_COMMAND_OK)){
			bOK=FALSE;
		}
		DBclear(DBRes);
	}

	return bOK;
}



/*
* Funzione di stampa della DC per una singola spedizione
* Parametri di ingresso:
*  pszPrinterName: nome della stampante
*  szOrdProg:      chiave
* Ritorno:
*  TRUE in caso di successo
*	 FALSE in caso di insuccesso
*/
ep_bool_t StampaDCSpedizione(char *pszPrinterName,char *szOrdProg,ep_bool_t bStorico)
{
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	int nPagine=0;
	ep_bool_t bOK=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stampa DC Spedizione [%s] Storico : %d",szOrdProg,bStorico);
#endif

	sprintf(szPrintFile,"%s/%s.dc",Cfg.szPathStampe,pszPrinterName);

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,bStorico)){
		/*
		* La DC viene stampata se si trova negli stati:
		*  (E) Evaso
		*  (X) XAB
		*  (D) Distinta
		*  (H) Storicizzato
		*/
		if(1 || IsXAB(DatiOrdine.szROSTATO)){
			/* Creazione del file di stampa */
			InitPrint(szPrintFile,MAX_RIGHE_DC,MAX_COLONNE_DC,Cfg.nPrimaRigaDC,Cfg.nUltimaRigaDC);

			if(!StampaDC(&DatiOrdine,bStorico)){
				bOK=FALSE;
			}
			/*
			* Chiudo il file di stampa
			*/
			nPagine=EndPrint(FALSE);
		} else {
			/*
			* La spedizione non e' in uno stato utile per la stampa DC 
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
* Funzione di stampa della DC per linea di spedizione
* Parametri di ingresso:
* pszPrinterName : nome della stampante
* szNMCED        : Numero Cedola
* szTPSPE        : Tipo Spedizione
* szCDLIN        : Codice Linea di Spedizione
* Ritorno:
*/
ep_bool_t StampaDCLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,ep_bool_t bStorico)
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
	trace_debug(TRUE, TRUE, "Stampa DC Linea [%s - %s - %s - %s]",szNMCED,szTPSPE,szCDLIN,szCDVET);
#endif
	sprintf(szPrintFile,"%s/%s.dc",Cfg.szPathStampe,pszPrinterName);

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from %s where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s'  and rocdve2='%s' order by ordprog;",szRicOrd,Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET);
	if((nTuples=DBntuples(DBRes))){
		/* Creazione del file di stampa */
		InitPrint(szPrintFile,MAX_RIGHE_DC,MAX_COLONNE_DC,Cfg.nPrimaRigaDC,Cfg.nUltimaRigaDC);

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
				if(1 || IsXAB(DatiOrdine.szROSTATO)){

					if(!StampaDC(&DatiOrdine,bStorico)){
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


