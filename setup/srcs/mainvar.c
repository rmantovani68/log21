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

CFGITEM CFGItems[] =  {
    {"General Settings", "CDSOC",                          CFG_TYPE_STRING, {pszString:Cfg.szCDSOC},                   {szString:"AM"},                       CFG_NO_WIDGET,       ""},
    {"General Settings", "CDMAG",                          CFG_TYPE_STRING, {pszString:Cfg.szCDMAG},                   {szString:"0000015"},                  CFG_NO_WIDGET,       ""},
    {"General Settings", "TipoOrdini",                     CFG_TYPE_STRING, {pszString:Cfg.szTipoOrdini},              {szString:"C"},                        CFG_NO_WIDGET,       ""},
    {"General Settings", "tmp_suffix",                     CFG_TYPE_STRING, {pszString:Cfg.szTmpSuffix},               {szString:"tmp"},                      CFG_NO_WIDGET,       ""},
    {"General Settings", "PathData",                       CFG_TYPE_STRING, {pszString:Cfg.szPathData},                {szString:"../data"},                  CFG_ENTRY_WIDGET,    "entry_path_data"},
    {"General Settings", "PathExport",                     CFG_TYPE_STRING, {pszString:Cfg.szPathExport},              {szString:"../export"},                CFG_ENTRY_WIDGET,    "entry_path_export  "},
    {"General Settings", "PathCorriereExport",             CFG_TYPE_STRING, {pszString:Cfg.szPathCorriereExport},      {szString:"../export"},                CFG_NO_WIDGET,       ""},
    {"General Settings", "PathExe",                        CFG_TYPE_STRING, {pszString:Cfg.szPathExe},                 {szString:"../exe"},                   CFG_ENTRY_WIDGET,    "entry_path_exe"},
    {"General Settings", "PathStampe",                     CFG_TYPE_STRING, {pszString:Cfg.szPathStampe},              {szString:"../stampe"},                CFG_ENTRY_WIDGET,    "entry_path_stampe  "},
    {"General Settings", "PathStorico",                    CFG_TYPE_STRING, {pszString:Cfg.szPathStorico},             {szString:"../storico"},               CFG_ENTRY_WIDGET,    "entry_path_storico "},
    {"General Settings", "PathTrace",                      CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},               {szString:"../trace"},                 CFG_NO_WIDGET,       ""},
    {"General Settings", "PrinterConsole",                 CFG_TYPE_STRING, {pszString:Cfg.szPrinterConsole},          {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_printer_console"},
    {"General Settings", "PrinterBilancia",                CFG_TYPE_STRING, {pszString:Cfg.szPrinterBilancia},         {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_printer_bilancia"},
    {"General Settings", "LabelPrinterConsole",            CFG_TYPE_STRING, {pszString:Cfg.szLabelPrinterConsole},     {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_label_printer_console "},
    {"General Settings", "LabelPrinterBilancia",           CFG_TYPE_STRING, {pszString:Cfg.szLabelPrinterBilancia},    {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_label_printer_bilancia"},
    {"General Settings", "XABPrinterConsole",              CFG_TYPE_STRING, {pszString:Cfg.szXABPrinterConsole},       {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_xab_printer"},
    {"General Settings", "DISTPrinterConsole",             CFG_TYPE_STRING, {pszString:Cfg.szDISTPrinterConsole},      {szString:"lp"},                       CFG_ENTRY_WIDGET,    "entry_dist_printer"},
    {"General Settings", "PrinterSettori",                 CFG_TYPE_STRING, {pszString:Cfg.szPrinterSettori},          {szString:"lp"},                       CFG_NO_WIDGET,       ""},
    {"General Settings", "ImportMonitor",                  CFG_TYPE_STRING, {pszString:Cfg.szImportMonitor},           {szString:"import.mon"},               CFG_ENTRY_WIDGET,    "entry_import_monitor"},
    {"General Settings", "ExportMonitor",                  CFG_TYPE_STRING, {pszString:Cfg.szExportMonitor},           {szString:"export.mon"},               CFG_ENTRY_WIDGET,    "entry_export_monitor"},
    {"General Settings", "ImportSpedizioni",               CFG_TYPE_STRING, {pszString:Cfg.szImportSpedizioni},        {szString:"import_spedizioni.txt"},    CFG_ENTRY_WIDGET,    "entry_import_spedizioni"},
    {"General Settings", "ImportCatalogo",                 CFG_TYPE_STRING, {pszString:Cfg.szImportCatalogo},          {szString:"import_catalogo.txt"},      CFG_ENTRY_WIDGET,    "entry_import_catalogo"},
    {"General Settings", "ImportTabelle",                  CFG_TYPE_STRING, {pszString:Cfg.szImportTabelle},           {szString:"import_tabelle.txt"},       CFG_ENTRY_WIDGET,    "entry_import_tabelle"},
    {"General Settings", "ExportSpedizioni",               CFG_TYPE_STRING, {pszString:Cfg.szExportSpedizioni},        {szString:"export_spedizioni.txt"},    CFG_ENTRY_WIDGET,    "entry_export_spedizioni"},
    {"General Settings", "ExportAnticipato",               CFG_TYPE_STRING, {pszString:Cfg.szExportAnticipato},        {szString:"export_anticipato.txt"},    CFG_NO_WIDGET,       ""},
    {"General Settings", "ExportRighe",                    CFG_TYPE_STRING, {pszString:Cfg.szExportRighe},             {szString:"export_righe.txt"},         CFG_NO_WIDGET,       ""},
    {"General Settings", "ExportFlussoCedola",             CFG_TYPE_STRING, {pszString:Cfg.szExportFlussoCedola},      {szString:"flusso_cedola"},            CFG_NO_WIDGET,       ""},
    {"General Settings", "ExportCorriere",                 CFG_TYPE_STRING, {pszString:Cfg.szExportCorriere},          {szString:"corriere.txt"},             CFG_ENTRY_WIDGET,    "entry_export_corriere"},
    {"General Settings", "ExportCorrAgg",                  CFG_TYPE_STRING, {pszString:Cfg.szExportCorrAgg},           {szString:"corr_agg.txt"},             CFG_ENTRY_WIDGET,    "entry_export_corr_agg"},
    {"General Settings", "ExportCorrSDA",                  CFG_TYPE_STRING, {pszString:Cfg.szExportCorrSDA},           {szString:"corr_sda.txt"},             CFG_ENTRY_WIDGET,    "entry_export_corr_sda"},
    {"General Settings", "NotaCorriere",                   CFG_TYPE_STRING, {pszString:Cfg.szNotaCorriere},            {szString:"NOTA CORRIERE"},            CFG_NO_WIDGET,       ""},
    {"General Settings", "TPSPE",                          CFG_TYPE_STRING, {pszString:Cfg.szTPSPE},                   {szString:""},                         CFG_NO_WIDGET,       ""},
    {"General Settings", "DSTSP",                          CFG_TYPE_STRING, {pszString:Cfg.szDSTSP},                   {szString:""},                         CFG_NO_WIDGET,       ""},
    {"General Settings", "ExportMonitorStorico",           CFG_TYPE_STRING, {pszString:Cfg.szExportMonitorStorico},    {szString:"export.mon"},               CFG_ENTRY_WIDGET,    "entry_export_monitor_storico"},
    {"General Settings", "ExportSpedizioniStorico",        CFG_TYPE_STRING, {pszString:Cfg.szExportSpedizioniStorico}, {szString:"export.txt"},               CFG_ENTRY_WIDGET,    "entry_export_spedizioni_storico"},
    {"General Settings", "Password",                       CFG_TYPE_STRING, {pszString:Cfg.szPassword},                {szString:"delta"},                    CFG_NO_WIDGET,       ""},
    {"General Settings", "TipoOrdinePreDistinta",          CFG_TYPE_STRING, {pszString:Cfg.szTipoOrdinePreDistinta},   {szString:"X"},                        CFG_NO_WIDGET,       ""},
                                                                                                                                                         
    {"DataBase Settings", "DBHost",                        CFG_TYPE_STRING, {pszString:Cfg.szDBHost},                  {szString:"localhost"},                CFG_ENTRY_WIDGET,    "entry_pghost"},
    {"DataBase Settings", "DBPort",                        CFG_TYPE_STRING, {pszString:Cfg.szDBPort},                  {szString:"5432"},                     CFG_ENTRY_WIDGET,    "entry_pgport"},
    {"DataBase Settings", "DBName",                        CFG_TYPE_STRING, {pszString:Cfg.szDBName},                  {szString:"picking"},                  CFG_ENTRY_WIDGET,    "entry_pgdatabase"},
	{"DataBase Settings", "DBUser",                        CFG_TYPE_STRING, {pszString:Cfg.szDBUser},                  {szString:"roberto"},                  CFG_NO_WIDGET,         ""},
	{"DataBase Settings", "DBPassword",                    CFG_TYPE_STRING, {pszString:Cfg.szDBPassword},              {szString:"3zin"},                     CFG_NO_WIDGET,         ""},
                                                                                                                                                         
    {"General Settings",  "Master",                        CFG_TYPE_BOOL,   {pbBool:&Cfg.bMaster               },    {bBool:    0},                           CFG_NO_WIDGET,       ""},        
    {"General Settings",  "IOSConnected",                  CFG_TYPE_BOOL,   {pbBool:&Cfg.bIOSConnected         },    {bBool:    0},                           CFG_NO_WIDGET,       ""},                
    {"General Settings",  "StampaPackingList",             CFG_TYPE_BOOL,   {pbBool:&Cfg.bStampaPackingList    },    {bBool:    0},                           CFG_NO_WIDGET,       ""},        
    {"General Settings",  "Rifornimenti",                  CFG_TYPE_BOOL,   {pbBool:&Cfg.bRifornimenti         },    {bBool:    0},                           CFG_NO_WIDGET,       ""},        

    {"Gestione Ordini",   "TipoStampe",                    CFG_TYPE_INT,    {pnInt:&Cfg.nTipoStampe            },    {nInt:PRINT_ASCII},                      CFG_NO_WIDGET,            ""},        
    {"General Settings",  "DebugLevel",                  CFG_TYPE_INT,    {pnInt:&Cfg.nDebugLevel          },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_debug_level"},
    {"General Settings",  "ColumnCharWidth",               CFG_TYPE_INT,    {pnInt:&Cfg.nColumnCharWidth       },    {nInt:    10},                           CFG_NO_WIDGET,            ""},        
    {"General Settings",  "ShmKey",                        CFG_TYPE_INT,    {pnInt:&Cfg.nShmKey                },    {nInt:   256},                           CFG_NO_WIDGET,            ""},        
    {"General Settings",  "SettoriShmKey",                 CFG_TYPE_INT,    {pnInt:&Cfg.nSettoriShmKey         },    {nInt:   257},                           CFG_NO_WIDGET,            ""},        
    {"General Settings",  "ServicePort",                   CFG_TYPE_INT,    {pnInt:&Cfg.nServicePort           },    {nInt:  8050},                           CFG_NO_WIDGET,            ""},        
    {"General Settings",  "ChiusuraCollo",                 CFG_TYPE_INT,    {pnInt:&Cfg.nChiusuraCollo         },    {nInt:     1},                           CFG_NO_WIDGET,            "rb_gestione_collo"},        
    {"General Settings",  "ImballiUbicati",                CFG_TYPE_INT,    {pnInt:&Cfg.nImballiUbicati        },    {nInt:     0},                           CFG_NO_WIDGET,            "rb_imballi_ubicati"},        
    {"General Settings",  "StampaRAC",                     CFG_TYPE_INT,    {pnInt:&Cfg.nStampaRAC             },    {nInt:     0},                           CFG_NO_WIDGET,            "rb_all_rac"},        
    {"General Settings",  "a2ps",                          CFG_TYPE_INT,    {pnInt:&Cfg.nA2ps                  },    {nInt:     0},                           CFG_NO_WIDGET,            "cb_a2ps"},        
    {"General Settings",  "Reindex",                       CFG_TYPE_INT,    {pnInt:&Cfg.nReindex               },    {nInt:     0},                           CFG_NO_WIDGET,            ""},        
    {"General Settings",  "SogliaCopiePrelievo",           CFG_TYPE_INT,    {pnInt:&Cfg.nSogliaCopiePre        },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_soglia_prelievo"},        
    {"General Settings",  "SogliaPallet",                  CFG_TYPE_INT,    {pnInt:&Cfg.nSogliaPallet          },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_soglia_pallet"},        
    {"General Settings",  "IncrementoCopiePallet",         CFG_TYPE_INT,    {pnInt:&Cfg.nIncrementoCopiePallet },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_incremento_copie_pallet"},        
    {"General Settings",  "InvioPeriodico",                CFG_TYPE_INT,    {pnInt:&Cfg.nInvioPeriodico        },    {nInt:     0},                           CFG_TOGGLE_BUTTON_WIDGET, "cb_invio_periodico"},        
    {"General Settings",  "OraInvioPeriodico",             CFG_TYPE_INT,    {pnInt:&Cfg.nOraInvioPeriodico     },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_invio_periodico"},        
    {"General Settings",  "ReindexPeriodico",              CFG_TYPE_INT,    {pnInt:&Cfg.nReindexPeriodico      },    {nInt:     0},                           CFG_TOGGLE_BUTTON_WIDGET, "cb_reindex_periodico"},        
    {"General Settings",  "OraReindexPeriodico",           CFG_TYPE_INT,    {pnInt:&Cfg.nOraReindexPeriodico   },    {nInt:     0},                           CFG_SPIN_BUTTON_WIDGET,   "sb_reindex_periodico"},        
    {"General Settings",  "TipoSelezione",                 CFG_TYPE_INT,    {pnInt:&Cfg.nTipoSelezione         },    {nInt:     GTK_SELECTION_MULTIPLE},      CFG_NO_WIDGET,            ""},        

    {"General Settings",  "InvioFileCorriereAutomatico",   CFG_TYPE_INT,    {pnInt:&Cfg.nInvioFileCorriereAutomatico    },    {nInt:     0},                  CFG_NO_WIDGET,            ""},        
    {"General Settings",  "InvioXABMailAutomatico",        CFG_TYPE_INT,    {pnInt:&Cfg.nInvioXABMailAutomatico         },    {nInt:     0},                  CFG_NO_WIDGET,            ""},        
    {"General Settings",  "InvioDistintaMailAutomatico",   CFG_TYPE_INT,    {pnInt:&Cfg.nInvioDistintaMailAutomatico    },    {nInt:     0},                  CFG_NO_WIDGET,            ""},        

    {"Tabella Ordini",    "lista_ordini_key",              CFG_TYPE_INT,    {pnInt:&Cfg.nOrdiniKeyField        },    {nInt:     0},                           CFG_NO_WIDGET,            ""},        

    {"main",              "Delay",                         CFG_TYPE_INT,    {pnInt:&Cfg.nMainDelay             },    {nInt:   100},                           CFG_NO_WIDGET,            ""},        
    {"main",              "InvioMailDelay",                CFG_TYPE_INT,    {pnInt:&Cfg.nInvioMailDelay        },    {nInt: 60000},                           CFG_NO_WIDGET,            ""},        
    {"main",              "SaveStateDelay",                CFG_TYPE_INT,    {pnInt:&Cfg.nSaveStateDelay        },    {nInt: 60000},                           CFG_NO_WIDGET,            ""},        
    {"main",              "StatisticaDelay",               CFG_TYPE_INT,    {pnInt:&Cfg.nStatisticaDelay       },    {nInt: 60000},                           CFG_NO_WIDGET,            ""},        
                                                                                                                   
    {"settori",           "NumeroSettori",                 CFG_TYPE_INT,    {pnInt:&Cfg.nNumeroSettori         },    {nInt:     7},                           CFG_NO_WIDGET,            ""},

    {NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}}
};

