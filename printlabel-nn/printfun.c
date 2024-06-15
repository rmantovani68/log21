/*
* printfun.c : Stampa Picking List : funzioni di utilita'
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli
*
* Copyright AeL srl 1998-2002
*
* 02-01-2002 rm+st : ATTENZIONE : i prezzi in euro sono interi (in centesimi)
*                    usare la funzione NotazioneConVirgola() per la normalizzazione
* 05-02-2002 rm    : riscrittura (da CodeBase a PostgreSQL)
*
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
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc(int s)
{
#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
#endif
    DBDisconnect();

	/*
	* avverto MAIN dello stop
	*/
	SendMessage(Cfg.nMainID,nPID, PROGRAM_STOPPED, NULL);

	exit(0);
} /* Fine SafeIntFunc */

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(void)
{
	char szParagraph[128];
	char szCurrentDirectory[128];
	char szItem[128];
	char szValue[128];
	char szCfgFileName[256];
	int nIndex;


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

	GetFileString(szParagraph,"TipoOrdini",    "N",            Cfg.szTipoOrdini,    80,szCfgFileName,NULL);

	GetFileString(szParagraph,"PathData",      "../data",      Cfg.szPathData,      80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExe",       "./",           Cfg.szPathExe,       80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathStampe",    "../stampe",    Cfg.szPathStampe,    80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExport",    "../export",    Cfg.szPathExport,    80,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathTrace",     "../trace",     Cfg.szPathTrace,     80, szCfgFileName, NULL);

	GetFileString(szParagraph,"LabelBaseName", "labelbase.cfg",Cfg.szLabelBaseName,    80,szCfgFileName,NULL);

	Cfg.nDebugVersion      = GetFileInt(szParagraph, "DebugVersion",     0, szCfgFileName,NULL);
	Cfg.nShmKey            = GetFileInt(szParagraph, "ShmKey",         256, szCfgFileName,NULL);
	Cfg.nA2ps              = GetFileInt(szParagraph, "a2ps",             0, szCfgFileName,NULL);


	/*
	* lettura del file di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	GetFileString(szParagraph,"PGHost",    "localhost",Cfg.szPGHost,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"PGPort",    "5432",     Cfg.szPGPort,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"PGDataBase","picking",  Cfg.szPGDataBase, 80,szCfgFileName,NULL); 


	ReadProcInfo(szCfgFileName);


	/*
	* parametri specifici dell'applicazione
	*/

	strcpy(szParagraph,ProcessGetName(nPID));
	/*
	* Delay in millisecondi (lo trasformo in microsecondi)
	*/
	Cfg.nDelay=1000*GetFileInt(szParagraph,"Delay",100,szCfgFileName,NULL);

	/*
	* Estraggo i parametri relativi al formato delle stampe
	*/

	strcpy(Cfg.szPrintCfg,Cfg.szPathData);
	strcat(Cfg.szPrintCfg,"/printlabel.cfg");

	Cfg.nTipoOrdinamentoStampaXAB      = GetFileInt(szParagraph,"TipoOrdinamentoStampaXAB",      1, Cfg.szPrintCfg, NULL);
	Cfg.nLinesPerForm                  = GetFileInt(szParagraph,"LinesPerForm",                 23, Cfg.szPrintCfg, NULL);
	Cfg.nPrintableRow                  = GetFileInt(szParagraph,"PrintableRow",                  9, Cfg.szPrintCfg, NULL);
	Cfg.nLengthTitle                   = GetFileInt(szParagraph,"LengthTitle",                  23, Cfg.szPrintCfg, NULL);
	Cfg.nLengthAuthor                  = GetFileInt(szParagraph,"LengthAuthor",                 12, Cfg.szPrintCfg, NULL);
	Cfg.nRigheStampanteParallela       = GetFileInt(szParagraph,"RigheStampanteParallela",      72, Cfg.szPrintCfg, NULL);
	Cfg.nColonneStampanteParallela     = GetFileInt(szParagraph,"ColonneStampanteParallela",    80, Cfg.szPrintCfg, NULL);
	Cfg.nRigheStampanteParallela_A4    = GetFileInt(szParagraph,"RigheStampanteParallela_A4",   72, Cfg.szPrintCfg, NULL);
	Cfg.nColonneStampanteParallela_A4  = GetFileInt(szParagraph,"ColonneStampanteParallela_A4", 80, Cfg.szPrintCfg, NULL);
	Cfg.nLibriPagina                   = GetFileInt(szParagraph,"LibriPagina",                  35, Cfg.szPrintCfg, NULL);
	Cfg.nAnnoXAB                       = GetFileInt(szParagraph,"Anno",                          0, Cfg.szPrintCfg, NULL);
	Cfg.nProgressivoXAB                = GetFileInt(szParagraph,"ProgrXAB",                      0, Cfg.szPrintCfg, NULL);
	Cfg.nPrimaRigaXAB                  = GetFileInt(szParagraph,"PrimaRigaXAB",                 21, Cfg.szPrintCfg, NULL);
	Cfg.nUltimaRigaXAB                 = GetFileInt(szParagraph,"UltimaRigaXAB",                55, Cfg.szPrintCfg, NULL);
	Cfg.nPrimaRigaXAB_A4               = GetFileInt(szParagraph,"PrimaRigaXAB_A4",              21, Cfg.szPrintCfg, NULL);
	Cfg.nUltimaRigaXAB_A4              = GetFileInt(szParagraph,"UltimaRigaXAB_A4",             55, Cfg.szPrintCfg, NULL);

	Cfg.nPrimaRigaPL_A4               = GetFileInt(szParagraph,"PrimaRigaPL_A4",                21, Cfg.szPrintCfg, NULL);
	Cfg.nUltimaRigaPL_A4              = GetFileInt(szParagraph,"UltimaRigaPL_A4",               55, Cfg.szPrintCfg, NULL);
	Cfg.nPrimaRigaDC                  = GetFileInt(szParagraph,"PrimaRigaDC",                   21, Cfg.szPrintCfg, NULL);
	Cfg.nUltimaRigaDC                 = GetFileInt(szParagraph,"UltimaRigaDC",                  55, Cfg.szPrintCfg, NULL);

	Cfg.nProgressivoDistinta           = GetFileInt(szParagraph,"ProgrDistinta",                 0, Cfg.szPrintCfg, NULL);
	Cfg.nPrimaRigaDistinta             = GetFileInt(szParagraph,"PrimaRigaDistinta",            19, Cfg.szPrintCfg, NULL);
	Cfg.nUltimaRigaDistinta            = GetFileInt(szParagraph,"UltimaRigaDistinta",           60, Cfg.szPrintCfg, NULL);

	GetFileString(szParagraph, "Fine",                   "-----FINE-----",               Cfg.szFine,                 40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "Righe",                  "RIGHE",                        Cfg.szRighe,                40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "Volumi",                 "VOLUMI",                       Cfg.szVolumi,               40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "CodMovimento",           "T.M.",                         Cfg.szCodMovimento,         40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "TipoNotaRAC",            "002",                          Cfg.szTipoNotaRAC,          40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "FineOrdineRighe",        "RIGHE",                        Cfg.szFineOrdineRighe,      40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "FineOrdineVolumi",       "VOLUMI",                       Cfg.szFineOrdineVolumi,     40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "FineBolla",              "TOTALE NR.",                   Cfg.szFineBolla,            40, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "Contenuto",              "LIBRI",                        Cfg.szContenuto,            80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "TipoNotaXAB",            "0XB",                          Cfg.szTipoNotaXAB,          80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "Segue",                  "<SEGUE>",                      Cfg.szSegue,                80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "SegueBAM",               "SEGUE B.A.M.",                 Cfg.szSegueBAM,             80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "SeguePL",                "SEGUE PACKING LIST",           Cfg.szSeguePL,              80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "Foglio",                 "FOGLIO",                       Cfg.szFoglio,               80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "VecchiaPaginaDistinta",  "",                             Cfg.szVecchiaPaginaDistinta,80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "NuovaPaginaDistinta",    "",                             Cfg.szNuovaPaginaDistinta,  80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "NomeMagazzino",          "MAGAZZINO CENTRALE MONDADORI", Cfg.szNomeMagazzino,        80, Cfg.szPrintCfg, NULL);
	GetFileString(szParagraph, "DataDocumento",          "DATA DOCUMENTO",               Cfg.szDataDocumento,        80, Cfg.szPrintCfg, NULL);


	/*
	* Caricamento delle righe su cui stampare i prodotti della RAC
	*/
	for (nIndex=0; nIndex<Cfg.nPrintableRow; nIndex++) {
		sprintf(szItem,"PrintingRow_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d",&nPrintingRow[nIndex]);
	}

	/*
	* Caricamento delle posizioni dei campi per la stampa della RAC
	*/
	for (nIndex=0; nIndex<NUM_FIELD_RAC; nIndex++) {
		sprintf(szItem,"Pos_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d,%d", &(Pos[nIndex].nSR), &(Pos[nIndex].nSC));
	}

	/*
	* Caricamento delle posizioni dei campi per la stampa della XAB
	*/
	for (nIndex=0; nIndex<NUM_FIELD_XAB; nIndex++){
		sprintf(szItem,"XABPos_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d,%d", &(XABPos[nIndex].nSR), &(XABPos[nIndex].nSC));
	}

	/*
	* Caricamento delle posizioni dei campi per la stampa della XAB su A4
	*/
	for (nIndex=0; nIndex<NUM_FIELD_XAB; nIndex++){
		sprintf(szItem,"XABPos_A4_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d,%d", &(XABPos_A4[nIndex].nSR), &(XABPos_A4[nIndex].nSC));
	}

	/*
	* Caricamento delle posizioni dei campi per la stampa della Packing List su A4
	*/
	for (nIndex=0; nIndex<NUM_FIELD_XAB; nIndex++){
		sprintf(szItem,"PLPos_A4_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d,%d", &(PLPos_A4[nIndex].nSR), &(PLPos_A4[nIndex].nSC));
	}

	/*
	* Caricamento delle posizioni dei campi per la stampa della Packing List su A4
	*/
	for (nIndex=0; nIndex<NUM_FIELD_XAB; nIndex++){
		sprintf(szItem,"DCPos_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d,%d", &(DCPos[nIndex].nSR), &(DCPos[nIndex].nSC));
	}

	/*
	* Caricamento delle posizioni dei campi per la stampa della distinta
	*/
	for (nIndex=0; nIndex<NUM_FIELD_DIS; nIndex++){
		sprintf(szItem,"DisPos_%02d",nIndex);
		GetFileString(szParagraph,szItem,"",szValue,10,Cfg.szPrintCfg,NULL);
		sscanf(szValue,"%d,%d", &(DisPos[nIndex].nSR), &(DisPos[nIndex].nSC));
	}
}

BOOL GetDatiOrdine(char *szOrdProg,PDATIORDINE pDatiOrdine,BOOL bStorico)
{
	DBresult *PGRes=NULL;
	BOOL bOK=TRUE;
	int nIndex=0;
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

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select \
		ORDPROG, ROCDRID, ROCDSPC, ROCDARE, ROCDVEN,\
		RODSCLI, ROCOCLI, ROINCLI, ROLOCLI, ROPRCLI,\
		ROCPCLI, RONZCLI, ROCDCLA, ROCDSPA, RODSCLA,\
		ROINCLA, ROCOCLA, ROLOCLA, ROPRCLA, ROCPCLA,\
		RONZCLA, ROSWSPF, RONMBAM, RODTBAM, ROTPTAR,\
		ROSWCOL, ROVLCON, ROTPSPE, ROCDLIN, ROCDSCA,\
		ROCDSSC, ROCDSOT, ROCDMOV, ROTPDOC, RODTEVA,\
		ROSWFIN, ROCDFPG, ROTPBUT, ROORINS, ROCDVAL,\
		ROVLCOE, RONMCED, ROCDVE1, ROCDVE2, ROCDVE3,\
		ROIDVET, RONMRGH, RONMRGP, RONMCPE, RONMCPP,\
		RONMCLL, RONMCLA, ROCDBTC, ROPSPRE, ROPSREA,\
		ROTMRCZ, ROTMINI, ROTMEVA, ROAAXAB, ROPRXAB,\
		RODTXAB, RONMDIS, RODTDIS, ROPRGLN, ROCDFLG,\
		ROSTATO, ORDTIPO from %s where ordprog='%s';",szRicOrd,szOrdProg);
	if(DBntuples(PGRes)){
		/* chiavi utilizzate per la ricerca nelle tabelle */
		strcpy(pDatiOrdine->szROCDSOC,SubStr(szOrdProg, 0,2));
		strcpy(pDatiOrdine->szROCDMAG,SubStr(szOrdProg, 2,7));
		strcpy(pDatiOrdine->szROAADOC,SubStr(szOrdProg, 9,2));
		strcpy(pDatiOrdine->szROPRDOC,SubStr(szOrdProg,11,6));

		strcpy(pDatiOrdine->szORDPROG,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Ordine */
		strcpy(pDatiOrdine->szROCDRID,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice Cliente */
		strcpy(pDatiOrdine->szROCDSPC,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Sede periferica commerciale */
		strcpy(pDatiOrdine->szROCDARE,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice area */
		strcpy(pDatiOrdine->szROCDVEN,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice venditore */
		strcpy(pDatiOrdine->szRODSCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Anagrafica punto vendita */
		strcpy(pDatiOrdine->szROCOCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Presso punto vendita */
		strcpy(pDatiOrdine->szROINCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Indirizzo punto vendita */
		strcpy(pDatiOrdine->szROLOCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Localita' punto vendita */
		strcpy(pDatiOrdine->szROPRCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Provincia punto vendita */
		strcpy(pDatiOrdine->szROCPCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* CAP punto vendita */
		strcpy(pDatiOrdine->szRONZCLI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Nazione punto vendita */
		strcpy(pDatiOrdine->szROCDCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice del cliente amministrativo (Destinatario) */
		strcpy(pDatiOrdine->szROCDSPA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Sede periferica amministrativa */
		strcpy(pDatiOrdine->szRODSCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Anagrafica cliente amministrativo */
		strcpy(pDatiOrdine->szROINCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Indirizzo cliente amministrativo */
		strcpy(pDatiOrdine->szROCOCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Presso cliente amministrativo */
		strcpy(pDatiOrdine->szROLOCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Localita' cliente amministrativo */
		strcpy(pDatiOrdine->szROPRCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Provincia cliente amministrativo */
		strcpy(pDatiOrdine->szROCPCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* CAP cliente amministrativo */
		strcpy(pDatiOrdine->szRONZCLA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Nazione cliente amministrativo */
		strcpy(pDatiOrdine->szROSWSPF,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Switch di spedizione fiscale */
		strcpy(pDatiOrdine->szRONMBAM,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Numero interno BAM */
		strcpy(pDatiOrdine->szRODTBAM,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data della BAM */
		strcpy(pDatiOrdine->szROTPTAR,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Tipo tariffa */
		strcpy(pDatiOrdine->szROSWCOL,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Switch tipo imballo (Colli:1 Pancali:4)  */
		pDatiOrdine->nROVLCON=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Valore del contrassegno in lire */
		strcpy(pDatiOrdine->szROTPSPE,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Tipo spedizione se forzati */
		strcpy(pDatiOrdine->szROCDLIN,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Linea se forzato */
		strcpy(pDatiOrdine->szROCDSCA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Scalo se forzato */
		strcpy(pDatiOrdine->szROCDSSC,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Sottoscalo se forzato */
		strcpy(pDatiOrdine->szROCDSOT,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice sottofascia (per posta) se forzato */
		strcpy(pDatiOrdine->szROCDMOV,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice del movimento */
		strcpy(pDatiOrdine->szROTPDOC,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Tipo del documento */
		strcpy(pDatiOrdine->szRODTEVA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data evasione (AAAAMMGG) */
		strcpy(pDatiOrdine->szROSWFIN,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Switch di instradamento forzato */
		strcpy(pDatiOrdine->szROCDFPG,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice forma di pagamento */
		strcpy(pDatiOrdine->szROTPBUT,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Tipo buttata (R)ifornimento/(C)edola) */
		strcpy(pDatiOrdine->szROORINS,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Ora Inserimento  (HH:MM:SS) */
		strcpy(pDatiOrdine->szROCDVAL,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice valuta della spedizione ('LIT','EUR') */
		pDatiOrdine->nROVLCOE=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Valore del contrassegno in Euro (in centesimi di EURO) */
		strcpy(pDatiOrdine->szRONMCED,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Numero Cedola */
		strcpy(pDatiOrdine->szROCDVE1,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice del primo vettore */
		strcpy(pDatiOrdine->szROCDVE2,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice del secondo vettore */
		strcpy(pDatiOrdine->szROCDVE3,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice del terzo vettore */
		strcpy(pDatiOrdine->szROIDVET,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Identificatore Vettore per sovrappacco */
		pDatiOrdine->nRONMRGH=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero Righe Richieste */
		pDatiOrdine->nRONMRGP=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero Righe di Produzione */
		pDatiOrdine->nRONMCPE=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero Copie Richieste */
		pDatiOrdine->nRONMCPP=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero Copie di Produzione */
		pDatiOrdine->nRONMCLL=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero Colli */
		pDatiOrdine->nRONMCLA=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero Colli di Linea Automatica */
		strcpy(pDatiOrdine->szROCDBTC,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Codice BTC */
		pDatiOrdine->nROPSPRE=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Peso Calcolato       (grammi) */
		pDatiOrdine->nROPSREA=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Peso Reale           (grammi) */
		strcpy(pDatiOrdine->szROTMRCZ,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data Ricezione       (datetime) */
		strcpy(pDatiOrdine->szROTMINI,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data Inizio Prelievo (datetime) */
		strcpy(pDatiOrdine->szROTMEVA,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data Evasione        (datetime) */
		pDatiOrdine->nROAAXAB=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Anno della XAB  */
		pDatiOrdine->nROPRXAB=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero della XAB */
		strcpy(pDatiOrdine->szRODTXAB,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data della XAB      (YYYYMMDD) */
		pDatiOrdine->nRONMDIS=atoi(DBgetvalue(PGRes,0,nIndex));   nIndex++;      /* Numero della distinta */
		strcpy(pDatiOrdine->szRODTDIS,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Data della distinta (YYYYMMDD) */
		strcpy(pDatiOrdine->szROPRGLN,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Progressivo Lancio */
		strcpy(pDatiOrdine->szROCDFLG,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Flag Ordine */
		strcpy(pDatiOrdine->szROSTATO,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Stato Ordine */
		strcpy(pDatiOrdine->szORDTIPO,DBgetvalue(PGRes,0,nIndex));   nIndex++;   /* Tipo Ordine */
	} else {
		bOK=FALSE;
	}
	DBclear(PGRes);

	return bOK;
}


BOOL GetDatiCollo (char *szOrdProg,int nCollo,PDATICOLLO pDatiCollo,BOOL bStorico)
{
	DBresult *PGRes=NULL;
	BOOL bOK=TRUE;
	int nIndex=0;
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

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select \
		ORDPROG, CPNMCOL, CPNMPPT, CPPSPRE,\
		CPPSREA, CPVLAFF, CPTPFOR, CPSWFPS,\
		CPSWCHM, CPVLAFE, CPSWLIN, CPNMRGH,\
		CPNMCPE, CPBRCDE, CPTMEVA, CPCDFLG,\
		CPSTATO from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdProg,nCollo);
	if(DBntuples(PGRes)){

		strcpy(pDatiCollo->szORDPROG,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Progressivo */
		pDatiCollo->nCPNMCOL=atoi(DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Numero del collo */
		strcpy(pDatiCollo->szCPNMPPT,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Numero pacco postale */
		pDatiCollo->nCPPSPRE=atoi(DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Peso Calcolato */
		pDatiCollo->nCPPSREA=atoi(DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Peso Reale */
		strcpy(pDatiCollo->szCPVLAFF,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Valore dell'affrancatura postale in LIRE */
		strcpy(pDatiCollo->szCPTPFOR,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Tipo formato */
		strcpy(pDatiCollo->szCPSWFPS,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Switch di forzatura peso */
		strcpy(pDatiCollo->szCPSWCHM,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Switch di chiusura manuale del collo */
		strcpy(pDatiCollo->szCPVLAFE,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Valore dell'affrancatura postale in EURO (centesimi) */
		strcpy(pDatiCollo->szCPSWLIN,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Switch Lavorazione Automatica/Manuale */
		pDatiCollo->nCPNMRGH=atoi(DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Totale Righe */
		pDatiCollo->nCPNMCPE=atoi(DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Totale Copie */
		strcpy(pDatiCollo->szCPBRCDE,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Codice a barre del collo (2+6+5+1) */
		strcpy(pDatiCollo->szCPTMEVA,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Data Evasione (datetime) */
		strcpy(pDatiCollo->szCPCDFLG,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Flag Collo */
		strcpy(pDatiCollo->szCPSTATO,DBgetvalue(PGRes,0,nIndex)); nIndex++;    /* Stato Collo */

	} else {
		bOK=FALSE;
	}
	DBclear(PGRes);
	
	return bOK;
}


BOOL StampaPackingListBolla(char *szOrdProg, char *szPrinterName)
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
	sprintf(szFileName,"%s/PL_%s",Cfg.szPathStampe,szOrdProg);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		if(WriteTestataOrdine(fp,szOrdProg,bStorico)){

			PGResColli=DBExecQuery(Cfg.nDebugVersion>2,"select cpnmcol from %s where ordprog='%s';",szColProd,szOrdProg);
			if(DBresultStatus(PGResColli)==DBRES_TUPLES_OK){
				if((nTuples=DBntuples(PGResColli))){
					for(nIndex=0;nIndex<nTuples;nIndex++){
						StampaPackingListCollo(szOrdProg,nIndex+1, NULL,fp,bStorico);
					}
				} else {
					if(bStorico){
						GetFileString("stampe","ordine", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
					} else {
						GetFileString("stampe","ordine_storico", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
					}
					nRC=PrintListFromSelect(fp,szSelectCmd, szOrdProg,0);
				}
				DBclear(PGResColli);
			}
		}
		/*
		* Salto pagina
		*/
		fprintf(fp,"");

		fclose(fp);
		if(nRC){
			sprintf(szTitle,"AeL - Stampa Packing List Bolla");
			PrintFile(szFileName,szPrinterName,szTitle,Cfg.nA2ps,0);
			unlink(szFileName);

			bRetVal=TRUE;
		}
	}
	return bRetVal;
}


BOOL StampaPackingListCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,BOOL bStorico)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC;
	BOOL bRetVal=FALSE;

	if(szPrinterName==NULL){
		fp=fp_file;
	} else {
		/*
		* Apertura del file per la stampa
		*/
		sprintf(szFileName,"%s/PLC_%s_%03d",Cfg.szPathStampe,szOrdProg,nCollo);
		fp=fopen(szFileName,"w");
		WriteTestataOrdine(fp,szOrdProg,bStorico);
	}
	if(fp){
		if(WriteTestataCollo(fp,szOrdProg,nCollo,bStorico)){

			if(bStorico){
				GetFileString("stampe","collo_storico", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
			} else {
				GetFileString("stampe","collo", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
			}

			nRC=PrintListFromSelect(fp,szSelectCmd, szOrdProg,nCollo);

			if(szPrinterName!=NULL){
				/*
				* Salto pagina
				*/
				fprintf(fp,"");

				fclose(fp);
#ifdef TRACE
				trace_out_vstr_date(1, "StampaPackingListCollo (%s - %d - %s)",szOrdProg,nCollo,szPrinterName);
#endif

				if(nRC){
					sprintf(szTitle,"AeL - Stampa Packing List Collo");
					PrintFile(szFileName,szPrinterName,szTitle,Cfg.nA2ps,0);
					unlink(szFileName);

					bRetVal=TRUE;
				}
			}
		}
	}
	return bRetVal;
}

BOOL WriteTestataOrdine(FILE *fp,char *szOrdprog,BOOL bStorico)
{
	DBresult *PGRes;
	BOOL bRetVal=FALSE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];

	if(bStorico){
		strcpy(szRicOrd,"ric_ord_stor");
		strcpy(szOrdProd,"ord_prod_stor");
		strcpy(szColProd,"col_prod_stor");
		strcpy(szRigProd,"rig_prod_stor");
	} else {
		strcpy(szRicOrd,"ric_ord");
		strcpy(szOrdProd,"ord_prod");
		strcpy(szColProd,"col_prod");
		strcpy(szRigProd,"rig_prod");
	}

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select rodscli,roincli,rocpcli,rolocli,roprcli,ronmcll,ronmrgh,ronmcpe,ropspre from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(PGRes)==DBRES_TUPLES_OK && (DBntuples(PGRes)==1));

	if (bRetVal){
		fprintf(fp,"Cliente : %s\n",DBgetvalue(PGRes,0,0));
		fprintf(fp,"          %s\n",DBgetvalue(PGRes,0,1));
		fprintf(fp,"          %s - %s %s\n",DBgetvalue(PGRes,0,2),DBgetvalue(PGRes,0,3),DBgetvalue(PGRes,0,4));
		fprintf(fp,"%s  Colli : %d  Righe : %3d  Copie : %4d\n",szOrdprog, atoi(DBgetvalue(PGRes,0,5)), atoi(DBgetvalue(PGRes,0,6)), atoi(DBgetvalue(PGRes,0,7)));
	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "WriteTestataOrdine(%s) -> fallita select",szOrdprog);
#endif
	}
	DBclear(PGRes);

	return bRetVal;
}

BOOL WriteTestataCollo(FILE *fp,char *szOrdprog, int nCollo,BOOL bStorico)
{
	DBresult *PGRes;
	DBresult *PGResCollo;
	char szStato[40];
	BOOL bRetVal=FALSE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];

	if(bStorico){
		strcpy(szRicOrd,"ric_ord_stor");
		strcpy(szOrdProd,"ord_prod_stor");
		strcpy(szColProd,"col_prod_stor");
		strcpy(szRigProd,"rig_prod_stor");
	} else {
		strcpy(szRicOrd,"ric_ord");
		strcpy(szOrdProd,"ord_prod");
		strcpy(szColProd,"col_prod");
		strcpy(szRigProd,"rig_prod");
	}

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select rodscli,roincli,rocpcli,rolocli,roprcli from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(PGRes)==DBRES_TUPLES_OK && (DBntuples(PGRes)==1));

	if (bRetVal){
		fprintf(fp,"\nPacking List Collo  : %s - %d\n",szOrdprog,nCollo);
		PGResCollo=DBExecQuery(Cfg.nDebugVersion>2,"select cpnmrgh,cpnmcpe,cppspre,cppsrea,data(cptmeva),ora(cptmeva) from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdprog,nCollo);
		if(DBresultStatus(PGResCollo)==DBRES_TUPLES_OK){
			fprintf(fp,"Righe : %3d  Copie  : %4d\n",atoi(DBgetvalue(PGResCollo,0,0)),atoi(DBgetvalue(PGResCollo,0,1)));
			fprintf(fp,"Peso Predeterminato : %8.3fKg\n",(float)((float)atoi(DBgetvalue(PGResCollo,0,2))/(float)1000));
			fprintf(fp,"Peso Reale          : %8.3fKg\n",(float)((float)atoi(DBgetvalue(PGResCollo,0,3))/(float)1000));
			fprintf(fp,"Evasione            : %s %s\n",DBgetvalue(PGResCollo,0,4),DBgetvalue(PGResCollo,0,5));
		}
		DBclear(PGResCollo);

		PGResCollo=DBExecQuery(Cfg.nDebugVersion>2,"select bnnmbnc from storico_bancali b, storico_colli_bancale c where b.key_bancale=b.key_bancale and c.ordprog='%s' and c.cpnmcol=%d;",szOrdprog,nCollo);
		if(DBresultStatus(PGResCollo)==DBRES_TUPLES_OK){
			fprintf(fp,"Bancale : %d \n",atoi(DBgetvalue(PGResCollo,0,0)));
		}
		DBclear(PGResCollo);

		/*
		* controllo l'eventualita' che il collo sia transitato in bilancia e scartato (FUORI TOLLERANZA opp FUORI LINEA) 
		*/
		PGResCollo=DBExecQuery(Cfg.nDebugVersion>2,"select cpstato from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdprog,nCollo);
		if(DBresultStatus(PGResCollo)==DBRES_TUPLES_OK && DBntuples(PGResCollo)==1){
			DBresult *PGResRighe;
			int nIndexRighe;
			strcpy(szStato,DBgetvalue(PGResCollo,0,0));

			if(!bStorico){
				if(szStato[0]==COLLO_FUORI_LINEA){
					/* controllo se il collo e' in bilancia (terminato) ed ha copie da prelevare fuori linea */
					PGResRighe=DBExecQuery(Cfg.nDebugVersion>2,"select ubicazione,rpcdpro,rpqtspe from ubicazioni, rig_prod where ordprog='%s' and rpnmcol=%d and ubcdflg='%c' and ubicazione=rpcdubi;",szOrdprog,nCollo,UBICAZIONE_MANUALE);
					if(DBresultStatus(PGResRighe)==DBRES_TUPLES_OK && DBntuples(PGResRighe)>0){
						fprintf(fp,"\n\n     -- COLLO DA PRELEVARE FUORI LINEA --\n\n");
						for(nIndexRighe=0;nIndexRighe<DBntuples(PGResRighe);nIndexRighe++){
							fprintf(fp,"     -- Prodotto %s - Ubic %s - Copie %d\n",
							DBgetvalue(PGResRighe,nIndexRighe,1),
							DBgetvalue(PGResRighe,nIndexRighe,0),
							atoi(DBgetvalue(PGResRighe,nIndexRighe,2)));
						}
						fprintf(fp,"\n     ------------------------------------\n\n");
					}
					DBclear(PGResRighe);
				} else if(szStato[0]==COLLO_SCARTATO){
					fprintf(fp,"\n\n     -- COLLO SCARTATO PERCHE' FUORI TOLLERANZA PESO --\n\n");
				}
			}
		}
		DBclear(PGResCollo);

	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "WriteTestataCollo(%s - %d) -> fallita select",szOrdprog,nCollo);
#endif
	}

	DBclear(PGRes);

	return bRetVal;
}

BOOL StampaConteggioImballi(char *szDataDal, char *szDataAl, char *szCedola, char *szPrinterName)
{
	char szSelectCmdImb[2048];
	char szSelectCmdPcf[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC=TRUE;
	int nRC2=TRUE;
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
	char szCedolaKey[128];

	
	if (atoi(szCedola)!=0){
		sprintf(szCedolaKey," and ronmced='%s' ",szCedola);
	}else{
		sprintf(szCedolaKey," ");
	}
	
	sprintf(szSelectCmdImb,"select cptpfor as \"FORMATO\",count(cptpfor) as \"N. COLLI\" from ric_ord_stor o, col_prod_stor c where o.ordprog=c.ordprog and date(cptmeva)>=date('%s') and date(cptmeva)<=date('%s') %s group by cptpfor order by cptpfor;",szDataDal,szDataAl,szCedolaKey);
	sprintf(szSelectCmdPcf,"select prfcpcf as \"FORMATO\",count(prfcpcf) as \"N. COLLI\" from ric_ord_stor o, col_prod_stor c, rig_prod_stor r, catalogo p where o.ordprog=c.ordprog and cptpfor='C' and   date(cptmeva)>=date('%s') and date(cptmeva)<=date('%s') and c.ordprog=r.ordprog and c.cpnmcol=r.rpnmcol and r.rpcdpro=p.prcdpro %s group by prfcpcf order by prfcpcf;",szDataDal,szDataAl,szCedolaKey);
	
	/*
	* Apertura del file per la stampa
	*/
	sprintf(szFileName,"%s/CI",Cfg.szPathStampe);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
	
		fprintf(fp,"\nPeriodo dal %s al %s\n\n",szDataDal,szDataAl );
		fprintf(fp,"Consumo Imballi\n");
		nRC=PrintListFromSelect(fp,szSelectCmdImb,NULL,0);
		fprintf(fp,"\nDettaglio Consumo Imballi Preconfezionati\n");
		nRC2=PrintListFromSelect(fp,szSelectCmdPcf,NULL,0);
		/*
		* Salto pagina
		*/
		fprintf(fp,"");

		fclose(fp);
		if(nRC){
			sprintf(szTitle,"AeL - Stampa Consumo Imballi");
			PrintFile(szFileName,szPrinterName,szTitle,Cfg.nA2ps,0);
			unlink(szFileName);

			bRetVal=TRUE;
		}
	}
	return bRetVal;
}

