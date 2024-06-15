/*
* volstruct.h
* 
* Progetto Easy Picking 2.0 : Europharco
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

typedef struct tagCfgStruct{
	char szTipoOrdini[80];
	char szLanguage[128];
	char szPathData[128];
	char szPathExport[128];
	char szPathDataBase[80];
	char szPathStorico[80];
	char szPathExe[128];
	char szPathTrace[128];
	char szFormatoMedio[128];

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

	int nDelay;
	int nMaxColliOrdine;
	int nSogliaCopiePre;
	int nSogliaPallet;
	int nIncrementoCopiePallet;
	int nDebugLevel;
	int nShmKey;
	ep_bool_t bAlone;
} CFGSTRUCT, *PCFGSTRUCT;


