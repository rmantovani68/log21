/*
* printdist.c : Stampa Distinta
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli/Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
* 02-01-2002 rm+st : ATTENZIONE : i prezzi in euro sono interi (in centesimi)
*                    usare la funzione NotazioneConVirgola() per la normalizzazione
* 05-02-2002 rm    : riscrittura (da CodeBase a PostgreSQL)
*
* 25-02-2002 rm : manca update ordine al termine della stampa XAB (prxab,dtxab ...)
* 09-10-2003 rm : Stampa distinta linea con discriminante per CEDOLA
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

BOOL IsDIST(char *szStato)
{
	switch(szStato[0]){
		case ORDINE_SPEDITO_HOST:
		case ORDINE_STAMPATA_XAB:
		case ORDINE_STAMPATA_DISTINTA:
		case ORDINE_FILE_INVIATO:
			return TRUE;
		break;
	}
	return FALSE;
}

/*
* Ritorna falso se non trova i dati nella tabella ttlv o ttts. Seganala la cosa nel trace file.
*/
BOOL InserisciDatiEveryPageDistinta(PDATIORDINE pDatiOrdine)
{
	BOOL bOK=TRUE;
	BOOL bFound=FALSE;
	char szNumDist[80];
	char szDSTSP[80];
	char szDate[80];
	char szDescrizioneLinea[128];
	char szFLNOT[80];
	DBresult *PGRes;

	GetDate(time((long *)0), szDate);

	DeleteData("EveryPage");
	sprintf(szNumDist,"%6.6d",pDatiOrdine->nRONMDIS);

	sprintf(szDescrizioneLinea,"%2s %2s/%2s-%2s",
		pDatiOrdine->szROTPSPE,
		pDatiOrdine->szROCDLIN,
		pDatiOrdine->szROCDSCA,
		pDatiOrdine->szROCDSSC);

	/*
	* Cerca in ttlv data la chiave
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdslin from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='%s' and elccdssc='%s';",
		pDatiOrdine->szROCDSOC,
		pDatiOrdine->szROCDMAG,
		pDatiOrdine->szROTPSPE,
		pDatiOrdine->szROCDLIN,
		pDatiOrdine->szROCDSCA,
		pDatiOrdine->szROCDSSC);
	if(DBntuples(PGRes)){
		InitData("EveryPage",DBgetvalue(PGRes,0,0),DisPos[F_DESCRIZIONETPSPED].nSR,DisPos[F_DESCRIZIONETPSPED].nSC,FALSE);
	} else {
		DBclear(PGRes);
		/*
		* rm 17-02-2005
		* Cerca in ttlv data la chiave - metto ** in scalo e sottoscalo
		*/
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdslin from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='**' and elccdssc='**';",
			pDatiOrdine->szROCDSOC,
			pDatiOrdine->szROCDMAG,
			pDatiOrdine->szROTPSPE,
			pDatiOrdine->szROCDLIN);
		if(DBntuples(PGRes)){
			InitData("EveryPage",DBgetvalue(PGRes,0,0),DisPos[F_DESCRIZIONETPSPED].nSR,DisPos[F_DESCRIZIONETPSPED].nSC,FALSE);
		} else {
			bOK=FALSE;
#ifdef TRACE
			trace_out_vstr(1,"Linea [%s][%s][%s][%s] non presente nella tabella ttlv",
				pDatiOrdine->szROTPSPE,
				pDatiOrdine->szROCDLIN,
				pDatiOrdine->szROCDSCA,
				pDatiOrdine->szROCDSSC);
#endif
		}
	}
	DBclear(PGRes);


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
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdstsp from ttts where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s';",
			pDatiOrdine->szROCDSOC,
			pDatiOrdine->szROCDMAG,
			pDatiOrdine->szROTPSPE);
		if(DBntuples(PGRes)){
			strcpy(szDSTSP,DBgetvalue(PGRes,0,0));
			bFound=TRUE;
		} else {
			bOK=FALSE;
			bFound=FALSE;
	#ifdef TRACE
			trace_out_vstr(1,"Tipo di Spedizione [%s] non presente nella tabella ttts", pDatiOrdine->szROTPSPE);
	#endif
		}
		DBclear(PGRes);
	}

	if(bFound){
		InitData("EveryPage",szDSTSP,DisPos[F_TPSPED].nSR,DisPos[F_TPSPED].nSC,FALSE);
	}

	InitData("EveryPage",szNumDist,DisPos[F_NUMERODISTINTA].nSR,DisPos[F_NUMERODISTINTA].nSC,FALSE);
	InitData("EveryPage","/@@",DisPos[F_NUMEROPAGINA].nSR,DisPos[F_NUMEROPAGINA].nSC,FALSE);
	InitData("EveryPage",Cfg.szDataDocumento,DisPos[F_DATADOCUMENTO].nSR,DisPos[F_DATADOCUMENTO].nSC,FALSE);
	InitData("EveryPage",szDate,DisPos[F_DATASTAMPA].nSR,DisPos[F_DATASTAMPA].nSC,FALSE);
	InitData("EveryPage",szDescrizioneLinea,DisPos[F_TPSPEDLINSCASSC].nSR,DisPos[F_TPSPEDLINSCASSC].nSC,FALSE);
	InitData("EveryPage",Cfg.szNomeMagazzino,DisPos[F_MAGAZZINO].nSR,DisPos[F_MAGAZZINO].nSC,FALSE);

	return(bOK);
}


void InserisciDatiNewPageDistinta(void)
{
	DeleteData("NewPage");
	InitData("NewPage",Cfg.szNuovaPaginaDistinta,Cfg.nPrimaRigaDistinta-1,DisPos[F_DESTINATARIO].nSC,FALSE);
}


BOOL InserisciDatiOldPageDistinta(BOOL bSegue,char *pszTipoEvento,PDATIORDINE pDatiOrdine,PTOTALI pTotaliScalo)
{
	BOOL bOK=TRUE;
	DBresult *PGRes;
	char szSpedizioni[80];
	char szPacchi[80];
	char szPeso[80];
	char szPezzi[80];
	char szDate[80];
	char szTime[80];

	DeleteData(pszTipoEvento);

	if (!bSegue){
		/*
		* Si tratta di un cambio scalo o di fine stampa
		*/
		/* vettore 1 */
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdsvet,elcinvet,elclovet from ttve where elccdsoc='%s' and elccdmag='%s' and elccdvet='%s';",
			pDatiOrdine->szROCDSOC,
			pDatiOrdine->szROCDMAG,
			pDatiOrdine->szROCDVE2);
		if(DBntuples(PGRes)){
			InitData(pszTipoEvento,DBgetvalue(PGRes,0,0),DisPos[F_DSVET1].nSR,DisPos[F_DSVET1].nSC,FALSE);
			InitData(pszTipoEvento,DBgetvalue(PGRes,0,1),DisPos[F_INVET1].nSR,DisPos[F_INVET1].nSC,FALSE);
			InitData(pszTipoEvento,DBgetvalue(PGRes,0,2),DisPos[F_LOVET1].nSR,DisPos[F_LOVET1].nSC,FALSE);
		} else {
			/*
			* Codice vettore non presente nella tabella ttve
			*/
		}
		DBclear(PGRes);

		/* vettore 2 */
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdsvet,elcinvet,elclovet from ttve where elccdsoc='%s' and elccdmag='%s' and elccdvet='%s';",
			pDatiOrdine->szROCDSOC,
			pDatiOrdine->szROCDMAG,
			pDatiOrdine->szROCDVE2);
		if(DBntuples(PGRes)){
			InitData(pszTipoEvento,DBgetvalue(PGRes,0,0),DisPos[F_DSVET1].nSR,DisPos[F_DSVET1].nSC,FALSE);
			InitData(pszTipoEvento,DBgetvalue(PGRes,0,1),DisPos[F_INVET1].nSR,DisPos[F_INVET1].nSC,FALSE);
			InitData(pszTipoEvento,DBgetvalue(PGRes,0,2),DisPos[F_LOVET1].nSR,DisPos[F_LOVET1].nSC,FALSE);
		} else {
			/*
			* Codice vettore non presente nella tabella ttve
			*/
		}
		DBclear(PGRes);

		sprintf(szSpedizioni,"%3d",pTotaliScalo->nSpedizioni);
		sprintf(szPacchi,    "%5d",pTotaliScalo->nPacchi);
		sprintf(szPeso,      "%7d",pTotaliScalo->nPeso);
		sprintf(szPezzi,     "%5d",pTotaliScalo->nPezzi);

		GetDate(time((long *)0),szDate);
		GetTime(time((long *)0),szTime);

		InitData(pszTipoEvento,szSpedizioni,DisPos[F_TOTALESPEDIZ].nSR,DisPos[F_TOTALESPEDIZ].nSC,FALSE);
		InitData(pszTipoEvento,szPacchi,DisPos[F_TOTALEPACCHI].nSR,DisPos[F_TOTALEPACCHI].nSC,FALSE);
		InitData(pszTipoEvento,szPeso,DisPos[F_TOTALEPESO].nSR,DisPos[F_TOTALEPESO].nSC,FALSE);
		InitData(pszTipoEvento,szPezzi,DisPos[F_TOTALEVOLUMI].nSR,DisPos[F_TOTALEVOLUMI].nSC,FALSE);
		InitData(pszTipoEvento,szDate,DisPos[F_DATARIITIRO].nSR,DisPos[F_DATARIITIRO].nSC,FALSE);
		InitData(pszTipoEvento,szTime,DisPos[F_ORARITIRO].nSR,DisPos[F_ORARITIRO].nSC,FALSE);
	} else {
		/*
		* Stesso scalo
		*/
		InitData("OldPage",Cfg.szVecchiaPaginaDistinta,DisPos[F_TOTALESPEDIZ].nSR,DisPos[F_TOTALESPEDIZ].nSC,FALSE);
	}
	return(bOK);
}


void InserisciDatiLastPageDistinta(PTOTALI pTotali)
{
	char szTotSpedizioni[80];
	char szTotPacchi[80];
	char szTotPeso[80];
	char szTotPezzi[80];

	sprintf(szTotSpedizioni,"%3d",pTotali->nSpedizioni);
	sprintf(szTotPacchi,    "%5d",pTotali->nPacchi);
	sprintf(szTotPeso,      "%7d",pTotali->nPeso);
	sprintf(szTotPezzi,     "%5d",pTotali->nPezzi);


	InitData("LastPage",szTotSpedizioni,DisPos[F_TOTALESPEDIZ].nSR-2,DisPos[F_TOTALESPEDIZ].nSC,FALSE);
	InitData("LastPage",szTotPacchi,DisPos[F_TOTALEPACCHI].nSR-2,DisPos[F_TOTALEPACCHI].nSC,FALSE);
	InitData("LastPage",szTotPeso,DisPos[F_TOTALEPESO].nSR-2,DisPos[F_TOTALEPESO].nSC,FALSE);
	InitData("LastPage",szTotPezzi,DisPos[F_TOTALEVOLUMI].nSR-2,DisPos[F_TOTALEVOLUMI].nSC,FALSE);
}


/*
* Questa funzione copia al piu' nLen caratteri di pszField in pszRow a partire dalla posizione 
* nColumn
*/

BOOL FillRow(char *pszRow,char *pszField,int nLen,int nColumn,int nAllineamento)
{
	BOOL bRetVal=TRUE;
	int nDim;
	int nIndex;
	char szBuffer[135];

	if (nLen>0 && nColumn>0){
		nDim=min(nLen,(int)strlen(pszRow)-nColumn);
		if (nDim>0){
			sprintf(szBuffer,"%*s",nDim,pszField);
			if (nAllineamento==LEFT)
				StrTrimLeft(szBuffer);
			for(nIndex=0;nIndex<(int)strlen(szBuffer);nIndex++)
				pszRow[nColumn+nIndex-1]=szBuffer[nIndex];
		} else {
			bRetVal=FALSE;
		}
	} else {
		bRetVal=FALSE;
	}
	return(bRetVal);
}



BOOL StampaOrdineDistinta(PDATIORDINE pDatiOrdine,PTOTALI pTotaliLinea,PTOTALI pTotaliScalo,int *pnRigaDistinta)
{
	char szRiga[255];
	char szBuffer[255];
	BOOL bOK=TRUE;

	pTotaliLinea->nSpedizioni++;
	pTotaliLinea->nPacchi+=pDatiOrdine->nRONMCLL;
	pTotaliLinea->nPezzi+=pDatiOrdine->nRONMCPE;
	pTotaliLinea->nPeso+=(pDatiOrdine->nROPSREA?pDatiOrdine->nROPSREA:pDatiOrdine->nROPSPRE);

	pTotaliScalo->nSpedizioni++;
	pTotaliScalo->nPacchi+=pDatiOrdine->nRONMCLL;
	pTotaliScalo->nPezzi+=pDatiOrdine->nRONMCPE;
	pTotaliScalo->nPeso+=(pDatiOrdine->nROPSREA?pDatiOrdine->nROPSREA:pDatiOrdine->nROPSPRE);

	if (Cfg.nUltimaRigaDistinta-*pnRigaDistinta<4){
		/*
		* Non sono rimaste un numero sufficiente di righe nella pagina per contenere un'altra
		* spedizione. Occorre un salto pagina.
		*/
		SkipPage(FALSE,FALSE,FALSE);
		*pnRigaDistinta=0;
	}
	/*
	* Prima riga
	*/
	memset(szRiga,' ',MAX_COLONNE_DIS);
	*(szRiga+MAX_COLONNE_DIS)='\0';
	sprintf(szBuffer,"%d",pTotaliScalo->nSpedizioni);
	FillRow(szRiga,szBuffer,2,DisPos[F_PROGRSPEDIZ].nSC,RIGHT);
	FillRow(szRiga,pDatiOrdine->szROPRDOC,6,DisPos[F_SPEDIZIONEDIS].nSC,LEFT);
	FillRow(szRiga,pDatiOrdine->szRODSCLA,19,DisPos[F_DESTINATARIO].nSC,LEFT);
	FillRow(szRiga,pDatiOrdine->szROINCLI,18,DisPos[F_DESTINAZIONE].nSC,LEFT);
	sprintf(szBuffer,"%d",(pDatiOrdine->nROPSREA?pDatiOrdine->nROPSREA:pDatiOrdine->nROPSPRE));
	FillRow(szRiga,szBuffer,7,DisPos[F_PESODISTINTA].nSC,RIGHT);
	sprintf(szBuffer,"%d",pDatiOrdine->nRONMCPE);
	FillRow(szRiga,szBuffer,5,DisPos[F_NUMEROPEZZI].nSC,RIGHT);
	sprintf(szBuffer,"%d",pDatiOrdine->nRONMCLL);
	FillRow(szRiga,szBuffer,5,DisPos[F_NUMEROPACCHI].nSC,RIGHT);
	PrintRow(szRiga,2,FALSE);

	/*
	* Seconda riga
	*/
	memset(szRiga,' ',MAX_COLONNE_DIS);
	*(szRiga+MAX_COLONNE_DIS)='\0';
	FillRow(szRiga,pDatiOrdine->szROLOCLI,15,DisPos[F_DESTINAZIONE].nSC,LEFT);
	FillRow(szRiga,pDatiOrdine->szRONMBAM,7,DisPos[F_DATABAM].nSC,LEFT);
	PrintRow(szRiga,1,FALSE);

	/*
	* Terza riga
	*/
	memset(szRiga,' ',MAX_COLONNE_DIS);
	*(szRiga+MAX_COLONNE_DIS)='\0';
	FillRow(szRiga,pDatiOrdine->szROCPCLI,6,DisPos[F_DESTINAZIONE].nSC,LEFT);
	FillRow(szRiga,pDatiOrdine->szROPRCLI,5,DisPos[F_DESTINAZIONE].nSC+7,LEFT);
	FillRow(szRiga,DateDDMMYY(pDatiOrdine->szRODTBAM,szBuffer),8,DisPos[F_DATABAM].nSC,LEFT);
	*pnRigaDistinta=PrintRow(szRiga,1,FALSE);

	return(bOK);
}


/*
* Funzione di gestione cambio scalo. Ritorna falso se non vengono trovati i dati nelle tabelle
* ttlv o ttts.
*/
BOOL CambioScalo(PDATIORDINE pDatiOrdine,PTOTALI pTotaliScalo)
{

	BOOL bOK=TRUE;

	InserisciDatiOldPageDistinta(FALSE,"OldPage",pDatiOrdine,pTotaliScalo);

	EnableNewPage(FALSE);
	SkipPage(FALSE,FALSE,FALSE);
	EnableNewPage(TRUE);

	if (!InserisciDatiEveryPageDistinta(pDatiOrdine)){
		bOK=FALSE;
	}
	InserisciDatiOldPageDistinta(TRUE,"OldPage",pDatiOrdine,pTotaliScalo);

	pTotaliScalo->nSpedizioni=0;
	pTotaliScalo->nPacchi=0;
	pTotaliScalo->nPeso=0;
	pTotaliScalo->nPezzi=0;

	return(bOK);
}


/*
* Stampa la distinta relativa alla linea identificata da 
* szNMCED : Numero Cedola
* szTPSPE : Tipo Spedizione
* szCDLIN : Codice Linea
*/
BOOL StampaDistinta(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN, char *szCDVET,BOOL bStorico)
{
	BOOL bOK=TRUE;
	BOOL bRistampa=TRUE;
	BOOL bRistampaXAB=TRUE;
	int nRigaDistinta=0;
	int nDistinta=0;
	int nAnno=0;
	char szScalo[80];
	char szPrintFile[128];
	char szData[128];
	int nTuples;
	int nIndex;
	DBresult *PGRes;
	DBresult *PGResDist;
	DBresult *PGResXAB;
	DBresult *PGResUpdate;
	TOTALI TotaliLinea,TotaliScalo;
	DATIORDINE DatiOrdine;
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
	trace_out_vstr_date(1,"StampaDistinta(%s,%s,%s,%s Storico : %d)",szNMCED,szTPSPE,szCDLIN,szCDVET,bStorico);
#endif
	/* 
	* data attuale ( per distinta ) 
	*/
	GetDate(time((long *)0), szData);

	sprintf(szPrintFile,"%s/%s.dist",Cfg.szPathStampe,pszPrinterName);

	InitPrint(szPrintFile,MAX_RIGHE_DIS,MAX_COLONNE_DIS,Cfg.nPrimaRigaDistinta,Cfg.nUltimaRigaDistinta);

	/*
	* Azzeramento TotaliLinea, TotaliScalo
	*/
	TotaliLinea.nSpedizioni=0;
	TotaliLinea.nPacchi=0;
	TotaliLinea.nPeso=0;
	TotaliLinea.nPezzi=0;

	TotaliScalo.nSpedizioni=0;
	TotaliScalo.nPacchi=0;
	TotaliScalo.nPeso=0;
	TotaliScalo.nPezzi=0;

	InserisciDatiNewPageDistinta();

	if(bStorico){
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from %s where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s' order by rotpspe,rocdlin,rocdve2,rocdsca,rocdssc,ordprog;",szRicOrd,Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET);
	} else {
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from %s where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s' and rostato  in ('%c','%c','%c') order by rotpspe,rocdlin,rocdve2,rocdsca,rocdssc,ordprog;",szRicOrd,Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET,ORDINE_STAMPATA_DISTINTA,ORDINE_STAMPATA_XAB,ORDINE_FILE_INVIATO);
	}
	if((nTuples=DBntuples(PGRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){
			GetDatiOrdine(DBgetvalue(PGRes,nIndex,0),&DatiOrdine,bStorico);

			if(nIndex==0){
				if(!bStorico && !IsDIST(DatiOrdine.szROSTATO)){
					bOK=FALSE;
				}
				/*
				* Ricavo progressivo distinta
				* ATTENZIONE : gestire ristampa
				*/
				if(!bStorico && !DatiOrdine.nRONMDIS){
					bRistampa=FALSE;
					PGResDist=DBExecQuery(Cfg.nDebugVersion>2,"select last_value from sequence_distinta;");
					if(DBntuples(PGResDist)){
						nDistinta=atoi(DBgetvalue(PGResDist,0,0))+1;
						DatiOrdine.nRONMDIS=nDistinta;
					} else {
						bOK=FALSE;
					}
					DBclear(PGResDist);
				} else {
					nDistinta=DatiOrdine.nRONMDIS;
					bRistampa=TRUE;
				}
				/* primo giro */
				InserisciDatiOldPageDistinta(TRUE,"OldPage",&DatiOrdine,&TotaliScalo);
				if (!InserisciDatiEveryPageDistinta(&DatiOrdine)){
					bOK=FALSE;
				}
				strcpy(szScalo,DatiOrdine.szROCDSCA);
			}

			/* 
			* rm 08-07-2002 : assegno progressivo XAB se non
			*                 ancora presente (su tutti gli ordini)
			* se non si tratta di una ristampa 
			* incremento il progressivo XAB
			*/
			if(!bStorico && !DatiOrdine.nROPRXAB){
				nAnno=atoi(LeftStr(GetDateYYYYMMDD(time((long *)0),szDateBuffer),4));
				/* 
				* Progressivo XAB 
				* se si tratta della prima volta che si stampa l'XAB di questo ordine
				* allora si calcolano e assegnano i valori e le sequenze relative
				*/
				PGResXAB=DBExecQuery(Cfg.nDebugVersion>2,"select nextval('sequence_xab');");
				if(DBntuples(PGResXAB)){
					DatiOrdine.nROPRXAB=atoi(DBgetvalue(PGResXAB,0,0));
					DatiOrdine.nROAAXAB=nAnno%100;
					strcpy(DatiOrdine.szRODTXAB,GetDateYYYYMMDD(time((long *)0),szDateBuffer));
				} else {
					bOK=FALSE;
				}
				DBclear(PGResXAB);
				bRistampaXAB=FALSE;
			} else {
				bRistampaXAB=TRUE;
			}
			/* 
			* aggiornare sequenza XAB e 
			* campi prxab e dtxab e aaxab in ordini 
			*/
			if(!bStorico && !bRistampaXAB && bOK){
				//PGResUpdate=DBExecQuery(Cfg.nDebugVersion>2,"update ric_ord set roprxab=%d,roaaxab=%d,rodtxab='%s' where ordprog='%s';",
				PGResUpdate=DBExecQuery(Cfg.nDebugVersion>2,"update ric_ord set roprxab=%d,roaaxab=%d,rodtxab='%s' where ordprog='%s';",
					DatiOrdine.nROPRXAB,
					DatiOrdine.nROAAXAB,
					DatiOrdine.szRODTXAB,
					DatiOrdine.szORDPROG
					);

				DBclear(PGResUpdate);
			}

			DatiOrdine.nRONMDIS=nDistinta;

			if (strcmp(szScalo,DatiOrdine.szROCDSCA)){
				/* cambio scalo */
				nRigaDistinta=0;
				strcpy(szScalo,DatiOrdine.szROCDSCA);
				CambioScalo(&DatiOrdine,&TotaliScalo);
			}
			if(!StampaOrdineDistinta(&DatiOrdine,&TotaliLinea,&TotaliScalo,&nRigaDistinta)){
				bOK=FALSE;	
			}
			/*
			* Esco dal ciclo se errore
			*/
			if(!bOK){
				break;
			}
		}
		if(bOK){
			/* 
			* aggiornare sequenza distinta e 
			* campi nmdis e dtdis in ordini 
			*/
			if(!bStorico && !bRistampa){
				PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select nextval('sequence_distinta');");
				if(DBntuples(PGRes)){
					nDistinta=atoi(DBgetvalue(PGRes,0,0));
					//PGResUpdate=DBExecQuery(Cfg.nDebugVersion>2,"update ric_ord set ronmdis=%d,rodtdis='%s',rostato='%c' where ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s';",
					PGResUpdate=DBExecQuery(Cfg.nDebugVersion>2,"update ric_ord set ronmdis=%d,rodtdis='%s',rostato='%c' where ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s' and rostato in ('%c','%c');",
						nDistinta,
						GetDateYYYYMMDD(time((long *)0),szDateBuffer),
						ORDINE_STAMPATA_DISTINTA,
						szNMCED,
						szTPSPE,
						szCDLIN,
						szCDVET,
						ORDINE_STAMPATA_DISTINTA,
						ORDINE_STAMPATA_XAB
						);
					if((PQresultStatus(PGResUpdate)!=DBRES_COMMAND_OK)){
						bOK=FALSE;
					}
					DBclear(PGResUpdate);
				} else {
					bOK=FALSE;
				}
				DBclear(PGRes);
			}

			InserisciDatiOldPageDistinta(FALSE,"LastPage",&DatiOrdine,&TotaliScalo);
			InserisciDatiLastPageDistinta(&TotaliLinea);

			/* chiudo il file di stampa */

			EndPrint(FALSE);

			PrintFileRaw(szPrintFile,pszPrinterName);
		} 
	}

	return(bOK);
}



