/*
* Modulo : recvar.c
* -----------------
* Modulo contenente la dichiarazione delle variabili globali utilizzate
*
*
* Data creazione 17-05-2001
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>

#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <msq_lib.h>
#include <proc_list.h>

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

CFGSTRUCT Cfg;
int nPID=0;                   /* Process ID */
PLINEA_STRUCT pLinea;
ep_bool_t bExitRequest=FALSE;

char szTimeBuffer[80];
char szDateBuffer[80];

/*
* Definizioni per le tabelle utilizzate in IMPORT/EXPORT
*/
#ifdef ELIMINATO
DBSTRUCT tCorriere;              /* Export Dati Corriere */
DBSTRUCT tCorriereAgg;           /* Export Dati Corriere - Aggiuntivo */
DBSTRUCT tCorrSDA;               /* Export Dati Corriere - SDA */
DBSTRUCT tQuadProd;              /* Quadratura di Produzione */
DBSTRUCT tUdcRestituitiExport;   /* Export Dati Flusso */
#endif

DBSTRUCT tOrdProd;               /* Ordini di Produzione */
DBSTRUCT tColProd;               /* Colli di Produzione */
DBSTRUCT tRigProd;               /* Righe di Produzione */
DBSTRUCT tEvasi;                 /* Evasi */
DBSTRUCT tContenutoColli;        /* Contenuto Colli */
DBSTRUCT tFlussoCedola;          /* Export Dati Flusso */

int nAllMsg;
int nCarIn;
ep_msg_header_t InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
