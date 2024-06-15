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
#include <libpq-fe.h>
#include <msq_lib.h>

#include <proc_list.h>
#include <dbfun.h>
#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"

CFGSTRUCT Cfg;
int nPID=0;                   /* Process ID */
PROCESSO ProcList[NUM_PROC];
PLINEA_STRUCT pLinea;
BOOL bExitRequest=FALSE;

char szTimeBuffer[80];
char szDateBuffer[80];

/*
* Definizioni per le tabelle utilizzate in IMPORT/EXPORT
*/
DBSTRUCT tRicOrd;     /* Ordini Ricevuti */
DBSTRUCT tRicArt;     /* Righe Ricevute */
DBSTRUCT tRicNote;    /* Note Ricevute */
DBSTRUCT tRicQuad;    /* Quadratura Ricezione */
DBSTRUCT tCatalogo;   /* Catalogo Prodotti */
DBSTRUCT tTTCS;
DBSTRUCT tTTFC;
DBSTRUCT tTTFP;
DBSTRUCT tTTGC;
DBSTRUCT tTTLS;
DBSTRUCT tTTLV;
DBSTRUCT tTTRC;
DBSTRUCT tTTTS;
DBSTRUCT tTTVE;
DBSTRUCT tTTEC;
