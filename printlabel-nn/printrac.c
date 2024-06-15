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
* szCDVET        : Codice Vettore
* Ritorno:
*/
BOOL StampaRACLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char * szCDVET,int nGID)
{
	PGresult *PGRes;
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	char szOrdProg[128];
	char szMsg[128];
	int nPagine=0;
	int nTuples;
	int nIndex;
	BOOL bOK=TRUE;
	BOOL bStampa=FALSE;
	FILE *fp;


#ifdef TRACE
	trace_out_vstr_date(1,"Stampa RAC Linea [%s - %s - %s - %s]",szNMCED,szTPSPE,szCDLIN,szCDVET);
#endif
	sprintf(szPrintFile,"%s/%s.rac_linea",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szPrintFile,"w"))!=(FILE *)NULL) {

		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from ric_ord where ordtipo='%s' and ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s' order by ordprog;",Cfg.szTipoOrdini,szNMCED,szTPSPE,szCDLIN,szCDVET);
		if((nTuples=PQntuples(PGRes))){

			for(nIndex=0;nIndex<nTuples;nIndex++){
				strcpy(szOrdProg,PQgetvalue(PGRes,nIndex,0));
				if(StampaEtichetteOrdine(fp,szOrdProg,pszPrinterName,nGID)){
					SendMessage(Cfg.nMainID, nPID, PRINTLABEL_RAC_STAMPATO,szOrdProg);
				} else {
					sprintf(szMsg,"Errore in stampa RAC ordine [%s]\n",szOrdProg);
					SendMessage(Cfg.nMainID, nPID, DISPLAY_MSG, szMsg);
				}
			}
		}
		DBclear(PGRes);
	
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
BOOL StampaAllRAC(char *pszPrinterName,int nGID)
{
	PGresult *PGRes;
	DATIORDINE DatiOrdine;
	char szPrintFile[128];
	char szOrdProg[128];
	char szMsg[128];
	int nPagine=0;
	int nTuples;
	int nIndex;
	BOOL bOK=TRUE;
	BOOL bStampa=FALSE;
	FILE *fp;


#ifdef TRACE
	trace_out_vstr_date(1,"Stampa ALL RAC");
#endif
	sprintf(szPrintFile,"%s/%s.all_rac",Cfg.szPathStampe,pszPrinterName);
	if ((fp=fopen(szPrintFile,"w"))!=(FILE *)NULL) {
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' order by roprgln;",
			ORDINE_ELABORATO,Cfg.szTipoOrdini);
		if((nTuples=PQntuples(PGRes))){

			for(nIndex=0;nIndex<nTuples;nIndex++){
				strcpy(szOrdProg,PQgetvalue(PGRes,nIndex,0));
				if(StampaEtichetteOrdine(fp,szOrdProg,pszPrinterName,nGID)){
					SendMessage(Cfg.nMainID, nPID, PRINTLABEL_RAC_STAMPATO,szOrdProg);
				} else {
					sprintf(szMsg,"Errore in stampa RAC ordine [%s]\n",szOrdProg);
					SendMessage(Cfg.nMainID, nPID, DISPLAY_MSG, szMsg);
				}
			}
		}
		DBclear(PGRes);

		fclose(fp);

		PrintFileRaw(szPrintFile,pszPrinterName);
	}

	return bOK;
}



/*
* StampaEtichetteOrdine(FILE *fp,char *pszOrdProg,char *pszPrinterName,BOOL bSendMsg,int nGID)
* Descrizione
*  Funzione che stampa le etichette di tutti i colli dell'ordine specificato
*	Parametri di ingresso
*  -pszOrdProg : progressivo ordine
*  -pszPrinterName: nome della stampante
*/
BOOL StampaEtichetteOrdine(FILE *fp,char *pszOrdProg,char *pszPrinterName,int nGID)
{
	char szFileName[128];
	FILE *fpEtichetta;
	char szSQLCmd[1024];
	BOOL bRetVal=FALSE;
	BOOL bRC;

	if(fp==(FILE *)NULL){
		sprintf(szFileName,"%s/O%s",Cfg.szPathStampe,pszOrdProg);
		if ((fpEtichetta=fopen(szFileName,"w"))==(FILE *)NULL) {
			return FALSE;
		}
	} else {
		fpEtichetta=fp;
	}

#ifdef TRACE
	trace_out_vstr_date(1, "StampaEtichetteOrdine(%s,%s)",pszOrdProg,pszPrinterName);
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
BOOL StampaEtichettaCollo(char *pszOrdProg,int nCollo,char *pszPrinterName,int nGID)
{
	char szFileName[128];
	FILE *fpEtichetta;
	char szSQLCmd[1024];
	BOOL bRetVal=TRUE;

#ifdef TRACE
	trace_out_vstr_date(1, "StampaEtichettaCollo(%s,%d,%s)",pszOrdProg,nCollo,pszPrinterName);
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

#define MAX_RIGHE_ETICHETTA 21

/*
* Descrizione
* Stampa etichetta
* Parametri di ingresso
*  fp : file sul quale si produce la stampa
*/
BOOL StampaEtichette(char *szSQLCmd,FILE *fp,char *szLabelPrinterName)
{
	FILE *fpBase;
	char szBuffer[256];
	char szDestination[256];
	char *pPtr;
	int nIndex;
	int nColloIndex;
	int nFieldIndex;
	BOOL bFound;
	BOOL bSaltaRiga=FALSE;
	char szLabelBaseFileName[256];
	char szOrdProg[256];
	char szDSTSP[256];
	int nCPNMCOL;
	int nORDPROG;
	int nCollo;
	int nRigaIndex=0;
	int nPagine=0;
	int nRighe;
	int nRigheCollo;
	PGresult *PGRes;
	PGresult *PGResRighe;
	BOOL bFirstUBIC=TRUE;
	BOOL bFirstCDPRO=TRUE;
	BOOL bFirstTITLE=TRUE;
	BOOL bFirstNMCPE=TRUE;
	BOOL bFirstPREZZO=TRUE;

#ifdef TRACE
	trace_out_vstr_date(1, "StampaEtichette(%s)",szSQLCmd);
#endif

	PGRes=DBExecQuery(Cfg.nDebugVersion,szSQLCmd);
	if(PQntuples(PGRes)){

		if((nCPNMCOL=PQfnumber(PGRes,"cpnmcol"))==-1){
			DBclear(PGRes);
			return FALSE;
		}
		if((nORDPROG=PQfnumber(PGRes,"ordprog"))==-1){
			DBclear(PGRes);
			return FALSE;
		}
	}
	DBclear(PGRes);

	/*
	* Rilancio la select dei colli da stampare
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion,szSQLCmd);
	if(PQntuples(PGRes)){
		for(nColloIndex=0;nColloIndex<PQntuples(PGRes);nColloIndex++){
			strcpy(szOrdProg,PQgetvalue(PGRes,nColloIndex,nORDPROG));
			nCollo=atoi(PQgetvalue(PGRes,nColloIndex,nCPNMCOL));
			GetDSTSP(szOrdProg,szDSTSP);

			/* primo file - etichetta vera e propria */
			sprintf(szLabelBaseFileName,"%s/%s.1",Cfg.szPathData,Cfg.szLabelBaseName);
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
							} else {
								/* normale campo db - stampo com'e' */
								if((nFieldIndex=PQfnumber(PGRes,LabelFields[nIndex].szFieldName))==-1){
									strcat(szDestination,"");
								} else {
									strcat(szDestination,StrTrimAll(PQgetvalue(PGRes,nColloIndex,nFieldIndex)));
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

			/* eventuale etichetta esselunga */
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
							} else {
								/* normale campo db - stampo com'e' */
								if((nFieldIndex=PQfnumber(PGRes,LabelFields[nIndex].szFieldName))==-1){
									strcat(szDestination,"");
								} else {
									strcat(szDestination,StrTrimAll(PQgetvalue(PGRes,nColloIndex,nFieldIndex)));
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


			PGResRighe=DBExecQuery(Cfg.nDebugVersion,"select ubicazione_ridotta(rpcdubi),rpcdpro,prdstit,to_char(rppzpre::float/100::float,'990D99'),lpad(rpqtord,5) from rig_prod r,catalogo c where r.rpcdpro=c.prcdpro and r.ordprog='%s' and r.rpnmcol=%d order by r.rpprrig;",szOrdProg,nCollo);
			nRigheCollo=PQntuples(PGResRighe);
			DBclear(PGResRighe);

			nRigaIndex=0;
			nPagine=0;
			while(nRigheCollo>nPagine*MAX_RIGHE_ETICHETTA){

				bFirstUBIC=TRUE;
				bFirstCDPRO=TRUE;
				bFirstTITLE=TRUE;
				bFirstNMCPE=TRUE;
				bFirstPREZZO=TRUE;

				/* secondo file - lista articoli */
				sprintf(szLabelBaseFileName,"%s/%s.2",Cfg.szPathData,Cfg.szLabelBaseName);
				if((fpBase=fopen(szLabelBaseFileName,"r"))!=(FILE *)NULL){

					PGResRighe=DBExecQuery(Cfg.nDebugVersion,"select ubicazione_ridotta(rpcdubi),rpcdpro,prdstit,to_char(rppzpre::float/100::float,'990D99'),lpad(rpqtord,5) from rig_prod r,catalogo c where r.rpcdpro=c.prcdpro and r.ordprog='%s' and r.rpnmcol=%d order by r.rpprrig;",szOrdProg,nCollo);

					if((nRighe=PQntuples(PGResRighe))){

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
										if(nRigaIndex<nRighe) strcat(szDestination, PQgetvalue(PGResRighe,nRigaIndex,0)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDCDPRO>")){
										if(bFirstCDPRO) {
											bFirstCDPRO=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, PQgetvalue(PGResRighe,nRigaIndex,1)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDTITLE>")){
										if(bFirstTITLE) {
											bFirstTITLE=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, PQgetvalue(PGResRighe,nRigaIndex,2)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<PREZZO>")){
										if(bFirstPREZZO) {
											bFirstPREZZO=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, PQgetvalue(PGResRighe,nRigaIndex,3)); else strcat(szDestination,"");
										nRigaIndex++;
									} else if(!strcmp((LabelFields[nIndex].szFieldID),"<IDNMCPE>")){
										if(bFirstNMCPE) {
											bFirstNMCPE=FALSE;
											nRigaIndex=nPagine*MAX_RIGHE_ETICHETTA;
										}
										if(nRigaIndex<nRighe) strcat(szDestination, PQgetvalue(PGResRighe,nRigaIndex,4)); else strcat(szDestination,"");
										nRigaIndex++;
									} else {
										/* normale campo db - stampo com'e' */
										if((nFieldIndex=PQfnumber(PGRes,LabelFields[nIndex].szFieldName))==-1){
											strcat(szDestination,"");
										} else {
											strcat(szDestination,StrTrimAll(PQgetvalue(PGRes,nColloIndex,nFieldIndex)));
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
					DBclear(PGResRighe);

					fclose(fpBase);
				}
				nPagine++;
			}
		}
	}
	DBclear(PGRes);

	return TRUE;
}


char *StampaListaCampo(char *szNomeCampo,char *szOrdProg,int nCollo)
{
	static char szLista[1024];
	PGresult *PGRes;
	int nIndex;
	char szBuffer[256];

	strcpy(szLista,"");

	PGRes=DBExecQuery(Cfg.nDebugVersion,"select %s from rig_prod where ordprog='%s' and rpnmcol=%d;",szNomeCampo,szOrdProg,nCollo);
	if(PQntuples(PGRes)){
		for(nIndex=0;nIndex<PQntuples(PGRes);nIndex++){
			sprintf(szBuffer,"%s\r\n",PQgetvalue(PGRes,nIndex,0));
			strcat(szLista,szBuffer);
		}
	}
	DBclear(PGRes);

	return szLista;
}

char *StampaListaUbicazioni(char *szOrdProg,int nCollo)
{
	static char szLista[1024];
	PGresult *PGRes;
	int nIndex;
	char szBuffer[256];

	strcpy(szLista,"");

	PGRes=DBExecQuery(Cfg.nDebugVersion,"select ubicazione_ridotta(rpcdubi) from rig_prod where ordprog='%s' and rpnmcol=%d;",szOrdProg,nCollo);
	if(PQntuples(PGRes)){
		for(nIndex=0;nIndex<PQntuples(PGRes);nIndex++){
			sprintf(szBuffer,"%s\r\n",PQgetvalue(PGRes,nIndex,0));
			strcat(szLista,szBuffer);
		}
	}
	DBclear(PGRes);

	return szLista;
}

char *StampaListaTitoli(char *szOrdProg,int nCollo)
{
	static char szLista[1024];
	PGresult *PGRes;
	int nIndex;
	char szBuffer[256];

	strcpy(szLista,"");

	PGRes=DBExecQuery(Cfg.nDebugVersion,"select prdstit,rpcdpro from rig_prod r,catalogo c where r.ordprog='%s' and r.rpnmcol=%d and r.rpcdpro=c.prcdpro;",szOrdProg,nCollo);
	if(PQntuples(PGRes)){
		for(nIndex=0;nIndex<PQntuples(PGRes);nIndex++){
			sprintf(szBuffer,"%s\r\n",PQgetvalue(PGRes,nIndex,0));
			strcat(szLista,szBuffer);
		}
	}
	DBclear(PGRes);

	return szLista;
}

BOOL GetDSTSP(char *szOrdProg,char *szDSTSP)
{
	BOOL bFound=TRUE;
	PGresult *PGRes;
	DATIORDINE DatiOrdine;
	char szFLNOT[80];
	

	if(!GetDatiOrdine(szOrdProg,&DatiOrdine,FALSE)){
		return FALSE;
	}
//	/*
//	* rm 17-03-2004 : gestione descrizione linea diviso per cedola 
//	*/
//	/*
//	* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
//	*/
//	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='%s';", 
//		DatiOrdine.szROCDSOC, 
//		DatiOrdine.szROCDMAG, 
//		DatiOrdine.szRONMCED, 
//		DatiOrdine.szROTPSPE,
//		DatiOrdine.szROCDLIN);
//	if(PQntuples(PGRes)){
//		strcpy(szDSTSP,PQgetvalue(PGRes,0,0));
//		bFound=TRUE;
//	} else {
//		bFound=FALSE;
//	}
//	DBclear(PGRes);
//
//	if(!bFound){
//		/* rm 27-09-2004 : primo giro bis: cerco con cdlin = '' */
//		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select elcdstsp from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='';", 
//			DatiOrdine.szROCDSOC, 
//			DatiOrdine.szROCDMAG, 
//			DatiOrdine.szRONMCED, 
//			DatiOrdine.szROTPSPE);
//		if(PQntuples(PGRes)){
//			strcpy(szDSTSP,PQgetvalue(PGRes,0,0));
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
		szOrdProg);
	if(PQntuples(PGRes)){
		strcpy(szDSTSP,PQgetvalue(PGRes,0,0));
		strcpy(szFLNOT,PQgetvalue(PGRes,0,1));
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
			DatiOrdine.szROCDSOC, 
			DatiOrdine.szROCDMAG, 
			DatiOrdine.szROTPSPE);

		if(PQntuples(PGRes)){
			strcpy(szDSTSP,PQgetvalue(PGRes,0,0));
		} else {
			strcpy(szDSTSP,"ERR");
		}
		DBclear(PGRes);
	}
	return TRUE;
}
