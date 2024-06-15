/*
* ev2002.h : gestione Bilancia EV2002
* 
* Progetto Mondadori Picking - Gestione pesatura bancali
*
* Autore : Roberto Mantovani - Stefano Tarroni
*
* Copyright A&L srl 2003-2021
*/
#pragma once

#include "cmp_time.h"


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

#ifdef DOC
struct msg_spec {
	char *msg_start;		/* message start string */
	int start_len;			/* start length to be checked exactly */
	int do_start_check;		/* check only for start char (but < start_len) */
	int start_char;			/* >0: start char to be checked for */
	int end_len;			/* >0: end length to be checked exactly */
	int do_end_check;		/* >0: check only for end char (but < end_len) */
	int end_char;			/* end char to be checked for (if any) */
	int do_checksum;		/* >0: add/read checksum after "end_char" */
	int do_checklen;		/* add/read check length just before "end_char" */
};
#endif

/* 
* Messaggi ricevibili dal processo di gestione bilancia e 
* relativo simulatore 
*/
MSG_SPEC msgRcv[] = {
	{"\x24", 1, 0, 0, 30, 1, 0x0a0d, 0, 0},
};
enum MsgRcv {
	EV2002_RCV_DATA,

	NUM_MSG_RCV,
};

typedef struct tagCfgStruct{
	char szPathTrace[128];
	char szCommDevice[80];
	int nBaudrate;
	int nParity;
	int nDataBits;
	int nStopBits;
	int nPortFD;
	int nDelay;
	int nSimulazione;
} CFGSTRUCT, *PCFGSTRUCT;

/*
* Inizio dichiarazione dei prototipi
*/
void SafeIntFunc();
void RxLoop(void);
void ReadConfiguration(void);
