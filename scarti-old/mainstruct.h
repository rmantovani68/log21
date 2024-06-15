/*
* mainstruct.h
* Dichiarazione Strutture Utilizzate 
* Gestione Scarti
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

typedef struct tagCfgStruct{
	char szCniCfg[80];          /* File di configurazione generale */
	int nProgramID;
	char szPathExe[80];
	char szPathData[80];
	char szPathExport[80];
	char szPathStampe[80];
	int nDelay;
	int nTimer;
	int nDebugVersion;
	char szPLCHost[128]; 
	int nPLC;
	int nPLCService;
	int nServicePort;
	int nShmKey; 
	char szPGHost[128]; 
	char szPGPort[128]; 
	char szPGDataBase[128]; 
	char szPrinter[128];
	int nShowDlgBarcode;                /* Visualizza barcode dlg */
	int nColumnCharWidth;
	int nA2ps;
	int nHtml2ps;
} CFGSTRUCT, *PCFGSTRUCT;

typedef struct tagColorStruct{
	char szColorName[128];
	GdkColor cColor;
} COLORSTRUCT, *PCOLORSTRUCT;
