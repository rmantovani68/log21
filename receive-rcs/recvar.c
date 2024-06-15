/*
* Modulo : recvar.c
* -----------------
* Modulo contenente la dichiarazione delle variabili globali utilizzate
*
* Data creazione 17-05-2001
*
* Autore : Roberto Mantovani
*
* Copyright SMD srl 2002
*/
#include <glib.h>

#include <stdio.h>

#include <ep-common.h>
#include <ep-db.h>
#include <shared.h>
#include <pmx_msq.h>

#include <proc_list.h>
#include <picking.h>

#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"

CFGSTRUCT Cfg;
int nPID=0;                   /* Process ID */
PROCESSO ProcList[NUM_PROC+1];
PLINEA_STRUCT pLinea;
ep_bool_t bExitRequest=FALSE;

char szTimeBuffer[80];
char szDateBuffer[80];

/*
* Definizioni per le tabelle utilizzate in IMPORT/EXPORT
*/
DBSTRUCT tRicOrd;     /* Ordini Ricevuti */
DBSTRUCT tRicArt;     /* Righe Ricevute */
DBSTRUCT tCatalogo;   /* Catalogo Prodotti */


CFGITEM CFGItems[] =  {
	{"General Settings","ProvenienzaOrdini", CFG_TYPE_STRING,{pszString:Cfg.szProvenienzaOrdini},  {szString:"C"},         CFG_NO_WIDGET, ""},
	{"General Settings","TipoOrdini",        CFG_TYPE_STRING,{pszString:Cfg.szTipoOrdini},         {szString:"C"},         CFG_NO_WIDGET, ""},
	{"General Settings","TipoUbicazioni",    CFG_TYPE_STRING,{pszString:Cfg.szTipoUbicazioni},     {szString:"C"},         CFG_NO_WIDGET, ""},
	{"General Settings","PathData",          CFG_TYPE_STRING,{pszString:Cfg.szPathData},           {szString:"../data"},   CFG_NO_WIDGET, ""},
	{"General Settings","PathExport",        CFG_TYPE_STRING,{pszString:Cfg.szPathExport},         {szString:"../export"}, CFG_NO_WIDGET, ""},
	{"General Settings","PathTrace",         CFG_TYPE_STRING,{pszString:Cfg.szPathTrace},          {szString:"../trace"},  CFG_NO_WIDGET, ""},
                                           
	{"General Settings","ImportMonitor",     CFG_TYPE_STRING,{pszString:Cfg.szImportMonitor},      {szString:""},          CFG_NO_WIDGET, "import.mon"},
	{"General Settings","ImportSpedizioni",  CFG_TYPE_STRING,{pszString:Cfg.szImportSpedizioni},   {szString:""},          CFG_NO_WIDGET, "import.txt"},
	{"General Settings","ImportRighe",       CFG_TYPE_STRING,{pszString:Cfg.szImportRighe},        {szString:""},          CFG_NO_WIDGET, "import.txt"},
	{"General Settings","ImportCatalogo",    CFG_TYPE_STRING,{pszString:Cfg.szImportCatalogo},     {szString:""},          CFG_NO_WIDGET, "import.txt"},

	{"DataBase Settings","DBHost",           CFG_TYPE_STRING,{pszString:Cfg.szDBHost},             {szString:"localhost"}, CFG_NO_WIDGET, ""},
	{"DataBase Settings","DBPort",           CFG_TYPE_STRING,{pszString:Cfg.szDBPort},             {szString:"5432"},      CFG_NO_WIDGET, ""},
	{"DataBase Settings","DBName",           CFG_TYPE_STRING,{pszString:Cfg.szDBName},             {szString:"database"},  CFG_NO_WIDGET, ""},
                                           
	{"receive","DebugLevel",     CFG_TYPE_INT,   {pnInt:&Cfg.nDebugLevel},           {nInt:0},               CFG_NO_WIDGET, ""},
	{"receive","DelayRicezione",   CFG_TYPE_INT,   {pnInt:&Cfg.nDelayRicezione},         {nInt:30},              CFG_NO_WIDGET, ""},
	{"receive","MainID",           CFG_TYPE_INT,   {pnInt:&Cfg.nMainID},                 {nInt:0},               CFG_NO_WIDGET, ""},

	{NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}},
};

