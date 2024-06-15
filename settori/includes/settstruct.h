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
    char szCniCfg[128];
    char szTipoOrdini[80];
    char szLanguage[128];
    char szPathData[128];
    char szPathExport[128];
    char szPathDataBase[80];
    char szPathStorico[80];
    char szPathExe[128];
    char szPathTrace[128];

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

    int nShmKey;
    int nSettoriShmKey;
    int nNumeroDisplays;
    int nNumeroCedole;
    int nNumeroSettori;
    int nDelay;
    int nDebugLevel;
    int nGetOrderDelay;
    int nSimDelay;
    int nNumeroIsole;
    int nMaxDamages;
    int nStampaRAC;
    int nImballiUbicati;
    int nFineOrdine;
    int nChiusuraCollo;
    ep_bool_t bAlone;
    ep_bool_t bTest;
    ep_bool_t bDisplay;
    ep_bool_t bAttesaAnalisiVolumetrica;
    ep_bool_t bRifornimenti;         /* Rifornimenti Abilitati/Disabilitati */
    int nPKLNumber;
    DISPLAYSTRUCT Displays[MAX_DISPLAYS];
    PKLSTRUCT PKL[MAX_PKL];
} CFGSTRUCT, *PCFGSTRUCT;


