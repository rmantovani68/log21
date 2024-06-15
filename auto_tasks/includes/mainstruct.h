/*
* Modulo : mainstruct.h
* -----------------------
* Definizione delle strutture utilizzate
*
*
* Data creazione 14/03/1996
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/


typedef struct tagCfgStruct{
	char szCfgFileName[80];
	char szTipoOrdini[80];
	char szTmpSuffix[80];
	char szPathExport[256];
	char szPathStorico[256];
	char szPathCorriereExport[256];
	char szPathCorriereExportBackup[256];
	char szPathPDFExport[256];
	char szPathPDFExportBackup[256];
	char szPathExportFlussoCedola[256];
	char szExportFlussoCedola[256];
	char szPathExportFlussoUdcRestituiti[256];
	char szExportFlussoUdcRestituiti[256];
	char szPathData[256];
	char szPathTrace[256];
	int nDebugLevel;

	char szExportMonitor[128];       /* Monitor di export */
	char szExportMonitorStorico[128];       /* Monitor di export */
	char szExportSpedizioni[128];       
	char szExportRighe[128];       
	char szExportSpedizioniStorico[128];
	char szExportCorriere[128];      /* File di export */
	char szExportCorriereAgg[128];       /* File di export Aggiuntivo */
	char szExportCorrSDA[128];       /* File di export SDA */

	char szPDFDistintaFileName[128];  
	char szPDFXABFileName[128];      

	char szImportMonitor[128];
	char szImportSpedizioni[128];
	char szImportCatalogo[128];
	char szImportTabelle[128];

	char szNotaCorriere[128];        /* Nota da inserire nel file per il corriere */
	char szDSTSP[128];               /* Tipo spedizione */
	char szTPSPE[128];               /* Descrizione */
	char szCDSOC[80];
	char szCDMAG[80];

	int nInvioPeriodico;
	int nOraInvioPeriodico;
	int nReindexPeriodico;
	int nOraReindexPeriodico;

	char szDBHost[128]; 
	char szDBPort[128]; 
	char szDBName[128]; 
	char szDBUser[128]; 
	char szDBPassword[128]; 

	int nDelaySendFileCorriere;
	int nDelaySendMailCorriere;
	int nDelayCheckMail;
	int nDelayCheckMailError;
	int nDelayCheckCambioStato;
	int	nDelayCheckFlussoUdcRestituiti;
	int nMainID;

	char szMailAddress[128]; 
	char szMailHost[128]; 
	char szMailUser[128]; 
	char szMailPassword[128]; 
	char szMailAddressDDTNovita[128]; 
	char szMailHostDDTNovita[128]; 
	char szMailUserDDTNovita[128]; 
	char szMailPasswordDDTNovita[128]; 
	char szMailAddressFlussoDati[128]; 
	char szMailHostFlussoDati[128]; 
	char szMailUserFlussoDati[128]; 
	char szMailPasswordFlussoDati[128]; 

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

typedef struct tagTask{
	int nId;
	int nCodice;
	char szParametri[512];
} ATASK, *PATASK;
