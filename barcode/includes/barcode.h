/*
* barcode.h : gestione Barcode Reader
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

/* Inizio dichiarazione delle costanti */

#define TIMEOUT							4
#define TIMEOUT_WITHIN			4
#define TIMEOUT_BEFORE			4
#define FETCH_DATA_OFFSET		4

#define CAN_CHAR            '\x18'

#ifdef DOCUMENTAZIONE
struct msg_spec {
	char *msg_start;      /* message start string */
	int start_len;        /* start length to be checked exactly */
	int do_start_check;   /* check only for start char (but < start_len) */
	int start_char;       /* >0: start char to be checked for */
	int end_len;          /* >0: end length to be checked exactly */
	int do_end_check;     /* >0: check only for end char (but < end_len) */
	int end_char;         /* end char to be checked for (if any) */
	int do_checksum;      /* >0: add/read checksum after "end_char" */
	int do_checklen;      /* add/read check length just before "end_char" */
};
typedef struct msg_spec MSG_SPEC;
#endif


/* Messaggi ricevibili dal lettore */
MSG_SPEC msgRcv[] = {
	{"\x02", 1, 0, 0, 100, 1, 0x0d0a, 0, 0},      /* STX dati CRLF */
	{"\x02\x18\x0d\x0a", 4, 0, 0, 0, 0, 0, 0, 0}, /* CAN	- NOREAD */
	{"00 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 00 */
	{"01 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 01 */
	{"02 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 02 */
	{"03 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 03 */
	{"04 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 04 */
	{"05 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 05 */
	{"06 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 06 */
	{"07 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 07 */
	{"08 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 08 */
	{"09 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 09 */
	{"10 ", 3, 0, 0, 0, 0, 0, 0, 0},      /* 10 */
	{"001\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 001 */
	{"002\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 002 */
	{"003\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 003 */
	{"004\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 004 */
	{"005\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 005 */
	{"006\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 006 */
	{"007\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 007 */
	{"008\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 008 */
	{"009\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 009 */
	{"010\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 010 */
	{"011\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 011 */
	{"012\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 012 */
	{"013\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 013 */
	{"014\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 014 */
	{"015\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 015 */
	{"016\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 016 */
	{"017\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 017 */
	{"018\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 018 */
	{"019\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 019 */
	{"020\x0d\x0a", 5, 0, 0, 0, 0, 0, 0, 0},      /* 020 */
	{NULL,   0, 0, 0, 100, 1, 0x0d0a, 0, 0},      /* <TEXT><CR><LF> */
};

enum MsgRcv {
	BCRD_RCV_BARCODE,
	BCRD_RCV_NOREAD,
	BCRD_RCV_00,
	BCRD_RCV_01,
	BCRD_RCV_02,
	BCRD_RCV_03,
	BCRD_RCV_04,
	BCRD_RCV_05,
	BCRD_RCV_06,
	BCRD_RCV_07,
	BCRD_RCV_08,
	BCRD_RCV_09,
	BCRD_RCV_10,
	BCRD_RCV_001,
	BCRD_RCV_002,
	BCRD_RCV_003,
	BCRD_RCV_004,
	BCRD_RCV_005,
	BCRD_RCV_006,
	BCRD_RCV_007,
	BCRD_RCV_008,
	BCRD_RCV_009,
	BCRD_RCV_010,
	BCRD_RCV_011,
	BCRD_RCV_012,
	BCRD_RCV_013,
	BCRD_RCV_014,
	BCRD_RCV_015,
	BCRD_RCV_016,
	BCRD_RCV_017,
	BCRD_RCV_018,
	BCRD_RCV_019,
	BCRD_RCV_020,
	BCRD_RCV_BARCODE_GRIPHON,

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
	int nMainID;
} CFGSTRUCT, *PCFGSTRUCT;

/*
* Inizio dichiarazione dei prototipi
*/
void ReadConfiguration(void);
void SafeIntFunc();
void RxLoop(void);
