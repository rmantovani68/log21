/*
* volvar.c : Analisi Volumetrica - Dichiarazione variabili globali
* 
* Progetto Easy Picking 4.0 : Wella
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
// #include <termio.h>
#include <sys/stat.h>

#include <glib.h>
#include <gio/gio.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <picking.h>

#include <proc_list.h>

#include	"volum.h"
#include	"volstruct.h"
#include	"volext.h"
#include	"volfun.h"

int nPID=0;
CFGSTRUCT	Cfg;

int nAllMsg;
int nCarIn;
ep_msg_header_t InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];

char szBufTrace[80];
char *pszNomeModulo;
char szTimeBuffer[80];
char szDateBuffer[80];

PLINEA_STRUCT pDatiLinea;     /* Dati di gestione linea */
IMBALLI Imballi;              /* Imballi */
