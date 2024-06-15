/*
* Modulo : rifext.c
* -----------------
* Include file con le dichiarazioni external delle variabili globali
*
* Progetto EMI Italiana SPA - Gestione Rifornimenti
*
* Data creazione 14/03/1996
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
extern int nProgramStatus;
extern int nOldProgramStatus;
extern BOOL bExitMainLoop;
extern int nActiveRow;

extern CFGSTRUCT Cfg;
extern PROCESSO ProcList[NUM_PROC];
extern MSGHDRINT InMsgStruct;

extern char **Messages;

extern char szTimeBuffer[80];
extern char szDateBuffer[80];

extern short tasto;
extern struct specifiche_box gb[NUMERO_BOX];
extern struct specifiche_edit ed[NUMERO_CAMPI];

extern char *pszPrecasUbic[MAX_RECORDS];
extern char *pszPrecasProd[MAX_RECORDS];
extern char *pszPrecasTitolo[MAX_RECORDS];
extern int nPrecasNumCopie[MAX_RECORDS];
extern int nPrecasGiacenza[MAX_RECORDS];

extern char *pszBolleBolla[MAX_RECORDS];
extern char *pszBolleData[MAX_RECORDS];
extern char *pszBolleStato[MAX_RECORDS];
extern char *pszBolleUbic[MAX_RECORDS];
extern char *pszBolleProd[MAX_RECORDS];
extern int nBolleNumCopie[MAX_RECORDS];

extern char *pszColliBolla[MAX_RECORDS];
extern char *pszColliCollo[MAX_RECORDS];
extern char *pszColliCodCli[MAX_RECORDS];
extern char *pszColliRagSoc[MAX_RECORDS];
extern char *pszColliTown[MAX_RECORDS];
extern char *pszColliProv[MAX_RECORDS];

extern char *pszColloUbic[MAX_RECORDS];
extern char *pszColloProd[MAX_RECORDS];
extern int nColloNumCopie[MAX_RECORDS];

extern char *pszCasUbic[MAX_RECORDS];
extern char *pszCasPro[MAX_RECORDS];
extern int nCasQCas[MAX_RECORDS];
extern int nCasQMax[MAX_RECORDS];
extern int nCasQRif[MAX_RECORDS];

extern char *pszArtUbic[MAX_RECORDS];
extern char *pszArtPro[MAX_RECORDS];
extern int nArtQCas[MAX_RECORDS];
extern int nArtQMax[MAX_RECORDS];
extern int nArtQRif[MAX_RECORDS];
extern int nArtQImp[MAX_RECORDS];
extern int nArtQRic[MAX_RECORDS];
extern int nArtQCon[MAX_RECORDS];
extern int nArtBlc[MAX_RECORDS];

extern char *pszDiffCod[MAX_DIFF];
extern char *pszDiffUbic[MAX_DIFF];
extern int nDiffQEmi[MAX_DIFF];
extern int nDiffQCni[MAX_DIFF];
extern int nDiffQDiff[MAX_DIFF];

extern char *pszMainMessages[10];
extern char *pszMainHeaders[10];
extern char *pszExitMessages[10];
extern char *pszExitHeaders[10];
extern char *pszConfirmMessages[10];
extern char *pszConfirmHeaders[10];
extern char *pszBolleHeaders[10];
extern char *pszColliHeaders[10];
extern char *pszColloHeaders[10];
extern char *pszCaselHeaders[10];
extern char *pszArtHeaders[10];
extern char *pszCercaUbicHeaders[10];
extern char *pszCercaProdHeaders[10];
extern char *pszDiffHeaders[10];
extern char *pszPrecasHeaders[10];

extern char szCercaUbic[80];
extern char szCercaProd[80];

extern char *pszCaricaArticolo[10];
extern int nCaricaArticoloQty;

extern char *pszEnqBolla[10];
extern int nEnqBolla;

extern char *pszModGiac[10];
extern int nModGiac;

extern char *pszModRif[10];
extern int nModRif;
