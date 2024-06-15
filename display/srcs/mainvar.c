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

/*
* Definizioni costanti colore
*/
GdkColor gLBLUE    = {0, 0x0000, 0x0000, 0xffff};
GdkColor gBLUE     = {0, 0x0000, 0x0000, 0x8000};
GdkColor gBLACK    = {0, 0x0000, 0x0000, 0x0000};
GdkColor gRED      = {0, 0x8000, 0x0000, 0x0000};
GdkColor gLRED     = {0, 0xffff, 0x0000, 0x0000};
GdkColor gLCYAN    = {0, 0xbb00, 0xccee, 0xffff};
GdkColor gLICE     = {0, 0xbefb, 0xc71b, 0xffff};
GdkColor gCYAN     = {0, 0x6000, 0xBB00, 0xFFFF};
GdkColor gWHITE    = {0, 0xffff, 0xffff, 0xffff};
GdkColor gYELLOW   = {0, 0xffff, 0x8888, 0x0000};
GdkColor gLYELLOW  = {0, 0xffff, 0xffff, 0xcccc};
GdkColor gLGREEN   = {0, 0x0000, 0xBB00, 0x0000};
GdkColor gGREEN    = {0, 0x0000, 0x8000, 0x0000};
GdkColor gGREYCYAN = {0, 0xdd00, 0xdd00, 0xee00};
GdkColor gLGREY    = {0, 0xcc00, 0xcc00, 0xcc00};
GdkColor gGREY     = {0, 0x8800, 0x8800, 0x8800};
GdkColor gSLGREY   = {0, 0xd75c, 0xd75c, 0xd75c};
GdkColor gSGREY    = {0, 0xc71b, 0xc30b, 0xc71b};
GdkColor gORANGE   = {0, 0x9b00, 0x7300, 0x0000};

GdkRGBA cLBLUE    = {(float)0x0000 / (float)0xffff, (float)0x0000 / (float)0xffff, (float)0xffff / (float)0xffff, 1.0};
GdkRGBA cBLUE     = {(float)0x0000 / (float)0xffff, (float)0x0000 / (float)0xffff, (float)0x8000 / (float)0xffff, 1.0};
GdkRGBA cBLACK    = {(float)0x0000 / (float)0xffff, (float)0x0000 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};
GdkRGBA cRED      = {(float)0x8000 / (float)0xffff, (float)0x0000 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};
GdkRGBA cLRED     = {(float)0xffff / (float)0xffff, (float)0x0000 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};
GdkRGBA cLCYAN    = {(float)0xbb00 / (float)0xffff, (float)0xccee / (float)0xffff, (float)0xffff / (float)0xffff, 1.0};
GdkRGBA cLICE     = {(float)0xbefb / (float)0xffff, (float)0xc71b / (float)0xffff, (float)0xffff / (float)0xffff, 1.0};
GdkRGBA cCYAN     = {(float)0x6000 / (float)0xffff, (float)0xBB00 / (float)0xffff, (float)0xFFFF / (float)0xffff, 1.0};
GdkRGBA cWHITE    = {(float)0xffff / (float)0xffff, (float)0xffff / (float)0xffff, (float)0xffff / (float)0xffff, 1.0};
GdkRGBA cYELLOW   = {(float)0xffff / (float)0xffff, (float)0x8888 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};
GdkRGBA cLYELLOW  = {(float)0xffff / (float)0xffff, (float)0xffff / (float)0xffff, (float)0xcccc / (float)0xffff, 1.0};
GdkRGBA cLGREEN   = {(float)0x0000 / (float)0xffff, (float)0xBB00 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};
GdkRGBA cGREEN    = {(float)0x0000 / (float)0xffff, (float)0x8000 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};
GdkRGBA cGREYCYAN = {(float)0xdd00 / (float)0xffff, (float)0xdd00 / (float)0xffff, (float)0xee00 / (float)0xffff, 1.0};
GdkRGBA cLGREY    = {(float)0xcc00 / (float)0xffff, (float)0xcc00 / (float)0xffff, (float)0xcc00 / (float)0xffff, 1.0};
GdkRGBA cGREY     = {(float)0x8800 / (float)0xffff, (float)0x8800 / (float)0xffff, (float)0x8800 / (float)0xffff, 1.0};
GdkRGBA cSLGREY   = {(float)0xd75c / (float)0xffff, (float)0xd75c / (float)0xffff, (float)0xd75c / (float)0xffff, 1.0};
GdkRGBA cSGREY    = {(float)0xc71b / (float)0xffff, (float)0xc30b / (float)0xffff, (float)0xc71b / (float)0xffff, 1.0};
GdkRGBA cORANGE   = {(float)0x9b00 / (float)0xffff, (float)0x7300 / (float)0xffff, (float)0x0000 / (float)0xffff, 1.0};

CFGSTRUCT Cfg;

CFGITEM CFGItems[] =  {

    {"General Settings",  "CDSOC",                         CFG_TYPE_STRING, {pszString:Cfg.szCDSOC},                   {szString:"AM"},                       CFG_NO_WIDGET,            ""},
    {"General Settings",  "CDMAG",                         CFG_TYPE_STRING, {pszString:Cfg.szCDMAG},                   {szString:"0000015"},                  CFG_NO_WIDGET,            ""},
    {"General Settings",  "TipoOrdini",                    CFG_TYPE_STRING, {pszString:Cfg.szTipoOrdini},              {szString:"C"},                        CFG_NO_WIDGET,            ""},
    {"General Settings",  "tmp_suffix",                    CFG_TYPE_STRING, {pszString:Cfg.szTmpSuffix},               {szString:"tmp"},                      CFG_NO_WIDGET,            ""},
    {"General Settings",  "PathData",                      CFG_TYPE_STRING, {pszString:Cfg.szPathData},                {szString:"../data"},                  CFG_ENTRY_WIDGET,         "entry_path_data"},
    {"General Settings",  "PathExe",                       CFG_TYPE_STRING, {pszString:Cfg.szPathExe},                 {szString:"../exe"},                   CFG_ENTRY_WIDGET,         "entry_path_exe"},
    {"General Settings",  "PathStampe",                    CFG_TYPE_STRING, {pszString:Cfg.szPathStampe},              {szString:"../stampe"},                CFG_ENTRY_WIDGET,         "entry_path_stampe"},
    {"General Settings",  "PathTrace",                     CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},               {szString:"../trace"},                 CFG_NO_WIDGET,            ""},
	{"General Settings",  "ShmKey",                        CFG_TYPE_INT,    {pnInt:&Cfg.nShmKey},                      {nInt:0},                              CFG_NO_WIDGET,            ""},
	{"General Settings",  "SettoriShmKey",                 CFG_TYPE_INT,    {pnInt:&Cfg.nSettoriShmKey},               {nInt:0},                              CFG_NO_WIDGET,            ""},
	{"General Settings",  "DisplayCfgFileName",            CFG_TYPE_STRING, {pszString:Cfg.szDisplayCfgFileName},      {szString:""},                         CFG_NO_WIDGET,            ""},
                                                                                                                                                          
    {"DataBase Settings", "DBHost",                        CFG_TYPE_STRING, {pszString:Cfg.szDBHost},                  {szString:"localhost"},                CFG_ENTRY_WIDGET,    "entry_pghost"},
    {"DataBase Settings", "DBPort",                        CFG_TYPE_STRING, {pszString:Cfg.szDBPort},                  {szString:"5432"},                     CFG_ENTRY_WIDGET,    "entry_pgport"},
    {"DataBase Settings", "DBName",                        CFG_TYPE_STRING, {pszString:Cfg.szDBName},                  {szString:"picking"},                  CFG_ENTRY_WIDGET,    "entry_pgdatabase"},
	{"DataBase Settings", "DBUser",                        CFG_TYPE_STRING, {pszString:Cfg.szDBUser},                  {szString:"roberto"},                  CFG_NO_WIDGET,         ""},
	{"DataBase Settings", "DBPassword",                    CFG_TYPE_STRING, {pszString:Cfg.szDBPassword},              {szString:"3zin"},                     CFG_NO_WIDGET,         ""},
                                                                                                                                                         
    {"Gestione Ordini",   "TipoStampe",                    CFG_TYPE_INT,    {pnInt:&Cfg.nTipoStampe            },      {nInt:PRINT_ASCII},                    CFG_NO_WIDGET,            ""},        
    {"General Settings",  "DebugLevel",                  CFG_TYPE_INT,    {pnInt:&Cfg.nDebugLevel          },      {nInt:     0},                         CFG_SPIN_BUTTON_WIDGET,   "sb_debug_level"},
    {"General Settings",  "ColumnCharWidth",               CFG_TYPE_INT,    {pnInt:&Cfg.nColumnCharWidth       },      {nInt:    10},                         CFG_NO_WIDGET,            ""},        
    {"General Settings",  "ImballiUbicati",                CFG_TYPE_INT,    {pnInt:&Cfg.nImballiUbicati        },      {nInt:     0},                         CFG_NO_WIDGET,            ""},        
                                                                                                                   
    {"main",              "Delay",                         CFG_TYPE_INT,    {pnInt:&Cfg.nMainDelay             },      {nInt:   100},                         CFG_NO_WIDGET,            ""},        
    {"settori",           "NumeroSettori",                 CFG_TYPE_INT,    {pnInt:&Cfg.nNumeroSettori         },      {nInt:     7},                         CFG_NO_WIDGET,            ""},

    {"netshm",            "MemCachedServerName",           CFG_TYPE_STRING, {pszString:Cfg.szMemCachedServerName},     {szString:"localhost"},                CFG_NO_WIDGET,            ""},
    {"netshm",            "MemCachedPortNumber",           CFG_TYPE_INT,    {pnInt:&Cfg.nMemCachedPortNumber   },      {nInt:     1211},                      CFG_NO_WIDGET,            ""},

    {"TEXT", "TEST",              CFG_TYPE_STRING, {pszString:Cfg.szTEST,           },              {szString:"TEST"            },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "PRELIEVO_IMBALLO",  CFG_TYPE_STRING, {pszString:Cfg.szPRELIEVO_IMBALLO},              {szString:"PRELIEVO IMBALLO"},                  CFG_NO_WIDGET,         ""},
    {"TEXT", "PRELIEVO",          CFG_TYPE_STRING, {pszString:Cfg.szPRELIEVO        },              {szString:"PRELIEVO"        },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "PASSA_SETT_SUCC",   CFG_TYPE_STRING, {pszString:Cfg.szPASSA_SETT_SUCC },              {szString:"PASSA"           },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "CHIUSURA_COLLO",    CFG_TYPE_STRING, {pszString:Cfg.szCHIUSURA_COLLO  },              {szString:"CHIUDI COLLO"    },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "FINE_ORDINE",       CFG_TYPE_STRING, {pszString:Cfg.szFINE_ORDINE     },              {szString:"FINE ORDINE"     },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "OFFLINE",           CFG_TYPE_STRING, {pszString:Cfg.szOFFLINE         },              {szString:"OFFLINE"         },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "START",             CFG_TYPE_STRING, {pszString:Cfg.szSTART           },              {szString:"START"           },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "STOP",              CFG_TYPE_STRING, {pszString:Cfg.szSTOP            },              {szString:"STOP"            },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "ERROR",             CFG_TYPE_STRING, {pszString:Cfg.szERROR           },              {szString:"ERROR"           },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "WAIT",              CFG_TYPE_STRING, {pszString:Cfg.szWAIT            },              {szString:"WAIT"            },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "ORDINE_COLLO",      CFG_TYPE_STRING, {pszString:Cfg.szORDINE_COLLO    },              {szString:"ORDINE / COLLO"  },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "SETTORE",           CFG_TYPE_STRING, {pszString:Cfg.szSETTORE         },              {szString:"SETTORE"         },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "OPERATORE",         CFG_TYPE_STRING, {pszString:Cfg.szOPERATORE       },              {szString:"OPERATORE"       },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "UBICAZIONE",        CFG_TYPE_STRING, {pszString:Cfg.szUBICAZIONE      },              {szString:"UBICAZIONE"      },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "CODICE",            CFG_TYPE_STRING, {pszString:Cfg.szCODICE          },              {szString:"CODICE"          },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "TITOLO",            CFG_TYPE_STRING, {pszString:Cfg.szTITOLO          },              {szString:"TITOLO"          },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "DA_PRELEVARE",      CFG_TYPE_STRING, {pszString:Cfg.szDA_PRELEVARE    },              {szString:"DA PRELEVARE"    },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "PRELEVATI",         CFG_TYPE_STRING, {pszString:Cfg.szPRELEVATI       },              {szString:"PRELEVATI"       },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "ORDINI",            CFG_TYPE_STRING, {pszString:Cfg.szORDINI          },              {szString:"ORDINI"          },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "COLLI",             CFG_TYPE_STRING, {pszString:Cfg.szCOLLI           },              {szString:"COLLI"           },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "RIGHE",             CFG_TYPE_STRING, {pszString:Cfg.szRIGHE           },              {szString:"RIGHE"           },                  CFG_NO_WIDGET,         ""},
    {"TEXT", "COPIE",             CFG_TYPE_STRING, {pszString:Cfg.szCOPIE           },              {szString:"COPIE"           },                  CFG_NO_WIDGET,         ""},

    {NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}}
};


PLINEA_STRUCT pDatiLinea;
PSETTORE pSettori;
SETTORE OldSettore;
int nOldStatoLinea;
int nActualFrame;
