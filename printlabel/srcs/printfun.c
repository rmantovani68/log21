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
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc(int s)
{
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stopped");
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
	int nIndex;


	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(Cfg.szCniCfg,szCurrentDirectory);
	strcat(Cfg.szCniCfg, __configuration_file__);

	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	GetFileString(szParagraph,"TipoOrdini",    "N",            Cfg.szTipoOrdini,    80,Cfg.szCniCfg,NULL);

	GetFileString(szParagraph,"PathData",      "../data",      Cfg.szPathData,      80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathExe",       "./exe",        Cfg.szPathExe,       80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathTrace",     "./trace",      Cfg.szPathTrace,     80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathStampe",    "../stampe",    Cfg.szPathStampe,    80,Cfg.szCniCfg,NULL);

	GetFileString(szParagraph,"LabelBaseName", "labelbase.cfg",Cfg.szLabelBaseName,    80,Cfg.szCniCfg,NULL);

	Cfg.nDebugLevel      = GetFileInt(szParagraph, "DebugLevel",     0, Cfg.szCniCfg,NULL);
	Cfg.nShmKey            = GetFileInt(szParagraph, "ShmKey",         256, Cfg.szCniCfg,NULL);
	Cfg.nA2ps              = GetFileInt(szParagraph, "a2ps",             0, Cfg.szCniCfg,NULL);
	Cfg.bAbilitaSDA        = GetFileInt(szParagraph, "AbilitaSDA",       0, Cfg.szCniCfg,NULL);


	/*
	* lettura del file di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	GetFileString(szParagraph,"DBHost",     "localhost", Cfg.szDBHost,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"DBPort",     "5432",      Cfg.szDBPort,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"DBName",     "momo",      Cfg.szDBName,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"DBUser",     "user",      Cfg.szDBUser,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"DBPassword", "pwd",       Cfg.szDBPassword, 80,Cfg.szCniCfg,NULL); 

	ReadProcInfo(Cfg.szCniCfg);


	/*
	* parametri specifici dell'applicazione
	*/

	strcpy(szParagraph,ProcessGetName(nPID));
	/*
	* Delay in millisecondi (lo trasformo in microsecondi)
	*/
	Cfg.nDelay=1000*GetFileInt(szParagraph,"Delay",100,Cfg.szCniCfg,NULL);

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


ep_bool_t GetDatiOrdine(char *szOrdProg,PDATIORDINE pDatiOrdine,ep_bool_t bStorico)
{
	DBresult *DBRes=NULL;
	ep_bool_t bOK=TRUE;
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

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select \
		ORDPROG, ROCDRID, ROCDSPC, ROCDARE, ROCDVEN,\
		RODSCLI, ROCOCLI, ROINCLI, ROLOCLI, ROPRCLI,\
		ROCPCLI, RONZCLI, ROCDCLA, ROCDSPA, RODSCLA,\
		ROINCLA, ROCOCLA, ROLOCLA, ROPRCLA, ROCPCLA,\
		RONZCLA, ROSWSPF, RONMBAM, RODTBAM, ROTPTAR,\
		ROSWCOL, ROVLCON, ROTPSPE, ROCDLIN, ROCDSCA,\
		ROCDSSC, ROCDSOT, ROCDMOV, ROTPDOC, RODTEVA,\
		ROSWFIN, ROCDFDB, ROTPBUT, ROORINS, ROCDVAL,\
		ROVLCOE, RONMCED, ROCDVE1, ROCDVE2, ROCDVE3,\
		ROIDVET, RONMRGH, RONMRGP, RONMCPE, RONMCPP,\
		RONMCLL, RONMCLA, ROCDBTC, ROPSPRE, ROPSREA,\
		ROTMRCZ, ROTMINI, ROTMEVA, ROAAXAB, ROPRXAB,\
		RODTXAB, RONMDIS, RODTDIS, ROPRGLN, ROCDFLG,\
		ROSTATO, ORDTIPO from %s where ordprog='%s';",szRicOrd,szOrdProg);
	if(DBntuples(DBRes)){
		/* chiavi utilizzate per la ricerca nelle tabelle */
		strcpy(pDatiOrdine->szROCDSOC,SubStr(szOrdProg, 0,2));
		strcpy(pDatiOrdine->szROCDMAG,SubStr(szOrdProg, 2,7));
		strcpy(pDatiOrdine->szROAADOC,SubStr(szOrdProg, 9,2));
		strcpy(pDatiOrdine->szROPRDOC,SubStr(szOrdProg,11,6));

		strcpy(pDatiOrdine->szORDPROG,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Ordine */
		strcpy(pDatiOrdine->szROCDRID,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice Cliente */
		strcpy(pDatiOrdine->szROCDSPC,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Sede periferica commerciale */
		strcpy(pDatiOrdine->szROCDARE,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice area */
		strcpy(pDatiOrdine->szROCDVEN,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice venditore */
		strcpy(pDatiOrdine->szRODSCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Anagrafica punto vendita */
		strcpy(pDatiOrdine->szROCOCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Presso punto vendita */
		strcpy(pDatiOrdine->szROINCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Indirizzo punto vendita */
		strcpy(pDatiOrdine->szROLOCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Localita' punto vendita */
		strcpy(pDatiOrdine->szROPRCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Provincia punto vendita */
		strcpy(pDatiOrdine->szROCPCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* CAP punto vendita */
		strcpy(pDatiOrdine->szRONZCLI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Nazione punto vendita */
		strcpy(pDatiOrdine->szROCDCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice del cliente amministrativo (Destinatario) */
		strcpy(pDatiOrdine->szROCDSPA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Sede periferica amministrativa */
		strcpy(pDatiOrdine->szRODSCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Anagrafica cliente amministrativo */
		strcpy(pDatiOrdine->szROINCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Indirizzo cliente amministrativo */
		strcpy(pDatiOrdine->szROCOCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Presso cliente amministrativo */
		strcpy(pDatiOrdine->szROLOCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Localita' cliente amministrativo */
		strcpy(pDatiOrdine->szROPRCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Provincia cliente amministrativo */
		strcpy(pDatiOrdine->szROCPCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* CAP cliente amministrativo */
		strcpy(pDatiOrdine->szRONZCLA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Nazione cliente amministrativo */
		strcpy(pDatiOrdine->szROSWSPF,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Switch di spedizione fiscale */
		strcpy(pDatiOrdine->szRONMBAM,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Numero interno BAM */
		strcpy(pDatiOrdine->szRODTBAM,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data della BAM */
		strcpy(pDatiOrdine->szROTPTAR,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Tipo tariffa */
		strcpy(pDatiOrdine->szROSWCOL,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Switch tipo imballo (Colli:1 Pancali:4)  */
		pDatiOrdine->nROVLCON=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Valore del contrassegno in lire */
		strcpy(pDatiOrdine->szROTPSPE,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Tipo spedizione se forzati */
		strcpy(pDatiOrdine->szROCDLIN,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Linea se forzato */
		strcpy(pDatiOrdine->szROCDSCA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Scalo se forzato */
		strcpy(pDatiOrdine->szROCDSSC,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Sottoscalo se forzato */
		strcpy(pDatiOrdine->szROCDSOT,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice sottofascia (per posta) se forzato */
		strcpy(pDatiOrdine->szROCDMOV,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice del movimento */
		strcpy(pDatiOrdine->szROTPDOC,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Tipo del documento */
		strcpy(pDatiOrdine->szRODTEVA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data evasione (AAAAMMGG) */
		strcpy(pDatiOrdine->szROSWFIN,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Switch di instradamento forzato */
		strcpy(pDatiOrdine->szROCDFDB,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice forma di pagamento */
		strcpy(pDatiOrdine->szROTPBUT,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Tipo buttata (R)ifornimento/(C)edola) */
		strcpy(pDatiOrdine->szROORINS,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Ora Inserimento  (HH:MM:SS) */
		strcpy(pDatiOrdine->szROCDVAL,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice valuta della spedizione ('LIT','EUR') */
		pDatiOrdine->nROVLCOE=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Valore del contrassegno in Euro (in centesimi di EURO) */
		strcpy(pDatiOrdine->szRONMCED,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Numero Cedola */
		strcpy(pDatiOrdine->szROCDVE1,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice del primo vettore */
		strcpy(pDatiOrdine->szROCDVE2,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice del secondo vettore */
		strcpy(pDatiOrdine->szROCDVE3,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice del terzo vettore */
		strcpy(pDatiOrdine->szROIDVET,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Identificatore Vettore per sovrappacco */
		pDatiOrdine->nRONMRGH=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero Righe Richieste */
		pDatiOrdine->nRONMRGP=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero Righe di Produzione */
		pDatiOrdine->nRONMCPE=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero Copie Richieste */
		pDatiOrdine->nRONMCPP=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero Copie di Produzione */
		pDatiOrdine->nRONMCLL=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero Colli */
		pDatiOrdine->nRONMCLA=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero Colli di Linea Automatica */
		strcpy(pDatiOrdine->szROCDBTC,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Codice BTC */
		pDatiOrdine->nROPSPRE=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Peso Calcolato       (grammi) */
		pDatiOrdine->nROPSREA=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Peso Reale           (grammi) */
		strcpy(pDatiOrdine->szROTMRCZ,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data Ricezione       (datetime) */
		strcpy(pDatiOrdine->szROTMINI,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data Inizio Prelievo (datetime) */
		strcpy(pDatiOrdine->szROTMEVA,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data Evasione        (datetime) */
		pDatiOrdine->nROAAXAB=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Anno della XAB  */
		pDatiOrdine->nROPRXAB=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero della XAB */
		strcpy(pDatiOrdine->szRODTXAB,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data della XAB      (YYYYMMDD) */
		pDatiOrdine->nRONMDIS=atoi(DBgetvalue(DBRes,0,nIndex));   nIndex++;      /* Numero della distinta */
		strcpy(pDatiOrdine->szRODTDIS,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Data della distinta (YYYYMMDD) */
		strcpy(pDatiOrdine->szROPRGLN,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Progressivo Lancio */
		strcpy(pDatiOrdine->szROCDFLG,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Flag Ordine */
		strcpy(pDatiOrdine->szROSTATO,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Stato Ordine */
		strcpy(pDatiOrdine->szORDTIPO,DBgetvalue(DBRes,0,nIndex));   nIndex++;   /* Tipo Ordine */
	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);

	return bOK;
}


ep_bool_t GetDatiCollo (char *szOrdProg,int nCollo,PDATICOLLO pDatiCollo,ep_bool_t bStorico)
{
	DBresult *DBRes=NULL;
	ep_bool_t bOK=TRUE;
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

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select \
		ORDPROG, CPNMCOL, CPNMPPT, CPPSPRE,\
		CPPSREA, CPVLAFF, CPTPFOR, CPSWFPS,\
		CPSWCHM, CPVLAFE, CPSWLIN, CPNMRGH,\
		CPNMCPE, CPBRCDE, CPTMEVA, CPCDFLG,\
		CPSTATO from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdProg,nCollo);
	if(DBntuples(DBRes)){

		strcpy(pDatiCollo->szORDPROG,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Progressivo */
		pDatiCollo->nCPNMCOL=atoi(DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Numero del collo */
		strcpy(pDatiCollo->szCPNMPPT,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Numero pacco postale */
		pDatiCollo->nCPPSPRE=atoi(DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Peso Calcolato */
		pDatiCollo->nCPPSREA=atoi(DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Peso Reale */
		strcpy(pDatiCollo->szCPVLAFF,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Valore dell'affrancatura postale in LIRE */
		strcpy(pDatiCollo->szCPTPFOR,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Tipo formato */
		strcpy(pDatiCollo->szCPSWFPS,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Switch di forzatura peso */
		strcpy(pDatiCollo->szCPSWCHM,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Switch di chiusura manuale del collo */
		strcpy(pDatiCollo->szCPVLAFE,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Valore dell'affrancatura postale in EURO (centesimi) */
		strcpy(pDatiCollo->szCPSWLIN,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Switch Lavorazione Automatica/Manuale */
		pDatiCollo->nCPNMRGH=atoi(DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Totale Righe */
		pDatiCollo->nCPNMCPE=atoi(DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Totale Copie */
		strcpy(pDatiCollo->szCPBRCDE,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Codice a barre del collo (2+6+5+1) */
		strcpy(pDatiCollo->szCPTMEVA,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Data Evasione (datetime) */
		strcpy(pDatiCollo->szCPCDFLG,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Flag Collo */
		strcpy(pDatiCollo->szCPSTATO,DBgetvalue(DBRes,0,nIndex)); nIndex++;    /* Stato Collo */

	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);
	
	return bOK;
}


ep_bool_t StampaPackingListBolla(char *szOrdProg, char *szPrinterName)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC=TRUE;
	int nIndex;
	int nTuples;
	ep_bool_t bRetVal=FALSE;
	DBresult *DBResColli;
	DBresult *DBRes;
	ep_bool_t bStorico=FALSE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];

	strcpy(szRicOrd,"ric_ord");
	strcpy(szOrdProd,"ord_prod");
	strcpy(szColProd,"col_prod");
	strcpy(szRigProd,"rig_prod");

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord where ordprog='%s';",szOrdProg);
	if(DBntuples(DBRes)==0){
		DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord_stor where ordprog='%s';",szOrdProg);
		if(DBntuples(DBRes)){
			bStorico=TRUE;
			strcpy(szRicOrd,"ric_ord_stor");
			strcpy(szOrdProd,"ord_prod_stor");
			strcpy(szColProd,"col_prod_stor");
			strcpy(szRigProd,"rig_prod_stor");
		}
	}
	DBclear(DBRes);

	/*
	* Apertura del file per la stampa
	*/
	sprintf(szFileName,"%s/PL_%s",Cfg.szPathStampe,szOrdProg);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		if(WriteTestataOrdine(fp,szOrdProg,bStorico)){

			DBResColli=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmcol from %s where ordprog='%s';",szColProd,szOrdProg);
			if(DBresultStatus(DBResColli)==DBRES_TUPLES_OK){
				if((nTuples=DBntuples(DBResColli))){
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
				DBclear(DBResColli);
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


ep_bool_t StampaPackingListCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,ep_bool_t bStorico)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC;
	ep_bool_t bRetVal=FALSE;

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
				trace_debug(TRUE, TRUE, "StampaPackingListCollo (%s - %d - %s)",szOrdProg,nCollo,szPrinterName);
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

ep_bool_t WriteTestataOrdine(FILE *fp,char *szOrdprog,ep_bool_t bStorico)
{
	DBresult *DBRes;
	ep_bool_t bRetVal=FALSE;
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

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rodscli,roincli,rocpcli,rolocli,roprcli,ronmcll,ronmrgh,ronmcpe,ropspre from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(DBRes)==DBRES_TUPLES_OK && (DBntuples(DBRes)==1));

	if (bRetVal){
		fprintf(fp,"Cliente : %s\n",DBgetvalue(DBRes,0,0));
		fprintf(fp,"          %s\n",DBgetvalue(DBRes,0,1));
		fprintf(fp,"          %s - %s %s\n",DBgetvalue(DBRes,0,2),DBgetvalue(DBRes,0,3),DBgetvalue(DBRes,0,4));
		fprintf(fp,"%s  Colli : %d  Righe : %3d  Copie : %4d\n",szOrdprog, atoi(DBgetvalue(DBRes,0,5)), atoi(DBgetvalue(DBRes,0,6)), atoi(DBgetvalue(DBRes,0,7)));
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "WriteTestataOrdine(%s) -> fallita select",szOrdprog);
#endif
	}
	DBclear(DBRes);

	return bRetVal;
}

ep_bool_t WriteTestataCollo(FILE *fp,char *szOrdprog, int nCollo,ep_bool_t bStorico)
{
	DBresult *DBRes;
	DBresult *DBResCollo;
	char szStato[40];
	ep_bool_t bRetVal=FALSE;
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

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rodscli,roincli,rocpcli,rolocli,roprcli from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(DBRes)==DBRES_TUPLES_OK && (DBntuples(DBRes)==1));

	if (bRetVal){
		fprintf(fp,"\nPacking List Collo  : %s - %d\n",szOrdprog,nCollo);
		DBResCollo=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmrgh,cpnmcpe,cppspre,cppsrea,data(cptmeva),ora(cptmeva) from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdprog,nCollo);
		if(DBresultStatus(DBResCollo)==DBRES_TUPLES_OK){
			fprintf(fp,"Righe : %3d  Copie  : %4d\n",atoi(DBgetvalue(DBResCollo,0,0)),atoi(DBgetvalue(DBResCollo,0,1)));
			fprintf(fp,"Peso Predeterminato : %8.3fKg\n",(float)((float)atoi(DBgetvalue(DBResCollo,0,2))/(float)1000));
			fprintf(fp,"Peso Reale          : %8.3fKg\n",(float)((float)atoi(DBgetvalue(DBResCollo,0,3))/(float)1000));
			fprintf(fp,"Evasione            : %s %s\n",DBgetvalue(DBResCollo,0,4),DBgetvalue(DBResCollo,0,5));
		}
		DBclear(DBResCollo);

		/*
		* controllo l'eventualita' che il collo sia transitato in bilancia e scartato (FUORI TOLLERANZA opp FUORI LINEA) 
		*/
		DBResCollo=DBExecQuery(Cfg.nDebugLevel>2,"select cpstato from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdprog,nCollo);
		if(DBresultStatus(DBResCollo)==DBRES_TUPLES_OK && DBntuples(DBResCollo)==1){
			DBresult *DBResRighe;
			int nIndexRighe;
			strcpy(szStato,DBgetvalue(DBResCollo,0,0));

			if(!bStorico){
				if(szStato[0]==COLLO_FUORI_LINEA){
					/* controllo se il collo e' in bilancia (terminato) ed ha copie da prelevare fuori linea */
					DBResRighe=DBExecQuery(Cfg.nDebugLevel>2,"select ubicazione,rpcdpro,rpqtspe from ubicazioni, rig_prod where ordprog='%s' and rpnmcol=%d and ubcdflg='%c' and ubicazione=rpcdubi;",szOrdprog,nCollo,UBICAZIONE_MANUALE);
					if(DBresultStatus(DBResRighe)==DBRES_TUPLES_OK && DBntuples(DBResRighe)>0){
						fprintf(fp,"\n\n     -- COLLO DA PRELEVARE FUORI LINEA --\n\n");
						for(nIndexRighe=0;nIndexRighe<DBntuples(DBResRighe);nIndexRighe++){
							fprintf(fp,"     -- Prodotto %s - Ubic %s - Copie %d\n",
							DBgetvalue(DBResRighe,nIndexRighe,1),
							DBgetvalue(DBResRighe,nIndexRighe,0),
							atoi(DBgetvalue(DBResRighe,nIndexRighe,2)));
						}
						fprintf(fp,"\n     ------------------------------------\n\n");
					}
					DBclear(DBResRighe);
				} else if(szStato[0]==COLLO_SCARTATO){
					fprintf(fp,"\n\n     -- COLLO SCARTATO PERCHE' FUORI TOLLERANZA PESO --\n\n");
				}
			}
		}
		DBclear(DBResCollo);

	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "WriteTestataCollo(%s - %d) -> fallita select",szOrdprog,nCollo);
#endif
	}

	DBclear(DBRes);

	return bRetVal;
}

/*
* restituisce il tipo di barcode da utilizzare per stampante Zebra Z4000
*/
char *TipoVettoreBarcode(DBresult *DBRes,int nIndex)
{
	int nROTPBCD;
	static char szTipoBarcode[80];
	ep_bool_t bOK=TRUE;

	nROTPBCD=84;

	if(bOK){
		if(strlen(StrTrimAll(DBgetvalue(DBRes,nIndex,nROTPBCD)))){
			switch(atoi(DBgetvalue(DBRes,nIndex,nROTPBCD))){
				case 1:  /* Code 39           */ strcpy(szTipoBarcode,"3"); break;
				case 2:  /* Interleaved 2/5   */ strcpy(szTipoBarcode,"2"); break;
				case 3:  /* Code 128          */ strcpy(szTipoBarcode,"C"); break;
				default: /* default (I 2/5)   */ strcpy(szTipoBarcode,"2"); break;
			}
		} else {
			strcpy(szTipoBarcode,"3");
		}
	} else {
		szTipoBarcode[0]='\0';
	}

	return szTipoBarcode;
}

/*
* rm 23-03-2018
* -------------
* restituisce numero progressivo multicollo dato ordine e numero collo 
* ritorna 
*         = 0 - se collo normale 
*         > 0 - se multicollo
*         < 0 - se errore
*/
int ProgressivoMultiCollo(char *szOrdProg,int nCollo)
{
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
	int nProgressivoMultiCollo=-1;
	int nCPPRGCL = 0;
	int nFirstPRGCL = 0;
	char szCPTPFOR[80];
	char szRPCDPRO[80];
	int nPRQTFFO = 0;
	int nColliMC=0;

	/*  stabilisco se si tratta di un prodotto multicollo  (qtffo < 0) */
	DBRes=DBExecQuery(Cfg.nDebugLevel>2," \
	select \
		cp.cpprgcl, \
		cp.cptpfor, \
		ca.prcdpro, \
		ca.prqtffo \
	from \
		catalogo ca,ric_ord ro,col_prod cp,rig_prod rp \
	where \
		cp.ordprog=ro.ordprog and \
		rp.ordprog=cp.ordprog and \
		rp.rpnmcol=cp.cpnmcol and \
		rp.ordprog='%s' and cpnmcol=%d;",szOrdProg,nCollo);
	if(DBntuples(DBRes)){
		nCPPRGCL = atoi(DBgetvalue(DBRes,0,0));      /* Progressivo collo */
		strcpy(szCPTPFOR , DBgetvalue(DBRes,0,1));   /* formato (deve essere FF) */
		strcpy(szRPCDPRO , DBgetvalue(DBRes,0,2));   /* Codice prodotto  */
		nPRQTFFO = atoi(DBgetvalue(DBRes,0,3));      /* Copie FF (deve essere < 0) */
	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);

	if(bOK==FALSE){
		return -1;
	}

	if(nPRQTFFO < 0 && szCPTPFOR[0]=='F'){
		/* 
		* si tratta di un multicollo 
		* ricavo la lista dei colli dell'ordine specificato associati a quel codice prodotto  
		* tale lista dovrà essere multiplo delle copie FF impostate (valore assoluto)
		* e uguale a (copieFF * copieSpedite)
		*/
		nPRQTFFO=-1*nPRQTFFO;

		DBRes=DBExecQuery(Cfg.nDebugLevel>2," \
		select \
			cp.cpprgcl, \
			rp.rpqtspe \
		from \
			catalogo ca,ric_ord ro,col_prod cp,rig_prod rp \
		where \
			cp.ordprog=ro.ordprog and \
			rp.ordprog=cp.ordprog and \
			rp.rpnmcol=cp.cpnmcol and \
			rp.ordprog='%s' and rpcdpro='%s' order by cpprgcl;",szOrdProg,szRPCDPRO);

		/* numero colli MC per ordine e prodotto */
		if((nColliMC = DBntuples(DBRes))){
			nFirstPRGCL=atoi(DBgetvalue(DBRes,0,0));   /* progressivo primo collo */

			/* 
			* il progressivo multi collo è il resto intero della divisione tra la differenza 
			* rispetto al primo multicollo per codice e il numero di colli per prodotto
			*/
			nProgressivoMultiCollo = ((nCPPRGCL-nFirstPRGCL)%nPRQTFFO)+1;
#ifdef TRACE
			trace_debug(TRUE, TRUE, "ProgressivoMultiCollo [%s-%d-'%s'] = %d",szOrdProg,nCollo,szRPCDPRO,nProgressivoMultiCollo);
#endif

		}

		DBclear(DBRes);

	}

	return nProgressivoMultiCollo;
}

/*
* interpreta i dati richiesti secondo documentazione
* e genera il barcode vettore
*/
char *VettoreBarcode(DBresult *DBRes,int nIndex)
{
	ep_bool_t bOK=TRUE;
	int nORDPROG;
	int nRODTBCD;
	int nROPRBOL;
	int nROFLPAR;
	int nROFLARR;
	int nCPNMCOL;
	int nCPPRGCL;
	char *pPtr;
	char *pData;
	char szBuffer[256];
	char szFieldData[256];
	char szFieldValue[256];
	static char szBarcodeVettore[256];
	int nLen;

	szBarcodeVettore[0]='\0';

	nORDPROG=0;     /* pqfnumber() */
	nRODTBCD=83;    /* pqfnumber() */
	nROPRBOL=90;    /* pqfnumber() */
	nROFLPAR=80;    /* pqfnumber() */
	nROFLARR=81;    /* pqfnumber() */
	nCPNMCOL=99;    /* pqfnumber() */
	nCPPRGCL=118;   /* pqfnumber() */


	if(bOK){

		strcpy(szBuffer,DBgetvalue(DBRes,nIndex,nRODTBCD));
		pData=szBuffer;

		/*
		* cerco l'inizio campo
		*/
		while((pPtr=strchr(pData,'['))!=(char *)NULL){
			*(pPtr)='\0';
			strcat(szBarcodeVettore,pData);
			pData=pPtr+1;
			/*
			* cerco la fine campo
			*/
			if((pPtr=strchr(pData,']'))!=(char *)NULL){
				*(pPtr)='\0';
				strcpy(szFieldData,pData);
				nLen=strlen(pData);
				/*
				* identifico e processo i dati richiesti
				*/
				switch(pData[0]){
					/* dati generici */
					case 'G':	/* Giorno del mese  */ sprintf(szFieldValue,"%0*d",nLen,atoi(GetDatef("%d"))); break;
					case 'M':	/* Mese dell'anno   */ sprintf(szFieldValue,"%0*d",nLen,atoi(GetDatef("%m"))); break;
					case 'A':	/* Anno             */ sprintf(szFieldValue,"%0*d",nLen,atoi(GetDatef("%Y"))); break;
					/* dati ordine */
					case 'O':	/* Codice Ordine    */ sprintf(szFieldValue,"%0*d",nLen,atoi(DBgetvalue(DBRes,nIndex,nORDPROG))); break;
					case 'B':	/* Prebolla         */ sprintf(szFieldValue,"%0*d",nLen,atoi(DBgetvalue(DBRes,nIndex,nROPRBOL))); break;
					case 'P':	/* Filiale Partenza */ sprintf(szFieldValue,"%0*d",nLen,atoi(DBgetvalue(DBRes,nIndex,nROFLPAR))); break;
					case 'R':	/* Filiale Arrivo   */ sprintf(szFieldValue,"%0*d",nLen,atoi(DBgetvalue(DBRes,nIndex,nROFLARR))); break;
					/* dati collo */
					case 'C':	/* Numero Collo     */ sprintf(szFieldValue,"%0*d",nLen,atoi(DBgetvalue(DBRes,nIndex,nCPNMCOL))); break;
					case 'S':	/* Segnacollo       */ sprintf(szFieldValue,"%0*d",nLen,atoi(DBgetvalue(DBRes,nIndex,nCPPRGCL))); break;
					case 'D':	/* Flag Check Digit */ break;
					default: strcpy(szFieldValue,"99999999"); break;
				}
				strcat(szBarcodeVettore,szFieldValue);
				pData=pPtr+1;
			}
		}
		strcat(szBarcodeVettore,pData);
	}

	return szBarcodeVettore;
}

/*
* interpreta i dati richiesti secondo documentazione
* e genera il barcode vettore
*/
char *BarcodeSegnacollo_OLD(DBresult *DBRes,int nIndex)
{
	int nORDPROG;
	int nRODTBCD;
	int nROPRBOL;
	int nROFLPAR;
	int nROFLARR;
	int nCPNMCOL;
	int nCPPRGCL;
	static char szBarcode[256];
	int nSegnacollo;

	szBarcode[0]='\0';

	nCPPRGCL=118;    /* pqfnumber() */


	nSegnacollo = atoi(DBgetvalue(DBRes,nIndex,nCPPRGCL));
	sprintf(szBarcode,"%018d",nSegnacollo);

	return szBarcode;
}

/*
* interpreta i dati richiesti secondo documentazione
* e genera il barcode per amazon
*/
char *BarcodeSegnacollo(DBresult *DBRes,int nTupleIndex)
{
	int nROCDAMZ;
	int nCPPRGCL;
	static char szBarcode[256];
	int nSegnacollo;
	int nCodiceAmazon;
	char szSegnaCollo[128];
	char szCodiceAmazon[128];
	int nSum;
	int nIndex;
	int nCheckDigit;
	char szCheckDigit[128];

	szBarcode[0]='\0';

	nROCDAMZ=92;     /* pqfnumber() */
	nCPPRGCL=118;    /* pqfnumber() */


	/* segnacollo */
	nSegnacollo = atoi(DBgetvalue(DBRes,nTupleIndex,nCPPRGCL));
	/* codice azienda - dovrebbe essere : 190519827 */
	nCodiceAmazon    = atoi(DBgetvalue(DBRes,nTupleIndex,nROCDAMZ));

	/* codice azienda amazon 9 cifre */
	sprintf(szCodiceAmazon,"%09d",nCodiceAmazon);
	/* segnacollo 6 cifre */
	sprintf(szSegnaCollo,"%08d",nSegnacollo);

	/* compongo codice GSIN amazon da 17 cifre */
	strcpy(szBarcode,"00");
	strcat(szBarcode,szCodiceAmazon);
	strcat(szBarcode,szSegnaCollo);

	/* 
	* calcolo il check digit secondo metodo qui documentato : 
	* riga GSIN : https://www.gs1.org/services/how-calculate-check-digit-manually 
	*/

	nSum = 0;
	for(nIndex=0; nIndex < strlen(szBarcode); nIndex++){
		if(nIndex % 2){
			/* cifre dispari x1 */
			nSum += (szBarcode[nIndex]-'0');
		} else {
			/* cifre pari x3 */
			nSum += (szBarcode[nIndex]-'0')*3;
		}
	}
	/* check digit = complemento base 10 della unità : esempio 57 -> 3 */
	nCheckDigit = 10 - ( nSum % 10 );
	/* nel caso in cui l'operazione risulti 10 ...  esempio 50 -> 0 */
	nCheckDigit %= 10;

	sprintf(szCheckDigit,"%d",nCheckDigit);

	strcat(szBarcode,szCheckDigit);


	return szBarcode;
}
