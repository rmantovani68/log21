/*
* volext.h
* 
* Progetto Easy Picking 2.0 
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#pragma once

#include "cmp_time.h"

extern int nPID;
extern CFGSTRUCT	Cfg;

extern int nAllMsg;
extern int nCarIn;
extern ep_msg_header_t InMsgStruct;
extern char szInMsg[INFO_LEN];
extern char szText[INFO_LEN];

extern char szBufTrace[80];
extern char *pszNomeModulo;
extern char szTimeBuffer[80];
extern char szDateBuffer[80];

extern PLINEA_STRUCT pDatiLinea;     /* Dati di gestione linea */
extern IMBALLI Imballi;              /* Imballi */
