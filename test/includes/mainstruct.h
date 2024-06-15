/*
* mainstruct.h
* Dichiarazione Strutture Utilizzate 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

typedef struct tagCfgStruct{
    char szCDSOC[80];
    char szCDMAG[80];
    char szTipoOrdini[80];
    char szTipoUbicazioni[80];
    char szTmpSuffix[80];
    ep_bool_t bRifornimenti;                     /* Rifornimenti Abilitati/Disabilitati */
    int nOrdiniKeyField;                    /* Indice campo chiave */
    int nAnno;                              /* Anno di lavoro (vedi sotto) */
    int nProgressivoLancio;                 /* Progressivo lancio - Azzerato annualmente */
    int nChiusuraCollo;                     /* Gestione chiusura collo */
    int nImballiUbicati;                    /* Gestione Imballi */
    int nTipoSelezione;                     /* Selection Type */
    int nTipoStampe;                        /* Print Type */
    char szFormatoMedio[80];                /* Formato Medio */
    int  nStampaRAC;                        /* Stampa RAC (DOPO_IL_LANCIO - INIZIO_PRELIEVO) */
    char szPathExe[80];
    char szPathTrace[80];
    char szPathData[80];
    char szPathStampe[80];
    char szPathStorico[80];
    char szPathExport[80];
    char szPathCorriereExport[80];

    char szImportMonitor[128];              /* Monitor di import */
    char szExportMonitor[128];              /* Monitor di export */
    char szImportSpedizioni[128];           /* File di import */
    char szImportRighe[128];                /* File di import */
    char szImportCatalogo[128];             /* File di import */
    char szImportTabelle[128];              /* File di import */

    char szExportSpedizioni[128];           /* File di export */
    char szExportRighe[128];                /* File di export */
    char szExportCorriere[128];             /* File di export */
    char szExportCorrAgg[128];              /* File di export Aggiuntivo */
    char szExportCorrSDA[128];              /* File di export SDA */
    char szExportFlussoCedola[128];         /* File di export Flusso Cedola */

    char szExportMonitorStorico[128];       /* Monitor di export */
    char szExportSpedizioniStorico[128];    /* File di export */
    char szExportAnticipato[128];           /* File di export */

    char szNotaCorriere[128];               /* Nota da inserire nel file per il corriere */
    char szDSTSP[128];                      /* Tipo spedizione */
    char szTPSPE[128];                      /* Descrizione */

    char szPrinterConsole[80];
    char szPrinterBilancia[80];
    char szLabelPrinterConsole[80];
    char szLabelPrinterBilancia[80];
    char szXABPrinterConsole[80];
    char szDISTPrinterConsole[80];
    char szPrinterSettori[80];

    int nMainDelay;
    int nMainTimer;
    int nInvioMailDelay;
    int nInvioMailTimer;
    int nSaveStateDelay;
    int nSaveStateTimer;
    int nStatisticaDelay;
    int nStatisticaTimer;

    int nSogliaCopiePre;
    int nSogliaPallet;
    int nIncrementoCopiePallet;

    int nColumnCharWidth;
    int nDebugLevel;
    int nA2ps;
    int nReindex;
    int nShmKey; 
    int nSettoriShmKey; 

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

    char szPLCHost[128]; 
    int nPLC;
    int nPLCService;

    int nServicePort;

    ep_bool_t bStampaEtichette;
    ep_bool_t bStampaPackingList;
    char szStampanteEtichette[128];
    char szStampantePackingList[128];
    int nNumeroIsole;
    int nNumeroSettori;
    int nInvioPeriodico;
    int nOraInvioPeriodico;
    int nReindexPeriodico;
    int nOraReindexPeriodico;
    char szTipoOrdinePreDistinta[128];
    ep_bool_t bMaster;
    ep_bool_t bIOSConnected;
    char szPassword[128];
    ep_bool_t bControllaCedola;

    int nDelay;
} CFGSTRUCT, *PCFGSTRUCT;

