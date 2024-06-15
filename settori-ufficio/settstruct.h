/*
* settstruct.h
* 
* Progetto Easy Picking 2.0 : Europharco
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

typedef struct tagPKLStruct{
	int nIOS;
	int nCPU;
	int nModulo;
	BYTE DamageCounter[MAX_RIGHE_PKL][MAX_COLONNE_PKL];
} PKLSTRUCT, *PPKLSTRUCT;

typedef struct tagCfgStruct{
	char szTipoOrdini[80];
	char szLanguage[128];
	char szPathData[128];
	char szPathExport[128];
	char szPathDataBase[80];
	char szPathStorico[80];
	char szPathExe[128];
	char szPathTrace[128];

	char szPGHost[128];
	char szPGPort[128];
	char szPGDataBase[128];

	int nShmKey;
	int nSettoriShmKey;
	int nNumeroDisplays;
	int nNumeroSettori;
	int nDelay;
	int nDebugVersion;
	int nGetOrderDelay;
	int nSimDelay;
	int nNumeroCedole;
	int nNumeroIsole;
	int nMaxDamages;
	int nStampaRAC;
	int nImballiUbicati;
	int nFineOrdine;
	int nChiusuraCollo;
	BOOL bAlone;
	BOOL bTest;
	BOOL bDisplay;
	BOOL bAttesaAnalisiVolumetrica;
	BOOL bRifornimenti;         /* Rifornimenti Abilitati/Disabilitati */
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
	int nIOS;
	int nCPU;
	int nModulo;
	int nRowIndex;
	int nColIndex;
	int nPriorita;
	int nIndex;
} UBICAZIONE, *PUBICAZIONE;

typedef struct TagUbicazioni {
	int nUbicazioni;
	UBICAZIONE Ubicazione[MAX_UBICAZIONI];
} UBICAZIONI, *PUBICAZIONI;


typedef struct TagOrdine {
	LINK_ITEM link;
	char szOrdProg[80];   /* Progressivo Spedizione */
	int nIsola;						/* Numero Isola  */
	int nColli;						/* Numero Colli dell'ordine  */
	int nRighe;						/* Numero Righe dell'ordine  */
	int nCopie;						/* Numero Copie dell'ordine  */
	char cFlag;						/* Flag di gestione ordine */
} ORDINE, *PORDINE;


typedef struct tagEvento {
	int nSettore;
	int nIsola;
	int nEvento;
	int nModulo;
	int nIOS;
	int nCPU;
	int nRiga;
	int nColonna;
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
