/*
* Modulo : recstruct.h
* -----------------------
* Definizione delle strutture utilizzate 
*
* Data creazione 14/03/1996
*
* Autore : Roberto Mantovani
*
* Copyright SMD srl 2002
*/


typedef struct tagCfgStruct{
	char szCfgFileName[80];
	int nDebugLevel;

	char szTipoOrdini[80];
	char szTipoUbicazioni[80];
	char szProvenienzaOrdini[80];

	char szPathExport[PATH_MAX];
	char szPathData[PATH_MAX];
	char szPathTrace[PATH_MAX];

	char szImportMonitor[PATH_MAX];
	char szImportSpedizioni[PATH_MAX];
	char szImportRighe[PATH_MAX];
	char szImportCatalogo[PATH_MAX];

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 

	int nDelayRicezione;
	int nMainID;
} CFGSTRUCT, *PCFGSTRUCT;

