/*
* mainstruct.h
* Dichiarazione Strutture Utilizzate 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

enum ActualFrame {
	FRAME_DATI_PRODOTTO,
	FRAME_STATISTICHE,
	FRAME_LISTA_COLLI,

	NUMERO_FRAME,
};

typedef struct tagCfgStruct{
	char szCDSOC[80];
	char szCDMAG[80];
	char szTipoOrdini[80];
	char szTmpSuffix[80];
	int nCedoleKeyField;        /* Indice campo chiave lista cedole */
	int nArticoliKeyField;      /* Indice campo chiave lista articoli */
	char szCniCfg[80];
	char szPathExe[256];
	char szPathData[256];
	char szPathStampe[256];
	char szPathExport[256];
	char szPathTrace[256];
    char szDisplayCfgFileName[128];

	char szPrinterConsole[80];
	char szMemCachedServerName[80];

	int nShmKey;
	int nSettoriShmKey;

	int nMemCachedPortNumber;
	int nTipoStampe;
	int nColumnCharWidth;
	int nDebugLevel;
	int nA2ps;

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

	int nNumeroSettori;
    int nImballiUbicati;

    int nMainDelay;
    int nMainTimer;

    char szTEST[128];
    char szPRELIEVO_IMBALLO[128];
    char szPRELIEVO[128];
    char szPASSA_SETT_SUCC[128];
    char szCHIUSURA_COLLO[128];
    char szFINE_ORDINE[128];
    char szOFFLINE[128];
    char szSTART[128];
    char szSTOP[128];
    char szERROR[128];
    char szWAIT[128];
    char szORDINE_COLLO[128];
    char szSETTORE[128];
    char szOPERATORE[128];
    char szUBICAZIONE[128];
    char szCODICE[128];
    char szTITOLO[128];
    char szDA_PRELEVARE[128];
    char szPRELEVATI[128];
    char szORDINI[128];
    char szCOLLI[128];
    char szRIGHE[128];
    char szCOPIE[128];




} CFGSTRUCT, *PCFGSTRUCT;

