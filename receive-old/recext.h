/*
* Modulo : recext.c
* -----------------
* Include file con le dichiarazioni external delle variabili globali
*
* Progetto EMI Italiana SPA - Gestione Rifornimenti
*
* Data creazione 17-05-2001
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

extern CFGSTRUCT Cfg;
extern int nPID;                   /* Process ID */
extern PROCESSO ProcList[NUM_PROC];
extern PLINEA_STRUCT pLinea;
extern BOOL bExitRequest;

extern char szTimeBuffer[80];
extern char szDateBuffer[80];

/*
* Definizioni per le tabelle utilizzate in IMPORT/EXPORT
*/
extern DBSTRUCT tRicOrd;     /* Ordini Ricevuti */
extern DBSTRUCT tRicArt;     /* Righe Ricevute */
extern DBSTRUCT tRicNote;    /* Note Ricevute */
extern DBSTRUCT tRicQuad;    /* Quadratura Ricezione */
extern DBSTRUCT tCatalogo;   /* Catalogo Prodotti */
extern DBSTRUCT tTTCS;
extern DBSTRUCT tTTFC;
extern DBSTRUCT tTTFP;
extern DBSTRUCT tTTGC;
extern DBSTRUCT tTTLS;
extern DBSTRUCT tTTLV;
extern DBSTRUCT tTTRC;
extern DBSTRUCT tTTTS;
extern DBSTRUCT tTTVE;
extern DBSTRUCT tTTEC;
