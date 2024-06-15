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

    {"General Settings", "PathData",                       CFG_TYPE_STRING, {pszString:Cfg.szPathData},                {szString:"../data"},                  CFG_ENTRY_WIDGET, ""},
    {"General Settings", "PathExe",                        CFG_TYPE_STRING, {pszString:Cfg.szPathExe},                 {szString:"../exe"},                   CFG_ENTRY_WIDGET, ""},
    {"General Settings", "PathStampe",                     CFG_TYPE_STRING, {pszString:Cfg.szPathStampe},              {szString:"../stampe"},                CFG_ENTRY_WIDGET, ""},
    {"General Settings", "PathTrace",                      CFG_TYPE_STRING, {pszString:Cfg.szPathTrace},               {szString:"../trace"},                 CFG_NO_WIDGET, ""},
    {"General Settings", "PrinterConsole",                 CFG_TYPE_STRING, {pszString:Cfg.szPrinterConsole},          {szString:"lp"},                       CFG_ENTRY_WIDGET, ""},
    {"General Settings", "PrinterBilancia",                CFG_TYPE_STRING, {pszString:Cfg.szPrinterBilancia},         {szString:"lp"},                       CFG_ENTRY_WIDGET, ""},
    {"General Settings", "LabelPrinterConsole",            CFG_TYPE_STRING, {pszString:Cfg.szLabelPrinterConsole},     {szString:"lp"},                       CFG_ENTRY_WIDGET, ""},
    {"General Settings", "LabelPrinterBilancia",           CFG_TYPE_STRING, {pszString:Cfg.szLabelPrinterBilancia},    {szString:"lp"},                       CFG_ENTRY_WIDGET, ""},
    {"General Settings", "ImportMonitor",                  CFG_TYPE_STRING, {pszString:Cfg.szImportMonitor},           {szString:"import.mon"},               CFG_ENTRY_WIDGET, ""},
    {"General Settings", "ExportMonitor",                  CFG_TYPE_STRING, {pszString:Cfg.szExportMonitor},           {szString:"export.mon"},               CFG_ENTRY_WIDGET, ""},
                                                                                                                                                         
    {"DataBase Settings", "PGHost",                        CFG_TYPE_STRING, {pszString:Cfg.szPGHost},                  {szString:"localhost"},                CFG_ENTRY_WIDGET, ""},
    {"DataBase Settings", "PGPort",                        CFG_TYPE_STRING, {pszString:Cfg.szPGPort},                  {szString:"5432"},                     CFG_ENTRY_WIDGET, ""},
    {"DataBase Settings", "PGDataBase",                    CFG_TYPE_STRING, {pszString:Cfg.szPGDataBase},              {szString:"picking"},                  CFG_ENTRY_WIDGET, ""},
                                                                                                                                                         
    {"General Settings",  "DebugLevel",                    CFG_TYPE_INT,    {pnInt:&Cfg.nDebugLevel},                  {nInt:1},                              CFG_SPIN_BUTTON_WIDGET, ""},
    {"General Settings",  "TipoSelezione",                 CFG_TYPE_INT,    {pnInt:&Cfg.nTipoSelezione},               {nInt:GTK_SELECTION_MULTIPLE},         CFG_NO_WIDGET, ""},
    {"General Settings",  "PrintErrorPage",                CFG_TYPE_INT,    {pnInt:&Cfg.nPrintErrorPage},              {nInt:0},                              CFG_NO_WIDGET, ""},

    {"PLC",  "PLC",                                        CFG_TYPE_INT,    {pnInt:&Cfg.nPlc},                         {nInt:1},                              CFG_NO_WIDGET, ""},
    {"PLC",  "PLCService",                                 CFG_TYPE_INT,    {pnInt:&Cfg.nPLCService},                  {nInt:8039},                           CFG_NO_WIDGET, ""},
    {"PLC",  "PLCHost",                                    CFG_TYPE_STRING, {pszString:Cfg.szPLCHost},                 {szString:"localhost"},                CFG_NO_WIDGET, ""},

    {"fine_linea",        "Delay",                         CFG_TYPE_INT,    {pnInt:&Cfg.nMainDelay},                   {nInt:   100},                         CFG_NO_WIDGET, ""},
    {"fine_linea",        "scanner_port",                  CFG_TYPE_INT,    {pnInt:&Cfg.nScannerPort},                 {nInt:  8051},                         CFG_NO_WIDGET, ""},
    {"fine_linea",        "bilancia_port",                 CFG_TYPE_INT,    {pnInt:&Cfg.nBilanciaPort},                {nInt:  8052},                         CFG_NO_WIDGET, ""},
    {"fine_linea",        "DSN",                           CFG_TYPE_STRING, {pszString:Cfg.DSN},                       {szString:"DSN"},                      CFG_NO_WIDGET, ""},
    {"fine_linea",        "user_name",                     CFG_TYPE_STRING, {pszString:Cfg.user_name},                 {szString:"user_name"},                CFG_NO_WIDGET, ""},
    {"fine_linea",        "pwd",                           CFG_TYPE_STRING, {pszString:Cfg.pwd},                       {szString:"pwd"},                      CFG_NO_WIDGET, ""},
    {"fine_linea",        "simulation",                    CFG_TYPE_INT,    {pnInt:&Cfg.nSimulation},                  {nInt:  0},                            CFG_NO_WIDGET, ""},

    {NULL,NULL,CFG_TYPE_NONE,{pszString:(char *)NULL},{szString:(char *)NULL}}
};

