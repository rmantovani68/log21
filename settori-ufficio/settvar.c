/*
* settvar.c : gestione settori di magazzino - Dichiarazione variabili globali
* 
* Progetto Easy Picking 2.0 : Europharco
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <dbfun.h>
#include <proc_list.h>
#include <picking.h>

#include	"linklist.h"
#include	"settstruct.h"
#include	"settext.h"
#include	"settfun.h"

int nPID=0;
CFGSTRUCT	Cfg;

int nAllMsg;
int nCarIn;
MSGHDRINT InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];

char szBufTrace[80];
char *pszNomeModulo;
char szTimeBuffer[80];
char szDateBuffer[80];

PLINEA_STRUCT pDatiLinea;     /* Dati di gestione sistema */
LINKED_LIST ListaOrdini[MAX_SETTORI];    /* Lista FIFO Ordini */
PSETTORE pSettori;            /* shared memory pointer */
UBICAZIONI Ubicazioni;        /* Ubicazioni */
EVENTO Evento;                /* Evento */

/*
* Connessione al DataBase PostgreSQL
*/
PGconn *DBConn;

int nPrimoSettoreAbilitato;

CEDOLASTRUCT Cedole[MAX_CEDOLE];
