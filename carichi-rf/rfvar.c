/*
* Modulo : rfvar.c
* -----------------
* Modulo contenente la dichiarazione delle variabili globali utilizzate
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19/06/2000
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1996-2000
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <ep-common.h>
#include <picking.h>
#include <proc_list.h>
#include <ep-db.h>

#include <ncurses.h>
#include <panel.h>
#include <form.h>
#include <menu.h>

#include <dbfun-deltacarichi.h>

#include "rf.h"
#include "rfstruct.h"
#include "rfext.h"
#include "rffun.h"


int nProgramStatus;
int nOldProgramStatus;
char cTipoForzatura;
char cOldFlag;

CFGSTRUCT Cfg;
MACCHINA Macchina;
OPERATORE Operatore;

char szTimeBuffer[80];
char szDateBuffer[80];
int nRowEdit;
int nColEdit;

int SCREEN_WIDTH=16;
int SCREEN_HEIGHT=4;

short tasto;

CFGITEM CFGItems[] =  {
	{"General Settings",  "DebugLevel",          CFG_TYPE_INT,    {pnInt:&Cfg.nDebugLevel},             {nInt:0},                  CFG_SPIN_BUTTON_WIDGET,   "sb_debug_version"},
	{"General Settings", "PathData",              CFG_TYPE_STRING, {pszString:Cfg.szPathData},             {szString:"../data"},      CFG_ENTRY_WIDGET,         "entry_path_data"},
	{"General Settings", "PathExe",               CFG_TYPE_STRING, {pszString:Cfg.szPathExe},              {szString:"../exe"},       CFG_ENTRY_WIDGET,         "entry_path_exe"},
	{"General Settings", "PathExport",            CFG_TYPE_STRING, {pszString:Cfg.szPathExport},           {szString:"../export"},    CFG_ENTRY_WIDGET,         "entry_path_export"},
	{"General Settings", "PathStampe",            CFG_TYPE_STRING, {pszString:Cfg.szPathStampe},           {szString:"../stampe"},    CFG_ENTRY_WIDGET,         "entry_path_stampe"},
	{"General Settings", "PathTrace",             CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},            {szString:"../trace"},     CFG_NO_WIDGET,            ""},
	{"General Settings", "PathStorico",           CFG_TYPE_STRING, {pszString:Cfg.szPathStorico},          {szString:"../storico"},   CFG_NO_WIDGET,            ""},

	{"DataBase Settings","DBHost",                CFG_TYPE_STRING, {pszString:Cfg.szDBHost},               {szString:"localhost"},    CFG_ENTRY_WIDGET,         "entry_db_host"},
	{"DataBase Settings","DBPort",                CFG_TYPE_STRING, {pszString:Cfg.szDBPort},               {szString:"5432"},         CFG_ENTRY_WIDGET,         "entry_db_port"},
	{"DataBase Settings","DBName",                CFG_TYPE_STRING, {pszString:Cfg.szDBName},               {szString:"database"},     CFG_ENTRY_WIDGET,         "entry_db_name"},
	{"DataBase Settings","DBUser",                CFG_TYPE_STRING, {pszString:Cfg.szDBUser},               {szString:"user"},         CFG_ENTRY_WIDGET,         "entry_db_user"},
	{"DataBase Settings","DBPassword",            CFG_TYPE_STRING, {pszString:Cfg.szDBPassword},           {szString:"pwd"},          CFG_ENTRY_WIDGET,         "entry_db_password"},
	{NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}}
};
