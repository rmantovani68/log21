/*
* printvar.c : Stampa Picking List - Dichiarazione variabili globali
* 
* Progetto Mondadori Picking
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
//#include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <proc_list.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"

CFGSTRUCT	Cfg;
int nPID=0;                   /* Process ID */

int nAllMsg;
int nCarIn;
ep_msg_header_t InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
PLINEA_STRUCT pDatiLinea;     /* Dati di gestione sistema */

char szBufTrace[80];
char *pszNomeModulo;
char szTimeBuffer[80];
char szDateBuffer[80];

POS Pos[NUM_FIELD_RAC]; 	  	/* Posizione dei campi nella stampa della RAC */
POS XABPos[NUM_FIELD_XAB];  	/* Posizione dei campi nella stampa della XAB */
POS XABPos_A4[NUM_FIELD_XAB];	/* Posizione dei campi nella stampa della XAB su A4*/
POS PLPos_A4[NUM_FIELD_XAB]; 	/* Posizione dei campi nella stampa della Packing List */
POS DCPos[NUM_FIELD_XAB];   	/* Posizione dei campi nella stampa del Documento di Trasporto */
POS DisPos[NUM_FIELD_DIS];  	/* Posizione dei campi nella stampa della Distinta */
int nPrintingRow[50];			  	/* Righe sulle quali stampare nella RAC */
char *pszForm[MAX_RAC_LINES]; /* Matrice di memorizzazione provvisoria per la RAC */
char szPRXAB[80];					  	/* Progressivo della XAB */

PLINEA_STRUCT pDatiLinea;     /* Dati di gestione sistema */

LABEL_FIELD_STRUCT LabelFields[256];
