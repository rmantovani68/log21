/*
* volfun.c :  funzioni di utilita' di Analisi volumetrica 
* 
* Progetto Easy Picking 4.0
*
* Autore : Roberto Mantovani - Stefano Tarroni
*
* Copyright A&L srl 2003-2021
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <glib.h>
#include <gio/gio.h>

#include <libpq-fe.h>
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

#include	"volum.h"
#include	"volstruct.h"
#include	"volext.h"
#include	"volfun.h"

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(ep_bool_t bReadProcInfo)
{
	char szParagraph[128];
	char szCurrentDirectory[128];
	char szCfgFileName[128];

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(szCfgFileName, szCurrentDirectory);
	strcat(szCfgFileName, "/");
	strcat(szCfgFileName, __configuration_file__);
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	GetFileString(szParagraph,"TipoOrdini",  "N",        Cfg.szTipoOrdini,        80,szCfgFileName,NULL);
	GetFileString(szParagraph,"Language",    "ita",      Cfg.szLanguage,          80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathData",    "../data",  Cfg.szPathData,          80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExport",  "../export",Cfg.szPathExport,        80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExe",     "./",       Cfg.szPathExe,           80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathTrace",   "../trace", Cfg.szPathTrace,         80,szCfgFileName,NULL);
	GetFileString(szParagraph,"FormatoMedio","2",        Cfg.szFormatoMedio,      80,szCfgFileName,NULL);

	Cfg.nShmKey                = GetFileInt(szParagraph,"ShmKey",             1000,szCfgFileName,NULL);
	Cfg.nDebugLevel          = GetFileInt(szParagraph,"DebugLevel",          0,szCfgFileName,NULL);
	Cfg.nMaxColliOrdine        = GetFileInt(szParagraph,"MaxColliOrdine",        0,szCfgFileName,NULL);
	Cfg.nSogliaPallet          = GetFileInt(szParagraph,"SogliaPallet",          0,szCfgFileName,NULL);
	Cfg.nIncrementoCopiePallet = GetFileInt(szParagraph,"IncrementoCopiePallet", 0,szCfgFileName,NULL);
	Cfg.nSogliaCopiePre        = GetFileInt(szParagraph,"SogliaCopiePrelievo",   0,szCfgFileName,NULL);
	Cfg.nDelay                 = GetFileInt(szParagraph,"Delay",               100,szCfgFileName,NULL);


	/*
	* lettura del file di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	GetFileString(szParagraph,"DBHost",     "localhost", Cfg.szDBHost,     80,szCfgFileName, NULL); 
	GetFileString(szParagraph,"DBPort",     "5432",      Cfg.szDBPort,     80,szCfgFileName, NULL); 
	GetFileString(szParagraph,"DBName",     "momo",      Cfg.szDBName,     80,szCfgFileName, NULL); 
	GetFileString(szParagraph,"DBUser",     "user",      Cfg.szDBUser,     80,szCfgFileName, NULL); 
	GetFileString(szParagraph,"DBPassword", "pwd",       Cfg.szDBPassword, 80,szCfgFileName, NULL); 

	if(bReadProcInfo){
        ReadProcInfo(szCfgFileName);
	}
}

#ifdef ELIMINATO
/*
* ReadProcInfo()
* Lettura dei parametri relativi ai processi 
*/
void ReadProcInfo(char *szFileConfig)
{
	char szProcIndex[128];
	char szProcBuffer[128];
	int nIndex;

	for (nIndex=0; nIndex<NUM_PROC; nIndex++) {
		sprintf(szProcIndex,"Proc_%02d",nIndex);
		GetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szFileConfig,NULL);
		sscanf(szProcBuffer, "%[^,],%d,%d,%d",
				ProcList[nIndex].szProcName,
				&ProcList[nIndex].nQKey,
				&ProcList[nIndex].nPriority,
				&ProcList[nIndex].bExecute);
		/* 
		* rm 16-01-2002 : Elimino gli spazi in testa e in coda per 
		*                 problemi con la RunSimpleProcess()  e la lettura della condifgurazione
		*/
		StrTrimAll(ProcList[nIndex].szProcName);

		ProcList[nIndex].nGId=nIndex+1;

		/*
		* resetto il numero coda messaggi del processo
		*/
		ProcList[nIndex].nQNumber=0;
	}
}

#endif

/*
* lettura dell'archivio Imballli
*/
int ReadImballi(PIMBALLI pImb)
{
	int nIndex;
	DBresult *DBRes = NULL;
	int nTuples;

	/*
	* Seleziono solo gli imballi con dimensioni diverse da 0 se tipo lavorazione 0 o 2 
	* oppure anche con volume nullo se tipo lavorazione = 1 (fuori linea)
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select codice, descriz, lunghezza, altezza, larghezza, tara, pesomax, volperc, tplavor,categoria from imballi where ((larghezza > 0 AND altezza > 0 AND lunghezza > 0) OR tplavor = 1) and ordtipo='%s';",Cfg.szTipoOrdini); 
	nTuples=DBntuples(DBRes);

	nIndex=0;
	pImb->nImballiAutomatici=0;
	pImb->nImballi=0;
	for(nIndex=0;nIndex<nTuples;nIndex++){
		strcpy(pImb->Imballo[nIndex].szCodice,  DBgetvalue(DBRes,nIndex,0));
		pImb->Imballo[nIndex].nLength    = atoi(DBgetvalue(DBRes,nIndex,2));
		pImb->Imballo[nIndex].nHeight    = atoi(DBgetvalue(DBRes,nIndex,3));
		pImb->Imballo[nIndex].nWidth     = atoi(DBgetvalue(DBRes,nIndex,4));
		pImb->Imballo[nIndex].nTara      = atoi(DBgetvalue(DBRes,nIndex,5));
		pImb->Imballo[nIndex].nPesoMax   = atoi(DBgetvalue(DBRes,nIndex,6));
		pImb->Imballo[nIndex].nVolPerc   = atoi(DBgetvalue(DBRes,nIndex,7));
		/* tipo imballo - 0:linea automatica, 1:linea manuale 2:evasione su chiusura*/
		pImb->Imballo[nIndex].nSwLinea   = atoi(DBgetvalue(DBRes,nIndex,8));
		pImb->Imballo[nIndex].nCategoria = atoi(DBgetvalue(DBRes,nIndex,9));

		pImb->Imballo[nIndex].nDim[0] = pImb->Imballo[nIndex].nLength;
		pImb->Imballo[nIndex].nDim[1] = pImb->Imballo[nIndex].nWidth;
		pImb->Imballo[nIndex].nDim[2] = pImb->Imballo[nIndex].nHeight;
		/*
		* ordino le dimensioni della riga
		*/
		qsort(pImb->Imballo[nIndex].nDim,3,sizeof(int),(int(*)())CmpInt);

		/*
		* calcolo volume scatola in cc (Centimetri Cubici)
		*/
		pImb->Imballo[nIndex].nVolume = (pImb->Imballo[nIndex].nLength * pImb->Imballo[nIndex].nWidth * pImb->Imballo[nIndex].nHeight)/1000;


		/*
		* calcolo volume utile scatola (percentuale volume totale)
		*/
		pImb->Imballo[nIndex].nVolumeUtile = pImb->Imballo[nIndex].nVolume * (float)((float)pImb->Imballo[nIndex].nVolPerc/(float)100);
		if( pImb->Imballo[nIndex].nSwLinea != COLLO_MANUALE){
			pImb->nImballiAutomatici++;
		}
		pImb->nImballi++;
	}
		
	DBclear(DBRes);

	return pImb->nImballi;
}


int CmpInt(int *pInt1,int *pInt2)
{
	return *pInt1 - *pInt2;
}

void SortImballi(PIMBALLI pImb)
{
	qsort(pImb->Imballo,pImb->nImballi,sizeof(IMBALLO),(int(*)())CmpImballo);
}

int CmpImballo(PIMBALLO pImb1,PIMBALLO pImb2)
{
	/* 
	* se il tipo lavorazione e' diverso
	*/
	if( pImb1->nSwLinea != pImb2->nSwLinea ){
		/*
		* Prima gli imballi con sw <> 1 e dopo quelli con sw = 1
		*/
		if(pImb2->nSwLinea == 1 || pImb1->nSwLinea == 1){
			return pImb2->nSwLinea == 1?-1:1;
		}
	}

	/* 
	* imballo 1 piu' piccolo di imballo 2 per volume utile
	*/
	if( pImb1->nVolumeUtile <= pImb2->nVolumeUtile){
		return -1;
	}
	/* 
	* imballo 1 piu' grande di imballo 2 per volume utile
	*/
	if( pImb1->nVolumeUtile > pImb2->nVolumeUtile){
		return 1;
	}

	return 0;
}

gint CmpRigaOrdine(gconstpointer P1,gconstpointer P2)
{
	PEP_RIGA pRiga1=(PEP_RIGA)P1;
	PEP_RIGA pRiga2=(PEP_RIGA)P2;


	/* 
	* se fuori formato la metto prima degli altri
	* con gestione di categoria FF
	*/
	if(pRiga1->nSwFF > pRiga2->nSwFF){
		return -1;
	} else if( pRiga1->nSwFF < pRiga2->nSwFF){
		return 1;
	}
	/* 
	* se si tratta di F.F.
	* ordino per categoria 
	*/
	if(pRiga1->nCategoriaFF < pRiga2->nCategoriaFF){
		return -1;
	} else if( pRiga1->nCategoriaFF > pRiga2->nCategoriaFF){
		return 1;
	}
	/* 
	* Preconfezionati
	* prima delle righe normali
	*/
	if(pRiga1->nSwPCF > pRiga2->nSwPCF){
		return -1;
	} else if(pRiga1->nSwPCF < pRiga2->nSwPCF){
		return 1;
	}

	/* 
	* ordino per priorita
	*/
	if(pRiga1->nPriorita < pRiga2->nPriorita){
		return -1;
	} else if( pRiga1->nPriorita > pRiga2->nPriorita){
		return 1;
	}
	return 0;
}

/*
* InsertRigProd()
* Crea una riga nell'archivio righe di produzione con i dati contenuti in DBRes
* PARAMETRI: Riga
*/
ep_bool_t InsertRigProd(PEP_RIGA pRiga, char cStato)
{
	DBresult *DBResInsert;
	ep_bool_t bRetValue=TRUE;


	DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into rig_prod ( ordprog, rpnmcol, rpcdpro, rpqtord, rpqtspe, rppzpre, rpprrow, rpprrig, rpcdubi, rpprior, rpswffo, rpswpcf, rpstato, rpcdflg  ) values ( '%s', %d, '%s', %d, %d, %d, %d, %d, '%s', %d, '%d', %d, '%c', '%c');",
		pRiga->szOrdProg,
		pRiga->nCollo,
		pRiga->szCodiceProdotto,
		pRiga->nCopieOrdinate,
		pRiga->nCopieCaricate,
		pRiga->nPrezzo,
		pRiga->nProgRigaRicevuta, 
		pRiga->nProgressivoRiga, 
		pRiga->szUbicazione,
		pRiga->nPriorita,
		pRiga->nSwFF,
		pRiga->nSwPCF,
		cStato,
		pRiga->cFlag);

	if(atoi(DBcmdTuples(DBResInsert))==0){
		bRetValue=FALSE;
	}
	DBclear(DBResInsert);

	return bRetValue;
}

/*
* InsertColProd(pCollo)
* Crea un collo nell'archivio colli di produzione 
*/
ep_bool_t InsertColProd(PCOLLO pCollo,char cStato)
{
	DBresult *DBResInsert;
	ep_bool_t bRetValue=TRUE;

	/*
	* Ricavo il codice a barre del collo
	*/
	strcpy(pCollo->szBarcode,ColloBarcode(pCollo));

	DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into col_prod ( ordprog, cptpfor, cpswlin, cpnmcol, cpnmrgh, cpnmcpe, cppspre, cpbrcde, cpstato, cpcdflg) values ( '%s','%s','%d',%d,%d,%d,%d,'%s','%c',' ');",
		pCollo->szOrdProg,
		pCollo->szCodiceImballo,
		pCollo->nSwLinea,
		pCollo->nCollo, 
		pCollo->nRighe, 
		pCollo->nCopie, 
		pCollo->nPeso, 
		pCollo->szBarcode,
		cStato);

	if(atoi(DBcmdTuples(DBResInsert))==0){
		bRetValue=FALSE;
	}

	DBclear(DBResInsert);

	return bRetValue;
}

/*
* UpdateOrdine(pOrdine)
* Aggiorna i dati di un ordine nell'archivio ordini di produzione 
*/
ep_bool_t UpdateOrdine(PORDINE pOrdine)
{
	DBresult *DBResUpdate;
	ep_bool_t bRetValue=TRUE;

	DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set ronmrgp=%d, ronmcpp=%d, ronmcla=%d, ronmcll=%d, ropspre=%d, rostato='%c' where ordprog='%s';",
		pOrdine->nRighe,
		pOrdine->nCopie,
		pOrdine->nColliAutomatici,
		pOrdine->nColli,
		pOrdine->nPesoUsato,
		pOrdine->cStato,
		pOrdine->szOrdProg);

	if(atoi(DBcmdTuples(DBResUpdate))==0){
		bRetValue=FALSE;
	}

	DBclear(DBResUpdate);

	return bRetValue;
}

ep_bool_t CambiaFlagRigaRicevuta(char *pszOrdProg,char *pszCodiceProdotto,char cFlag)
{
	ep_bool_t bOK=FALSE;
	DBresult *DBRes;

	/* rettifica riga ricevuta */
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_art set racdflg='%c' where ordprog='%s' and racdpro='%s';",cFlag,pszOrdProg,pszCodiceProdotto);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK  || atoi(DBcmdTuples(DBRes))==0){
		bOK=FALSE;
#ifdef TRACE
		trace_debug(TRUE, TRUE, "fallito update riga ricevuta Ordine [%s] Prodotto [%s]",pszOrdProg,pszCodiceProdotto);
#endif
	}
	DBclear(DBRes);
	
	return bOK;
}

/*
* AnalisiVolumetrica()
* Analisi Volumetrica degli ordini lanciati,
*/
int AnalisiVolumetrica(char cStato,ep_bool_t bControl,int nProcIndex,ep_bool_t bTest)
{
	int nOrdiniDaAnalizzare;
	int nOrdiniAnalizzati;
	int nOrdineIndex;
	char szOrdine[128];
	int nSWCol;
	DBresult *DBResOrdini;
	ep_bool_t bPallet=FALSE;

	/*
	* Seleziono gli ordini 'segnati' per l'analisi volumetrica
	* ordinamento per sequenza di lancio
	*/
	DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,roswcol from ric_ord where rostato='%c' and ordtipo='%s' order by roprgln;",cStato,Cfg.szTipoOrdini);

	nOrdiniDaAnalizzare=DBntuples(DBResOrdini);
	nOrdiniAnalizzati=0;

	for(nOrdineIndex=0; nOrdineIndex<nOrdiniDaAnalizzare; nOrdineIndex++){
		strcpy(szOrdine,DBgetvalue(DBResOrdini,nOrdineIndex,0));
		nSWCol=atoi(DBgetvalue(DBResOrdini,nOrdineIndex,1));
		if(nSWCol==ORDINE_PALLET){
			bPallet=TRUE;
		} else {
			bPallet=FALSE;
		}
		if(AnalisiVolumetricaOrdine(szOrdine,bControl,bPallet,FALSE,FALSE,nProcIndex,bTest)){
			nOrdiniAnalizzati++;
		}
	}

	DBclear(DBResOrdini);
		
	return nOrdiniAnalizzati;
}

/*
* ep_bool_t AnalisiVolumetricaOrdine()
* Analisi Volumetrica dell'ordine specificato
*/
ep_bool_t AnalisiVolumetricaOrdine(char *szOrdProg,ep_bool_t bCheckTables,ep_bool_t bPallet,ep_bool_t bForzatura,ep_bool_t bProdottiNonUbicati,int nProcIndex,ep_bool_t bTest)
{
	GList    *RigheCollo=NULL;
	GList    *Righe2=NULL;
	GList    *Righe=NULL;
	GList    *Colli=NULL;
	GList    *list=NULL;
	PEP_RIGA pRiga;
	PEP_RIGA pRigaInsert;
	PCOLLO   pCollo;
	PCOLLO   pColloInsert;
	ep_bool_t bFine=FALSE;
	ep_bool_t bErroreAnalisi=FALSE;
	ep_bool_t bOK=TRUE;
	char     szMsg[256];
	char     szBuffer[128];
	int      nRC;
	int      nCollo;
	int      nTuples;
	int      nIndex;
	int      nRigaIndex;
	int      nRigheOrdine;
	DBresult *DBResOrdini;
	DBresult *DBResRighe;
	DBresult *DBResUbi;
	DBresult *DBResCat;
	DBresult *DBResDelete;
	DBresult *DBRes;
	EP_RIGA  Riga;
	COLLO    Collo;
	ORDINE   Ordine;

	/*
	* Seleziono gli ordini 'segnati' per l'analisi volumetrica
	* ordinamento per sequenza di lancio
	*/
	DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,rocdrid,ordtipo,roprgln,rostato,rocdflg from ric_ord where ordprog='%s';",szOrdProg);

	if(DBntuples(DBResOrdini)){
		
		strcpy(Ordine.szOrdProg,    DBgetvalue(DBResOrdini,0,0));
		strcpy(Ordine.szOrdTipo,    DBgetvalue(DBResOrdini,0,2));


		Ordine.nProgressivoLancio=atoi(DBgetvalue(DBResOrdini,0,3));
		strcpy(szBuffer,DBgetvalue(DBResOrdini,0,4)); Ordine.cStato=szBuffer[0];
		strcpy(szBuffer,DBgetvalue(DBResOrdini,0,5)); Ordine.cFlag=szBuffer[0];
	} else {
		bOK=FALSE;
	}
	DBclear(DBResOrdini);

	/*
	* reset dati ordine
	*/
	Ordine.nColli=0;
	Ordine.nColliAutomatici=0;
	Ordine.nRighe=0;
	Ordine.nCopie=0;
	Ordine.nPesoTotale=0;
	Ordine.nPesoUsato=0;
	Ordine.nVolumeTotale=0;
	Ordine.nVolumeUsato=0;
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Analisi Ordine       [%s]",Ordine.szOrdProg);
	trace_debug(FALSE, TRUE, "Forzatura            [%d]",bForzatura);
	trace_debug(FALSE, TRUE, "Check Tabelle        [%d]",bCheckTables);
	trace_debug(FALSE, TRUE, "Prodotti non ubicati [%d]",bProdottiNonUbicati);
	trace_debug(FALSE, TRUE, "Collo F.to  Peso    Volume Vol.%% Toll.%%");
	trace_debug(FALSE, TRUE, "----- ----- ------- ------ ----- ------");
#endif
	Ordine.cStato=ORDINE_IN_ANALISI;
	bOK=UpdateOrdine(&Ordine);

	/*
	* Cancellazione eventuali dati di produzione
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from col_prod where ordprog='%s';",Ordine.szOrdProg);
	if(DBntuples(DBRes)){
		DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from col_prod where ordprog='%s';",Ordine.szOrdProg); DBclear(DBResDelete);
	}
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from rig_prod where ordprog='%s';",Ordine.szOrdProg);
	if(DBntuples(DBRes)){
		DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from rig_prod where ordprog='%s';",Ordine.szOrdProg); DBclear(DBResDelete);
	}
	DBclear(DBRes);

	/*
	* -----------------------------------------
	* PRIMA SCANSIONE DELL'ANALISI VOLUMETRICA
	* -----------------------------------------
	*/

	/*
	* Seleziono le righe relative all'ordine in oggetto
	*/

	if(bCheckTables){
		DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"select racdpro, rapzpre, raqtord , raprrow from ric_art where ordprog='%s';",Ordine.szOrdProg);

		nRigheOrdine=DBntuples(DBResRighe);

		for(nRigaIndex=0; nRigaIndex<nRigheOrdine; nRigaIndex++){
			strcpy(Riga.szOrdProg,Ordine.szOrdProg);
			Riga.nCollo=0;
			Riga.nProgressivoRiga=0;

			strcpy(Riga.szCodiceProdotto,DBgetvalue(DBResRighe,nRigaIndex,0));
			Riga.nPrezzo           = atoi(DBgetvalue(DBResRighe,nRigaIndex,1));
			Riga.nCopieOrdinate    = atoi(DBgetvalue(DBResRighe,nRigaIndex,2));
			Riga.nProgRigaRicevuta = atoi(DBgetvalue(DBResRighe,nRigaIndex,3));

			/*
			* Assegno l'ubicazione
			*/
			DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione,ubqtcas,ubqtimp,priorita from ubicazioni where ubitipo='%s' and codprod='%s' order by priorita;", Cfg.szTipoOrdini,Riga.szCodiceProdotto);
			if((nTuples=DBntuples(DBResUbi))){
				int nQTCas,nQTImp;

				if(nTuples>1){
					/* segnalo la multipla istanza */
					if(Cfg.nDebugLevel>1){
						sprintf(szMsg,"ATTENZIONE : Prodotto [%s] presente in multipla istanza in ubicazioni !\n",Riga.szCodiceProdotto); 
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
					}
#ifdef TRACE
					trace_debug(FALSE, TRUE, "Prodotto [%s] presente in multipla istanza in ubicazioni !",Riga.szCodiceProdotto); 
#endif
				}

				/*
				* Prendo la prima ubicazione in ordine di priorita
				*/
				strcpy(Riga.szUbicazione,DBgetvalue(DBResUbi,0,0));
				nQTCas = atoi(DBgetvalue(DBResUbi,0,1));
				nQTImp = atoi(DBgetvalue(DBResUbi,0,2));
				Riga.nPriorita = atoi(DBgetvalue(DBResUbi,0,3));
				/*
				* nel caso in cui la riga non abbia producibilita'
				* e l'ordine deve entrare lo stesso segnalo la riga come forzata/tagliata
				*/
				Riga.cFlag=' ';

				/*
				* Gestione forzatura ordine
				*/
				if(bForzatura){
					if(Riga.nCopieOrdinate > nQTCas-nQTImp){
						Riga.cFlag=RIGA_TAGLIATA;

						CambiaFlagRigaRicevuta(Ordine.szOrdProg,Riga.szCodiceProdotto,RIGA_TAGLIATA);

						/* 
						* rendo disponibili le copie presenti
						* se superiori alla soglia minima di prelievo (in %)
						*/
						if(nQTCas-nQTImp >= max(Riga.nCopieOrdinate*Cfg.nSogliaCopiePre/100,1)){
							Riga.nCopieOrdinate=nQTCas-nQTImp;
						} else {
							Riga.nCopieOrdinate=0;
						}
					} else {
						CambiaFlagRigaRicevuta(Ordine.szOrdProg,Riga.szCodiceProdotto,' ');
					}
				}
			} else {
				/*
				* MANCANZA UBICAZIONE PER IL PRODOTTO RELATIVO ALLA RIGA
				* Se sono in forzatura ordine setto il flag a 'U' (mancanza ubicazione)
				*/
				if(bProdottiNonUbicati){
					Riga.cFlag=RIGA_NON_UBICATA;
					strcpy(Riga.szUbicazione,"");
					Riga.nPriorita = 0;
				} else {
					/* segnalo la mancanza del prodotto in catalogo */
					if(Cfg.nDebugLevel>1){
						sprintf(szMsg,"ATTENZIONE : Prodotto [%s] non presente in ubicazioni !\n",Riga.szCodiceProdotto); 
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
					}
#ifdef TRACE
					trace_debug(FALSE, TRUE, "Prodotto [%s] non presente in ubicazioni !",Riga.szCodiceProdotto); 
#endif
					bOK=FALSE;
				}
			}
			DBclear(DBResUbi);

			/*
			* Assegno i dati del prodotto dal catalogo
			*/
			DBResCat=DBExecQuery(Cfg.nDebugLevel>1,"select prlungh, prlargh, praltez, prpesgr, prswffo,prqtffo,prqtpcf,prqtpcp,prfcpcf,prctffo from catalogo where prcdpro='%s' ;", Riga.szCodiceProdotto);
			if((nTuples=DBntuples(DBResCat))){
				if(nTuples>1){
					/* segnalo la multipla istanza */
					if(Cfg.nDebugLevel>1){
						sprintf(szMsg,"ATTENZIONE : Prodotto [%s] presente in multipla istanza in catalogo !\n",Riga.szCodiceProdotto); 
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
					}
#ifdef TRACE
					trace_debug(FALSE, TRUE, "Prodotto [%s] presente in multipla istanza in catalogo !",Riga.szCodiceProdotto); 
#endif
				}

				Riga.nLength       = atoi(DBgetvalue(DBResCat,0,0));
				Riga.nWidth        = atoi(DBgetvalue(DBResCat,0,1));
				Riga.nHeight       = atoi(DBgetvalue(DBResCat,0,2));
				Riga.nPesoCopia    = atoi(DBgetvalue(DBResCat,0,3));
				Riga.nSwFF         = atoi(DBgetvalue(DBResCat,0,4));
				Riga.nCopieFF      = atoi(DBgetvalue(DBResCat,0,5));
				Riga.nCopiePCF     = atoi(DBgetvalue(DBResCat,0,6));
				Riga.nCopiePCP     = atoi(DBgetvalue(DBResCat,0,7));
				strcpy(Riga.szFCPCF,      DBgetvalue(DBResCat,0,8));
				Riga.nCategoriaFF  = atoi(DBgetvalue(DBResCat,0,9));

				Riga.nSwPCF    = Riga.nCopiePCF>0;
			} else {
				/* segnalo la mancanza del prodotto in catalogo */
				if(Cfg.nDebugLevel>1){
					sprintf(szMsg,"ATTENZIONE : Prodotto [%s] non presente in catalogo !\n",Riga.szCodiceProdotto); 
					SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
				}
#ifdef TRACE
				trace_debug(FALSE, TRUE, "Prodotto [%s] non presente in catalogo !",Riga.szCodiceProdotto); 
#endif
				bOK=FALSE;
			}
			DBclear(DBResCat);

			if(bOK){
				if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
					memcpy(pRigaInsert,&Riga,sizeof(EP_RIGA));
					Righe=g_list_append(Righe,pRigaInsert);
				} else {
					sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
					SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
					trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
				}
			}
			
		}
		DBclear(DBResRighe);
	} else {
		/*
		* Seleziono tutto in un unico comando sql
		* il rischio e' di perdere qualcosa senza segnalare niente
		* (catalogo / ubicazioni )
		*/
		DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"select r.racdpro, r.raqtord, u.ubicazione, u.priorita, c.prlungh, c.prlargh, c.praltez, c.prpesgr, c.prswffo, c.prqtffo, c.prqtpcf, c.prqtpcp, c.prfcpcf, c.prctffo,r.raprrow from ric_art r,catalogo c,ubicazioni u where r.racdpro=c.prcdpro and r.racdpro=u.codprod and r.ordprog='%s';",Ordine.szOrdProg);

		nRigheOrdine=DBntuples(DBResRighe);

		for(nRigaIndex=0; nRigaIndex<nRigheOrdine; nRigaIndex++){
			strcpy(Riga.szOrdProg,Ordine.szOrdProg);
			Riga.nCollo=0;
			Riga.cFlag=' ';
			Riga.nProgressivoRiga=0;
			Riga.nPrezzo=0;

			strcpy(Riga.szCodiceProdotto,DBgetvalue(DBResRighe,nRigaIndex,0));
			Riga.nCopieOrdinate   = atoi(DBgetvalue(DBResRighe,nRigaIndex,1));

			strcpy(Riga.szUbicazione,    DBgetvalue(DBResRighe,nRigaIndex,2));
			Riga.nPriorita        = atoi(DBgetvalue(DBResRighe,nRigaIndex,3));

			Riga.nLength          = atoi(DBgetvalue(DBResRighe,nRigaIndex,4));
			Riga.nWidth           = atoi(DBgetvalue(DBResRighe,nRigaIndex,5));
			Riga.nHeight          = atoi(DBgetvalue(DBResRighe,nRigaIndex,6));
			Riga.nPesoCopia       = atoi(DBgetvalue(DBResRighe,nRigaIndex,7));
			Riga.nSwFF            = atoi(DBgetvalue(DBResRighe,nRigaIndex,8));
			Riga.nCopieFF         = atoi(DBgetvalue(DBResRighe,nRigaIndex,9));
			Riga.nCopiePCF        = atoi(DBgetvalue(DBResRighe,nRigaIndex,10));
			Riga.nCopiePCP        = atoi(DBgetvalue(DBResRighe,nRigaIndex,11));
			strcpy(Riga.szFCPCF,         DBgetvalue(DBResRighe,nRigaIndex,12));
			Riga.nCategoriaFF     = atoi(DBgetvalue(DBResRighe,nRigaIndex,13));
			Riga.nProgRigaRicevuta= atoi(DBgetvalue(DBResRighe,nRigaIndex,14));

			Riga.nSwPCF = Riga.nCopiePCF>0;

			if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
				memcpy(pRigaInsert,&Riga,sizeof(EP_RIGA));
				Righe=g_list_append(Righe,pRigaInsert);
			} else {
				sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
				SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
				trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
			}
			
		}
		DBclear(DBResRighe);
	}

	if(bOK==FALSE){
		/*
		* Problemi di analisi ordine (catalogo /ubicazioni), l'ordine non viene elaborato
		*/
		/*
		* Messaggio di errore in analisi ordine al MAIN
		*/
		SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, Ordine.szOrdProg);
		bErroreAnalisi=TRUE;


		g_list_free(Righe);
		g_list_free(Righe2);
		g_list_free(RigheCollo);
		g_list_free(Colli);

		return FALSE;
	}


	if((list=g_list_first(Righe))){
		do {
			pRiga = list->data;

			if(pRiga==(PEP_RIGA)NULL){
				/*
				* Problemi di allocazione ?
				*/
				bOK=FALSE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Riga NULL !!");
#endif
				continue;
			}
			/* calcolo volume totale ordine */
			Ordine.nVolumeTotale += (pRiga->nLength*pRiga->nWidth*pRiga->nHeight*pRiga->nCopieOrdinate)/1000;
			Ordine.nPesoTotale   += pRiga->nPesoCopia*pRiga->nCopieOrdinate;
			if(bPallet){
				/*
				* Se la spedizione e' a Pallet niente FF
				*/
				pRiga->nSwFF=0;

				pRiga->nSwPCF=pRiga->nCopiePCP>0;
				/*
				* Preconfezionato pallets
				*/
				if(pRiga->nSwPCF){
					int nColliPCP;
					int nRestoPCP;

					if(pRiga->nCopiePCP<=0){
						sprintf(szMsg,"ATTENZIONE : prodotto [%s] numero di copie PCP<=0 (%d)\n",pRiga->szCodiceProdotto,pRiga->nCopiePCP);
#ifdef TRACE
						trace_debug(TRUE, TRUE, "Prodotto [%s] numero di copie PCP<=0 (%d)",pRiga->szCodiceProdotto,pRiga->nCopiePCP);
#endif
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
						bOK=FALSE;
					} else {
						/*
						* ------------------------
						* Gestione Preconfezionato
						* ------------------------
						* aggiungo in testa alla lista per 
						* evitare di ripassarle all'infinito
						*/
						nColliPCP   = pRiga->nCopieOrdinate / pRiga->nCopiePCP;
						nRestoPCP   = pRiga->nCopieOrdinate % pRiga->nCopiePCP;
						/*
						* righe di produzione per il PCP
						*/
						/* riga di prelievo PCP */
						pRiga->nCopieOrdinate = pRiga->nCopiePCP;
						for(nIndex=0;nIndex<nColliPCP;nIndex++){
							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								Righe2=g_list_append(Righe2,pRigaInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
						/*
						* se c'e' del resto creo una riga di produzione apposita
						*/
						if(nRestoPCP){
							/* riga di prelievo normale */
							pRiga->nCopieOrdinate = nRestoPCP;
							pRiga->nSwPCF = 0;

							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								Righe2=g_list_append(Righe2,pRigaInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
					}
				} else {
					/* riga pallet normale */
					if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
						memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
						Righe2=g_list_append(Righe2,pRigaInsert);
					} else {
						sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
						trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
						bOK=FALSE;
					}
				}
			} else {
				/*
				* ----------------------
				* Gestione Fuori Formato
				* ----------------------
				* se riga FF creo subito le righe definitive
				* aggiungo in testa alla lista per 
				* evitare di ripassarle all'infinito
				*/
				if(pRiga->nSwFF && pRiga->nCategoriaFF==0){
					int nColliFF;             /* Colli di Fuori Formato */
					int nRestoFF;             /* Resto di Fuori Formato da mettere nei colli di linea */

					if(pRiga->nCopieFF==0){
						sprintf(szMsg,"ATTENZIONE : prodotto [%s] numero di copie FF==0 (%d)\n",pRiga->szCodiceProdotto,pRiga->nCopieFF);
#ifdef TRACE
						trace_debug(TRUE, TRUE, "Prodotto [%s] numero di copie FF==0 (%d)",pRiga->szCodiceProdotto,pRiga->nCopieFF);
#endif
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
						bOK=FALSE;
					} else {
						/*
						* 20-03-2018 rm : modifica per gestire più colli di fuori formato per copia di prodotto FF
						* se qtffo è negativa il numero assoluto è il numero di colli per copia
						*/
						if(pRiga->nCopieFF>0){
							nColliFF=pRiga->nCopieOrdinate / pRiga->nCopieFF;
							nRestoFF=pRiga->nCopieOrdinate % pRiga->nCopieFF;
						} else {
							/* colli FF uguale a copie ordinate per valore assoluto copie FF */
							nColliFF=(pRiga->nCopieOrdinate) * (-1*pRiga->nCopieFF);
							nRestoFF=0;
						}

						/*
						* righe di produzione per colli di Fuori Formato (F)
						*/
						for(nIndex=0;nIndex<nColliFF;nIndex++){
							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								if(pRigaInsert->nCopieFF>0){
									/* inserisco riga normale */
									pRigaInsert->nCopieOrdinate = pRigaInsert->nCopieFF;
								} else {
									/* 
									* 23-03-2018 rm 
									* -------------
									* copie FF negative : significa che si tratta di riga FF multi-collo
									* ogni copia di un prodotto FF multi-collo genera tanti colli quanti specificati nel campo Copie FF
									* della riga di tabella catalogo corrispondente (PRQTFFO) in valore assoluto
									*/
									if(nIndex==0){
										/* se primo collo FF inserisco prima riga con copie FF a 1 */
										pRigaInsert->nCopieOrdinate = 1;
									} else {
										/* se successivo collo FF inserisco riga con copie FF a 0 */
										pRigaInsert->nCopieOrdinate = 0;
									}
								}
								Righe2=g_list_append(Righe2,pRigaInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
						/*
						* se c'e' del resto creo una riga di produzione (FF) apposita
						*/
						if(nRestoFF){
							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								pRigaInsert->nCopieOrdinate = nRestoFF;
								Righe2=g_list_append(Righe2,pRigaInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
					}
				} else if(pRiga->nSwPCF){
					int nColliPCF;
					int nRestoPCF;

					if(pRiga->nCopiePCF<=0){
						sprintf(szMsg,"ATTENZIONE : prodotto [%s] numero di copie PCF<=0 (%d)\n",pRiga->szCodiceProdotto,pRiga->nCopiePCF);
#ifdef TRACE
						trace_debug(TRUE, TRUE, "Prodotto [%s] numero di copie PCF<=0 (%d)",pRiga->szCodiceProdotto,pRiga->nCopiePCF);
#endif
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
						bOK=FALSE;
					} else {
						/*
						* ------------------------
						* Gestione Preconfezionato
						* ------------------------
						* aggiungo in testa alla lista per 
						* evitare di ripassarle all'infinito
						*/
						nColliPCF   = pRiga->nCopieOrdinate / pRiga->nCopiePCF;
						nRestoPCF   = pRiga->nCopieOrdinate % pRiga->nCopiePCF;
						/*
						* righe di produzione per il PCF
						*/
						/* riga di prelievo PCF */
						pRiga->nCopieOrdinate = pRiga->nCopiePCF;
						for(nIndex=0;nIndex<nColliPCF;nIndex++){
							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								Righe2=g_list_append(Righe2,pRigaInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
						/*
						* se c'e' del resto creo una riga di produzione apposita
						*/
						if(nRestoPCF){
							/* riga di prelievo normale */
							pRiga->nCopieOrdinate = nRestoPCF;
							pRiga->nSwPCF = 0;

							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								Righe2=g_list_append(Righe2,pRigaInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
					}
				} else {
					/* riga normale */
					if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
						memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
						Righe2=g_list_append(Righe2,pRigaInsert);
					} else {
						sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
						SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
						trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
						bOK=FALSE;
					}
				}
			}
		} while((list=g_list_next(list)));
	}

	if(bOK==FALSE){
		/*
		* Problemi di analisi ordine (catalogo /ubicazioni), l'ordine non viene elaborato
		*/
		/*
		* Messaggio di errore in analisi ordine al MAIN
		*/
		SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, Ordine.szOrdProg);
		bErroreAnalisi=TRUE;


		g_list_free(Righe);
		g_list_free(Righe2);
		g_list_free(RigheCollo);
		g_list_free(Colli);

		return FALSE;
	}

	/*
	* Al termine del giro ottengo un vettore di righe da elaborare
	* che posso ordinare per priorita' di prelievo (e categoria di imballo F.F.)
	*/
	Righe2=g_list_sort(Righe2,CmpRigaOrdine);
	
#ifdef TRACE_LIST
	if((list=g_list_first(Righe2))){
		trace_debug(FALSE, TRUE, "%-4.4s %-12.12s %-12.12s %-4.4s %-1.1s %-1.1s %-1.1s", "N.C.", "CODICE", "UBICAZIONE", "QT", "F", "C", "P");
		trace_debug(FALSE, TRUE, "%-4.4s %-12.12s %-12.12s %-4.4s %-1.1s %-1.1s %-1.1s", "----", "------------", "------------", "----", "-", "-", "-");
		do {
			pRiga = list->data;

			if(pRiga==(PEP_RIGA)NULL){
				/*
				* Problemi di allocazione ?
				*/
				bOK=FALSE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Riga NULL !!");
#endif
				continue;
			}

			trace_debug(TRUE, TRUE, "%4d %-12.12s %-12.12s %4d %d %d %d",
				pRiga->nCollo,
				pRiga->szCodiceProdotto,
				pRiga->szUbicazione,
				pRiga->nCopieOrdinate,
				pRiga->nSwFF,
				pRiga->nCategoriaFF,
				pRiga->nSwPCF);
		} while((list=g_list_next(list)));
		trace_debug(FALSE, TRUE, "%-4.4s %-12.12s %-12.12s %-4.4s %-1.1s %-1.1s %-1.1s", "----", "------------", "------------", "----", "-", "-", "-");
		trace_debug(FALSE, TRUE, "Numero Righe : %d",g_list_length(Righe));
	}
#endif

	/*
	* --------------------------------------------------
	* FINE PRIMA SCANSIONE DELL'ANALISI VOLUMETRICA
	* --------------------------------------------------
	* INIZIO SECONDA SCANSIONE DELL'ANALISI VOLUMETRICA
	* --------------------------------------------------
	*/
	/*
	* reset dati ordine
	*/
	Ordine.nColli=0;
	Ordine.nColliAutomatici=0;
	Ordine.nRighe=0;
	Ordine.nCopie=0;
	Ordine.nVolumeUsato=0;

	/*
	* Assegno il codice progressivo al primo collo da creare
	*/
	nCollo=1;
	strcpy(Collo.szOrdProg,Ordine.szOrdProg);

	CreazioneCollo(&Collo,nCollo++,bPallet);

	if((list=g_list_first(Righe2))){
		do {
			pRiga = list->data;

			if(pRiga==(PEP_RIGA)NULL){
				/*
				* Problemi di allocazione ?
				*/
				bOK=FALSE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Riga NULL !!");
#endif
				continue;
			}

			pRiga->nDim[0] = pRiga->nLength;
			pRiga->nDim[1] = pRiga->nWidth;
			pRiga->nDim[2] = pRiga->nHeight;

			/*
			* ordino le dimensioni della riga
			*/
			qsort(pRiga->nDim,3,sizeof(int),(int(*)())CmpInt);

			/*
			* calcolo volume riga in cc (Centimetri Cubici)
			*/
			pRiga->nVolumeCopia   = (pRiga->nLength * pRiga->nWidth * pRiga->nHeight)/1000; 
			pRiga->nVolumeTotale  = (pRiga->nLength * pRiga->nWidth * pRiga->nHeight * pRiga->nCopieOrdinate)/1000; 
			pRiga->nPesoTotale    = pRiga->nPesoCopia * pRiga->nCopieOrdinate;
			pRiga->nCopieCaricate = 0;

			if(pRiga->nVolumeCopia==0){
				sprintf(szMsg,"ATTENZIONE : il prodotto [%s] ha volume NULLO\n",pRiga->szCodiceProdotto);
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Prodotto [%s] : volume NULLO",pRiga->szCodiceProdotto);
#endif
				SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
			}

			do {
				/*
				* Prelievo prodotti riga ed inserimento nel collo
				*/
				nRC=InsertRiga(&Ordine,&Collo,pRiga,bPallet);

				switch(nRC){
					case VOL_PRELIEVO_OK:
					{

#ifdef TRACE_ELIMINATO
						trace_debug(TRUE, TRUE, "PRELIEVO OK  : %2d %s %4d %4d",Collo.nCollo,pRiga->szCodiceProdotto,pRiga->nCopieCaricate,Collo.nProgressivoRiga);
#endif

						/*
						* scarico le informazioni della riga prelevata
						*/
						if(pRiga->nCopieCaricate){
							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								pRigaInsert->nCollo=Collo.nCollo;
								pRigaInsert->nProgressivoRiga=Collo.nProgressivoRiga;
								RigheCollo=g_list_append(RigheCollo,pRigaInsert);
								Collo.nProgressivoRiga++;
								Collo.nRighe++;
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}
					}
					break;
					case VOL_CHIUDI_COLLO:	
					{
						int nCopieRimaste;

#ifdef TRACE_ELIMINATO
						trace_debug(TRUE, TRUE, "CHIUDI COLLO : %2d %s %4d %4d",Collo.nCollo,pRiga->szCodiceProdotto,pRiga->nCopieCaricate,Collo.nProgressivoRiga);
#endif

						if(pRiga->nCopieCaricate){
							nCopieRimaste = pRiga->nCopieOrdinate-pRiga->nCopieCaricate;
							/* 
							* chiudo comunque il collo 
							* e in piu' creo una nuova riga di prelievo 
							* se necessario
							*/
							/*
							* cambio i dati di numero copie della riga attuale
							*/
							pRiga->nCopieOrdinate=pRiga->nCopieCaricate;
							pRiga->nPesoTotale=pRiga->nPesoCopia*pRiga->nCopieCaricate;

							if((pRigaInsert=(PEP_RIGA)malloc(sizeof(EP_RIGA)))){
								memcpy(pRigaInsert,pRiga,sizeof(EP_RIGA));
								pRigaInsert->nCollo=Collo.nCollo;
								pRigaInsert->nProgressivoRiga=Collo.nProgressivoRiga;
								RigheCollo=g_list_append(RigheCollo,pRigaInsert);
								Collo.nProgressivoRiga++;
								Collo.nRighe++;

								pRiga->nCopieOrdinate = nCopieRimaste;
								pRiga->nPesoTotale    = nCopieRimaste*pRiga->nPesoCopia;
								pRiga->nVolumeTotale  = nCopieRimaste*pRiga->nVolumeCopia;
								pRiga->nCopieCaricate=0;
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}
						}

						if(Collo.nCopie && Collo.nRighe){
							if(Collo.nIndiceImballo>=Imballi.nImballi){
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Indice imballo non corretto !!");
#endif
								bOK=FALSE;
							}

							strcpy(Collo.szCodiceImballo,Imballi.Imballo[Collo.nIndiceImballo].szCodice);
							Collo.nSwLinea=Imballi.Imballo[Collo.nIndiceImballo].nSwLinea;
							Collo.nPeso+=Imballi.Imballo[Collo.nIndiceImballo].nTara;

							if((pColloInsert=(PCOLLO)malloc(sizeof(COLLO)))){
								memcpy(pColloInsert,&Collo,sizeof(COLLO));
								Colli=g_list_append(Colli,pColloInsert);
							} else {
								sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
								SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
								trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
								bOK=FALSE;
							}

							/*
							* Aggiorno i dati dell'ordine in corso di analisi
							*/
							Ordine.nColli++;
							if(Collo.nSwLinea!=COLLO_MANUALE){
								Ordine.nColliAutomatici++;
							}
							Ordine.nRighe+=Collo.nRighe;
							Ordine.nCopie+=Collo.nCopie;
							Ordine.nVolumeUsato+=Collo.nVolume;
							Ordine.nPesoUsato+=Collo.nPeso;
#ifdef TRACE
							if(Imballi.Imballo[Collo.nIndiceImballo].nVolume){
								trace_out_vstr(1,"   %2d %-5.5s %7.2f %6d   %2d%%   %2d%%",
									Collo.nCollo,
									Collo.szCodiceImballo,
									(float)((float)Collo.nPeso/(float)1000),
									Collo.nVolume,
									(int)((float)(Collo.nVolume*100)/(float)Imballi.Imballo[Collo.nIndiceImballo].nVolume),
									Imballi.Imballo[Collo.nIndiceImballo].nVolPerc);
							} else {
								trace_debug(FALSE, TRUE, "   %2d %-5.5s %7.2f", Collo.nCollo, Collo.szCodiceImballo, (float)((float)Collo.nPeso/(float)1000));
							}
#endif

							/*
							* conteggio volume copie contenute nel collo
							*/
							CreazioneCollo(&Collo,nCollo++,bPallet);

						} else {
#ifdef TRACE
							trace_debug(TRUE, TRUE, "Ordine [%s] Collo [%d] VUOTO (R:%dC:%d)!",Ordine.szOrdProg,Collo.nCollo,Collo.nRighe,Collo.nCopie);
#endif
							/*
							* se il collo attuale e' vuoto
							* segnalo errore a video
							*/
							sprintf(szMsg,"ATTENZIONE : Ordine [%s] Collo [%d] VUOTO (R:%dC:%d)\n",Ordine.szOrdProg,Collo.nCollo,Collo.nRighe,Collo.nCopie);
							SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
						}
					}
					break;
					case VOL_COLLO_PCF_NON_PRESENTE:
						/*
						* ERRORE IRREVERSIBILE
						* segnalare all'operatore
						*/
						sprintf(szMsg,"Ordine [%s],Tipo Collo Preconfezionato [%s] non presente in archivio imballi",Ordine.szOrdProg,pRiga->szFCPCF);
						SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, szMsg);
						bOK=FALSE;
						bFine=TRUE;
					break;
					case VOL_COLLO_FF_NON_PRESENTE:
						/*
						* ERRORE IRREVERSIBILE
						* segnalare all'operatore
						*/
						sprintf(szMsg,"Ordine [%s],Tipo Collo Fuori Formato [%c] non presente in archivio imballi",Ordine.szOrdProg,COLLO_FUORI_FORMATO);
						SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, szMsg);
						bOK=FALSE;
						bFine=TRUE;
					break;
					case VOL_DIM_PROD_ERRATE:
						/*
						* ERRORE IRREVERSIBILE
						* segnalare all'operatore
						*/
						sprintf(szMsg,"Ordine [%s],Dimensioni Prodotto [%s] errate",Ordine.szOrdProg,pRiga->szCodiceProdotto);
						SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, szMsg);
						bFine=TRUE;
						bOK=FALSE;
					break;
					case VOL_CATEGORIA_NON_TROVATA:
						/*
						* ERRORE IRREVERSIBILE
						* segnalare all'operatore
						*/
						sprintf(szMsg,"Ordine [%s],Categoria imballi [%d] non presente\n",Ordine.szOrdProg,pRiga->nCategoriaFF);
						SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, szMsg);
						bFine=TRUE;
						bOK=FALSE;
					break;
				}
			} while(!bFine && pRiga->nCopieOrdinate>pRiga->nCopieCaricate);
		} while(!bFine && (list=g_list_next(list)));
	}

	if(bOK==FALSE){
		/*
		* Messaggio di errore in analisi ordine al MAIN
		*/
		SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, Ordine.szOrdProg);
		bErroreAnalisi=TRUE;


		g_list_free(Righe);
		g_list_free(Righe2);
		g_list_free(RigheCollo);
		g_list_free(Colli);

		return FALSE;
	}
	/*
	* chiusura ultimo collo 
	*/
	if(Collo.nCopie==0 || Collo.nRighe==0){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Ordine [%s] Collo [%d] VUOTO !",Ordine.szOrdProg,Collo.nCollo);
#endif
		/*
		* collo vuoto
		*/
		sprintf(szMsg,"ATTENZIONE : Ordine [%s] Collo [%d] VUOTO\n",Ordine.szOrdProg,Collo.nCollo);
		SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
	} else {
		/* 
		* rm 07-12-01 : il collo viene creato solo se pieno
		*/
		strcpy(Collo.szCodiceImballo,Imballi.Imballo[Collo.nIndiceImballo].szCodice);
		Collo.nSwLinea=Imballi.Imballo[Collo.nIndiceImballo].nSwLinea;

		/* 
		* Aggiungo la tara al peso calcolato del collo 
		*/
		Collo.nPeso+=Imballi.Imballo[Collo.nIndiceImballo].nTara;

		if((pColloInsert=(PCOLLO)malloc(sizeof(COLLO)))){
			memcpy(pColloInsert,&Collo,sizeof(COLLO));
			Colli=g_list_append(Colli,pColloInsert);
		} else {
			sprintf(szMsg,"ATTENZIONE : errore interno in analisi volumetrica\n");
			SendMessage(nProcIndex, PROC_VOLUM, DISPLAY_MSG, szMsg);
#ifdef TRACE
			trace_debug(FALSE, TRUE, "Errore in malloc !!");
#endif
			bOK=FALSE;
		}

		if(bOK==FALSE){
			/*
			* Messaggio di errore in analisi ordine al MAIN
			*/
			SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, Ordine.szOrdProg);
			bErroreAnalisi=TRUE;


			g_list_free(Righe);
			g_list_free(Righe2);
			g_list_free(RigheCollo);
			g_list_free(Colli);

			return FALSE;
		}

		/*
		* Aggiorno i dati dell'ordine in corso di analisi
		*/
		Ordine.nColli++;
		if(Collo.nSwLinea!=COLLO_MANUALE){
			Ordine.nColliAutomatici++;
		}
		Ordine.nRighe      += Collo.nRighe;
		Ordine.nCopie      += Collo.nCopie;
		Ordine.nPesoUsato  += Collo.nPeso;
		Ordine.nVolumeUsato+= Collo.nVolume;
#ifdef TRACE
		if(Imballi.Imballo[Collo.nIndiceImballo].nVolume){
			trace_out_vstr(1,"   %2d %-5.5s %7.2f %6d   %2d%%   %2d%%",
				Collo.nCollo,
				Collo.szCodiceImballo,
				(float)((float)Collo.nPeso/(float)1000),
				Collo.nVolume,
				(int)((float)(Collo.nVolume*100)/(float)Imballi.Imballo[Collo.nIndiceImballo].nVolume),
				Imballi.Imballo[Collo.nIndiceImballo].nVolPerc);
		} else {
			trace_debug(FALSE, TRUE, "   %2d %-5.5s %7.2f", Collo.nCollo, Collo.szCodiceImballo, (float)((float)Collo.nPeso/(float)1000));
		}
#endif
	}


#ifdef TRACE
	trace_debug(FALSE, TRUE, "----- ----- ------- ------ ----- ------");
	trace_debug(TRUE, TRUE, "Fine Analisi Ordine [%s]", Ordine.szOrdProg);
	trace_debug(FALSE, TRUE, "Numero Colli   : %7d",Ordine.nColli);
	trace_debug(FALSE, TRUE, "Numero Righe   : %7d",Ordine.nRighe);
	trace_debug(FALSE, TRUE, "Numero Copie   : %7d",Ordine.nCopie);
	trace_debug(FALSE, TRUE, "Peso Totale    : %7.2f",(float)((float)Ordine.nPesoTotale/(float)1000));
#endif

#ifdef TRACE_ELIMINATO
	trace_debug(FALSE, TRUE, "Lista Colli");
	if((list=g_list_first(Colli))){
		do {
			pCollo = list->data;

			if(pCollo==(PCOLLO)NULL){
				/*
				* Problemi di allocazione ?
				*/
				bOK=FALSE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Collo NULL !!");
#endif
				continue;
			}

			trace_debug(FALSE, TRUE, "%s %4d", pCollo->szOrdProg, pCollo->nCollo);
		} while((list=g_list_next(list)));
		trace_debug(FALSE, TRUE, "Numero Colli : %d",g_list_length(Colli));
	}
	trace_debug(FALSE, TRUE, "Lista Righe");
	if((list=g_list_first(RigheCollo))){
		do {
			pRiga = list->data;

			if(pRiga==(PEP_RIGA)NULL){
				/*
				* Problemi di allocazione ?
				*/
				bOK=FALSE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Riga NULL !!");
#endif
				continue;
			}

			trace_out_vstr(1,"%s %4d %s %s %4d %d",
				pRiga->szOrdProg,
				pRiga->nCollo,
				pRiga->szCodiceProdotto,
				pRiga->szUbicazione,
				pRiga->nCopieOrdinate,
				pRiga->nSwFF);
		} while((list=g_list_next(list)));
		trace_debug(FALSE, TRUE, "Numero Righe : %d",g_list_length(RigheCollo));
	}
#endif

	/*
	* Fine ordine
	*/
	if(!bErroreAnalisi){
		if(bTest){
			Ordine.cStato=ORDINE_RICEVUTO;
			bOK=UpdateOrdine(&Ordine);
		} else {
			Ordine.cStato=ORDINE_ELABORATO;
			bOK=UpdateOrdine(&Ordine);


			/*
			* scrittura colli
			*/
			if(bOK && (list=g_list_first(Colli))){
				do {
					pCollo = list->data;

					if(pCollo==(PCOLLO)NULL){
						/*
						* Problemi di allocazione ?
						*/
						bOK=FALSE;
	#ifdef TRACE
						trace_debug(TRUE, TRUE, "Collo NULL !!");
	#endif
						continue;
					}

					if(!InsertColProd(pCollo,COLLO_ELABORATO)){
						bOK=FALSE;
					}
				} while(bOK && (list=g_list_next(list)));
			}
			/*
			* scrittura righe
			*/
			if(bOK && (list=g_list_first(RigheCollo))){
				do {
					pRiga = list->data;

					if(pRiga==(PEP_RIGA)NULL){
						/*
						* Problemi di allocazione ?
						*/
						bOK=FALSE;
	#ifdef TRACE
						trace_debug(TRUE, TRUE, "Riga NULL !!");
	#endif
						continue;
					}

					if(!InsertRigProd(pRiga,RIGA_ELABORATA)){
						bOK=FALSE;
					}
				} while(bOK && (list=g_list_next(list)));
			}
		}
		if(bOK!=FALSE){
			SendMessage(nProcIndex, PROC_VOLUM, VOLUM_END_ANALISI_ORDINE, Ordine.szOrdProg);
		} else {
			SendMessage(nProcIndex, PROC_VOLUM, VOLUM_ERRORE_ANALISI_ORDINE, Ordine.szOrdProg);
			bErroreAnalisi=TRUE;
		}
	}
		
	g_list_free(Righe);
	g_list_free(Righe2);
	g_list_free(RigheCollo);
	g_list_free(Colli);

	return !bErroreAnalisi;
}

/*
* CreazioneCollo()
* apre un nuovo collo (il piu' piccolo)
* se bPallet=TRUE allora apre un collo di tipo PALLET
*/
ep_bool_t CreazioneCollo(PCOLLO pCollo,int nCollo,ep_bool_t bPallet)
{
	ep_bool_t bOK=TRUE;

	pCollo->nIndiceImballo=0;
	pCollo->nCategoria=-1;

	if(bPallet){
		while(pCollo->nIndiceImballo<Imballi.nImballi && Imballi.Imballo[pCollo->nIndiceImballo].szCodice[0]!=COLLO_PALLET){
			pCollo->nIndiceImballo++;	
		}
		if(pCollo->nIndiceImballo>=Imballi.nImballi){
			bOK=FALSE;
		}
	} else {
		strcpy(pCollo->szCodiceImballo,Imballi.Imballo[pCollo->nIndiceImballo].szCodice);
	}

	pCollo->nCollo=nCollo;
	pCollo->nVolume=0;
	pCollo->nPeso=0;
	pCollo->nCopie=0;
	pCollo->nRighe=0;
	pCollo->nProgressivoRiga=0;

	return bOK;
}

/*
* Creazione del Barcode collo + check digit
* calcolo del check digit
* secondo l'algoritmo 'Modulo 10 , Fattore 3'
* 10-((somma delle cifre pari)*3+(somma delle cifre dispari) modulo 10)
*/
char *ColloBarcode(PCOLLO pCollo)
{
	static char szBuffer[80];
	char szAnno[80];
	char szNumCollo[80];
	char szCheckDigit[80];
	int nCheckDigit;
	int nSommaPari;
	int nSommaDispari;
	int nTotale;
	int nIndex;
	int nAnno;

	nAnno=atoi(SubStr(GetDateYYYYMMDD(szDateBuffer),1,2));

	sprintf(szNumCollo,"%04d",pCollo->nCollo);
	sprintf(szAnno,"%02d",nAnno);
	// AAOOOOOOOCCCCD (Anno/Ordine/Collo/check Digit)
	strcpy(szBuffer,szAnno);
	strcat(szBuffer,SubStr(pCollo->szOrdProg, 2,7));
	strcat(szBuffer,szNumCollo);

	nSommaPari=0;
	nSommaDispari=0;
	for(nIndex=0;nIndex<strlen(szBuffer);nIndex++){
		if(nIndex%2){
			nSommaDispari+=(szBuffer[nIndex]-'0');
		} else {
			nSommaPari+=(szBuffer[nIndex]-'0');
		}
	}
	nTotale=nSommaPari*3+nSommaDispari;
	nCheckDigit=(10-(nTotale%10))%10;
	szCheckDigit[0]=nCheckDigit+'0';
	szCheckDigit[1]='\0';
	strcat(szBuffer,szCheckDigit);


	return (szBuffer);
}


/*
* InsertRiga 
*/
int InsertRiga(PORDINE pOrdine,PCOLLO pCollo,PEP_RIGA pRiga,ep_bool_t bPallet)
{
	int nRigaCopieCollo=0;
	ep_bool_t bFine;
	ep_bool_t bFound;
	int nVolumeUtile;
	int nPesoMax;
	int nInc;
	int nCat;     /* Categoria di FF */
	int nSwFF;    /* Switch di FF */
	int nFirst;   /* Indice primo imballo per categoria */
	int nLast;    /* Indice ultimo imballo per categoria */
	ep_bool_t bCambioImballo=TRUE;	/* gestione ultimi due colli */

	if(!bPallet){
		/* 
		* se il prodotto e' PCF
		*/
		if(pRiga->nSwPCF){
			if(pCollo->nCopie){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "PRODOTTO PCF e Copie=%d !!",pCollo->nCopie);
#endif
				return VOL_CHIUDI_COLLO;
			}
			/*
			* cerco l'imballo PCF
			*/
			pCollo->nIndiceImballo=0;
			while(pCollo->nIndiceImballo<Imballi.nImballi && Imballi.Imballo[pCollo->nIndiceImballo].szCodice[0]!=COLLO_PRECONFEZIONATO){
				pCollo->nIndiceImballo++;	
			}
			/*
			* se non trovo il tipo collo Fuori Formato
			* segnalo errore
			*/
			if(pCollo->nIndiceImballo == Imballi.nImballi){
				return VOL_COLLO_PCF_NON_PRESENTE;
			} else {
				/*
				* carico la riga nel collo e lo chiudo
				*/
				pRiga->nCopieCaricate=pRiga->nCopieOrdinate;

				pCollo->nVolume += pRiga->nVolumeCopia*pRiga->nCopieCaricate;
				pCollo->nPeso   += pRiga->nPesoCopia*pRiga->nCopieCaricate;
				pCollo->nCopie  += pRiga->nCopieCaricate;

#ifdef TRACE_ELIMINATO
				trace_debug(TRUE, TRUE, "PC-P:[%s]: %3d copie %4dx%4dx%4d = %6d - V.C: %8d V.U: %8d",
					pRiga->szCodiceProdotto,
					pRiga->nCopieCaricate,
					pRiga->nLength,
					pRiga->nWidth,
					pRiga->nHeight,
					(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
					pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif

				return VOL_CHIUDI_COLLO;
			}
		}
		/* 
		* se il prodotto e' FF e non ha categoria FF
		*/
		if(pRiga->nSwFF && pRiga->nCategoriaFF==0){
			if(pCollo->nCopie){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "PRODOTTO FF e Copie=%d !!",pCollo->nCopie);
#endif
				return VOL_CHIUDI_COLLO;
			}
			/*
			* cerco l'imballo FF
			*/
			pCollo->nIndiceImballo=0;
			while(pCollo->nIndiceImballo<Imballi.nImballi && Imballi.Imballo[pCollo->nIndiceImballo].szCodice[0]!=COLLO_FUORI_FORMATO){
				pCollo->nIndiceImballo++;	
			}
			/*
			* se non trovo il tipo collo Fuori Formato
			* segnalo errore
			*/
			if(pCollo->nIndiceImballo == Imballi.nImballi){
				return VOL_COLLO_FF_NON_PRESENTE;
			} else {
				/*
				* carico la riga nel collo e lo chiudo
				*/
				pRiga->nCopieCaricate=pRiga->nCopieOrdinate;

				pCollo->nVolume += pRiga->nVolumeCopia;
				pCollo->nPeso   += pRiga->nPesoCopia;
				pCollo->nCopie  += pRiga->nCopieOrdinate;

#ifdef TRACE_ELIMINATO
				trace_debug(TRUE, TRUE, "FF-P:[%s]: %3d copie %4dx%4dx%4d = %6d - V.C: %8d V.U: %8d",
					pRiga->szCodiceProdotto,
					pRiga->nCopieCaricate,
					pRiga->nLength,
					pRiga->nWidth,
					pRiga->nHeight,
					(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
					pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
				return VOL_CHIUDI_COLLO;
			}
		}
		/* 
		* riga di normale prelievo 
		* o categoria di fuori formato
		*/
		nCat=pRiga->nCategoriaFF;
		nSwFF=pRiga->nSwFF;
		nFirst=0;
		nLast=0;

		bFound=FALSE;
		while( nFirst<Imballi.nImballi){
			if(Imballi.Imballo[nFirst].nCategoria==nCat && Imballi.Imballo[nFirst].nSwLinea==nSwFF){
				bFound=TRUE;
				break;
			}
			nFirst++;
		}

		if(bFound==FALSE){
			return VOL_CATEGORIA_NON_TROVATA;
		}

		nLast=Imballi.nImballi-1;
		while(nLast){
			if(Imballi.Imballo[nLast].nCategoria==nCat && Imballi.Imballo[nLast].nSwLinea==nSwFF){
				bFound=TRUE;
				break;
			}
			nLast--;
		}

		if(bFound==FALSE){
			return VOL_CATEGORIA_NON_TROVATA;
		}
#ifdef TRACE_ELIMINATO
		trace_debug(FALSE, TRUE, "C:%d F:%d L:%d",nCat,nFirst,nLast);
#endif

		if(pCollo->nCategoria!=nCat){
			if(pCollo->nCopie){
#ifdef TRACE
				trace_debug(FALSE, TRUE, "--- Cambiata Categoria");
#endif
				return VOL_CHIUDI_COLLO;
			} else {
				pCollo->nCategoria=nCat;
				pCollo->nIndiceImballo=nFirst;
			}
		}


		/*
		* controllo dimensionale per colli da fare in automatico 
		* se non ci sta nel piu' grande torno subito errore
		*/
		if( Imballi.Imballo[nLast].nDim[0]<pRiga->nDim[0] || 
				Imballi.Imballo[nLast].nDim[1]<pRiga->nDim[1] || 
				Imballi.Imballo[nLast].nDim[2]<pRiga->nDim[2]){

			/*
			* l'articolo non ci sta nell'imballo piu' grande.
			* Se non ci sono imballi per contenere l'articolo (troppo grande)
			* ritorno : VOL_DIM_PROD_ERRATE;
			* (errore non reversibile : blocco della analisi volumetrica)
			*/
			return VOL_DIM_PROD_ERRATE;
		}
		/*
		* se l'articolo ci sta' : controllo il volume del collo
		*/
		/*
		* provo a farci stare una copia alla volta (dimensioni singolo prodotto / peso)
		* se il collo non e' il piu' grande cambio il tipo imballo
		*/
		nRigaCopieCollo=0;
		bFine=FALSE;
		while(!bFine && pCollo->nIndiceImballo<nLast+1){
			/*
			* se ho finito le copie mi fermo (ci stanno tutte)
			*/
			if(nRigaCopieCollo==pRiga->nCopieOrdinate){
				bFine=TRUE;
				continue;
			}
			/* 
			* controllo peso 
			*/
			if(pCollo->nPeso+pRiga->nPesoCopia*(nRigaCopieCollo+1) > Imballi.Imballo[pCollo->nIndiceImballo].nPesoMax){
				/* 
				* mi fermo perche' il peso e' gia troppo alto
				*/
				bFine=TRUE;
				continue;
			}
			/* 
			* controllo dimensionale 
			*/
			if( Imballi.Imballo[pCollo->nIndiceImballo].nDim[0]<pRiga->nDim[0] || 
					Imballi.Imballo[pCollo->nIndiceImballo].nDim[1]<pRiga->nDim[1] ||
					Imballi.Imballo[pCollo->nIndiceImballo].nDim[2]<pRiga->nDim[2] ||
					pCollo->nVolume+pRiga->nVolumeCopia*(nRigaCopieCollo+1) > Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile){

				if(bCambioImballo==FALSE || pCollo->nIndiceImballo==nLast){
					/* 
					* sono arrivato all'ultimo imballo
					*/
					bFine=TRUE;
					continue;
				} else {
					int nVolumeRimanente;
					int nPesoRimanente;
					/*
					* provo con l'imballo di dimensioni immediatamente superiori
					*/
					if(pCollo->nIndiceImballo+1==nLast){
						/*
						* e' il collo precedente al piu' grande
						*/
						nVolumeRimanente=pOrdine->nVolumeTotale-pOrdine->nVolumeUsato;
						nPesoRimanente=pOrdine->nPesoTotale-pOrdine->nPesoUsato;
#ifdef TRACE_ELIMINATO
						trace_debug(FALSE, TRUE, "Cambio Imballo : Ordine [%s] Collo %2d - Imballo %s - Volume Rim. : %6d Peso Rim. :%7.2f", pCollo->szOrdProg, pCollo->nCollo, pCollo->szCodiceImballo, nVolumeRimanente,(float)((float)nPesoRimanente/(float)1000));
#endif
						/*
						* rm 19-03-1999 - Gestione ultimi 2 colli
						* NON cambio imballo da tipo 2 a tipo 1 se il volume rimanente dell'ordine
						* e' maggiore del volume di un tipo 3 e minore del doppio del volume di un tipo 2
						* l'obiettivo e' quello di non avere l'ultimo collo piccolo poco pieno
						* (vale anche per il peso)
						* rm 01-09-2008 : cambio imballo se si tratta del primo collo dell'ordine
						*                 o se il collo ha righe/copie 0
						*/
						if(pCollo->nCollo>1 && pCollo->nRighe &&
							(nVolumeRimanente > Imballi.Imballo[nLast].nVolumeUtile && 
							 nVolumeRimanente < Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile*2)){
							bCambioImballo=FALSE;
#ifdef TRACE_ELIMINATO
							trace_debug(FALSE, FALSE, " - NON Cambio Imballo");
#endif
						}
					}

					if(bCambioImballo){
						pCollo->nIndiceImballo++;
						continue;
					}
				}
			}
			/* 
			* incremento il numero di copie solo se non ho cambiato l'imballo
			* e solo se ci sta (rm 03-03-2006)
			* altrimenti rifaccio il controllo peso/dimensioni
			*/
			if(pCollo->nVolume+pRiga->nVolumeCopia*(nRigaCopieCollo+1) <= Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile){
				nRigaCopieCollo++;
			} else {
				bFine=TRUE;
			}
		}
		if(nRigaCopieCollo==pRiga->nCopieOrdinate){
			/*
			* ci sta la riga completa, carico il collo e passo alla prossima
			*/
			pRiga->nCopieCaricate = pRiga->nCopieOrdinate;
			pCollo->nVolume += pRiga->nVolumeTotale;
			pCollo->nPeso   += pRiga->nPesoTotale;
			pCollo->nCopie  += pRiga->nCopieOrdinate;

#ifdef TRACE_ELIMINATO
			trace_debug(TRUE, TRUE, "P-P :[%s]: %3d copie %4dx%4dx%4d = %6d - V.C: %8d V.U: %8d",
				pRiga->szCodiceProdotto,
				pRiga->nCopieCaricate,
				pRiga->nLength,
				pRiga->nWidth,
				pRiga->nHeight,
				(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
				pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
			return VOL_PRELIEVO_OK;
		} else if(nRigaCopieCollo){
			/*
			* a questo punto chiudo il collo e ne apro un altro
			*/
			pRiga->nCopieCaricate=nRigaCopieCollo;
			pCollo->nVolume += (nRigaCopieCollo * pRiga->nVolumeCopia);
			pCollo->nPeso   += nRigaCopieCollo * pRiga->nPesoCopia;
			pCollo->nCopie  += nRigaCopieCollo;
#ifdef TRACE_ELIMINATO
		trace_debug(TRUE, TRUE, "S-P :[%s]: %3d copie %4dx%4dx%4d = %6d - V.C: %8d V.U: %8d",
			pRiga->szCodiceProdotto,
			pRiga->nCopieCaricate,
			pRiga->nLength,
			pRiga->nWidth,
			pRiga->nHeight,
			(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
			pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
			return VOL_CHIUDI_COLLO;
		} else {
			/*
			* non ci sta neanche una copia : chiudo il collo e apro il successivo
			*/
			return VOL_CHIUDI_COLLO;
		}
	} else {
		/*
		* Lavorazione PALLET
		*/
		/* 
		* se il prodotto e' PCF
		*/
		if(pRiga->nSwPCF){
			if(pCollo->nCopie){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "PRODOTTO PCP e Copie=%d !!",pCollo->nCopie);
#endif
				return VOL_CHIUDI_COLLO;
			}
			/*
			* cerco l'imballo PALLET
			*/
			pCollo->nIndiceImballo=0;
			while(pCollo->nIndiceImballo<Imballi.nImballi && Imballi.Imballo[pCollo->nIndiceImballo].szCodice[0]!=COLLO_PALLET){
				pCollo->nIndiceImballo++;	
			}
			/*
			* se non trovo il tipo collo Pallet
			* segnalo errore
			*/
			if(pCollo->nIndiceImballo == Imballi.nImballi){
				return VOL_COLLO_PCF_NON_PRESENTE;
			} else {
				/*
				* carico la riga nel collo e lo chiudo
				*/
				pRiga->nCopieCaricate=pRiga->nCopieOrdinate;

				pCollo->nVolume += pRiga->nVolumeCopia*pRiga->nCopieCaricate;
				pCollo->nPeso   += pRiga->nPesoCopia*pRiga->nCopieCaricate;
				pCollo->nCopie  += pRiga->nCopieCaricate;

#ifdef TRACE_ELIMINATO
				trace_debug(TRUE, TRUE, "PC-P:[%s]: %3d copie %4dx%4dx%4d = %6d - V.C: %8d V.U: %8d",
					pRiga->szCodiceProdotto,
					pRiga->nCopieCaricate,
					pRiga->nLength,
					pRiga->nWidth,
					pRiga->nHeight,
					(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
					pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif

				return VOL_CHIUDI_COLLO;
			}
		}
		/* gestione pallet */
		nRigaCopieCollo=0;
		bFine=FALSE;

		nVolumeUtile=Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile;
		nPesoMax=Imballi.Imballo[pCollo->nIndiceImballo].nPesoMax;
		/*
		* Controllo inserimento riga completa (Peso e Volume)
		*/
		if( pCollo->nPeso+pRiga->nPesoCopia*pRiga->nCopieOrdinate <= nPesoMax && pCollo->nVolume+pRiga->nVolumeCopia*pRiga->nCopieOrdinate <= nVolumeUtile){
			/*
			* ci sta la riga completa, carico il collo e passo alla prossima
			*/
			pRiga->nCopieCaricate = pRiga->nCopieOrdinate;
			pCollo->nVolume += pRiga->nVolumeTotale;
			pCollo->nPeso   += pRiga->nPesoTotale;
			pCollo->nCopie  += pRiga->nCopieOrdinate;

#ifdef TRACE
			trace_debug(TRUE, TRUE, "P-CN:[%s]: %5d copie %4dX%4dX%4d : %8d V.C: %8d V.U: %8d",
				pRiga->szCodiceProdotto,
				pRiga->nCopieCaricate,
				pRiga->nLength,
				pRiga->nWidth,
				pRiga->nHeight,
				(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
				pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
			return VOL_PRELIEVO_OK;
		} else {
			/*
			* Riprovo con volumeutile sommato a sogliapallet
			*/
			nVolumeUtile = Imballi.Imballo[pCollo->nIndiceImballo].nVolume * (float)((float)(Imballi.Imballo[pCollo->nIndiceImballo].nVolPerc+Cfg.nSogliaPallet)/(float)100);
			nPesoMax = Imballi.Imballo[pCollo->nIndiceImballo].nPesoMax + (Imballi.Imballo[pCollo->nIndiceImballo].nPesoMax *(float)((float)Cfg.nSogliaPallet/(float)100));
			if( pCollo->nPeso+pRiga->nPesoCopia*pRiga->nCopieOrdinate <= nPesoMax && pCollo->nVolume+pRiga->nVolumeCopia*pRiga->nCopieOrdinate <= nVolumeUtile){
				/*
				* ci sta la riga completa, carico il collo e passo alla prossima
				*/
				pRiga->nCopieCaricate = pRiga->nCopieOrdinate;
				pCollo->nVolume += pRiga->nVolumeTotale;
				pCollo->nPeso   += pRiga->nPesoTotale;
				pCollo->nCopie  += pRiga->nCopieCaricate;

#ifdef TRACE
				trace_debug(TRUE, TRUE, "P-CS:[%s]: %5d copie %4dX%4dX%4d : %8d V.C: %8d V.U: %8d",
					pRiga->szCodiceProdotto,
					pRiga->nCopieCaricate,
					pRiga->nLength,
					pRiga->nWidth,
					pRiga->nHeight,
					(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
					pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
				return VOL_PRELIEVO_OK;
			}
			/*
			* sono ancora qui ...
			* significa che la riga non ci sta neanche aumentando il volume utile.
			* provo a vedere se senza la riga il bancale e' gia' abbastanza pieno.
			*/
			nVolumeUtile = Imballi.Imballo[pCollo->nIndiceImballo].nVolume * (float)((float)(Imballi.Imballo[pCollo->nIndiceImballo].nVolPerc-Cfg.nSogliaPallet)/(float)100);
			if( pCollo->nVolume > nVolumeUtile){
#ifdef TRACE_ELIMINATO
				trace_debug(TRUE, TRUE, "P-VS:[%s]: %5d copie %4dX%4dX%4d : %8d V.C: %8d V.U: %8d",
					pRiga->szCodiceProdotto,
					pRiga->nCopieCaricate,
					pRiga->nLength,
					pRiga->nWidth,
					pRiga->nHeight,
					(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
					pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
				return VOL_CHIUDI_COLLO;
			}
		}
		/*
		* sono ancora qui ... mannaggia!
		* allora spezzo la riga 
		*/
		nInc=Cfg.nIncrementoCopiePallet;
		while(!bFine){
			/*
			* se ho finito le copie mi fermo (ci stanno tutte)
			*/
			if(nRigaCopieCollo==pRiga->nCopieOrdinate){
				bFine=TRUE;
				continue;
			}
			/* 
			* controllo peso 
			*/
			if(pCollo->nPeso+pRiga->nPesoCopia*(nRigaCopieCollo+nInc) > Imballi.Imballo[pCollo->nIndiceImballo].nPesoMax){
				/* 
				* mi fermo perche' il peso e' gia troppo alto
				*/
				if(nRigaCopieCollo==0 && nInc>=10){
					nInc/=10;
				} else {
					bFine=TRUE;
				}
				continue;
			}
			if( pCollo->nVolume+pRiga->nVolumeCopia*(nRigaCopieCollo+nInc) > Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile){

				/*
				* Se non ci sta neanche una copia  provo a dividere per 10 l'incremento
				* se invece ho gia' spezzato la riga allora finisco il riempimento
				*/
				if(nRigaCopieCollo==0 && nInc>=10){
					nInc/=10;
				} else {
					bFine=TRUE;
				}
				continue;
			}
			nRigaCopieCollo+=nInc;
		}
		/* fine del riempimento pallet */
		if(nRigaCopieCollo==pRiga->nCopieOrdinate){
			/*
			* ci sta la riga completa, carico il collo e passo alla prossima
			*/
			pRiga->nCopieCaricate = pRiga->nCopieOrdinate;
			pCollo->nVolume += pRiga->nVolumeTotale;
			pCollo->nPeso   += pRiga->nPesoTotale;
			pCollo->nCopie  += nRigaCopieCollo;

#ifdef TRACE_ELIMINATO
			trace_debug(TRUE, TRUE, "P-CN:[%s]: %5d copie %4dX%4dX%4d : %8d V.C: %8d V.U: %8d",
				pRiga->szCodiceProdotto,
				pRiga->nCopieCaricate,
				pRiga->nLength,
				pRiga->nWidth,
				pRiga->nHeight,
				(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
				pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
			return VOL_PRELIEVO_OK;
		} else if(nRigaCopieCollo){
			/*
			* a questo punto chiudo il collo e ne apro un altro
			*/
			pRiga->nCopieCaricate=nRigaCopieCollo;
			pCollo->nVolume += (nRigaCopieCollo * pRiga->nVolumeCopia);
			pCollo->nPeso   += nRigaCopieCollo * pRiga->nPesoCopia;
			pCollo->nCopie  += nRigaCopieCollo;
			pCollo->nRighe++;
#ifdef TRACE_ELIMINATO
			trace_debug(TRUE, TRUE, "P-SN:[%s]: %5d copie %4dX%4dX%4d : %8d V.C: %8d V.U: %8d",
				pRiga->szCodiceProdotto,
				pRiga->nCopieCaricate,
				pRiga->nLength,
				pRiga->nWidth,
				pRiga->nHeight,
				(pRiga->nLength* pRiga->nWidth* pRiga->nHeight * pRiga->nCopieCaricate)/1000,
				pCollo->nVolume,Imballi.Imballo[pCollo->nIndiceImballo].nVolumeUtile);
#endif
			return VOL_CHIUDI_COLLO;
		} else {
			return VOL_CHIUDI_COLLO;
		}
	}
}

