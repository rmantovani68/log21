/*
* Modulo : rfext.h
* -----------------
* include file  contenente la dichiarazione external delle variabili globali utilizzate
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19/06/2000
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1996-2000
*/

#include "cmp_time.h"

extern int nProgramStatus;
extern int nOldProgramStatus;
extern char cTipoForzatura;
extern char cOldFlag;

extern CFGSTRUCT Cfg;
extern MACCHINA Macchina;
extern OPERATORE Operatore;
extern DATI_LAVORAZIONE_ORDINE DatiLavorazioneOrdine;

//extern char **Messages;
extern int nProgressivoSottordine[MAX_RIGHE_IN_LISTA];


extern char szTimeBuffer[80];
extern char szDateBuffer[80];
extern int nRowEdit;
extern int nColEdit;

extern int nIndiceRiga;
extern int nTotaleRighe;
extern int bPrimaCarica;
extern int nCopieCaricate;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern short tasto;

extern CFGITEM CFGItems[];
