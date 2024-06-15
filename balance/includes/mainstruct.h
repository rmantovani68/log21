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
	int nCedoleKeyField;        /* Indice campo chiave lista cedole */
	int nArticoliKeyField;      /* Indice campo chiave lista articoli */
	char szCniCfg[80];
	char szPathExe[256];
	char szPathData[256];
	char szPathStampe[256];
	char szPathExport[256];
	char szPathTrace[256];

	char szPrinterConsole[80];

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
} CFGSTRUCT, *PCFGSTRUCT;

