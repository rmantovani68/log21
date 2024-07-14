/*
* mainstruct.h
* Dichiarazione Strutture Utilizzate 
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
* Autore : Roberto Mantovani
*/

typedef struct tagPKLStruct{
	int nIOS;
	int nCPU;
	int nModulo;
	BYTE DamageCounter[MAX_RIGHE_PKL][MAX_COLONNE_PKL];
} PKLSTRUCT, *PPKLSTRUCT;

typedef struct tagCfgStruct{
	char szCniCfg[80];
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

	int nShmKey;
	int nSettoriShmKey;
	int nDelay;
	int nSimDelay;
	int nNumeroCedole;
	int nNumeroSettori;
	int nNumeroIsole;
	int nNumeroDisplays;
	int nMaxDamages;
	int nPKLNumber;
	DISPLAYSTRUCT Displays[MAX_DISPLAYS];
	PKLSTRUCT PKL[MAX_PKL];
} CFGSTRUCT, *PCFGSTRUCT;


typedef struct TagUbicazione {
	char szCodProd[40];
	char szCodUbi[40];
	char szLocazione[40];
	int  nSettore;
	int  nIsola;
	int  nDisplay;
	int  nPLCNum;
	int  nPriorita;
	int  nIndex;
} UBICAZIONE, *PUBICAZIONE;

typedef struct TagUbicazioni {
	int nUbicazioni;
	UBICAZIONE Ubicazione[MAX_UBICAZIONI];
} UBICAZIONI, *PUBICAZIONI;


typedef struct tagEvento {
	int nSettore;
	int nIsola;
	int nEvento;
	int nCPUNum;
	PUBICAZIONE pUbicazione;
} EVENTO, *PEVENTO;

typedef struct {
	int nCedola;
	long lLastOrdine;
	int nOrdini;
	int nRighe;
	int nCopie;
	int nStartSettore;
	int nEndSettore;
} CEDOLASTRUCT,* PCEDOLASTRUCT;
