/*
* mainvar.c
* Variabili Globali
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef TRACE
	#include <trace.h>
#endif

#include <gtk/gtk.h>
#include <signal.h>
#include <libpq-fe.h>

#include "interface.h"
#include "support.h"

#include <proc_list.h>
#include <ca_file.h>
#include <pmx_msq.h>
#include <dbfun.h>

#include "picking.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"


CFGSTRUCT Cfg;
PSETTORE pSettori;            /* shared memory pointer */
UBICAZIONI Ubicazioni;        /* Ubicazioni */
EVENTO Evento;                /* Evento */
PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */
char szDateBuffer[128];
char szTimeBuffer[128];
char szActualTableName[128];  /* selezione per tabella a video */
char szActualKey[128];  /* chiave per tabella a video */

/*
* Connessione al DataBase PostgreSQL
*/
PGconn *DBConn;

/* widgets utilizzati nell'applicazione */
GtkWidget *main_window;
GtkWidget *txt_msgs;

/*
* Definizioni costanti colore
*/
GdkColor cLBLUE    = {0, 0x0000, 0x0000, 0xffff};
GdkColor cBLUE     = {0, 0x0000, 0x0000, 0x8000};
GdkColor cBLACK    = {0, 0x0000, 0x0000, 0x0000};
GdkColor cRED      = {0, 0x8000, 0x0000, 0x0000};
GdkColor cLRED     = {0, 0xffff, 0x0000, 0x0000};
GdkColor cLCYAN    = {0, 0xbb00, 0xccee, 0xffff};
GdkColor cLICE     = {0, 0xbefb, 0xc71b, 0xffff};
GdkColor cCYAN     = {0, 0x6000, 0xBB00, 0xFFFF};
GdkColor cWHITE    = {0, 0xffff, 0xffff, 0xffff};
GdkColor cYELLOW   = {0, 0xffff, 0x8888, 0x0000};
GdkColor cLYELLOW  = {0, 0xffff, 0xffff, 0xcccc};
GdkColor cLGREEN   = {0, 0x0000, 0xBB00, 0x0000};
GdkColor cGREEN    = {0, 0x0000, 0x8000, 0x0000};
GdkColor cGREYCYAN = {0, 0xdd00, 0xdd00, 0xee00};
GdkColor cLGREY    = {0, 0xcc00, 0xcc00, 0xcc00};
GdkColor cGREY     = {0, 0x8800, 0x8800, 0x8800};
GdkColor cSLGREY   = {0, 0xd75c, 0xd75c, 0xd75c};
GdkColor cSGREY    = {0, 0xc71b, 0xc30b, 0xc71b};
GdkColor cORANGE   = {0, 0x9b00, 0x7300, 0x0000};
