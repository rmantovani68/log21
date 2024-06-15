/*
* printvar.c 
* 
* Easy Picking 4.0 : Stampe generiche
*
* Autore : Roberto Mantovani / Luca Guerrini
*
* Copyright A&L srl 2003-2021
*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glib.h>

#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <shared.h>
#include <pmx_msq.h>

#include <picking.h>
#include <proc_list.h>


#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"

CFGSTRUCT	Cfg;

char szBufTrace[80];
char *pszNomeModulo;
char szTimeBuffer[80];
char szDateBuffer[80];

CFGITEM CFGItems[] =  {
	{"General Settings","TipoOrdini",       CFG_TYPE_STRING, {pszString:Cfg.szTipoOrdini},        {szString:"L"},                     CFG_NO_WIDGET,   ""},
	{"General Settings","TipoUbicazioni",   CFG_TYPE_STRING, {pszString:Cfg.szTipoUbicazioni},    {szString:"L"},                     CFG_NO_WIDGET,   ""},
	{"General Settings","PathData",         CFG_TYPE_STRING, {pszString:Cfg.szPathData},          {szString:"../data"},               CFG_NO_WIDGET,   ""},
	{"General Settings","PathExe",          CFG_TYPE_STRING, {pszString:Cfg.szPathExe},           {szString:"../exe"},                CFG_NO_WIDGET,   ""},
	{"General Settings","PathTrace",        CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},         {szString:"../trace"},              CFG_NO_WIDGET,   ""},
	{"General Settings","PathLabel",        CFG_TYPE_STRING, {pszString:Cfg.szPathLabel},         {szString:"../label"},              CFG_NO_WIDGET,   ""},
	{"General Settings","PathStampe",       CFG_TYPE_STRING, {pszString:Cfg.szPathStampe},        {szString:"../stampe"},             CFG_NO_WIDGET,   ""},

	{"DataBase Settings","DBHost",          CFG_TYPE_STRING, {pszString:Cfg.szDBHost},            {szString:"localhost"},             CFG_NO_WIDGET,   ""},
	{"DataBase Settings","DBPort",          CFG_TYPE_STRING, {pszString:Cfg.szDBPort},            {szString:"5432"},                  CFG_NO_WIDGET,   ""},
	{"DataBase Settings","DBName",          CFG_TYPE_STRING, {pszString:Cfg.szDBName},            {szString:"database"},              CFG_NO_WIDGET,   ""},
	{"DataBase Settings","DBUser",          CFG_TYPE_STRING, {pszString:Cfg.szDBUser},            {szString:"roberto"},               CFG_NO_WIDGET,         ""},
	{"DataBase Settings","DBPassword",      CFG_TYPE_STRING, {pszString:Cfg.szDBPassword},        {szString:"3zin"},                  CFG_NO_WIDGET,         ""},

	{"DataBase Settings","ConnectionName",  CFG_TYPE_STRING, {pszString:Cfg.szConnectionName},    {szString:"connection"},              CFG_NO_WIDGET,   ""},

	{"printman","DebugLevel",             CFG_TYPE_INT,    {pnInt:&Cfg.nDebugLevel},          {nInt:0},                           CFG_NO_WIDGET,   ""},
	
	{NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}}
};
