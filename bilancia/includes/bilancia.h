/*
* bilancia.h : gestione Bilancia
* 
* Progetto Mondadori Picking
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#pragma once

#include "cmp_time.h"

#if 0
#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define FALSE	0
#define TRUE 	1
#define BOOL_TYPE_DEFINED
#endif
#endif

#ifndef MIN_MAX_DEFINED
#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))
#endif

#ifndef BYTE_TYPE_DEFINED
typedef unsigned char BYTE;
#define BYTE_TYPE_DEFINED
#endif

#define CH_DLE				0x10
#define CH_STX				0x02
#define CH_ETX				0x03
#define CH_NAK				0x15
#define CH_RET				0x0d

/* Inizio dichiarazione delle costanti */

#define TIMEOUT							4
#define TIMEOUT_WITHIN			0
#define TIMEOUT_BEFORE			4
#define FETCH_DATA_OFFSET		4

/* 
* Messaggi ricevibili dal processo di gestione bilancia e 
* relativo simulatore 
*/
MSG_SPEC msgRcv[] = {
	{"\x24", 1, 0, 0, 22, 1, 0x0a0d, 0, 0},
	{"\x1b\x05", 2, 0, 0, 2, 0, 0, 0, 0},
};
enum MsgRcv {
	ALFA_RCV_DATA,
	ALFA_SIM_RCV_TX_REQ,

	NUM_MSG_RCV,
};

/* 
* Messaggi trasmettibili alla bilancia ALFA  e 
* relativo simulatore 
*/
MSG_SPEC msgSnd[] = {
	{"\x1b\x05", 2, 0, 0, 2, 0, 0, 0, 0},
	{"\x24", 1, 0, 0, 21, 0, 0, 0, 0},
};
enum 
{
	ALFA_SND_REQ,	
	ALFA_SIM_SND_DATA,

	NUM_MSG_SND,
};

enum 
{
	TX_CONTINUE,	
	TX_REQUEST,

	NUM_TX_MODES,
};

typedef struct tagCfgStruct{
	char szPathTrace[128];
	char szTransmitMode[80];
	int nTransmitMode;
	char szCommDevice[80];
	int nBaudrate;
	int nParity;
	int nDataBits;
	int nStopBits;
	int nPortFD;
	int nDelay;
	int nMainID;
} CFGSTRUCT, *PCFGSTRUCT;

/*
* Inizio dichiarazione dei prototipi
*/
void SafeIntFunc();
void RxLoop(void);
void ReadConfiguration(void);
