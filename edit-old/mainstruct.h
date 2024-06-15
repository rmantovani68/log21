/*
* mainstruct.h
* Dichiarazione Strutture Utilizzate 
* Easy Picking 3.0
* Copyright A&L srl 2003
* Autore : Roberto Mantovani
*/

typedef struct tagCfgStruct{
	char szCfgFile[80];
	char szTipoOrdini[80];
	char szTipoUbicazioni[80];
	char szPathExe[80];
	char szPathData[80];
	char szPathStampe[80];
	char szPrinterConsole[80];

	int nCharWidth;
	int nMainDelay;
	int nTimer;
	int nA2ps;

	int nDebugVersion;
	char szPGHost[128]; 
	char szPGPort[128]; 
	char szPGDataBase[128]; 
} CFGSTRUCT, *PCFGSTRUCT;

