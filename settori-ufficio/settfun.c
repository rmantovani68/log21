/*
* settfun.c : gestione settori di magazzino : funzioni di gestione
* 
* Progetto Easy Picking 3.0 : wella
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <glib.h>

#include <glib.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <picking.h>
#include <proc_list.h>

#include <ep-common.h>
#include <ep-db.h>

#include	"linklist.h"
#include	"settstruct.h"
#include	"settext.h"
#include	"settfun.h"


unsigned int nGetOrderTimePassed[MAX_ISOLE];
struct timeval tvOldTime[MAX_ISOLE];

/*
* void ReadConfiguration(void)
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(BOOL bReadProcInfo)
{
	char szParagraph[128];
	char szCurrentDirectory[128];
	char szProcIndex[128];
	char szProcBuffer[128];
	char szCfgFileName[128];
	int nIndex;


	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(szCfgFileName,szCurrentDirectory);
	strcat(szCfgFileName,"/");
	strcat(szCfgFileName, __configuration_file__);

	strcpy(szParagraph,"General Settings");
	GetFileString(szParagraph,"TipoOrdini", "X",           Cfg.szTipoOrdini,   80, szCfgFileName, NULL);
	GetFileString(szParagraph,"PathData",   "../data",     Cfg.szPathData,     80, szCfgFileName, NULL);
	GetFileString(szParagraph,"PathExport", "../export",   Cfg.szPathExport,   80, szCfgFileName, NULL);
	GetFileString(szParagraph,"PathExe",    "./",          Cfg.szPathExe,      80, szCfgFileName, NULL);
	GetFileString(szParagraph,"PathTrace",  "../trace",    Cfg.szPathTrace,    80, szCfgFileName, NULL);

	Cfg.nShmKey            = GetFileInt(szParagraph,"ShmKey",         256, szCfgFileName,NULL);
	Cfg.nSettoriShmKey     = GetFileInt(szParagraph,"SettoriShmKey",  257, szCfgFileName,NULL);
	Cfg.nStampaRAC         = GetFileInt(szParagraph,"StampaRAC",        0, szCfgFileName,NULL);
	Cfg.nImballiUbicati    = GetFileInt(szParagraph,"ImballiUbicati",   0, szCfgFileName,NULL);
	Cfg.nChiusuraCollo     = GetFileInt(szParagraph,"ChiusuraCollo",    1, szCfgFileName,NULL);
	Cfg.nFineOrdine        = GetFileInt(szParagraph,"FineOrdine",       1, szCfgFileName,NULL);
	Cfg.nDebugVersion      = GetFileInt(szParagraph,"DebugVersion",     0, szCfgFileName,NULL);
	Cfg.bRifornimenti      = GetFileInt(szParagraph,"Rifornimenti",     0, szCfgFileName, NULL);

	if(bReadProcInfo){
		ReadProcInfo(szCfgFileName);
	}

	/*
	* parametri specifici dell'applicazione
	*/
	/* Delay in millisecondi */
	Cfg.nDelay          = GetFileInt(ProcessGetName(PROC_SETTORI),"Delay",         100, szCfgFileName,NULL);
	Cfg.nGetOrderDelay  = GetFileInt(ProcessGetName(PROC_SETTORI),"GetOrderDelay",1000, szCfgFileName,NULL);

	Cfg.nNumeroIsole    = GetFileInt(ProcessGetName(PROC_SETTORI),"NumeroIsole",     1, szCfgFileName,NULL);
	Cfg.nNumeroSettori  = GetFileInt(ProcessGetName(PROC_SETTORI),"NumeroSettori",  12, szCfgFileName,NULL);

	/* Numero di errori IOS */
	Cfg.nMaxDamages     = GetFileInt(ProcessGetName(PROC_SETTORI),"MaxDamages",      5, szCfgFileName,NULL);

	/* Numero dei moduli PKL */
	Cfg.nPKLNumber      = GetFileInt(ProcessGetName(PROC_IOS),"PKLNumber",           1, szCfgFileName,NULL);

	/*
	* dati relativi ai moduli PKL
	*/
	for(nIndex=0; nIndex < Cfg.nPKLNumber; nIndex++){
		sprintf(szProcIndex,"PKL_%02d",nIndex);
		GetFileString(ProcessGetName(PROC_IOS),szProcIndex, "", szProcBuffer, 80,szCfgFileName,NULL);

		sscanf(szProcBuffer, "%d,%d,%d",
			&(Cfg.PKL[nIndex].nIOS),
			&(Cfg.PKL[nIndex].nCPU),
			&(Cfg.PKL[nIndex].nModulo));
	}
	/*
	* lettura dei dati di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	GetFileString(szParagraph,"PGHost",    "localhost",Cfg.szPGHost,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"PGPort",    "5432",     Cfg.szPGPort,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"PGDataBase","picking",  Cfg.szPGDataBase, 80,szCfgFileName,NULL); 

}



/*
* cerca in archivio operativo ordini il primo ordine da prelevare 
* Se StampaRAC = 0 : in stato 'R' : Stampata RAC
* Se StampaRAC = 1 : in stato 'A' : Elaborato
*/
BOOL GetOrdine(PORDINE pOrdine,int nIsola)
{
	DBresult *PGRes = NULL;
	int nTuples;
	char cStatoOrdine;
	char szBuffer[80];
	BOOL bFound=FALSE;
	BOOL bProducibile=FALSE;
	int nIndex;
	int nColliAutomatici;

	switch(Cfg.nStampaRAC){
		case DOPO_IL_LANCIO:
			cStatoOrdine = ORDINE_STAMPATA_RAC;
		break;
		case INIZIO_PRELIEVO:
			cStatoOrdine = ORDINE_ELABORATO;
		break;
	}

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select rostato,ordprog,ronmcll,ronmrgp,ronmcpp,rocdflg,ronmcla from ric_ord where rostato='%c' AND rocdflg!='%c' and ordtipo='%s' order by roprgln,ordprog;",cStatoOrdine,ORDINE_BLOCCATO,Cfg.szTipoOrdini);

	nTuples=DBntuples(PGRes);
#ifdef TRACE_ELIMINATO
	trace_out_vstr_date(1,"GetOrdine(%d) : Trovati %d ordini",nIsola,nTuples);
#endif

	/*
	* A questo punto ho ottenuto 0 o piu' ordini di produzione in stato 'A' : Analizzati, 
	* relativi all'isola di magazzino in oggetto e con almeno un collo di produzione
	*/
	nIndex=0;
	while(nIndex<nTuples && !bFound){
		nColliAutomatici=atoi(DBgetvalue(PGRes,nIndex,6));
		if(nColliAutomatici){
			strcpy(pOrdine->szOrdProg,DBgetvalue(PGRes,nIndex,1));
			pOrdine->nColli=atoi(DBgetvalue(PGRes,nIndex,2));
			pOrdine->nRighe=atoi(DBgetvalue(PGRes,nIndex,3));
			pOrdine->nCopie=atoi(DBgetvalue(PGRes,nIndex,4));
			strcpy(szBuffer,DBgetvalue(PGRes,nIndex,5)); pOrdine->cFlag=szBuffer[0];
			bFound=TRUE;
			continue;
		} else {
			char szMsg[128];
			/*
			* rm 14-11-2001 : prima verifico la producibilita' !
			*/
			strcpy(pOrdine->szOrdProg,DBgetvalue(PGRes,nIndex,1));
			pOrdine->nColli=atoi(DBgetvalue(PGRes,nIndex,2));
			pOrdine->nRighe=atoi(DBgetvalue(PGRes,nIndex,3));
			pOrdine->nCopie=atoi(DBgetvalue(PGRes,nIndex,4));
			strcpy(szBuffer,DBgetvalue(PGRes,nIndex,5)); pOrdine->cFlag=szBuffer[0];

			if(!OrdineProducibile(pOrdine)){
#ifdef TRACE
				trace_out_vstr_date(1,"GetOrdine : Ordine MANUALE [%s] NON PRODUCIBILE", pOrdine->szOrdProg);
#endif
				sprintf(szMsg,"ATTENZIONE : Ordine MANUALE [%s] NON PRODUCIBILE\n", pOrdine->szOrdProg);
				SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
				SendMessage(PROC_MAIN, PROC_SETTORI, REFRESH_ORDINE, pOrdine->szOrdProg);
				bProducibile=FALSE;
			} else {
				/*
				* Se l'ordine e' producibile allora impegno le copie 
				* (verranno evasi da main (EvadiOrdine())
				*/
				ProduzioneOrdine(pOrdine);
				bProducibile=TRUE;
			}

			if(bProducibile){
				/*
				* Messaggio di Ordine in prelievo (fasullo) a MAIN
				* per fare scattare la gestione stampe su ordine in prelievo
				*/
				sprintf(szMsg,"ATTENZIONE : Ordine [%s] senza colli di linea : EVASIONE AUTOMATICA\n",DBgetvalue(PGRes,nIndex,1)); 
				SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
				trace_out_vstr_date(1,"Ordine [%s] EVASO : nessun collo di linea automatica", DBgetvalue(PGRes,nIndex,1));
#endif
				CambiaStatoOrdine(DBgetvalue(PGRes,nIndex,1), ORDINE_PRELEVATO);
				SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_PRELEVATO, DBgetvalue(PGRes,nIndex,1));
			}
		}
		nIndex++;
	}

	DBclear(PGRes);

	return(bFound);
}

/*
* 10-12-2001 rm
* Verifico forzatura ordine
*/
BOOL OrdineForzato(PORDINE pOrdine)
{

	if(pOrdine->cFlag==ORDINE_FORZATO){
		/*
		* L'ordine e' forzato
		*/
		return TRUE;
	}
	return FALSE;
}

/*
* 30-08-2001 rm
* Problema : se le copie richieste sono 180 e la qt.max e' 30 come faccio ??????
* l'ordine rimane bloccato a vita !!!
*/
BOOL OrdineProducibile(PORDINE pOrdine)
{
	DBresult *PGRes = NULL;
	DBresult *PGResUbi = NULL;
	DBresult *PGResRighe = NULL;
	int nTuples;
	char szCodProd[128];
	char szUbicazione[128];
	int nQTRic;	/* Quantita' Richiesta per il prodotto */
	int nQTCas;	/* Quantita' presente in ubicazione */
	int nQTImp;	/* Quantita' impegnata in ubicazione */
	BOOL bProducibile=TRUE;
	BOOL bRetValue=TRUE;
	int nIndex;

	if(!Cfg.bRifornimenti){
		return TRUE;
	}

	/*
	* Si ragiona per righe di produzione
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select rpcdpro,sum(rpqtspe) from rig_prod where ordprog='%s' group by rpcdpro;",pOrdine->szOrdProg);

	if (PGRes && DBresultStatus(PGRes) == DBRES_TUPLES_OK ) {
		if((nTuples=DBntuples(PGRes))){
			for(nIndex=0;nIndex<nTuples;nIndex++){
				/*
				* Verifico producibilita' riga
				*/
				strcpy(szCodProd,DBgetvalue(PGRes,nIndex,0));
				nQTRic=atoi(DBgetvalue(PGRes,nIndex,1));

				/*
				* verifico giacenza e impegnato
				*/
				PGResUbi=DBExecQuery(Cfg.nDebugVersion>1,"select ubcdubi,ubqtcas,ubqtimp from ubicazioni where codprod='%s' and ubtpubi='%s' order by ubprior;",szCodProd,Cfg.szTipoOrdini);
				if(DBntuples(PGResUbi)){
					strcpy(szubcdubi,DBgetvalue(PGResUbi,0,0));
					nQTCas=atoi(DBgetvalue(PGResUbi,0,1));
					nQTImp=atoi(DBgetvalue(PGResUbi,0,2));

					/*
					* Controllo giacenze
					*/

					if(nQTRic > nQTCas-nQTImp){
						/*
						* Se almeno una riga non e' producibile allora 
						* significa che l'ordine non e' producibile 
						*/
						bProducibile=FALSE;

						/*
						* Marco la riga ricevuta come bloccata / bloccante
						*/
						PGResRighe=DBExecQuery(Cfg.nDebugVersion>1,"update ric_art set racdflg='%c' where ordprog='%s' and racdpro='%s';",RIGA_BLOCCATA,pOrdine->szOrdProg,szCodProd);
						DBclear(PGResRighe);
					}
				}
				DBclear(PGResUbi);
			}
		}
	} else {
		bRetValue=FALSE;
	}

	DBclear(PGRes);

	if(bRetValue){
		if(!bProducibile){
			/*
			* cambio flg
			*/
			CambiaFlagOrdine(pOrdine->szOrdProg, ORDINE_BLOCCATO);
		}
	}

	return bRetValue ? bProducibile : bRetValue;
}

BOOL ProduzioneOrdine(PORDINE pOrdine)
{
	DBresult *PGRes = NULL;
	DBresult *PGResImp = NULL;
	int nTuples;
	char szBuffer[128];
	char cFlag;
	char szCodProd[128];
	char szUbicazione[128];
	int nQTRic;	/* Quantita' Richiesta per il prodotto */
	int nQTCas;	/* Quantita' presente in ubicazione */
	int nQTImp;	/* Quantita' impegnata in ubicazione */
	int nCollo;
	int nProgRiga;
	BOOL bRetValue=TRUE;
	int nIndex;

	if(!Cfg.bRifornimenti){
		return TRUE;
	}

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"begin work;");
	DBclear(PGRes);

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select rpcdpro,rpcdubi,rpnmcol,rpprrig,rpqtord,ubqtcas,ubqtimp,rpcdflg from rig_prod,ubicazioni where rpcdubi=ubicazione and ordprog='%s';",pOrdine->szOrdProg);
	if((nTuples=DBntuples(PGRes))){
		for(nIndex=0;nIndex<nTuples;nIndex++){

			strcpy(szCodProd,DBgetvalue(PGRes,nIndex,0));
			strcpy(szubcdubi,DBgetvalue(PGRes,nIndex,1));

			nCollo=atoi(DBgetvalue(PGRes,nIndex,2));
			nProgRiga=atoi(DBgetvalue(PGRes,nIndex,3));
			nQTRic=atoi(DBgetvalue(PGRes,nIndex,4));
			nQTCas=atoi(DBgetvalue(PGRes,nIndex,5));
			nQTImp=atoi(DBgetvalue(PGRes,nIndex,6));
			strcpy(szBuffer,DBgetvalue(PGRes,nIndex,7)); cFlag=szBuffer[0];

			/*
			* aggiorno l'impegnato della ubicazione
			* rm 29-01-02 : modificata la gestione della evasione righe per evitare scritture non aggiornate
			*/
			PGResImp=DBExecQuery(Cfg.nDebugVersion>1,"update ubicazioni set ubqtimp=ubqtimp+%d where ubicazione='%s';",nQTRic,szUbicazione);
			if(atoi(DBcmdTuples(PGResImp))==0){
				bRetValue=FALSE;
			}
			DBclear(PGResImp);
			PGResImp=DBExecQuery(Cfg.nDebugVersion>1,"update rig_prod set rpcdflg='%c' where ordprog='%s' and rpnmcol=%d and rpprrig=%d;",RIGA_IMPEGNATA,pOrdine->szOrdProg,nCollo,nProgRiga);
			if(atoi(DBcmdTuples(PGResImp))==0){
				bRetValue=FALSE;
			}
			DBclear(PGResImp);

		}
	} else {
		bRetValue=FALSE;
	}

	DBclear(PGRes);
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"commit work;");
	DBclear(PGRes);

	return bRetValue;
}


/*
* Operazioni di inizio prelievo nel settore
* ricerca di un ordine in attesa di prelievo e sua immissione in linea
*/
BOOL InizioPrelievoSettore(PORDINE pOrdine,PSETTORE pSettore)
{
	char szMsgTxt[128];
	DBresult *PGRes;
	PDISPLAYSTRUCT pDisplay;
	int nTuples;
	int nIndex;
	int nRigaIndex;
	char szMsg[128];

	/*
	* comincio ad elaborare l'ordine :
	* esistono righe di prelievo in ubicazioni automatiche e in colli automatici ?
	*   SI :
	*      Esistono prodotti da prelevare in questo settore ?
	*      SI : 
	*        Carico tutte le righe dell'ordine relative al settore in esame 
	*      NO : 
	*        Passo ordine a settore successivo
	*   NO :
	*      Chiudo il collo e l'ordine
	*/
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select r.ordprog,r.rpcdpro,r.rpcdubi,r.rpqtspe,r.rpprrig,r.rpnmcol,u.ubplcnm,u.ubnmdsp,u.ubnmset,c.cpswlin from rig_prod as r, ubicazioni as u, col_prod as c where r.ordprog='%s' and r.ordprog=c.ordprog and r.rpnmcol=c.cpnmcol and u.ubcdubi=r.rpcdubi and u.ubnmisl=%d and r.rpcdflg not in ('%c','%c') and u.ubtpubi='%s' and c.cpswlin='%d' order by u.ubprior,r.rpnmcol,r.rpprrig;",
		pOrdine->szOrdProg,
		pSettore->nIsola,
		RIGA_TAGLIATA,
		RIGA_NON_UBICATA,
		Cfg.szTipoOrdini,
		COLLO_AUTOMATICO);

	if (!PGRes || DBresultStatus(PGRes) != DBRES_TUPLES_OK ) {
		/* errore */
#ifdef TRACE
		trace_out_vstr_date(1,"ATTENZIONE : ERRORE IN RICERCA UBICAZIONI ORDINE [%s]",pOrdine->szOrdProg);
#endif
		sprintf(szMsg,"ATTENZIONE : ERRORE IN RICERCA UBICAZIONI ORDINE [%s]\n", pOrdine->szOrdProg);
		SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
	} else {

		nTuples=DBntuples(PGRes);

		if(nTuples){
			/* 
			* ci sono righe di ubicazione automatica 
			*/

			/*
			* Resetto il numero copie
			*/
			nIndex=0;
			while(nIndex<nTuples){
				pSettore->RigaOrdine[nIndex].nNumCopie = 0;
				nIndex++;
			}
			/*
			* A questo punto ho ottenuto 0 o piu' righe di produzione 
			* relativi all'isola di magazzino ed al settore in oggetto 
			*/
			nIndex=0;
			nRigaIndex=0;
			while(nIndex<nTuples){
				/*
				* Se si tratta di una riga relativa al settore in esame ...
				*/
				if(pSettore->nubnmset==atoi(DBgetvalue(PGRes,nIndex,12))){
					strcpy(pSettore->RigaOrdine[nRigaIndex].szCodSped, DBgetvalue(PGRes,nIndex,0));
					strcpy(pSettore->RigaOrdine[nRigaIndex].szCodProd, DBgetvalue(PGRes,nIndex,1));
					strcpy(pSettore->RigaOrdine[nRigaIndex].szCodUbi,  DBgetvalue(PGRes,nIndex,2));
					if(Cfg.nChiusuraCollo){
						pSettore->RigaOrdine[nRigaIndex].nNumCopie         = atoi(DBgetvalue(PGRes,nIndex,3));
						pSettore->RigaOrdine[nRigaIndex].nProgressivo      = atoi(DBgetvalue(PGRes,nIndex,4));
						/*
						* prelievi differenziati per collo
						*/
						pSettore->RigaOrdine[nRigaIndex].nNumCollo         = atoi(DBgetvalue(PGRes,nIndex,5));
					} else {
						pSettore->RigaOrdine[nRigaIndex].nNumCopie        += atoi(DBgetvalue(PGRes,nIndex,3));
						pSettore->RigaOrdine[nRigaIndex].nProgressivo      = atoi(DBgetvalue(PGRes,nIndex,4));
						/*
						* prelievi non differenziati per collo
						*/
						pSettore->RigaOrdine[nRigaIndex].nNumCollo         = 1;
					}
					pSettore->RigaOrdine[nRigaIndex].nPLCNum           = atoi(DBgetvalue(PGRes,nIndex,6));
					pSettore->RigaOrdine[nRigaIndex].nDisplay          = atoi(DBgetvalue(PGRes,nIndex,7));
					pSettore->RigaOrdine[nRigaIndex].nIsola            = pSettore->nIsola;
					pSettore->RigaOrdine[nRigaIndex].nStato=0;

					/*
					* Assegnazione valore prossimo collo a scopo chiusura collo prima del passaggio a 
					* settore successivo
					*/
					if(Cfg.nChiusuraCollo){
						/*
						* Gestione Collo
						*/
						if(nIndex+1<nTuples){
							pSettore->RigaOrdine[nRigaIndex].nNextCollo = atoi(DBgetvalue(PGRes,nIndex+1,5));
						} else {
							pSettore->RigaOrdine[nRigaIndex].nNextCollo = -1;
						}
					} else {
						/*
						* Gestione Ordine
						*/
						if(nIndex+1<nTuples){
							pSettore->RigaOrdine[nRigaIndex].nNextCollo = 1;
						} else {
							pSettore->RigaOrdine[nRigaIndex].nNextCollo = -1;
						}
					}

					if(Cfg.nChiusuraCollo){
						nRigaIndex++;
					} else {
						if(nIndex+1<nTuples){
							if(strcmp(pSettore->RigaOrdine[nRigaIndex].szCodProd, DBgetvalue(PGRes,nIndex+1,1))){
								nRigaIndex++;
							}
						} else {
							nRigaIndex++;
						}
					}
				}
				nIndex++;
			}

			DBclear(PGRes);

			pSettore->nIndiceRigaOrdine=0;         /* puntatore alla riga d'ordine da prelevare */
			pSettore->nNumeroRigheOrdine=nRigaIndex;   /* numero delle righe d'ordine da prelevare */
			/*
			* se ci sono righe d'ordine da prelevare in questo settore
			* comincio dalla prima
			* altrimenti passo al prossimo settore
			*/
			if(nRigaIndex){
				PRIGA_PRELIEVO pRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);

				/* 
				* setto lo stato della riga (ATTESA_PRELIEVO)
				*/
				pRigaOrdine->nStato=ATTESA_PRELIEVO;

				/*
				* setto i dati relativi alla riga d'ordine corrente del settore
				*/
				strcpy(pSettore->szCodSped,pRigaOrdine->szCodSped);
				strcpy(pSettore->szCodProd,pRigaOrdine->szCodProd);
				strcpy(pSettore->szCodUbi,pRigaOrdine->szCodUbi);
				pSettore->nNumCollo=pRigaOrdine->nNumCollo;
				pSettore->nNumCopie=pRigaOrdine->nNumCopie;
				pSettore->nPLCNum=pRigaOrdine->nPLCNum;

				pSettore->nDisplay=GetDisplay(pRigaOrdine->nDisplay,pSettore->nIsola);

				pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

				/*
				* scrivo i dati per il prelievo sul display
				*/
				/*
				* OrdProg : OOOOOO
				* O : Ordine
				* si visualizza : OOOO (Ultime 4 cifre)
				*/
				strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
				sprintf(pDisplay->szRiga_2_Display,"P%3d",pSettore->nNumCopie);
				pDisplay->nStatoRiga1=NORMAL;
				pDisplay->nStatoRiga2=NORMAL;

				UpdateDisplay(pDisplay,TUTTO);

				/*
				* accendo la luce di prelievo prodotto
				*/
				pSettore->nStatoLampada=1;
				SetLampada(pSettore->nSettore,pSettore->nPLCNum);
				/*
				* setto lo stato settore in attesa di conferma copie prelevate
				*/
				pSettore->nStatoubnmset=PRELIEVO_COPIE;
			} else {
				/*
				* non ci sono righe per questo settore
				* scrivo i dati per il passaggio sul display
				*/
				pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

				strcpy(pSettore->szCodSped,pOrdine->szOrdProg);
				strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
				strcpy(pDisplay->szRiga_2_Display,"PPPP");
				pDisplay->nStatoRiga1=NORMAL;
				pDisplay->nStatoRiga2=BLINK;
				UpdateDisplay(pDisplay,TUTTO);

				/*
				* faccio confermare l'ultima lampada premuta 
				*/
				pSettore->nStatoLampada=1;

				SetLampada(pSettore->nSettore,pSettore->nPLCNum);

				pSettore->nStatoubnmset=PASSA_SETT_SUCC;
			}
		} else {
			/* non ci sono righe di ubicazione automatica : chiudo l'ordine */
			/*
			* scrivo i dati per il fine ordine sul display
			*/
			strcpy(pSettore->szCodSped,pOrdine->szOrdProg);

			pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
			strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
			strcpy(pDisplay->szRiga_2_Display,"FFFF");
			pDisplay->nStatoRiga1=NORMAL;
			pDisplay->nStatoRiga2=BLINK;
			UpdateDisplay(pDisplay,TUTTO);

			/*
			* faccio confermare l'ultima lampada premuta 
			*/
			pSettore->nStatoLampada=1;

			SetLampada(pSettore->nSettore,pSettore->nPLCNum);

			pSettore->nStatoubnmset=FINE_ORDINE;
			/*
			* Messaggio di Ordine Terminato a MAIN
			*/
			CambiaStatoOrdine(pOrdine->szOrdProg, ORDINE_PRELEVATO);
			/*
			* Messaggio di Ordine terminato a MAIN
			*/
			sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);
			SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_PRELEVATO, szMsgTxt);
		}
	}
	return TRUE;
}


/*
* GestioneSettore()
* Gestione settore di magazzino
* logica di funzionamento del sequenziatore 
* si tratta di un automa a stati finiti
*/
BOOL GestioneSettore(EVENTO *pEvento)
{
	static BOOL bProducibile=TRUE;
	ORDINE Ordine;
	PDISPLAYSTRUCT pDisplay;
	PORDINE pOrdine;
	DBresult *PGRes = NULL;
	int nTuples;
	char szMsgTxt[128];
	char szMsg[128];
	int nIndex;
	int nSettoreIndex;
	PSETTORE pSettore;
	struct timeval tvActualTime;


	if((nSettoreIndex=GetSettore(pEvento->nIsola,pEvento->nSettore))==-1){
#ifdef TRACE
		trace_out_vstr_date(1,"GestioneSettore(I:%d,S:%d) : settore non presente !",pEvento->nIsola,pEvento->nSettore);
#endif
		return FALSE;
	}
	pubnmset=&pSettori[nSettoreIndex];

#ifdef TRACE_ELIMINATO
	trace_out_vstr_date(1,"GestioneSettore(I:%d,S:%d) : settore %d",pEvento->nIsola,pEvento->nSettore,nSettoreIndex);
#endif
	/*
	* caso di settore in stato di attesa 
	* e nessun evento verificatosi
	*/
	if(pSettore->nStatoubnmset==ATTESA && pEvento->nEvento==-1){
		/*
		* - se si tratta del primo ubnmset, 
		* - se la lista ordini e' vuota,
		* - se l'archivio rig_prod contiene prelievi di righe sul primo settore :
		* si mettono queste righe d'ordine in lista  
		*/
		if(pSettore->nTipoubnmset==SETTORE_START){
			if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))==NULL){
				/*
				* Gestione Timeout per get ordine (evita di ricercare un ordine sul settore di start con 
				* 'troppa' insistenza).
				*/
				gettimeofday(&tvActualTime,NULL);
				/* calcolo il tempo trascorso dall'ultimo tentativo */
				nGetOrderTimePassed[pSettore->nIsola]=(((tvActualTime.tv_sec-tvOldTime[pSettore->nIsola].tv_sec)*1000) + ((tvActualTime.tv_usec-tvOldTime[pSettore->nIsola].tv_usec)/1000));   /* milli-secondi */
#ifdef TRACE_ELIMINATO
					trace_out_vstr_date(1,"Settore : %d Isola : %d : nGetOrderTimePassed = %6d",pSettore->nSettore,pSettore->nIsola,nGetOrderTimePassed[pSettore->nIsola]);
#endif
				/*
				* Verifico se e' passato il timeout per la ricerca dell'ordine da mandare sul settore di start
				* se l'ordine precedente non era producibile
				* passo SUBITO al prossimo ...
				*/
				if(bProducibile==FALSE || nGetOrderTimePassed[pSettore->nIsola] > Cfg.nGetOrderDelay){
#ifdef TRACE_ELIMINATO
					trace_out_vstr_date(1,"Settore : %d Isola : %d : Azzero nGetOrderTimePassed (%d)",pSettore->nSettore,pSettore->nIsola,nGetOrderTimePassed[pSettore->nIsola]);
#endif
					nGetOrderTimePassed[pSettore->nIsola]=0;
					gettimeofday(&(tvOldTime[pSettore->nIsola]),NULL);
				}
				if(nGetOrderTimePassed[pSettore->nIsola]==0){
					/* 
					* se non ci sono ordini forzati in attesa di analisi
					*/
					if(!Cfg.bAttesaAnalisiVolumetrica){
						/* 
						* Cerco un ordine in attesa di prelievo
						*/
						if(GetOrdine(&Ordine,pSettore->nIsola)){
							/* 
							* se ordine forzato rilancio la analisi volumetrica
							*/
							if(OrdineForzato(&Ordine)){
								SendMessage(PROC_VOLUM, PROC_SETTORI, ANALISI_ORDINE, Ordine.szOrdProg);
								/* 
								* Attesa di analisi volumetrica ordine 
								*/
								Cfg.bAttesaAnalisiVolumetrica=TRUE;
							}

							if(!Cfg.bAttesaAnalisiVolumetrica){
								/*
								* rm 29-08-2001 
								* Verifico che l'ordine sia producibile
								* o che sia 'forzato'  :  da gestire
								* o che sia 'bloccato' :  da gestire
								*/
								if(!OrdineProducibile(&Ordine)){
#ifdef TRACE
									trace_out_vstr_date(1,"Ordine [%s] NON PRODUCIBILE", Ordine.szOrdProg);
#endif
									sprintf(szMsg,"ATTENZIONE : Ordine [%s] NON PRODUCIBILE\n", Ordine.szOrdProg);
									SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
									SendMessage(PROC_MAIN, PROC_SETTORI, REFRESH_ORDINE, Ordine.szOrdProg);
									bProducibile=FALSE;
								} else {
									/*
									* Se l'ordine e' producibile allora impegno le copie 
									*/
									ProduzioneOrdine(&Ordine);
									bProducibile=TRUE;
								}

								if(bProducibile){
#ifdef TRACE
									trace_out_vstr_date(1,"Inizio Prelievo Ordine [%s] su Isola %d", Ordine.szOrdProg,pSettore->nIsola);
#endif
									if(Ordine.nRighe==0 || Ordine.nCopie==0){
#ifdef TRACE
										trace_out_vstr_date(1,"Ordine [%s] VUOTO", Ordine.szOrdProg);
#endif
										sprintf(szMsg,"ATTENZIONE : Ordine [%s] VUOTO : EVASIONE AUTOMATICA\n", Ordine.szOrdProg);
										SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);

										/*
										* Messaggio di Ordine in evaso a MAIN
										*/
										SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_EVASO, Ordine.szOrdProg);
									} else {
										/*
										* alloco un nuovo ordine da prelevare
										*/
										pOrdine=(PORDINE)malloc(sizeof(ORDINE));
										memset(pOrdine,0,sizeof(ORDINE));

										strcpy(pOrdine->szOrdProg,Ordine.szOrdProg);   /* Chiave Spedizione */
										pOrdine->nubnmisl=Ordine.nIsola;
										pOrdine->nColli=Ordine.nColli;
										pOrdine->nRighe=Ordine.nRighe;
										pOrdine->nCopie=Ordine.nCopie;

										CambiaStatoOrdine(pOrdine->szOrdProg, ORDINE_IN_PRELIEVO);

										/*
										* Messaggio di Ordine in prelievo a MAIN
										*/
										sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);
										SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_IN_PRELIEVO, szMsgTxt);


										/*
										* aggiungo l'ordine alla coda di questo settore
										*/
										link_add(&(ListaOrdini[nSettoreIndex]),pOrdine);
									}/* if(Ordine.nRighe==0 || Ordine.nCopie==0)                      - Verifica contenuto ordine */
								}  /* if(bProducibile)                                              - Verifica producibilita' ordine */
							}    /* if(!Cfg.bAttesaAnalisiVolumetrice)                            - Flag di attesa analisi volumetrica per ordini forzati*/
						}      /* if(GetOrdine(&Ordine,pSettore->nIsola))                       - Ricerca ordine in attesa di prelievo */
					}        /* if(!Cfg.bAttesaAnalisiVolumetrice)                            - Flag di attesa analisi volumetrica per ordini forzati*/
				}          /* if(nGetOrderTimePassed[pSettore->nIsola]==0)                  - Tempo di ritardo sulla ricerca ordine */
			}            /* if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))==NULL) - Verifica di coda vuota */
		}              /* if(pSettore->nTipoubnmset==SETTORE_START)                     - Verifica di settore di start */

		/*
		* caso valido per tutti i settori, compreso il primo:
		* se la coda ordini e' vuota non faccio niente
		* altrimenti passo alla elaborazione del primo ordine in coda
		*/
		if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){

			if(pSettore->nTipoubnmset==SETTORE_START && Cfg.nImballiUbicati==1){
				PIMBALLO_PRELIEVO pImballo;
				/*
				* Se si tratta di un settore di start e gestisco
				* gli imballi a ubicazione
				* passo al prelievo imballi e stampa etichetta per i colli AUTOMATICI
				* altrimenti passo alla gestione righe di prelievo
				*/
				PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select c.ordprog,c.cptpfor,count(c.cptpfor),u.ubplcnm,u.ubnmdsp,u.ubnmset from col_prod as c, ubicazioni as u where ordprog='%s' and c.cpswlin!='1' and u.codprod=c.cptpfor and u.ubnmisl=%d group by c.cptpfor,c.ordprog,c.ordprog,u.ubplcnm,u.ubnmddsp,u.ubnmset,u.ubprior order by u.ubprior;",pOrdine->szOrdProg,pSettore->nIsola);

				nTuples=DBntuples(PGRes);
				for(nIndex=0;nIndex<nTuples;nIndex++){
					pImballo=&(pSettore->Imballo[nIndex]);
					strcpy(pImballo->szOrdProg,   DBgetvalue(PGRes,nIndex,0));
					strcpy(pImballo->szFormato,   DBgetvalue(PGRes,nIndex,1));
					pImballo->nNumCollo    = atoi(DBgetvalue(PGRes,nIndex,2));
					pImballo->nPLCNum      = atoi(DBgetvalue(PGRes,nIndex,3));
					pImballo->nDisplay     = atoi(DBgetvalue(PGRes,nIndex,4));
				}
				DBclear(PGRes);

				pSettore->nIndiceImballo=0;
				pSettore->nNumeroImballi=nIndex;
				pImballo=&(pSettore->Imballo[pSettore->nIndiceImballo]);

				strcpy(pSettore->szCodSped,pImballo->szOrdProg);
				pSettore->nNumCollo = pImballo->nNumCollo;
				pSettore->nPLCNum   = pImballo->nPLCNum;
				pSettore->nDisplay  = GetDisplay(pImballo->nDisplay,pSettore->nIsola);

				/*
				* scrivo i dati per il prelievo imballi sul display
				*/
				pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
				strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
				sprintf(pDisplay->szRiga_2_Display,"I%3d",pSettore->nNumCollo);
				pDisplay->nStatoRiga1=NORMAL;
				pDisplay->nStatoRiga2=NORMAL;

				UpdateDisplay(pDisplay,TUTTO);

				/*
				* accendo la luce di prelievo prodotto
				*/
				pSettore->nStatoLampada=1;
				SetLampada(pSettore->nSettore,pSettore->nPLCNum);
				/*
				* setto lo stato settore in attesa di conferma copie prelevate
				*/
				pSettore->nStatoubnmset=PRELIEVO_IMBALLI;
			} else {
				InizioPrelievoSettore(pOrdine,pSettore);
			}
		}
	}

	switch(pEvento->nEvento){
		case IOS_PKL_BUTTON_PRESSED_SETTORE_NUMERO:
			switch(pSettore->nStatoSettore){
				case PRELIEVO_IMBALLI:
				{
					PIMBALLO_PRELIEVO pImballo;
					/*
					* sono in attesa prelievo imballo
					* e' stato premuto il tasto di prelievo effettuato (riga/colonna giuste)
					* - se si tratta dell'ultimo imballo passo al prelievo righe
					*/
					/*
					* controllo che la riga/colonna siano quelle giuste
					*/
					switch(pEvento->nEvento){
						case IOS_PKL_BUTTON_PRESSED_SETTORE_NUMERO:
						{
							if(pEvento->nubnmset==pSettore->nSettore && pEvento->nPLCNum==pSettore->nPLCNum){

								/* reset lampada */
								pSettore->nStatoLampada=0;

								if(pSettore->nIndiceImballo+1<pSettore->nNumeroImballi){
									/*
									* STAMPA ETICHETTA IMBALLO ATTUALE e 
									* ----------------------------------
									* Prelievo imballo successivo
									*/
									pSettore->nIndiceImballo++;
									pImballo=&(pSettore->Imballo[pSettore->nIndiceImballo]);

									strcpy(pSettore->szCodSped,pImballo->szOrdProg);
									pSettore->nNumCollo = pImballo->nNumCollo;
									pSettore->nPLCNUm   = pImballo->nPLCNUm;
									pSettore->nDisplay  = GetDisplay(pImballo->nDisplay,pSettore->nIsola);

									/*
									* scrivo i dati per il prelievo imballo sul display
									*/
									pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
									strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
									sprintf(pDisplay->szRiga_2_Display,"I%3d",pSettore->nNumCollo);
									pDisplay->nStatoRiga1=NORMAL;
									pDisplay->nStatoRiga2=NORMAL;

									UpdateDisplay(pDisplay,TUTTO);

									/*
									* accendo la luce di prelievo prodotto
									*/
									pSettore->nStatoLampada=1;
									SetLampada(pSettore->nSettore,pSettore->nPLCNum);
									/*
									* setto lo stato settore in attesa di conferma copie prelevate
									*/
									pSettore->nStatoubnmset=PRELIEVO_IMBALLI;
								} else {
									/*
									* scrivo i dati per il prelievo imballo sul display
									*/
									pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
									strcpy(pDisplay->szRiga_1_Display,"    ");
									strcpy(pDisplay->szRiga_2_Display,"    ");
									pDisplay->nStatoRiga1=NORMAL;
									pDisplay->nStatoRiga2=NORMAL;

									UpdateDisplay(pDisplay,TUTTO);
									/*
									* Fine Prelievo imballi
									*/
									if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
										InizioPrelievoSettore(pOrdine,pSettore);
									}
								}
							}
						}
						break;
					}
				}
				break;
				case PRELIEVO_COPIE:
				{
					/*
					* sono in attesa prelievo
					* e' stato premuto il tasto di prelievo effettuato (riga/colonna giuste)
					* - se si tratta dell'ultima riga del collo chiudo il collo
					* - se ci sono ancora righe in questo settore vado avanti nel prelievo
					* - se ci sono altre righe nei settori successivi passo il collo al settore successivo 
					*/
					/*
					* controllo che la riga/colonna siano quelle giuste
					*/
					switch(pEvento->nEvento){
						case IOS_PKL_BUTTON_PRESSED_SETTORE_NUMERO:
						{
							PRIGA_PRELIEVO pOldRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
							PRIGA_PRELIEVO pRigaOrdine=pOldRigaOrdine;

							if(pEvento->nubnmset==pSettore->nSettore && pEvento->nPLCNum==pSettore->nPLCNum){

								/* reset lampada */
								pSettore->nStatoLampada=0;
								/*
								* prelievo confermato
								* setto lo stato della riga (PRELEVATA)
								* passo alla prossima riga da prelevare (se c'e')
								* se il collo e' diverso allora chiudo il collo
								*/
								pRigaOrdine->nStato=PRELEVATA;

								/*
								* Aggiorna i dati della riga ordine in oggetto con il codice operatore, data e ora prelievo e stato
								*/
								UpdateRigaOrdine(pRigaOrdine,pSettore);

								if(pSettore->nIndiceRigaOrdine==pSettore->nNumeroRigheOrdine-1){
									if(!Cfg.nChiusuraCollo && pRigaOrdine->nNextCollo==-1){
										/*
										* scrivo i dati per il fine ordine sul display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										strcpy(pDisplay->szRiga_2_Display,"FFFF");
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=BLINK;
										UpdateDisplay(pDisplay,TUTTO);

										/*
										* faccio confermare l'ultima lampada premuta 
										*/
										pSettore->nStatoLampada=1;

										SetLampada(pSettore->nSettore,pSettore->nPLCNum);

										pSettore->nStatoubnmset=FINE_ORDINE;
									} else if(!Cfg.nChiusuraCollo || pOldRigaOrdine->nNumCollo==pOldRigaOrdine->nNextCollo){
										/*
										* finite le righe d'ordine
										* se il prossimo collo e' uguale a quello attuale
										* allora faccio il passaggio a settore successivo
										* altrimenti faccio la chiusura collo
										*/

										/*
										* scrivo i dati per il passaggio sul display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										strcpy(pDisplay->szRiga_2_Display,"PPPP");
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=BLINK;
										UpdateDisplay(pDisplay,TUTTO);

										/*
										* faccio confermare l'ultima lampada premuta 
										*/
										pSettore->nStatoLampada=1;

										SetLampada(pSettore->nSettore,pSettore->nPLCNum);

										pSettore->nStatoubnmset=PASSA_SETT_SUCC;
									} else {
										/*
										* chiudo il collo se in gestione collo e non ordine (rif.Wella)
										*/
										/*
										* scrivo i dati per il passaggio sul display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										sprintf(pDisplay->szRiga_2_Display,"C%3d", pOldRigaOrdine->nNumCollo);
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=BLINK;
										UpdateDisplay(pDisplay,TUTTO);

										/*
										* faccio confermare l'ultima lampada premuta 
										*/
										pSettore->nStatoLampada=1;

										SetLampada(pSettore->nSettore,pSettore->nPLCNum);

										pSettore->nStatoubnmset=CHIUSURA_COLLO;
									}
								} else {
									/*
									* ci sono ancora righe d'ordine da fare
									* controllo prima che non ci sia da chiudere il collo
									*/
									if(!Cfg.nChiusuraCollo || pOldRigaOrdine->nNumCollo==pOldRigaOrdine->nNextCollo){
										pSettore->nIndiceRigaOrdine++;
										pRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
										/*
										* il collo e' lo stesso : faccio il prelievo
										*/
										/* 
										* setto lo stato della riga (ATTESA_PRELIEVO)
										*/
										pRigaOrdine->nStato=ATTESA_PRELIEVO;

										/*
										* setto i dati della riga corrente del settore
										*/
										strcpy(pSettore->szCodSped,pRigaOrdine->szCodSped);
										strcpy(pSettore->szCodProd,pRigaOrdine->szCodProd);
										strcpy(pSettore->szCodUbi,pRigaOrdine->szCodUbi);
										pSettore->nNumCollo=pRigaOrdine->nNumCollo;
										pSettore->nNumCopie=pRigaOrdine->nNumCopie;
										pSettore->nModulo=pRigaOrdine->nModulo;
										pSettore->nRiga=pRigaOrdine->nRiga;
										pSettore->nColonna=pRigaOrdine->nColonna;
										if(pSettore->nDisplay!=pRigaOrdine->nDisplay){
											/*
											* Pulisco il display
											*/
											pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
											strcpy(pDisplay->szRiga_1_Display,"    ");
											strcpy(pDisplay->szRiga_2_Display,"    ");
											pDisplay->nStatoRiga1=NORMAL;
											pDisplay->nStatoRiga2=NORMAL;
											UpdateDisplay(pDisplay,TUTTO);
										}
										pSettore->nDisplay = GetDisplay(pRigaOrdine->nDisplay,pSettore->nIsola);
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
										/*
										* scrivo i dati per il prelievo sul display
										*/
										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										sprintf(pDisplay->szRiga_2_Display,"P%3d",pSettore->nNumCopie);
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=NORMAL;
										UpdateDisplay(pDisplay,TUTTO);
										/*
										* accendo la luce di prelievo prodotto
										*/
										pSettore->nStatoLampada=1;
										SetLampada(pSettore->nSettore,pSettore->nPLCNum);
										/*
										* setto lo stato settore in attesa di conferma copie prelevate
										*/
										pSettore->nStatoubnmset=PRELIEVO_COPIE;
									} else {
										/*
										* il collo e' diverso : faccio la chiusura collo
										*/
										/*
										* scrivo i dati per la chiusura sul display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										sprintf(pDisplay->szRiga_2_Display,"C%3d", pOldRigaOrdine->nNumCollo);
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=BLINK;
										UpdateDisplay(pDisplay,TUTTO);

										/*
										* faccio confermare l'ultima lampada premuta 
										*/
										pSettore->nStatoLampada=1;

										SetLampada(pSettore->nSettore,pSettore->nPLCNum);

										pSettore->nStatoubnmset=CHIUSURA_COLLO;
									}
								}
							}
						}
						break;
					}
				}
				break;
				case PASSA_SETT_SUCC:
				{
					/*
					* dopo conferma operatore
					* infilo l'ordine in coda al primo settore abilitato successivo al presente
					* e mi rimetto in attesa prossimo ordine in coda
					*/
					/*
					* sono in attesa di conferma passaggio a settore successivo
					* e' stato premuto il tasto di conferma passaggio (riga/colonna giuste)
					* metto l'ordine in coda al settore successivo e
					* resetto il display e passo allo stato di attesa
					*/
					/*
					* controllo che la riga/colonna siano quelle giuste
					*/
					switch(pEvento->nEvento){
						case IOS_PKL_BUTTON_PRESSED:
							if(pEvento->nubnmset==pSettore->nSettore && pEvento->nPLCNum==pSettore->nPLCNum){

								/* reset lampada */
								pSettore->nStatoLampada=0;

								pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

								strcpy(pDisplay->szRiga_1_Display,"    ");
								strcpy(pDisplay->szRiga_2_Display,"    ");
								pDisplay->nStatoRiga1=NORMAL;
								pDisplay->nStatoRiga2=NORMAL;
								UpdateDisplay(pDisplay,TUTTO);

								/*
								* passaggio ordine confermato
								*/
								pSettore->szCodSped[0]='\0';
								pSettore->szCodProd[0]='\0';
								pSettore->szCodUbi[0]='\0';
								pSettore->nNumCollo=0;
								pSettore->nNumCopie=0;
								pSettore->nCPUNum=pSettore->nFirstCPUNum;
								pSettore->nIndiceRigaOrdine=0;
								pSettore->nNumeroRigheOrdine=0;

								if(pSettore->nTipoubnmset==SETTORE_END){
									/*
									* rimuovo il primo ordine dalla coda
									*/
									if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){

										link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
										free(pOrdine);
									}
								} else {
									/*
									* metto il primo ordine in coda al settore successivo
									* e lo rimuovo dalla coda del settore attuale
									*/
									if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
										sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);

										/*
										* Messaggio di Ordine Settore Successivo a MAIN
										*/
										SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_SETT_SUCC, szMsgTxt);
										/*
										* rimuovo l'ordine dal settore attuale
										*/
										link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
										/*
										* aggiungo l'ordine al primo settore abilitato successivo al presente
										*/
										/*
										* DA SISTEMARE ... GESTIRE I SETTORI PER ISOLA !!!!
										*/
										while(pSettori[nSettoreIndex].nubnmisl==pSettore->nIsola && pSettori[nSettoreIndex].nStatoubnmset==DISABILITATO){
											nSettoreIndex++;
										}
#ifdef TRACE
										trace_out_vstr_date(1,"Passo Ordine [%s] a Settore [%2d]", pOrdine->szOrdProg,nSettoreIndex+1);
#endif

										link_add(&(ListaOrdini[nSettoreIndex+1]),pOrdine);
									}
								}

								pSettore->nStatoubnmset=ATTESA;
							}
						break;
					}
				}
				break;
				case CHIUSURA_COLLO:
				{
					/*
					* sono in attesa di chiusura collo
					* e' stato premuto il tasto di chiusura effettuata (riga/colonna giuste)
					* - se ci sono altre righe in questo settore: 
					*   - prelievo copie
					* - se si tratta dell'ultima riga del collo:
					*   - se si tratta dell'ultimo collo faccio fine ordine
					*   - se c'e' un altro collo passo al settore successivo
					*/
					/*
					* controllo che la riga/colonna siano quelle giuste
					*/
					switch(pEvento->nEvento){
						case IOS_PKL_BUTTON_PRESSED_SETTORE_NUMERO:
						{
							PRIGA_PRELIEVO pOldRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
							PRIGA_PRELIEVO pRigaOrdine;

							if(pEvento->nubnmset==pSettore->nSettore && pEvento->nPLCNum==pSettore->nPLCNum){

								/* reset lampada */
								pSettore->nStatoLampada=0;
								/*
								* chiusura collo confermata
								*/
								if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
									CambiaStatoCollo(pOrdine->szOrdProg,pSettore->nNumCollo,COLLO_CHIUSO);

									/*
									* Messaggio di Collo Chiuso a MAIN
									*/
									sprintf(szMsgTxt,"%d,%d,%s,%d", pSettore->nIsola, pSettore->nSettore, pOrdine->szOrdProg,pSettore->nNumCollo);
									SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_COLLO_PRELEVATO, szMsgTxt);

								}
								/*
								* passo alla prossima riga da prelevare (se c'e')
								*/
								if(pSettore->nIndiceRigaOrdine==pSettore->nNumeroRigheOrdine-1){
									/*
									* non ci sono piu' righe da prelevare in questo settore;
									* visto che sono in conferma chiusura collo questo significa che :
									*  - se il prossimo collo ESISTE verra' iniziato da uno dei prossimi settori
									*    e quindi faccio il passaggio a settore successivo
									*  - se il prossimo collo NON ESISTE passo a Fine Ordine
									*
									*/
									if(pOldRigaOrdine->nNextCollo==-1){
										/* Fine Ordine */
										if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
											/*
											* Messaggio di Ordine Terminato a MAIN
											*/
											CambiaStatoOrdine(pOrdine->szOrdProg, ORDINE_PRELEVATO);
											/*
											* Messaggio di Ordine terminato a MAIN
											*/
											sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);
											SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_PRELEVATO, szMsgTxt);
										}
										/*
										* scrivo i dati per il fine ordine sul display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										strcpy(pDisplay->szRiga_2_Display,"FFFF");
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=BLINK;
										UpdateDisplay(pDisplay,TUTTO);

										/*
										* faccio confermare l'ultima lampada premuta 
										*/
										pSettore->nStatoLampada=1;

										SetLampada(pSettore->nSettore,pSettore->nPLCNum);

										pSettore->nStatoubnmset=FINE_ORDINE;

									} else {
										/* Passa a Settore Successivo */
										/*
										* scrivo i dati per il passaggio sul display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
										strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
										strcpy(pDisplay->szRiga_2_Display,"PPPP");
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=BLINK;
										UpdateDisplay(pDisplay,TUTTO);


										/*
										* faccio confermare l'ultima lampada premuta 
										*/
										pSettore->nStatoLampada=1;

										SetLampada(pSettore->nSettore,pSettore->nPLCNum);

										pSettore->nStatoubnmset=PASSA_SETT_SUCC;
									}
								} else {
									/*
									* ci sono altre righe da prelevare in questo settore
									*/
									pSettore->nIndiceRigaOrdine++;
									pRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
									/*
									* il collo e' lo stesso : faccio il prelievo
									*/
									/* 
									* setto lo stato della riga (ATTESA_PRELIEVO)
									*/
									pRigaOrdine->nStato=ATTESA_PRELIEVO;

									/*
									* setto i dati di lavoro del settore
									*/
									strcpy(pSettore->szCodSped,pRigaOrdine->szCodSped);
									strcpy(pSettore->szCodProd,pRigaOrdine->szCodProd);
									strcpy(pSettore->szCodUbi,pRigaOrdine->szCodUbi);
									pSettore->nNumCollo=pRigaOrdine->nNumCollo;
									pSettore->nNumCopie=pRigaOrdine->nNumCopie;
									pSettore->nPLCNum=pRigaOrdine->nPLCNum;
									if(pSettore->nDisplay!=pRigaOrdine->nDisplay){
										/*
										* Pulisco il display
										*/
										pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
										strcpy(pDisplay->szRiga_1_Display,"    ");
										strcpy(pDisplay->szRiga_2_Display,"    ");
										pDisplay->nStatoRiga1=NORMAL;
										pDisplay->nStatoRiga2=NORMAL;
										UpdateDisplay(pDisplay,TUTTO);
									}
									pSettore->nDisplay = GetDisplay(pRigaOrdine->nDisplay,pSettore->nIsola);
									/*
									* scrivo i dati per il prelievo sul display
									*/
									pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
									strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
									sprintf(pDisplay->szRiga_2_Display,"P%3d",pSettore->nNumCopie);
									pDisplay->nStatoRiga1=NORMAL;
									pDisplay->nStatoRiga2=NORMAL;
									UpdateDisplay(pDisplay,TUTTO);
									/*
									* accendo la luce di prelievo prodotto
									*/
									pSettore->nStatoLampada=1;
									SetLampada(pSettore->nSettore,pSettore->nPLCNum);
									/*
									* setto lo stato settore in attesa di conferma copie prelevate
									*/
									pSettore->nStatoubnmset=PRELIEVO_COPIE;
								}
							}
						}
						break;
					}
				}
				break;
				case FINE_ORDINE:
				{
					/*
					* sono in attesa di conferma fine ordine
					* e' stato premuto il tasto di conferma fine ordine (riga/colonna giuste)
					* resetto il display e passo allo stato di attesa
					*/
					/*
					* controllo che la riga/colonna siano quelle giuste
					*/
					switch(pEvento->nEvento){
						case IOS_PKL_BUTTON_PRESSED_SETTORE_NUMERO:
							pSettore->nPLCNum=pRigaOrdine->nPLCNum;

							if(pSettore->nSettore=pRigaOrdine->nSettore && pSettore->nPLCNum=pRigaOrdine->nPLCNum) {

								pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
								strcpy(pDisplay->szRiga_1_Display,"    ");
								strcpy(pDisplay->szRiga_2_Display,"    ");
								pDisplay->nStatoRiga1=NORMAL;
								pDisplay->nStatoRiga2=NORMAL;
								UpdateDisplay(pDisplay,TUTTO);



								/* reset lampada */
								pSettore->nStatoLampada=0;

								/* metto il settore in attesa di ordini da gestire */
								pSettore->nStatoubnmset=ATTESA;

								/*
								* fine ordine confermata
								*/
								pSettore->szCodSped[0]='\0';
								pSettore->szCodProd[0]='\0';
								pSettore->szCodUbi[0]='\0';
								pSettore->nNumCollo=0;
								pSettore->nNumCopie=0;
								pSettore->nPLCNum=pSettore->nFirstPLCNum;
								pSettore->nDisplay=pSettore->nFirstDisplay;
								pSettore->nIndiceRigaOrdine=0;
								pSettore->nNumeroRigheOrdine=0;

								/*
								* Gestione di fine settore particolare per Wella (NO RULLIERA DI SCARICO)
								*/
								if(Cfg.nFineOrdine || pSettore->nTipoubnmset==SETTORE_END){

									/*
									* rimuovo il primo ordine dalla coda
									*/
									if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){

										link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
										free(pOrdine);
									}
								} else {
									/*
									* Faccio proseguire l'ordine fino all'ultimo settore dell'isola
									* (SETTORE DI END)
									*/
									/*
									* metto il primo ordine in coda al settore successivo
									* e lo rimuovo dalla coda del settore attuale
									*/
									if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
										sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);

										/*
										* Messaggio di Ordine Settore Successivo a MAIN
										*/
										SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_SETT_SUCC, szMsgTxt);
										/*
										* rimuovo l'ordine dal settore attuale
										*/
										link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
										/*
										* aggiungo l'ordine al primo settore abilitato successivo al presente
										*/
										/*
										* DA SISTEMARE ... GESTIRE I SETTORI PER ISOLA !!!!
										*/
										while(pSettori[nSettoreIndex].nubnmisl==pSettore->nIsola && pSettori[nSettoreIndex].nStatoubnmset==DISABILITATO){
											nSettoreIndex++;
										}
#ifdef TRACE
										trace_out_vstr_date(1,"Passo Ordine [%s] a Settore [%2d]", pOrdine->szOrdProg,nSettoreIndex);
#endif

										link_add(&(ListaOrdini[nSettoreIndex+1]),pOrdine);
									}
								}
							}
						break;
					}
				}
				break;
			}
		break;
	}

	return TRUE;
}

/*
* lettura delle ubicazioni in archivio
*/
int ReadUbicazioni(PUBICAZIONI pUbi)
{
	int nIndex;
	DBresult *PGRes = NULL;
	int nTuples;

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select ubcdubi, ubcdubi, ubnmisl, ubnmset, ubnmdsp, ubplcnm, ubprior, cnistato from ubicazioni where ubtpubi='%s';",Cfg.szTipoOrdini);

	nTuples=DBntuples(PGRes);

	for(nIndex=0;nIndex<nTuples;nIndex++){

		strcpy(pUbi->Ubicazione[nIndex].szCodUbi,         DBgetvalue(PGRes,nIndex,0));
		strcpy(pUbi->Ubicazione[nIndex].szCodProd,        DBgetvalue(PGRes,nIndex,1));
		pUbi->Ubicazione[nIndex].nIsola            = atoi(DBgetvalue(PGRes,nIndex,2));
		pUbi->Ubicazione[nIndex].nSettore          = atoi(DBgetvalue(PGRes,nIndex,3));
		pUbi->Ubicazione[nIndex].nDisplay          = atoi(DBgetvalue(PGRes,nIndex,4));
		pUbi->Ubicazione[nIndex].nPLCNum           = atoi(DBgetvalue(PGRes,nIndex,5));
		pUbi->Ubicazione[nIndex].nPriorita         = atoi(DBgetvalue(PGRes,nIndex,6));

		pUbi->Ubicazione[nIndex].nIndex=nIndex;
		
	}
	DBclear(PGRes);
	pUbi->nUbicazioni=nIndex;

	return nIndex;
}

void SortUbicazioni(PUBICAZIONI pUbi)
{
	qsort(pUbi->ubcdubi,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

int CmpUbicazione(PUBICAZIONE pUbi1,PUBICAZIONE pUbi2)
{
	if(pUbi1->nSettore < pUbi2->nSettore){
		return -1;
	}
	if(pUbi1->nSettore > pUbi2->nSettore){
		return 1;
	}
	if(pUbi1->nPLCNum < pUbi2->nPLCNum){
		return -1;
	}
	if(pUbi1->nPLCNum > pUbi2->nPLCNum){
		return 1;
	}
	return 0;
}


PUBICAZIONE SearchUbicazione(PUBICAZIONI pUbi,int nSettore,int nPLCNum)
{
	UBICAZIONE Ubi;

	Ubi.nSettore=nSettore;
	Ubi.nPLCNum=nPLCNum;

	return bsearch(&Ubi,pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

/*
* Visualizza dati su display A&L PKDIS
*/
void UpdateDisplay(PDISPLAYSTRUCT pDisplay,int nRiga)
{
}

void DisplayWriteString(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString)
{
}

int GetBCDNibble(unsigned char cChar)
{
	return 0;
}

/*
* DisplayWriteStringBCD(nIOS, nCPU, nModulo, nRowIndex, nMode, szString,nDot,nBlink)
* Comando di settaggio display PKDIS usando comandi BCD (11,12,13,14-18,19)
*/
void DisplayWriteStringBCD(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString,unsigned char nDot,unsigned char nBlink)
{
}

void ScongelaPKL(void)
{
	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SCONGELA, NULL);
}

void CongelaPKL(void)
{
	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_CONGELA, NULL);
}

void SetLampada(int nSettore,int PLCNum)
{
	char szBuffer[40];

	sprintf(szBuffer,"%d,%d",nSettore, PLCNum);

	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SET_RC_SETTORE_NUMERO, szBuffer);
}

void ResetLampada(int nSettore,int PLCNum)
{
	char szBuffer[40];

	sprintf(szBuffer,"%d,%d",nSettore, PLCNum);

	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_RC_SETTORE_NUMERO, szBuffer);
}

void ResetLuci(void)
{
	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_ALL, NULL);
}

void ResetSettori(void)
{
	int nIndex;

	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
		ResetSettore(pSettori[nIndex].nIsola,pSettori[nIndex].nSettore);
		/*
		* rm 11-07-2001 : ritardo per evitare problemi con la IOS
		*/
		usleep(1000*Cfg.nDelay);
	}
}

void ResetSettore(int nIsola,int nSettore)
{
	PORDINE pOrdine;
	int nFirstDisplay=0;
	int nFirstPLCNum=0;
	DBresult *PGRes = NULL;
	int nTuples;
	int nIndex;

#ifdef TRACE
	trace_out_vstr_date(1,"ResetSettore(Isola:%d,Settore:%d)",nIsola,nSettore);
#endif

	/*
	* Cerco l'indice del settore relativo ai parametri isola e settore passati alla funzione
	*/
	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
		if(pSettori[nIndex].nIsola==nIsola && pSettori[nIndex].nSettore==nSettore){
			pSettori[nIndex].nIndiceRigaOrdine=0;        /* puntatore alla riga d'ordine da prelevare */
			pSettori[nIndex].nNumeroRigheOrdine=0;       /* numero delle righe d'ordine da prelevare */
			/* 
			* Lista FIFO Ordini 
			*/
			while((pOrdine=link_pop(&(ListaOrdini[nIndex])))){
				free(pOrdine);
			}
			if(pSettori[nIndex].nStatoSettore!=DISABILITATO){
				pSettori[nIndex].nStatoubnmset=ATTESA;       /* Stato attuale del settore */
			}
			/*
			* Determino i dati della prima ubicazione del settore in oggetto
			*/
			PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select ubnmdsp, ubplcnm from ubicazioni where ubtpubi='%s' AND ubnmisl='%d' AND ubnmset='%4d' order by ubprior;",Cfg.szTipoOrdini, nIsola,nSettore);

			nTuples=DBntuples(PGRes);

			if(nTuples>=1){
				nFirstDisplay = GetDisplay(atoi(DBgetvalue(PGRes,0,0)),nIsola);
				nFirstPLCNum  = atoi(DBgetvalue(PGRes,0,1));
			} else {
#ifdef TRACE
				trace_out_vstr_date(1,"ResetSettore(Isola:%d,Settore:%d) : Non esistono ubicazioni per il settore in oggetto",nIsola,nSettore);
#endif
			}
			pSettori[nIndex].nFirstIOS=nFirstPLCNum;
			pSettori[nIndex].nFirstDisplay=nFirstDisplay;

			pSettori[nIndex].nPLCNum=nFirstPLCNum;       /* Nro PLC Tasto Premuto / da premere */
			pSettori[nIndex].nDisplay=nFirstDisplay;     /* Display */
			pSettori[nIndex].nNumCopie=0;                /* Numero Copie */
			pSettori[nIndex].nNumCollo=0;                /* Numero Collo */
			pSettori[nIndex].nStatoLampada=0;            /* Stato Lampada */
			pSettori[nIndex].szCodSped[0]='\0';          /* Codice Ordine Attivo */
			pSettori[nIndex].szCodProd[0]='\0';          /* Codice Prodotto da prelevare/prelevato*/
			pSettori[nIndex].szCodUbi[0]='\0';           /* Codice Ubicazione del prodotto da prelevare/prelevato*/

			DBclear(PGRes);
		}

	}

	/*
	* Cerco l'indice del o dei displays relativo/i ai parametri isola e settore passati alla funzione
	*/
	for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
		if(Cfg.Displays[nIndex].nubnmisl==nIsola && Cfg.Displays[nIndex].nubnmset==nSettore){
			strcpy(Cfg.Displays[nIndex].szRiga_1_Display,"----");     /* Riga 1 del display */
			strcpy(Cfg.Displays[nIndex].szRiga_2_Display,"----");     /* Riga 2 del display */
			Cfg.Displays[nIndex].nStatoRiga1=NORMAL;
			Cfg.Displays[nIndex].nStatoRiga2=NORMAL;

			UpdateDisplay(&(Cfg.Displays[nIndex]),TUTTO);
		}
	}
}

/*
* cambia il flag dell'ordine in PRODUZIONE e RICEZIONE
*/
BOOL CambiaFlagOrdine(char *szOrdProg, char cFlag)
{
	BOOL bRetValue=TRUE;
	DBresult *PGRes = NULL;

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"update ric_ord set rocdflg='%c' where ordprog='%s';",cFlag,szOrdProg);
	if(atoi(DBcmdTuples(PGRes))==0){
		bRetValue=FALSE;
	}
	DBclear(PGRes);

	return bRetValue;
}



/*
* cambia lo stato dell'ordine in PRODUZIONE e RICEZIONE
*/
BOOL CambiaStatoOrdine(char *szOrdProg, char cStato)
{
	BOOL bRetVal=TRUE;
	DBresult *PGRes = NULL;

#ifdef TRACE
	trace_out_vstr_date(1,"CambiaStatoOrdine [%s][%c]",szOrdProg,cStato);
#endif

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"update ric_ord set rostato='%c' where ordprog='%s';",cStato,szOrdProg);
	if (!PGRes || DBresultStatus(PGRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
#ifdef TRACE
		trace_out_vstr_date(1,"Fallito update su ric_ord [%s]",szOrdProg);
#endif
	}
	DBclear(PGRes);

	return(bRetVal);
}

BOOL CambiaStatoColliOrdine(char *szOrdProg, char cStato)
{
	int nColli=0;
	BOOL bRetVal=TRUE;
	DBresult *PGRes = NULL;

#ifdef TRACE
	trace_out_vstr_date(1,"CambiaStatoColliOrdine [%s][%c] : ",szOrdProg,cStato);
#endif
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"update col_prod set cpstato='%c' where ordprog='%s';",cStato,szOrdProg);
	if (!PGRes || DBresultStatus(PGRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
#ifdef TRACE
		trace_out_vstr_date(1,"Fallito update su col_prod [%s]",szOrdProg);
#endif
	}
	nColli=atoi(DBcmdTuples(PGRes));

	DBclear(PGRes);

#ifdef TRACE
	trace_out_vstr(1,"Cambiati %d colli",nColli);
#endif

	return(nColli!=0);
}

/*
* rm 09-10-2000
* -------------
* CambiaStatoRighe Ordine NON viene utilizzata...
* serve una funzione di cambio stato RIGA ordine
* per la gestione statistiche di prelievo operatore
*/
BOOL CambiaStatoRigheOrdine(char *szOrdProg,int nCollo,char cStato)
{
	int nRighe=0;
	BOOL bRetVal=TRUE;
	DBresult *PGRes = NULL;

#ifdef TRACE
	trace_out_vstr_date(1,"CambiaStatoRigheOrdine [%s][%c] : ",szOrdProg,cStato);
#endif
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"update rig_prod set rpstato='%c' where ordprog='%s';",cStato,szOrdProg);
	if (!PGRes || DBresultStatus(PGRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
#ifdef TRACE
		trace_out_vstr_date(1,"Fallito update su rig_prod [%s]",szOrdProg);
#endif
	}
	nRighe=atoi(DBcmdTuples(PGRes));

	DBclear(PGRes);

#ifdef TRACE
	trace_out_vstr(1,"Cambiate %d righe",nRighe);
#endif

	return(nRighe!=0);
}


/*
* UpdateRigaOrdine()
* ATTENZIONE : ragiona per codice ordine e codice ubicazione
* NON UTILIZZARE IL NUMERO COLLO A CAUSA DELLA GESTIONE PRELIEVO PER ORDINE : VEDI InizioPrelievoSettore()
*/
BOOL UpdateRigaOrdine(PRIGA_PRELIEVO pRigaOrdine,PSETTORE pSettore)
{
	BOOL bRetVal=TRUE;
	DBresult *PGRes = NULL;
	char cStato;
	char szSQLCmd[128];

	switch(pRigaOrdine->nStato){
		default:
		case ATTESA_PRELIEVO:
			cStato=RIGA_ELABORATA;
			/*
			* modifico solo lo stato della riga
			*/
			sprintf(szSQLCmd,"update rig_prod set rpstato='%c' where ordprog='%s' AND rpcdubi='%s';",
				cStato,
				pRigaOrdine->szCodSped,
				pRigaOrdine->szCodUbi);
		break;
		case PRELEVATA:
			cStato=RIGA_PRELEVATA;
			/*
			* In caso di prelievo setto codice operatore e data/ora prelievo
			*/
			sprintf(szSQLCmd,"update rig_prod set rpstato='%c',rpcdopr='%s',rptmpre='now' where ordprog='%s' AND rpnmcol=%d and rpcdubi='%s';",
				cStato,
				pSettore->szCodOperatore,
				pRigaOrdine->szCodSped,
				pRigaOrdine->nNumCollo,
				pRigaOrdine->szCodUbi);

		break;
	}
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,szSQLCmd);

	if (!PGRes || DBresultStatus(PGRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
#ifdef TRACE
		trace_out_vstr_date(1,"Fallito update su rig_prod [%s,%d]",pRigaOrdine->szCodSped,pRigaOrdine->nProgressivo);
#endif
	}
	DBclear(PGRes);


	return(bRetVal);
}

BOOL CambiaStatoCollo(char *szOrdProg,int nCollo,char cStato)
{
	BOOL bRetVal=TRUE;
	DBresult *PGRes = NULL;

#ifdef TRACE
	trace_out_vstr_date(1,"CambiaStatoCollo [%s][%d][%c] : ",szOrdProg,nCollo,cStato);
#endif
	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"update col_prod set cpstato='%c' where ordprog='%s' AND cpnmcol = %d;",cStato,szOrdProg,nCollo);
	if (!PGRes || DBresultStatus(PGRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
#ifdef TRACE
		trace_out_vstr_date(1,"Fallito update su col_prod [%s]",szOrdProg);
#endif
	}
	DBclear(PGRes);

	return(bRetVal);
}

int GetTipoOrdine(char *szOrdProg)
{
	int nIndex;
	int nTuples;
	DBresult *PGRes = NULL;

	PGRes=DBExecQuery(Cfg.nDebugVersion>1,"select cpswlin from col_prod where ordprog='%s';",szOrdProg);

	nIndex=0;
	while(nIndex<nTuples){
		/*
		* controllo se esiste almeno un collo automatico 
		*/
		if(atoi(DBgetvalue(PGRes,nIndex,0))==0){
			DBclear(PGRes);
			return (ORDINE_AUTOMATICO);
		}
	}
	DBclear(PGRes);
	return (ORDINE_MANUALE);
}

int GetSettore(int nIsola,int nSettore)
{
	int nIndex;

	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
		if(pSettori[nIndex].nubnmisl==nIsola && pSettori[nIndex].nubnmset==nSettore){
			return nIndex;
		}
	}
	return -1;
}

/*
* SaveState(char *pszFileName)
* Salvataggio stato settori
*/
BOOL SaveState(char *pszFileName)
{
	FILE *fp;
	int nIndex;
	PORDINE pOrdine;
	BOOL bRetValue=TRUE;

#ifdef TRACE
	trace_out_vstr_date(1, "Salvataggio stato settori : [%s]", pszFileName);
#endif

	if((fp=fopen(pszFileName,"w"))!=(FILE *)NULL){
		/*
		* salvo la strutture dei settori
		*/
		for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
			/* 
			* struttura di definizione del settore 
			*/
			if(!fwrite((char *)&(pSettori[nIndex]),sizeof(SETTORE),1,fp)){
#ifdef TRACE
				trace_out_vstr_date(1, "Errore in salvataggio stato settori : fwrite()" );
#endif
				bRetValue=FALSE;
			}
			/* 
			* numero ordini relativi al settore 
			*/
			if(!fwrite((char *)&(ListaOrdini[nIndex].n_link),sizeof(int),1,fp)){
#ifdef TRACE
				trace_out_vstr_date(1, "Errore in salvataggio stato settori : fwrite()");
#endif
				bRetValue=FALSE;
			}
			for(pOrdine=NULL; (pOrdine=link_next(&(ListaOrdini[nIndex]),pOrdine));){
				if(!fwrite((char *)pOrdine,sizeof(ORDINE),1,fp)){
#ifdef TRACE
					trace_out_vstr_date(1, "Errore in salvataggio stato settori : fwrite()");
#endif
					bRetValue=FALSE;
				}
			}
		}
		/*
		* salvo la strutture dei displays
		*/
		for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
			/* 
			* struttura di definizione del display 
			*/
			if(!fwrite((char *)&(Cfg.Displays[nIndex]),sizeof(DISPLAYSTRUCT),1,fp)){
#ifdef TRACE
				trace_out_vstr_date(1, "Errore in salvataggio stato displays : fwrite()" );
#endif
				bRetValue=FALSE;
			}
		}
		/*
		* Chiusura del file
		*/
		fclose(fp);
	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "Errore in salvataggio stato settori : fopen(%s)", pszFileName);
#endif
		bRetValue=FALSE;
	}
	return bRetValue;
}


/*
* RestoreState(char *pszFileName)
* Ripristino stato settori
*/
BOOL RestoreState(char *pszFileName)
{
	BOOL bRetValue=TRUE;
	FILE *fp;
	int nIndex;
	int nOrdine;
	int nItems;
	PORDINE pOrdine;


#ifdef TRACE
		trace_out_vstr_date(1, "Ripristino stato settori : [%s]", pszFileName);
#endif

	if((fp=fopen(pszFileName,"r"))!=(FILE *)NULL){
		/*
		* leggo le strutture dei settori
		*/
		for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
			/* 
			* struttura di definizione del settore 
			*/
			if(!fread((char *)&pSettori[nIndex],sizeof(SETTORE),1,fp)){
#ifdef TRACE
				trace_out_vstr_date(1, "Errore in caricamento stato settori : fread()");
#endif
				bRetValue=FALSE;
			} else {
				/* 
				* Svuoto la lista FIFO Ordini attuale
				*/
				while((pOrdine=link_pop(&(ListaOrdini[nIndex])))){
					free(pOrdine);
				}
				/* 
				* numero ordini collegati al settore 
				*/
				if(!fread((char *)&nItems,sizeof(int),1,fp)){
#ifdef TRACE
					trace_out_vstr_date(1, "Errore in caricamento numero ordini : fread()");
#endif
					bRetValue=FALSE;
				} else {
					for(nOrdine=0;nOrdine<nItems;nOrdine++){
						pOrdine=(PORDINE)malloc(sizeof(ORDINE));
						memset(pOrdine,0,sizeof(ORDINE));
						if(!fread((char *)pOrdine,sizeof(ORDINE),1,fp)){
#ifdef TRACE
							trace_out_vstr_date(1, "Errore in caricamento stato ordini : fread()");
#endif
							bRetValue=FALSE;
						} else {
							link_add(&(ListaOrdini[nIndex]),pOrdine);
						}
					}
				}
			}
		}

		/*
		* leggo le strutture dei settori
		*/
		for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
			/* 
			* struttura di definizione del display 
			*/
			if(!fread((char *)&Cfg.Displays[nIndex],sizeof(DISPLAYSTRUCT),1,fp)){
#ifdef TRACE
				trace_out_vstr_date(1, "Errore in caricamento stato display : fread()");
#endif
				bRetValue=FALSE;
			}
		}
		if(bRetValue){
			/*
			* ripristino stato displays e luci
			*/
			for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
				if(pSettori[nIndex].nStatoLampada==1){
					SetLampada( pSettori[nIndex].nSettore,pSettori[nIndex].nPLCNum);
				}
			}
			for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
				UpdateDisplay(&(Cfg.Displays[nIndex]),TUTTO);
			}
		}
		/*
		* Chiusura file
		*/
		fclose(fp);
	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "Errore in rispristino stato settori : fopen(%s)", pszFileName);
#endif
		bRetValue=FALSE;
	}
	return bRetValue;
}

/*
* OrdProg : OOOOOO
* si visualizza : OOOO (ultime 4 cifre ordine)
*/
char *GetDisplayData(char *pszOrdProg)
{
	static char szData[128];

	strcpy(szData,RightStr(pszOrdProg,4));

	return szData;
}

void SetStatoLinea(int nStatoLinea)
{
	if(pDatiLinea) pDatiLinea->nStatoLinea=nStatoLinea;
}

int GetStatoLinea(void)
{
	if(pDatiLinea) return (pDatiLinea->nStatoLinea);
	return LINEA_IN_STOP;
}



/*
* int ReadDisplaysInfo(DISPLAYSTRUCT pDisplays)
*
* - Legge dalla relazione displays le informazioni relative ai displays della linea e
*   le memorizza nella struttura DISPLAYSTRUCT;
* - ritorna il numero dei displays memorizzati nella relazione 
*/
int ReadDisplaysInfo(PDISPLAYSTRUCT pDisplays)
{
	DBresult *PGres;
	int nDisplays=0;
	int nIndex=0;
	
	PGres=DBExecQuery(Cfg.nDebugVersion>1,"select dsnmdsp,dsnmisl,dsnmset from displays order by display;");

	if(DBresultStatus(PGres) == DBRES_TUPLES_OK && (nDisplays=DBntuples(PGres))){

		/* memorizzo le informazioni nella struttura preposta */
		for(nIndex=0; nIndex<nDisplays; nIndex++){

				pDisplays[nIndex].nDisplay = atoi(DBgetvalue(PGres,nIndex,0));
				pDisplays[nIndex].nIsola   = atoi(DBgetvalue(PGres,nIndex,1));
				pDisplays[nIndex].nSettore = atoi(DBgetvalue(PGres,nIndex,2));
		}

		DBclear(PGres);

	} else {
		char szMsg[256];

		/* c'e' qualcosa che non va', lo segnalo */
		sprintf(szMsg,"ATTENZIONE : fallita ReadDisplaysInfo() - Exit(1) di SETTORI");
		SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
		trace_out_vstr_date(1,szMsg);
#endif

		DBclear(PGres);
		exit(1);

	}
	return nDisplays;
}

/*
* int ReadSettoriInfo(void)
*
* - Legge dalla relazione settori le informazioni relative ai settori della linea e
*   le memorizza nella struttura SETTORISTRUCT;
* - controlla che il numero dei settori memorizzati nella relazione del DB
*   coincida col il parametro <nNumeroSettori> passatogli
*/
void ReadSettoriInfo(int nNumeroSettori)
{
	DBresult *PGres;
	int nSettori=0;
	int nIndex=0;

	PGres=DBExecQuery(Cfg.nDebugVersion>1,"select stnmset,stnmisl,stcdflg,sttpset,stcdope from settori order by settore;");

	if(DBresultStatus(PGres) == DBRES_TUPLES_OK && ((nSettori=DBntuples(PGres))==nNumeroSettori) ){

#ifdef TRACE
		trace_out_vstr(1,"SETT I S T OPERATORE");
		trace_out_vstr(1,"---- - - - ---------");
#endif
		/* memorizzo le informazioni nella struttura preposta */
		for(nIndex=0; nIndex<nSettori; nIndex++){

			pSettori[nIndex].nSettore       = atoi(DBgetvalue(PGres,nIndex,0));
			pSettori[nIndex].nIsola         = atoi(DBgetvalue(PGres,nIndex,1));
			pSettori[nIndex].nStatoSettore  = atoi(DBgetvalue(PGres,nIndex,2));
			pSettori[nIndex].nTipoSettore   = atoi(DBgetvalue(PGres,nIndex,3));
			strcpy(pSettori[nIndex].szCodOperatore,DBgetvalue(PGres,nIndex,4));
#ifdef TRACE
			trace_out_vstr(1,"%4d %1d %1d %1d [%s]",
				pSettori[nIndex].nSettore,
				pSettori[nIndex].nIsola,
				pSettori[nIndex].nStatoSettore,
				pSettori[nIndex].nTipoSettore,
				pSettori[nIndex].szCodOperatore);
#endif
		}
#ifdef TRACE
		trace_out_vstr(1,"---- - - - ---------");
#endif
	
		DBclear(PGres);

	} else {
		char szMsg[256];

		/* c'e' qualcosa che non va', lo segnolo */
		sprintf(szMsg,"ATTENZIONE : fallita ReadSettoriInfo() - Exit(1) di SETTORI");
		SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
		trace_out_vstr_date(1,szMsg);
#endif

		DBclear(PGres);
		exit(1);
	}
}


int GetDisplay(int nDisplay,int nIsola)
{
	int nIndex;
	BOOL bFound=FALSE;
	
	for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
		if((Cfg.Displays[nIndex].nDisplay==nDisplay) && (Cfg.Displays[nIndex].nubnmisl==nIsola)){
			bFound=TRUE;
			break;
		}
	}
	if(!bFound){
		return 0;
	}
	
	return nIndex;
}

void SaveStateIntFunc()
{
	char szBuffer[128];

	sprintf(szBuffer,"%s/settori.stato",Cfg.szPathData);
	SaveState(szBuffer);
}

/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
#endif

    DetachSharedMemory((char *)pDatiLinea);
    DetachSharedMemory((char *)pSettori);

	if(!DeleteSharedMemory(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE))){
#ifdef TRACE
		trace_out_vstr_date(1,"Error Removing Settori Shared Memory (%d)", Cfg.nSettoriShmKey);
#endif
	}

	if(Cfg.bAlone){

        DeleteProcessMsgQ(PROC_SETTORI);

		DetachSharedMemory((char *)pDatiLinea);

		if(!DeleteSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT))){
			trace_out_vstr_date(1,"Error Removing Shared Memory (%d)", Cfg.nShmKey);
		}
		pDatiLinea=(PLINEA_STRUCT)NULL;

		TerminateProcess(PROC_IOS);
	}
	DBDisconnect();

	/*
	* avverto MAIN dello stop
	*/
	SendMessage(PROC_MAIN, PROC_SETTORI,  PROGRAM_STOPPED, NULL);

	exit(1);
} /* Fine SafeIntFunc */
