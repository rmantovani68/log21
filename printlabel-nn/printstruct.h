/*
* printstruct.h
* 
* Progetto Mondadori Picking
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/


typedef struct tagPos {
	int nSR;		/* Starting Row			*/
	int nSC;		/* Starting Column	*/
} POS;

typedef struct tagCfgStruct{
	/*
	* Dati generali
	*/
	int nDebugVersion;
	char szCniCfg[128];
	char szPrintCfg[128];
	char szTipoOrdini[80];

	char szPathExe[128];
	char szPathTrace[128];
	char szPathData[128];
	char szPathStampe[128];
	char szPathExport[128];

	char szLabelBaseName[80];
	
	/*
	* Dati specifici del processo (printlabel)
	*/
	char szFileName[128];  /* Nome completo del file: path/filename */
	char szFine[40];			 /* Identificativo di stampa della riga totali dell'ordine 	*/
	char szRighe[40];			 /* Righe totali dell'ordine: identificativo totale righe 		*/
	char szVolumi[40];		 /* Volumi totali dell'ordine: identificativo totale ordini	*/

	char szCodMovimento[40];
	char szTipoNotaRAC[40];
	char szFineOrdineRighe[40];
	char szFineOrdineVolumi[40];
	char szFineBolla[40];
	char szContenuto[80];
	char szTipoNotaXAB[80];
	char szSegue[80];
	char szSegueBAM[80];
	char szSeguePL[80];
	char szFoglio[80];
	char szVecchiaPaginaDistinta[80];
	char szNuovaPaginaDistinta[80];
	char szNomeMagazzino[128];
	char szDataDocumento[80];

	int nTipoOrdinamentoStampaXAB; 
	int nLinesPerForm;/* Lines per form	*/ 
	int nLengthTitle;	/* Lunghezza stampabile del campo titolo	*/
	int nLengthAuthor;/* Lunghezza stampabile del campo autore  */
	int nPrintableRow;/* Numero di righe stampabili per foglio	*/
	int nShmKey;
	int nDelay;
	int nRigheStampanteParallela;
	int nColonneStampanteParallela;
	int nRigheStampanteParallela_A4;
	int nColonneStampanteParallela_A4;
	int nLibriPagina;
	int nAnnoXAB;
	int nProgressivoXAB;
	int nPrimaRigaXAB;
	int nUltimaRigaXAB;
	int nPrimaRigaXAB_A4;
	int nUltimaRigaXAB_A4;
	int nPrimaRigaPL_A4;
	int nUltimaRigaPL_A4;
	int nPrimaRigaDC;
	int nUltimaRigaDC;
	int nPrimaRigaDistinta;
	int nUltimaRigaDistinta;
	int nProgressivoDistinta;

	/*
	* Database settings
	*/
	char szPGHost[128]; 
	char szPGPort[128]; 
	char szPGDataBase[128]; 

	int nMainID;								/* Main ID */
	int nA2ps;                  /* usa a2ps nelle stampe */
} CFGSTRUCT, *PCFGSTRUCT;


typedef struct TagDatiRigaStruct {
	char szRPCDPRO[128];
	char szRPCDUBI[128];
	int nRPQTSPE;
	int nRAPZPRO;
	char szPRDSTIT[128];
	char szPRDSAUT[128];
} DATIRIGA, *PDATIRIGA;

typedef struct TagDatiColloStruct {
	char szORDPROG[128];    /* Progressivo */
	int nCPNMCOL;           /* Numero del collo */
	char szCPNMPPT[128];    /* Numero pacco postale */
	int nCPPSPRE;           /* Peso Calcolato */
	int nCPPSREA;           /* Peso Reale */
	char szCPVLAFF[128];    /* Valore dell'affrancatura postale in LIRE */
	char szCPTPFOR[128];    /* Tipo formato */
	char szCPSWFPS[128];    /* Switch di forzatura peso */
	char szCPSWCHM[128];    /* Switch di chiusura manuale del collo */
	char szCPVLAFE[128];    /* Valore dell'affrancatura postale in EURO (centesimi) */
	char szCPSWLIN[128];    /* Switch Lavorazione Automatica/Manuale */
	int nCPNMRGH;           /* Totale Righe */
	int nCPNMCPE;           /* Totale Copie */
	char szCPBRCDE[128];    /* Codice a barre del collo (2+6+5+1) */
	char szCPTMEVA[128];    /* Data Evasione (datetime) */
	char szCPCDFLG[128];    /* Flag Collo */
	char szCPSTATO[128];    /* Stato Collo */
} DATICOLLO, *PDATICOLLO;

typedef struct TagDatiOrdineStruct {
	char szROCDSOC[128];   /* Societa' */
	char szROCDMAG[128];   /* Magazzino */
	char szROAADOC[128];   /* Anno Documento */
	char szROPRDOC[128];   /* Codice Documento */
	char szORDPROG[128];   /* Chiave Ordine Completa */
	char szROCDRID[128];   /* Codice Cliente */
	char szROCDSPC[128];   /* Sede periferica commerciale */
	char szROCDARE[128];   /* Codice area */
	char szROCDVEN[128];   /* Codice venditore */
	char szRODSCLI[128];   /* Anagrafica punto vendita */
	char szROCOCLI[128];   /* Presso punto vendita */
	char szROINCLI[128];   /* Indirizzo punto vendita */
	char szROLOCLI[128];   /* Localita' punto vendita */
	char szROPRCLI[128];   /* Provincia punto vendita */
	char szROCPCLI[128];   /* CAP punto vendita */
	char szRONZCLI[128];   /* Nazione punto vendita */
	char szROCDCLA[128];   /* Codice del cliente amministrativo (Destinatario) */
	char szROCDSPA[128];   /* Sede periferica amministrativa */
	char szRODSCLA[128];   /* Anagrafica cliente amministrativo */
	char szROINCLA[128];   /* Indirizzo cliente amministrativo */
	char szROCOCLA[128];   /* Presso cliente amministrativo */
	char szROLOCLA[128];   /* Localita' cliente amministrativo */
	char szROPRCLA[128];   /* Provincia cliente amministrativo */
	char szROCPCLA[128];   /* CAP cliente amministrativo */
	char szRONZCLA[128];   /* Nazione cliente amministrativo */
	char szROSWSPF[128];   /* Switch di spedizione fiscale */
	char szRONMBAM[128];   /* Numero interno BAM */
	char szRODTBAM[128];   /* Data della BAM */
	char szROTPTAR[128];   /* Tipo tariffa */
	char szROSWCOL[128];   /* Switch tipo imballo (Colli:1 Pancali:4)  */
	int nROVLCON;          /* Valore del contrassegno in lire */
	char szROTPSPE[128];   /* Tipo spedizione se forzati */
	char szROCDLIN[128];   /* Linea se forzato */
	char szROCDSCA[128];   /* Scalo se forzato */
	char szROCDSSC[128];   /* Sottoscalo se forzato */
	char szROCDSOT[128];   /* Codice sottofascia (per posta) se forzato */
	char szROCDMOV[128];   /* Codice del movimento */
	char szROTPDOC[128];   /* Tipo del documento */
	char szRODTEVA[128];   /* Data evasione (AAAAMMGG) */
	char szROSWFIN[128];   /* Switch di instradamento forzato */
	char szROCDFPG[128];   /* Codice forma di pagamento */
	char szROTPBUT[128];   /* Tipo buttata (R)ifornimento/(C)edola) */
	char szROORINS[128];   /* Ora Inserimento  (HH:MM:SS) */
	char szROCDVAL[128];   /* Codice valuta della spedizione ('LIT','EUR') */
	int nROVLCOE;          /* Valore del contrassegno in Euro (in centesimi di EURO) */
	char szRONMCED[128];   /* Numero Cedola */
	char szROCDVE1[128];   /* Codice del primo vettore */
	char szROCDVE2[128];   /* Codice del secondo vettore */
	char szROCDVE3[128];   /* Codice del terzo vettore */
	char szROIDVET[128];   /* Identificatore Vettore per sovrappacco */
	int nRONMRGH;          /* Numero Righe Richieste */
	int nRONMRGP;          /* Numero Righe di Produzione */
	int nRONMCPE;          /* Numero Copie Richieste */
	int nRONMCPP;          /* Numero Copie di Produzione */
	int nRONMCLL;          /* Numero Colli */
	int nRONMCLA;          /* Numero Colli di Linea Automatica */
	char szROCDBTC[128];   /* Codice BTC */
	int nROPSPRE;          /* Peso Calcolato       (grammi) */
	int nROPSREA;          /* Peso Reale           (grammi) */
	char szROTMRCZ[128];   /* Data Ricezione       (datetime) */
	char szROTMINI[128];   /* Data Inizio Prelievo (datetime) */
	char szROTMEVA[128];   /* Data Evasione        (datetime) */
	int nROAAXAB;          /* Anno della XAB  */
	int nROPRXAB;          /* Numero della XAB */
	char szRODTXAB[128];   /* Data della XAB      (YYYYMMDD) */
	int nRONMDIS;          /* Numero della distinta */
	char szRODTDIS[128];   /* Data della distinta (YYYYMMDD) */
	char szROPRGLN[128];   /* Progressivo Lancio */
	char szROCDFLG[128];   /* Flag Ordine */
	char szROSTATO[128];   /* Stato Ordine */
	char szORDTIPO[128];   /* Tipo Ordine */
} DATIORDINE, *PDATIORDINE;

typedef struct TagTotaliPagina{
	int nSpedizioni;
	int nPacchi;
	int nPeso;
	int nPezzi;
} TOTALI, *PTOTALI;

typedef struct tagLabelFieldStruct{
	char szFieldID[80];
	char szFieldType[80];
	char szTableName[80];
	char szFieldName[80];
	char szFieldDescr[80];
} LABEL_FIELD_STRUCT, *PLABEL_FIELD_STRUCT;
