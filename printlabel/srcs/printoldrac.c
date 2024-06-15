/*
* printrac.c : Stampa Moduli di raccolta
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli
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
#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"
#include	"linklist.h"
#include	"stampa.h"


/* 
********************************************************************************************
*
* Funzione: 
* void PrintRC(int nRow, int nColumn, char *pszString,int nLen)
*
* Descrizione:
* Scrive i primi nLen caratteri di pszString nella matrice di caratteri pszForm
* a partire dalla posizione specificata da nRow,nColumn
*
********************************************************************************************
*/
void PrintRC(int nRow, int nColumn, char *pszString)
{
	if(pszString){
		memcpy(pszForm[nRow]+nColumn,pszString,strlen(pszString));
	}
}


void AllocaPagina(void)
{
	int nIndex;

	for(nIndex=0;nIndex<MAX_RAC_LINES;nIndex++){
		pszForm[nIndex]=malloc(MAX_RAC_LINE_LEN+1);
	}
}


void LiberaPagina(void)
{
	int nIndex;

	for(nIndex=0;nIndex<MAX_RAC_LINES;nIndex++){
		free(pszForm[nIndex]);
	}
}


void PulisciPagina(int nLen)
{
	int nIndex;
	char szRiga[MAX_RAC_LINE_LEN+1];

	memset(szRiga,' ',MAX_RAC_LINE_LEN);
	szRiga[MAX_RAC_LINE_LEN]='\0';
	for (nIndex=1;nIndex<=nLen;nIndex++){
		strcpy(pszForm[nIndex],szRiga);
	}
}


void ScaricaPagina(FILE *fp, int nLen)
{
	int nIndex;

	for (nIndex=1;nIndex<=nLen;nIndex++){
		fprintf(fp,"%s\n",pszForm[nIndex]);
	}
}

void StampaEtichetta(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo)
{
	char szPacco[128];
	char szR1C2[128];
	char szR1C4[128];
	char szPeso[128];
	char szAppoggio[128];
	char szDataStampa[128];
	char szDSTSP[128];
	int nIndex;
	DBresult *DBRes;
	ep_bool_t bFound=TRUE;

	/*
	* rm 17-03-2004 : gestione descrizione linea diviso per cedola 
	*/
	/*
	* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='%s';", 
		pDatiOrdine->szROCDSOC, 
		pDatiOrdine->szROCDMAG, 
		pDatiOrdine->szRONMCED, 
		pDatiOrdine->szROTPSPE,
		pDatiOrdine->szROCDLIN);
	if(DBntuples(DBRes)){
		strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
		bFound=TRUE;
	} else {
		bFound=FALSE;
	}
	DBclear(DBRes);

	if(!bFound){
		/* rm 27-09-2004 : primo giro bis: cerco con cdlin = '' */
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='';", 
			pDatiOrdine->szROCDSOC, 
			pDatiOrdine->szROCDMAG, 
			pDatiOrdine->szRONMCED, 
			pDatiOrdine->szROTPSPE);
		if(DBntuples(DBRes)){
			strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
			bFound=TRUE;
		} else {
			bFound=FALSE;
		}
		DBclear(DBRes);
	}


	if(!bFound){
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select elcdstsp from ttts where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s';", 
			pDatiOrdine->szROCDSOC, 
			pDatiOrdine->szROCDMAG, 
			pDatiOrdine->szROTPSPE);

		if(DBntuples(DBRes)){
			strcpy(szDSTSP,DBgetvalue(DBRes,0,0));
		} else {
			strcpy(szDSTSP,"ERR");
		}
		DBclear(DBRes);
	}

	GetDate(time((long *)0),szDataStampa);

	sprintf(szPacco,"%3d/%3d",pDatiCollo->nCPNMCOL,pDatiOrdine->nRONMCLL);
	sprintf(szR1C2,"%s-%s-%s",pDatiOrdine->szROCDRID,pDatiOrdine->szROCDSPA,pDatiOrdine->szROCDSPC);
	sprintf(szR1C4,"%s/%s-%s",pDatiOrdine->szROCDLIN,pDatiOrdine->szROCDSCA,pDatiOrdine->szROCDSSC);
	sprintf(szPeso,"%6d",pDatiCollo->nCPPSPRE);

	PrintRC(Pos[F_ETISPEDIZ].nSR,     Pos[F_ETISPEDIZ].nSC,     pDatiOrdine->szROPRDOC);
	PrintRC(Pos[F_ETINUMPACCO].nSR,   Pos[F_ETINUMPACCO].nSC,   szPacco);
	PrintRC(Pos[F_ETIR1C1].nSR,       Pos[F_ETIR1C1].nSC,       pDatiOrdine->szROCDRID);
	PrintRC(Pos[F_ETIR1C2].nSR,       Pos[F_ETIR1C2].nSC,       szR1C2);
	PrintRC(Pos[F_ETIR1C3].nSR,       Pos[F_ETIR1C3].nSC,       pDatiOrdine->szROTPSPE);
	PrintRC(Pos[F_ETIR1C4].nSR,       Pos[F_ETIR1C4].nSC,       szR1C4);
	PrintRC(Pos[F_ETIR2].nSR,         Pos[F_ETIR2].nSC,         pDatiOrdine->szRODSCLI);
	PrintRC(Pos[F_ETIR3].nSR,         Pos[F_ETIR3].nSC,         pDatiOrdine->szROCOCLI);
	PrintRC(Pos[F_ETIR4].nSR,         Pos[F_ETIR4].nSC,         pDatiOrdine->szROINCLI);
	PrintRC(Pos[F_ETIR5C1].nSR,       Pos[F_ETIR5C1].nSC,       pDatiOrdine->szROCPCLI);
  PrintRC(Pos[F_ETIR5C2].nSR,       Pos[F_ETIR5C2].nSC,       pDatiOrdine->szROLOCLI);
	PrintRC(Pos[F_ETIR5C3].nSR,       Pos[F_ETIR5C3].nSC,       pDatiOrdine->szROPRCLI);
	PrintRC(Pos[F_ETIR6].nSR,         Pos[F_ETIR6].nSC,         pDatiOrdine->szRONZCLI);
	PrintRC(Pos[F_ETIPESO].nSR,       Pos[F_ETIPESO].nSC,       szPeso);
	PrintRC(Pos[F_ETIDATABAM].nSR,    Pos[F_ETIDATABAM].nSC,    pDatiOrdine->szRODTEVA);
	PrintRC(Pos[F_ETIDATASTAMPA].nSR, Pos[F_ETIDATASTAMPA].nSC, szDataStampa);
	PrintRC(Pos[F_DSLINEA].nSR,       Pos[F_DSLINEA].nSC,       szDSTSP);

	/*
	* Questo pezzo di codice serve a costruire il codice da inviare alla stampante relativo
	* alla parte grafica. In pratica ciascuna pagina del modulo RAC viene stampata secondo le
	* seguenti modalita':
	* - dalla riga 1 alla riga 16 si invia alla stampante cio' che si vuole stampare direttamente in
	*   codice ascii (modalita' normal della stampante)
	* - le righe dalla 17 alla 22 contenendo delle parti grafiche (codice a barre, formato collo,
	*   codice sovrappacco) vengono stampate attraverso la creazione di un form
	* - le righe 23 e 24 vengono di nuovo inviate in codice ascii 
	* La stampante printronix ha tre modalita' di funzionamento:
	* 1. In modalita' normal stampa quanto riceve
	* 2. In modalita' create (si passa dalla modalita' normal alla modalita' create inviando alla
	*    stampante il comando ^CREATE) si costruisce un form nel senso che si definiscono posizioni
	*    e dimensioni di cio' che si vuole stampare (e questo e' cio' che fa la funzione InitFile)
	* 3. In modalita' execute (si passa dalla modalita' normal alla modalita' execute inviando alla 
	*    stampante il comando ^EXECUTE) si stampa il form creato con la modalita' create passandogli
	*    il contenuto di cio' che si vuole stampare
	* Pertanto cio' che si deve ottenere (ossia cio' che si deve inviare alla stampante)
	* e' qualcosa del tipo:
	* - prime 16 righe del modulo RAC  
	* - ^EXECUTE;MODDOC
	* - ^AF1;*<riga 17>*				di queste righe vengono stampate solo i primi 85 caratteri
	* - ...											ovvero solo la parte sinistra del modulo RAC
	* - ^AF6;*<riga 21>*
	* - ^AF7;*<formato collo>*
	* - ^AF8;*<codice sovrappacco>*
	* - ^BF1;*<barcode>*
	* - CR LF
	* - ^NORMAL
	* - righe 23 e 24 del modulo RAC
	*/
	strcpy(pszForm[Cfg.nLinesPerForm+5],pszForm[Cfg.nLinesPerForm-1]); /* ultime due righe di RAC */
	strcpy(pszForm[Cfg.nLinesPerForm+6],pszForm[Cfg.nLinesPerForm]);
	for (nIndex=22;nIndex>=17;nIndex--){
		sprintf(szAppoggio,"^AF%d;*",nIndex-16);
		strncat(szAppoggio,pszForm[nIndex],85);
		strcat(szAppoggio,"*");
		strcpy(pszForm[nIndex+1],szAppoggio);
	}
	strcpy(pszForm[17],"^EXECUTE;MODDOC");
	sprintf(szAppoggio,"^AF7;*%s*",pDatiCollo->szCPTPFOR);  strcpy(pszForm[Cfg.nLinesPerForm],szAppoggio);
	sprintf(szAppoggio,"^AF8;*%s*",pDatiOrdine->szROIDVET); strcpy(pszForm[Cfg.nLinesPerForm+1],szAppoggio);
	sprintf(szAppoggio,"^BF1;*%s*",pDatiCollo->szCPBRCDE);  strcpy(pszForm[Cfg.nLinesPerForm+2],szAppoggio);
	strcpy(pszForm[Cfg.nLinesPerForm+3],"");
	strcpy(pszForm[Cfg.nLinesPerForm+4],"^NORMAL");

	/*
	* Mancano:
	*  - il campo il cui valore e' definito dall'utente al momento del lancio della stampa
	*/
}


void StampaFinePagina(PDATIORDINE pDatiOrdine, PDATICOLLO pDatiCollo,int nPagina, ep_bool_t bStampaNumeroCollo)
{
	char szPagina[40];
	char szNumCollo[40];

	/*
	* Stampa il fine pagina in memoria 
	*/
	sprintf(szPagina,"%4d",nPagina);
	sprintf(szNumCollo,"%3d",pDatiCollo->nCPNMCOL);

	PrintRC(Pos[F_SPEDIZ].nSR,    Pos[F_SPEDIZ].nSC,    pDatiOrdine->szROPRDOC);
	PrintRC(Pos[F_DATA].nSR,      Pos[F_DATA].nSC,      pDatiOrdine->szRODTEVA);
	PrintRC(Pos[F_PAG].nSR,       Pos[F_PAG].nSC,       szPagina);
	PrintRC(Pos[F_CODDEST].nSR,Pos[F_CODDEST].nSC,      pDatiOrdine->szROCDRID);
	if (bStampaNumeroCollo) {
		PrintRC(Pos[F_PACCO].nSR, Pos[F_PACCO].nSC, szNumCollo);
	}
}


void StampaFineCollo(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo)
{
	char szTotColli[40];
	char szTotRigheCollo[40];
	char szTotPezziCollo[40];

	sprintf(szTotColli,"%3d",pDatiOrdine->nRONMCLL);
	sprintf(szTotRigheCollo,"%4d",pDatiCollo->nCPNMRGH);
	sprintf(szTotPezziCollo,"%4d",pDatiCollo->nCPNMCPE);

	PrintRC(Pos[F_TOTPACCHI].nSR, Pos[F_TOTPACCHI].nSC, szTotColli);
	PrintRC(Pos[F_RIGHE].nSR,     Pos[F_RIGHE].nSC,     szTotRigheCollo);
	PrintRC(Pos[F_VOLUMI].nSR,    Pos[F_VOLUMI].nSC,    szTotPezziCollo);
}


void StampaFineOrdine(PDATIORDINE pDatiOrdine)
{
	char szTotRigheOrdine[40];
	char szTotPezziOrdine[40];

	sprintf(szTotRigheOrdine,"%5d",pDatiOrdine->nRONMRGP);
	sprintf(szTotPezziOrdine,"%6d",pDatiOrdine->nRONMCPP);

	PrintRC(Pos[F_TOTRIGHE].nSR,  Pos[F_STOCCAGGIO].nSC, Cfg.szFine);
	PrintRC(Pos[F_TOTRIGHE].nSR,  Pos[F_TITOLO].nSC,     Cfg.szCodMovimento);
	PrintRC(Pos[F_TOTRIGHE].nSR, 	Pos[F_FORIGHE].nSC,  	 Cfg.szFineOrdineRighe);
	PrintRC(Pos[F_TOTRIGHE].nSR,  Pos[F_TOTRIGHE].nSC,   szTotRigheOrdine);
	PrintRC(Pos[F_TOTRIGHE].nSR, 	Pos[F_FOVOLUMI].nSC,   Cfg.szFineOrdineVolumi);
	PrintRC(Pos[F_TOTVOLUMI].nSR, Pos[F_TOTVOLUMI].nSC,  szTotPezziOrdine);
}


/*
* Creazione del form per la stampa della parte grafica
* RAC
*/
void InitFile(FILE *fp)
{
	fprintf(fp,"^CREATE;MODDOC;72\n");
	fprintf(fp,"ALPHA\n");
	fprintf(fp,"AF1;85;1;1;0;0\n");
	fprintf(fp,"AF2;85;2;1;0;0\n");
	fprintf(fp,"AF3;85;3;1;0;0\n");
	fprintf(fp,"AF4;85;4;1;0;0\n");
	fprintf(fp,"AF5;85;5;1;0;0\n");
	fprintf(fp,"AF6;85;6;1;0;0\n");
	fprintf(fp,"AF7;2;%d;%d;3;3\n",Pos[F_ETIFORCOLLO].nSR,Pos[F_ETIFORCOLLO].nSC);
	fprintf(fp,"AF8;2;%d;%d;6;5\n",Pos[F_ETICODSOVR].nSR,Pos[F_ETICODSOVR].nSC);
	fprintf(fp,"STOP\n");
	fprintf(fp,"BARCODE\n");
	fprintf(fp,"I-2/5;H10;BF1;14;DARK;1;%d\n",Pos[F_ETIBARCODE].nSC);
	fprintf(fp,"PDF;B\n");
	fprintf(fp,"STOP\n");
	fprintf(fp,"END\n");
}


/* 
********************************************************************************************
*
* Funzione: 
* StampaBolla (FILE *fp, char *szOrdProg)
*
* Descrizione:
* Scrive sul file specificato il codice di programmazione della stampante printronix
* relativo alla RAC indicata da szOrdProg
* 
* Riceve:
* fp        : file pointer al file di programmazione della stampante printronix
* szOrdProg : chiave ordine
*
* Ritorna:
* TRUE se la scrittura e' terminata con successo 
* FALSE in caso di problemi
*
* Nel caso in cui il valore di ritorno sia FALSE, vedere il trace per 
* avere indicazioni sulla causa dell'insuccesso
*
********************************************************************************************
*/
ep_bool_t StampaBolla(FILE *fp, char *szOrdProg)
{
	DBresult *DBRes=NULL;
	DATIORDINE DatiOrdine;
	DATICOLLO  DatiCollo;
	DATIRIGA   DatiRiga;
	int nLines;
	int nRigheStampate;
	int nPagina;
	int nColloIndex;
	int nRigaIndex;
	int nIndex;
	int nTuples;
	ep_bool_t bOK=TRUE;


#ifdef TRACE
	trace_debug(TRUE, TRUE, "StampaBolla(%s)",szOrdProg);
#endif

	nLines=Cfg.nLinesPerForm;

	if(GetDatiOrdine(szOrdProg,&DatiOrdine,FALSE)){

		AllocaPagina();
		PulisciPagina(nLines);

		for(nColloIndex=0;nColloIndex<DatiOrdine.nRONMCLL;nColloIndex++){
			if(GetDatiCollo(szOrdProg,nColloIndex+1,&DatiCollo,FALSE)){
				nPagina=1;
				nRigheStampate=0;

				DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rp.rpcdpro,rp.rpcdubi,rp.rpqtspe,ra.rapzpre,ca.prdstit,ca.prdsaut \
					from rig_prod rp,ric_art ra,catalogo ca where rp.rpcdpro=ra.racdpro and rp.rpcdpro=ca.prcdpro and rp.ordprog=ra.ordprog and rp.ordprog='%s' and rp.rpnmcol=%d order by rp.rpprrig;",
					szOrdProg,nColloIndex+1);

				if((nTuples=DBntuples(DBRes))){

					for(nRigaIndex=0;nRigaIndex<nTuples;nRigaIndex++){
						strcpy(DatiRiga.szRPCDPRO,DBgetvalue(DBRes,nRigaIndex,0));
						strcpy(DatiRiga.szRPCDUBI,DBgetvalue(DBRes,nRigaIndex,1));
						DatiRiga.nRPQTSPE=atoi(DBgetvalue(DBRes,nRigaIndex,2));
						DatiRiga.nRAPZPRO=atoi(DBgetvalue(DBRes,nRigaIndex,3));
						strcpy(DatiRiga.szPRDSTIT,DBgetvalue(DBRes,nRigaIndex,4));
						strcpy(DatiRiga.szPRDSAUT,DBgetvalue(DBRes,nRigaIndex,5));

						StampaRiga(&DatiRiga,nRigheStampate);
						nRigheStampate++;
						if (nRigheStampate==Cfg.nPrintableRow){
							StampaFinePagina(&DatiOrdine,&DatiCollo,nPagina,TRUE);
							nRigheStampate=0;
							/* 
							* rm 11-02-2002
							* verifica se necessita una nuova pagina 
							*/
							if(nRigaIndex+1<DatiCollo.nCPNMRGH){
								/* NO */
								if (nPagina==1) {
									StampaEtichetta(&DatiOrdine,&DatiCollo);
									nLines=Cfg.nLinesPerForm+6;
								} else {
									nLines=Cfg.nLinesPerForm;
								}
								ScaricaPagina(fp,nLines);
								PulisciPagina(nLines);
								nPagina++;
							}
						}
					}
				} else {
					bOK=FALSE;
				}
				DBclear(DBRes);

				if (bOK) {
					StampaFinePagina(&DatiOrdine,&DatiCollo,nPagina,TRUE);
					StampaFineCollo(&DatiOrdine,&DatiCollo);
					/*
					* Se si tratta dell'ultimo collo dell'ordine non scaricare su file ASCII
					*/
					if(nColloIndex+1<DatiOrdine.nRONMCLL){
						/* NO */
						if (nPagina==1) {
							StampaEtichetta(&DatiOrdine,&DatiCollo);
							nLines=Cfg.nLinesPerForm+6;
						} else {
							nLines=Cfg.nLinesPerForm;
						}
						ScaricaPagina(fp,nLines);
						PulisciPagina(nLines);
					}
				}
			}
		}
		StampaFineOrdine(&DatiOrdine);
		if (nPagina==1) {
			StampaEtichetta(&DatiOrdine,&DatiCollo);
			nLines=Cfg.nLinesPerForm+6;
		} else {
			nLines=Cfg.nLinesPerForm;
		}
		ScaricaPagina(fp,nLines);
		PulisciPagina(nLines);
	}
	if (bOK) {
		/*
		* Stampa note di fine ordine
		* se sono di tipo RAC
		*/
		nPagina=1;
		nRigheStampate=0;
		PulisciPagina(Cfg.nLinesPerForm);
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select RNFRTXT from ric_note where ordprog='%s' and rntptxt='%s' order by rnnmtxt;",
			szOrdProg,Cfg.szTipoNotaRAC);
		if((nTuples=DBntuples(DBRes))){
			/*
			* Ci sono note da stampare
			*/
			for(nIndex=0;nIndex<nTuples;nIndex++){

				PrintRC(nPrintingRow[nRigheStampate], Pos[F_NOTE].nSC, DBgetvalue(DBRes,nIndex,0));
				nRigheStampate++;
				if (nRigheStampate==Cfg.nPrintableRow) {
					/*
					* Nuova pagina
					*/
					StampaFinePagina(&DatiOrdine,&DatiCollo,nPagina,FALSE);
					nPagina++;
					ScaricaPagina(fp,Cfg.nLinesPerForm);
					PulisciPagina(Cfg.nLinesPerForm);
					nRigheStampate=0;
				}
			}
		}
		DBclear(DBRes);

		if (nRigheStampate) {
			StampaFinePagina(&DatiOrdine,&DatiCollo,nPagina,FALSE);
			ScaricaPagina(fp,Cfg.nLinesPerForm);
			PulisciPagina(Cfg.nLinesPerForm);
		}
	}
	LiberaPagina();

	return bOK;
}

void StampaRiga(PDATIRIGA pDatiRiga, int nRiga)
{
	char szQTSPE[40];
	char szPZPRO[40];
	char szCDUBI[40];
	char szUB1[40];
	char szUB2[40];
	char szUB3[40];
	char szTmpBuf[128];

	sprintf(szQTSPE,"%4d",pDatiRiga->nRPQTSPE);
	sprintf(szPZPRO,"%7s ", NotazioneConVirgola(pDatiRiga->nRAPZPRO,szTmpBuf));

	strcpy(szUB1, SubStr(pDatiRiga->szRPCDUBI,0,3));	/* Fila */
	strcpy(szUB2, SubStr(pDatiRiga->szRPCDUBI,6,3));  /* Colonna */
	strcpy(szUB3, SubStr(pDatiRiga->szRPCDUBI,9,3));  /* Piano */

	sprintf(szCDUBI,"%s.%s.%s",szUB1,szUB2,szUB3);

	PrintRC(nPrintingRow[nRiga], Pos[F_STOCCAGGIO].nSC, szCDUBI);
	PrintRC(nPrintingRow[nRiga], Pos[F_QUANTITA].nSC,   szQTSPE);
	PrintRC(nPrintingRow[nRiga], Pos[F_TITOLO].nSC,     pDatiRiga->szPRDSTIT);
	PrintRC(nPrintingRow[nRiga], Pos[F_AUTORE].nSC,     pDatiRiga->szPRDSAUT);
	PrintRC(nPrintingRow[nRiga], Pos[F_CODPRO].nSC,     pDatiRiga->szRPCDPRO);
	PrintRC(nPrintingRow[nRiga], Pos[F_PREZZO].nSC,     szPZPRO);
}


/* 
********************************************************************************************
*
* Funzione: 
* PrintBollaRAC (char *pszPrinterName, char *pszKey) 
*
* Descrizione:
* Invia alla stampante printronix specificata il file di programmazione per la stampa della
* bolla specificata
* 
* Riceve:
* char - *pszPrinterName nome della stampante printronix sulla quale si intende stampare
* char - *pszKey codice di spedizione nel formato ELCCDSOD+ELCCDMAG+ELCAADOC+ELCPRDOC
*
* Ritorna:
* TRUE se la stampa e' terminata con successo 
* FALSE in caso di problemi
*
* Nel caso in cui il valore di ritorno sia FALSE, vedere il tracefile per 
* avere indicazioni sulla causa dell'insuccesso
*
********************************************************************************************
*/
ep_bool_t PrintBollaRAC(char *pszPrinterName, char *szOrdProg)
{
	FILE *fp;
	char szFileName[256];
	ep_bool_t bOK=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "PrintBollaRAC(%s)",szOrdProg);
#endif

	/*
	* Apertura del file ASCII da inviare alla stampante specificata
	* e relativa inizializzazione
	*/
	sprintf(szFileName,"%s/%s.rac",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		InitFile(fp);

		bOK=StampaBolla(fp, szOrdProg);
		fclose(fp);
	} else {
		bOK=FALSE;
	}
	if (bOK){
		PrintFileRaw(szFileName,pszPrinterName);
	}
	return(bOK);	
}

/*
********************************************************************************************
*
* Funzione: 
* PrintRACLinea (char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN) 
*
* Descrizione:
* Invia alla stampante printronix specificata il file di programmazione per la stampa di
* tutte le bolle elaborate
* 
* Riceve:
* char - *pszPrinterName nome della stampante printronix sulla quale si intende stampare
*
* Ritorna:
* TRUE se la stampa e' terminata con successo 
* FALSE in caso di problemi
*
* Nel caso in cui il valore di ritorno sia FALSE, vedere il tracefile per 
* avere indicazioni sulla causa dell'insuccesso
*
********************************************************************************************
*/
ep_bool_t PrintRACLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN) 
{
	FILE *fp;
	char szFileName[256];
	char szMsg[256];
	ep_bool_t bOK=TRUE;
	DBresult *DBRes=NULL;
	int nIndex;
	int nTuples;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "PrintRACLinea(%s-%s:%s:%s)",pszPrinterName,szNMCED,szTPSPE,szCDLIN);
#endif

	/*
	* Apertura del file ASCII da inviare alla stampante specificata
	* e relativa inizializzazione
	*/
	sprintf(szFileName,"%s/%s.rac",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		InitFile(fp);

		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s' order by roprgln;", ORDINE_ELABORATO,Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN);
		if((nTuples=DBntuples(DBRes))){
			for(nIndex=0;nIndex<nTuples;nIndex++){
				if(StampaBolla(fp, DBgetvalue(DBRes,nIndex,0))==FALSE){
					sprintf(szMsg,"Errore in stampa RAC ordine [%s]\n",DBgetvalue(DBRes,nIndex,0));
					SendMessage(Cfg.nMainID, nPID, DISPLAY_MSG, szMsg);
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Errore in stampa RAC ordine [%s]",DBgetvalue(DBRes,nIndex,0));
#endif
				} else {
					SendMessage(Cfg.nMainID, nPID, PRINTLABEL_RAC_STAMPATO,DBgetvalue(DBRes,nIndex,0));
				}
			}
		} else {
			bOK=FALSE;
		}

		fclose(fp);
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Errore in apertura file [%s]",szFileName);
#endif
		bOK=FALSE;
	}
	if (bOK){
		PrintFileRaw(szFileName,pszPrinterName);
	}
	return(bOK);	
}


/*
********************************************************************************************
*
* Funzione: 
* PrintBolleElaborateRAC (char *pszPrinterName) 
*
* Descrizione:
* Invia alla stampante printronix specificata il file di programmazione per la stampa di
* tutte le bolle elaborate
* 
* Riceve:
* char - *pszPrinterName nome della stampante printronix sulla quale si intende stampare
*
* Ritorna:
* TRUE se la stampa e' terminata con successo 
* FALSE in caso di problemi
*
* Nel caso in cui il valore di ritorno sia FALSE, vedere il tracefile per 
* avere indicazioni sulla causa dell'insuccesso
*
********************************************************************************************
*/
ep_bool_t PrintBolleRAC(char *pszPrinterName)
{
	FILE *fp;
	char szFileName[256];
	char szMsg[256];
	ep_bool_t bOK=TRUE;
	DBresult *DBRes=NULL;
	int nIndex;
	int nTuples;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "PrintBolleRAC(%s)",pszPrinterName);
#endif

	/*
	* Apertura del file ASCII da inviare alla stampante specificata
	* e relativa inizializzazione
	*/
	sprintf(szFileName,"%s/%s.rac",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		InitFile(fp);

		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' order by roprgln;",
			ORDINE_ELABORATO,Cfg.szTipoOrdini);
		if((nTuples=DBntuples(DBRes))){
			for(nIndex=0;nIndex<nTuples;nIndex++){
				if(StampaBolla(fp, DBgetvalue(DBRes,nIndex,0))==FALSE){
					sprintf(szMsg,"Errore in stampa RAC ordine [%s]\n",DBgetvalue(DBRes,nIndex,0));
					SendMessage(Cfg.nMainID, nPID, DISPLAY_MSG, szMsg);
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Errore in stampa RAC ordine [%s]",DBgetvalue(DBRes,nIndex,0));
#endif
				} else {
					SendMessage(Cfg.nMainID, nPID, PRINTLABEL_RAC_STAMPATO,DBgetvalue(DBRes,nIndex,0));
				}
			}
		} else {
			bOK=FALSE;
		}

		fclose(fp);
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Errore in apertura file [%s]",szFileName);
#endif
		bOK=FALSE;
	}
	if (bOK){
		PrintFileRaw(szFileName,pszPrinterName);
	}
	return(bOK);	
}
