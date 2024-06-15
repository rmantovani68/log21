/*
* mainvar.c
* Variabili Globali
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#include <gtk/gtk.h>
#include <glib.h>

#include <ep-common.h>

#include <proc_list.h>
#include <picking.h>
#include "mainstruct.h"

CFGSTRUCT Cfg;

STRINGA_EV2002 stringa_ev2002;
ep_bool_t bColloPresente;
COLLO_ORDINE Collo;

CFGITEM CFGItems[] =  {

    {"General Settings", "CDSOC",                          CFG_TYPE_STRING, {pszString:Cfg.szCDSOC},                   {szString:"AM"},                       CFG_NO_WIDGET,       ""},
    {"General Settings", "CDMAG",                          CFG_TYPE_STRING, {pszString:Cfg.szCDMAG},                   {szString:"0000015"},                  CFG_NO_WIDGET,       ""},
    {"General Settings", "TipoOrdini",                     CFG_TYPE_STRING, {pszString:Cfg.szTipoOrdini},              {szString:"N"},                        CFG_NO_WIDGET,       ""},
    {"General Settings", "TipoUbicazioni",                 CFG_TYPE_STRING, {pszString:Cfg.szTipoUbicazioni},          {szString:"N"},                        CFG_NO_WIDGET,       ""},
    {"General Settings", "tmp_suffix",                     CFG_TYPE_STRING, {pszString:Cfg.szTmpSuffix},               {szString:"tmp"},                      CFG_NO_WIDGET,       ""},
    {"General Settings", "PathData",                       CFG_TYPE_STRING, {pszString:Cfg.szPathData},                {szString:"../data"},                  CFG_ENTRY_WIDGET,    "entry_path_data"},
    {"General Settings", "PathExport",                     CFG_TYPE_STRING, {pszString:Cfg.szPathExport},              {szString:"../export"},                CFG_ENTRY_WIDGET,    "entry_path_export  "},
    {"General Settings", "PathCorriereExport",             CFG_TYPE_STRING, {pszString:Cfg.szPathCorriereExport},      {szString:"../export"},                CFG_NO_WIDGET,       ""},
    {"General Settings", "PathExe",                        CFG_TYPE_STRING, {pszString:Cfg.szPathExe},                 {szString:"../exe"},                   CFG_ENTRY_WIDGET,    "entry_path_exe"},
    {"General Settings", "PathStampe",                     CFG_TYPE_STRING, {pszString:Cfg.szPathStampe},              {szString:"../stampe"},                CFG_ENTRY_WIDGET,    "entry_path_stampe  "},
    {"General Settings", "PathStorico",                    CFG_TYPE_STRING, {pszString:Cfg.szPathStorico},             {szString:"../storico"},               CFG_ENTRY_WIDGET,    "entry_path_storico "},
    {"General Settings", "PathTrace",                      CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},               {szString:"../trace"},                 CFG_NO_WIDGET,       ""},
    {"General Settings", "PrinterConsole",                 CFG_TYPE_STRING, {pszString:Cfg.szPrinterConsole},          {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_printer_console"},
    {"General Settings", "LabelPrinterConsole",            CFG_TYPE_STRING, {pszString:Cfg.szLabelPrinterConsole},     {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_label_printer_console "},
    {"General Settings", "TPSPE",                          CFG_TYPE_STRING, {pszString:Cfg.szTPSPE},                   {szString:""},                         CFG_NO_WIDGET,       ""},
    {"General Settings", "DSTSP",                          CFG_TYPE_STRING, {pszString:Cfg.szDSTSP},                   {szString:""},                         CFG_NO_WIDGET,       ""},
    {"General Settings", "Password",                       CFG_TYPE_STRING, {pszString:Cfg.szPassword},                {szString:"delta"},                    CFG_NO_WIDGET,       ""},
                                                                                                                                                         
    {"DataBase Settings", "DBHost",                        CFG_TYPE_STRING, {pszString:Cfg.szDBHost},                  {szString:"localhost"},                CFG_ENTRY_WIDGET,    "entry_pghost"},
    {"DataBase Settings", "DBPort",                        CFG_TYPE_STRING, {pszString:Cfg.szDBPort},                  {szString:"5432"},                     CFG_ENTRY_WIDGET,    "entry_pgport"},
    {"DataBase Settings", "DBName",                        CFG_TYPE_STRING, {pszString:Cfg.szDBName},                  {szString:"picking"},                  CFG_ENTRY_WIDGET,    "entry_pgdatabase"},
	{"DataBase Settings", "DBUser",                        CFG_TYPE_STRING, {pszString:Cfg.szDBUser},                  {szString:"roberto"},                  CFG_NO_WIDGET,         ""},
	{"DataBase Settings", "DBPassword",                    CFG_TYPE_STRING, {pszString:Cfg.szDBPassword},              {szString:"3zin"},                     CFG_NO_WIDGET,         ""},

    {"PLC",               "PLC",                           CFG_TYPE_INT,    {pnInt:&Cfg.nPLC},                         {nInt:0},                              CFG_NO_WIDGET,       ""},        
    {"PLC",               "PLCService",                    CFG_TYPE_INT,    {pnInt:&Cfg.nPLCService},                  {nInt:8039},                           CFG_NO_WIDGET,       ""},        
    {"PLC",               "PLCHost",                       CFG_TYPE_STRING, {pszString:Cfg.szPLCHost},                 {szString:"PLCHost"},                  CFG_NO_WIDGET,       ""},

    {"Gestione Ordini",   "TipoStampe",                    CFG_TYPE_INT,    {pnInt:&Cfg.nTipoStampe            },    {nInt:PRINT_ASCII},                      CFG_NO_WIDGET,            ""},        
    {"General Settings",  "DebugLevel",                    CFG_TYPE_INT,    {pnInt:&Cfg.nDebugLevel            },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_debug_level"},
    {"General Settings",  "ServicePort",                   CFG_TYPE_INT,    {pnInt:&Cfg.nServicePort           },    {nInt:  8050},                           CFG_NO_WIDGET,            ""},        
    {"General Settings",  "a2ps",                          CFG_TYPE_INT,    {pnInt:&Cfg.nA2ps                  },    {nInt:     0},                           CFG_TOGGLE_BUTTON_WIDGET, "cb_a2ps"},        
    {"General Settings",  "ControllaCedola",               CFG_TYPE_BOOL,   {pbBool:&Cfg.bControllaCedola      },    {bBool:    0},                           CFG_NO_WIDGET,            ""},

    {"General Settings",  "TipoSelezione",                 CFG_TYPE_INT,    {pnInt:&Cfg.nTipoSelezione         },    {nInt:     GTK_SELECTION_MULTIPLE},      CFG_NO_WIDGET,            ""},        

    {"check_bancali",     "Delay",                         CFG_TYPE_INT,    {pnInt:&Cfg.nMainDelay             },    {nInt:   100},                           CFG_NO_WIDGET,            ""},        
                                                        
    {NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}}
};

