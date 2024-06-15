/*
* Modulo : rfstruct.h
* -----------------------
* Definizione delle strutture utilizzate nel processo RF
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19-06-2000
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1996-2000
*/
typedef struct tagCfgStruct{
	char szCfgFileName[128];
	int nDebugLevel;

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

	char szPathExe[128];
	char szPathTrace[128];
	char szPathData[128];
	char szPathStampe[128];
	char szPathExport[128];
	char szPathStorico[128];

	int nMainId;             /* Main ID */
	int nAbilitaForzaturaRF;
	int nKeyField;            /* Indice campo chiave */
	int nColloNumeroField;    /* Indice campo collo */
	int nBarcodeField;        /* Indice campo Barcode */
	char szPrinterName[80];
	char szLanguage[80];
	char szSuperPwd[20];
	char szSuperPwdCfm[20];
	char szChiefPwd[20];
	char szChiefPwdCfm[20];
	char szPwd[20];
	int nActualPwd;
	unsigned long ulSuperPwd;
	unsigned long ulChiefPwd;
	char szHostName[256];
	char szBarcodePort[256];
	int nMainDelay;
	int nDelayUrgenze;
	int nBilanciaTimeout;
	int nPLC;
	int nTimer;
	int nShmKey; 
	int nRcvDataSize; 
	ep_bool_t bShmCreated;
	int nTipoForzatura;
	char nHostService;

} CFGSTRUCT, *PCFGSTRUCT;

typedef struct tagOperatore {
	char szIdOperatore[32];       /* Codice Operatore addetto al prelievo */
	char szPasswordOperatore[32]; /* Cognome Operatore addetto al prelievo */
	char szNomeOperatore[32];     /* Nome Operatore addetto al prelievo */
	char szCognomeOperatore[32];  /* Cognome Operatore addetto al prelievo */
} OPERATORE, *POPERATORE;

typedef struct tagMacchina {
	char szNome[32];                 /* Nome macchina (PC,Kyman,Bulk,...) */
	int nNumero;                     /* Numero macchina (0,1,...)*/
	char szCodice[32];               /* Codice Macchina */
	char szUbicazioneAssociata[32];  /* Codice Ubicazione Associata */
} MACCHINA, *PMACCHINA;

typedef struct tagRigaCarico {
	int nProgressivo;
	char szCodiceUdcPrelievo[32];
	char cDeposito;
	char szCodiceUbicazione[32];
	char szUbicazione[32];
	char szBarcodeProdotto[32];
	char szCodiceProdotto[32];
	char szTitoloProdotto[64];
	int nPrezzo;
	int nQuantita;
	char cStato;
} RIGA_CARICO, *PRIGA_CARICO;

typedef struct tagDatiLavorazioneOrdineStruct{
	char szProgressivoSottordine[32];
	int nProgressivoSottordine;
	int nProgressivoOrdine;
	int nSottordine;
	char cTipoOrdine;
	char szDescrizioneOrdine[64];
	GSList *slListaRighe;
	int nRigheInLista;
	ep_bool_t bUdcAttivo;
	int nProgressivoUdc;
	char szCodiceUdcScarico[32];
	char szCodiceUdcPrelievo[32];
	char szBarcodeProdotto[32];
	char szCopieLetteProdotto[32];
	char szUbicazioneScarico[32];
	char szDepositoScarico[32];
	char szCatastaScarico[32];
	char szScaffalaturaScarico[32];
	char szCampataScarico[32];
	char szPianoScarico[32];
	char szLivelloScarico[32];
	char szPosizioneScarico[32];
	char szCodiceUdcMacero[32];
	char szCodiceClassifica[32];
	char szTitoloProdotto[64];
	char szCodiceProdotto[32];
} DATI_LAVORAZIONE_ORDINE, *P_DATI_LAVORAZIONE_ORDINE;
