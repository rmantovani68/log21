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
#pragma once

#include "cmp_time.h"

extern CFGSTRUCT Cfg;
extern int nPID;                   /* Process ID */
extern PLINEA_STRUCT pLinea;
extern ep_bool_t bExitRequest;

extern char szTimeBuffer[80];
extern char szDateBuffer[80];

/*
* Definizioni per le tabelle utilizzate in IMPORT/EXPORT
*/

#ifdef ELIMINATO
extern DBSTRUCT tCorriere;              /* Export Dati Corriere */
extern DBSTRUCT tCorriereAgg;           /* Export Dati Corriere - Aggiuntivo */
extern DBSTRUCT tCorrSDA;               /* Export Dati Corriere - SDA */
extern DBSTRUCT tQuadProd;              /* Quadratura di Produzione */
extern DBSTRUCT tUdcRestituitiExport;   /* Export Dati Flusso */
#endif

extern DBSTRUCT tOrdProd;               /* Ordini di Produzione */
extern DBSTRUCT tColProd;               /* Colli di Produzione */
extern DBSTRUCT tRigProd;               /* Righe di Produzione */
extern DBSTRUCT tEvasi;                 /* Evasi */
extern DBSTRUCT tContenutoColli;        /* Contenuto Colli */
extern DBSTRUCT tFlussoCedola;          /* Export Dati Flusso */

extern int nAllMsg;
extern int nCarIn;
extern ep_msg_header_t InMsgStruct;
extern char szInMsg[INFO_LEN];
extern char szText[INFO_LEN];
