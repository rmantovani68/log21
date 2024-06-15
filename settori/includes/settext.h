/*
* settext.h
* 
* Progetto Easy Picking 4.0
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#pragma once

#include "cmp_time.h"

extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __copyright__[];
extern char __authors__[];
extern char __configuration_file__[];

extern int nPID;
extern CFGSTRUCT    Cfg;

extern int nAllMsg;
extern int nCarIn;
extern ep_msg_header_t InMsgStruct;
extern char szInMsg[INFO_LEN];
extern char szText[INFO_LEN];

extern char szTimeBuffer[80];
extern char szDateBuffer[80];

extern PLINEA_STRUCT pDatiLinea;     /* Dati di gestione sistema */
extern LINKED_LIST ListaOrdini[MAX_SETTORI];    /* Lista FIFO Ordini */
extern PSETTORE pSettori;            /* shared memory pointer */
extern UBICAZIONI Ubicazioni;        /* Ubicazioni */
extern EVENTO Evento;                /* Evento */

/*
* Connessione al DataBase PostgreSQL
*/
extern DBconn *DBConn;

extern CEDOLASTRUCT Cedole[MAX_CEDOLE];

extern int nPrimoSettoreAbilitato;
