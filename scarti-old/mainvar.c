/*
* mainvar.c
* Gestione Scarti
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
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

#include <pmx_msq.h>
#include <dbfun.h>
#include <plcsock.h>
#include <proc_list.h>
#include <picking.h>

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"


CFGSTRUCT Cfg;
PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */
char szDateBuffer[128];
char szTimeBuffer[128];
char szReadBarcode[128];

/*
* Connessione al DataBase PostgreSQL
*/
PGconn *DBConn;

/*
* widgets utilizzati nell'applicazione
*/
GtkWidget *main_window;
GtkWidget *dlg_trace;
GtkWidget *txt_trace;


/*
* definizione dei colori
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

COLORSTRUCT Colors[] =  {
	{"Blue Chiaro",   {0, 0x0000, 0x0000, 0xffff}},
	{"Blue",          {0, 0x0000, 0x0000, 0x8000}},
	{"Nero",          {0, 0x0000, 0x0000, 0x0000}},
	{"Rosso",         {0, 0x8000, 0x0000, 0x0000}},
	{"Rosso Chiaro",  {0, 0xffff, 0x0000, 0x0000}},
	{"Azzurro",       {0, 0xbb00, 0xccee, 0xffff}},
	{"Ghiaccio",      {0, 0xbefb, 0xc71b, 0xffff}},
	{"Ciano",         {0, 0x6000, 0xBB00, 0xFFFF}},
	{"Bianco",        {0, 0xffff, 0xffff, 0xffff}},
	{"Giallo",        {0, 0xffff, 0x8888, 0x0000}},
	{"Giallo Chiaro", {0, 0xffff, 0xffff, 0xcccc}},
	{"Verde Chiaro",  {0, 0x0000, 0xBB00, 0x0000}},
	{"Verde",         {0, 0x0000, 0x8000, 0x0000}},
	{"Grigio",        {0, 0xdd00, 0xdd00, 0xee00}},
	{"Grigio Chiaro", {0, 0xcc00, 0xcc00, 0xcc00}},
	{"Grigio 1",      {0, 0x8800, 0x8800, 0x8800}},
	{"Grigio 2",      {0, 0xd75c, 0xd75c, 0xd75c}},
	{"Grigio 3",      {0, 0xc71b, 0xc30b, 0xc71b}},
	{"Arancione",     {0, 0x9b00, 0x7300, 0x0000}},
	{"",     {0, 0, 0, 0}}
};
