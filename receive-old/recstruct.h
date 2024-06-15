/*
* Modulo : rifstruct.h
* -----------------------
* Definizione delle strutture utilizzate nel processo Rifornimenti
*
*
* Data creazione 14/03/1996
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/


typedef struct tagCfgStruct{
	char szCniCfg[80];
	char szTipoOrdini[80];
	char szPathExport[128];
	char szPathFtpImport[128];
	char szPathData[128];
	int nDebugVersion;

	char szImportMonitor[128];
	char szImportSpedizioni[128];
	char szImportCatalogo[128];
	char szImportTabelle[128];

	char szPGHost[128]; 
	char szPGPort[128]; 
	char szPGDataBase[128]; 

	int nDelayRicezione;
	int nMainID;
} CFGSTRUCT, *PCFGSTRUCT;

typedef struct tagDatiDistribuzione{
	char szSWCOL[128];
	char szTPSPE[128];
	char szCDLIN[128];
	char szCDSCA[128];
	char szCDSSC[128];
	char szIDVET[128];
	char szCDVE1[128];
	char szCDVE2[128];
} DATI_DISTRIBUZIONE, *PDATI_DISTRIBUZIONE;
