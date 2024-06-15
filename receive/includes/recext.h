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
extern DBSTRUCT tRicOrd;     /* Ordini Ricevuti */
extern DBSTRUCT tRicArt;     /* Righe Ricevute */
extern DBSTRUCT tCatalogo;   /* Catalogo Prodotti */
extern CFGITEM CFGItems[];
