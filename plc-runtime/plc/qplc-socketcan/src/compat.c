/*
* @(#) compat.c 1.30 Thu Dec  7 16:13:51 CET 2000
*
* Questo modulo contiene definizioni rimaste per compatibilita` con
* "isaker" (sia maledetto il nome suo).
*
* 07/06/96 GG 1.0 Prima stesura.
* 22/07/96 GG 1.1 Corretta la funzione "compat_chkhalt".
* 03/09/96 GG 1.2 Aggiunto un meccanismo per la creazione selettiva di
*             segnali di scambio standard.
* 04/09/96 GG 1.3 Aggiunto il riesame delle variabili condivise trovate
*             gia` definite al lancio.
* 06/09/96 GG 1.4 Aggiunta la funzione "sys_err" per compatibilita` con
*             i programmi BiEsse.
* 10/09/96 GG 1.5 Modificata la sintassi delle funzioni "compat...edge".
*             Aggiunta la funzione "compat_register_edge".
* 11/09/96 GG 1.6 Nuovi segnali di scambio.
* 16/09/96 GG 1.7 Aggiunta segnalazione di errore per fallita definizione dei
*             segnali di scambio associati ai tasti.
* 17/09/96 GG 1.8 Corretta la logica di "START" "vecchio stile". Prima,
*             una singola richiesta di start poteva essere elaborata piu`
*             di una volta.
* 08/10/96 GG 1.9 Aggiornata la tabella dei segnali di scambio.
* 10/10/96 GG 1.10 Aggiunta la funzione "compat_begin_cycle", da chiamare
*             all'inizio del ciclo, dopo "hat". Servono ad aggiustare il valore
*             dei segnali di scambio "RIPCIC", "SOSPCIC", "CLEAR_PLC" e
*             "RESET_PLC" (come faceva isaker).
* 22/10/96 GG 1.11 Aggiunto il controllo sulla configurazione di I/O in
*             stile isaker.
* 29/10/96 GG 1.12 Rovesciata la logica d'uso delle variabili ombra. Ora
*             sono le ombre ad essere associate ai segnali dei tasti, ed
*             i segnali di scambio le destinazioni finali. Questo dovrebbe
*             facilitare la vita a chi usa questi segnali.
* 06/11/96 GG 1.13 Corretta la funzione "compat_start", che non vuotava
*             la tabella dei segnali di scambio non azzerabili. Effetto
*             del buco : al terzo "HALT-GO" smetteva di funzionare il tasto
*             "START".
* 25/11/96 GG 1.14 Utilizzata la nuova funzione di libreria "AttachNvram"
*             per mappare la RAM non volatile. Un piccolo passo per un Plc,
*             un grande passo per l'intero XNC.
* 26/11/96 GG 1.15 Lievi modifiche per inizializzare anche i nuovi campi
*             aggiunti alla struttura di stato globale (nvram...).
* 06/11/96 GG 1.16 Spostate alcune definizioni per facilitare la costruzione
*             della libreria di accesso al Plc.
* 12/12/96 GG 1.17 Iniziati i lavori per il debugger in tempo reale.
* 11/12/96 GG 1.18 Aggiunto un comando per reinizializzare il sistema dei
*             breakpoint.
* 07/01/97 GG 1.19 Eliminati due riferimenti residui a "NVRAM_SIZ_PLC".
* 16/01/97 GG 1.20 Aggiunta posizione del breakpoint come risposta al
*             comando di definizione di break.
* 06/02/97 GG 1.21 Aggiunto (uffa !) un segnale di scambio nuovo (Lelli,
*             e chi altrimenti ?).
* 19/02/97 GG 1.22 Corretto un problema nel calcolo dell'indirizzo della
*             RAM non volatile. Con due IOS installate, smetteva di funzionare.
*             Aggiunta la variabile NVRAM_pub, equivalente ad NVRAM, ma
*             piu` adatta ad essere utilizzata dal compilatore IL, perche`
*             evita possibili conflitti con nomi definiti dal programmatore.
* 03/06/97 GG 1.23 Aggiunto l'azzeramento delle variabili con l'attributo
*             "RESET". Aggiunto l'annullamento dei fronti pendenti al GO.
* 11/06/97 GG 1.24 Gettate le basi per l'eliminazione delle "variabili non
*             azzerabili": i quattro segnali di scambio non azzerabili ora
*             "possono" essere sostituiti da altri quattro segnali "invisibili"
*             (perche` non pubblicati, e perche` il loro nome inizia per "$",
*             cosi` non sono accessibili da IL). La creazione di questi
*             nuovi segnali e` pero` affidata ad "hatsh", il Plc si limita
*             a controllarne l'esistenza, per poter funzionare alla vecchia
*             maniera con versioni precedenti di "hatsh.oo".
* 14/07/97 GG 1.25 Sostituita una "sprintf" con "util_safe_sprintf".
* 11/11/97 GG 1.26 Rese globali le funzioni legate al trace (non ancora).
*             Aggiunta la chiamata di una callback apposita per il trace
*             "nuovo stile".
* 09/03/98 GG 1.27 Ridotta l'emissione di errori in caso di mancanza della
*             tastiera speciale.
* 18/03/98 GG 1.28 Eliminata completamente l'emissione di errori legati
*             alla tastiera speciale. Tanto non interessava a nessuno...
* 02/04/98 GG 1.29 Eliminato un warning. Nulla di serio.
* 07/12/00 GG 1.30 Evitato l'aggancio dei segnali di scambio dei tasti
*             se non e` attivata la loro generazione (utile solo per
*             picking).
*/

#include <stdio.h>
#include <string.h>

#include <ios.h>
#include <nvram.h>
#include <plcsym.h>

#include "qplc.h"
#include "dr_ios.h"
#include "compat.h"
#include "cback.h"
#include "util.h"

#include "libh.h"

#include "debugger.h"

/*
* Variabili e definizioni associate al trace "vecchio stile".
*/

/* Tipo rappresentante un campione nel buffer di trace. */
typedef unsigned long trc_record_t;

/* Numero massimo di variabili tracciabili. */
#define MAX_OLD_TRACE_VAR (sizeof(trc_record_t) * 8)

/* Dimensione del buffer di trace. */
/* NOTA : ESAGERARE NON SERVE ! */
#define MAX_OLD_TRACE_SIZE 800 /* 400+400 campioni. */

/* Stato del trace. */
typedef struct {
/* Flag di trace attivo. */
	int active;
/* Numero di variabili sorvegliate. */
	int nvar;
/* Posizione di scrittura nel buffer. */
	int position;
/* Posizione dell'ultimo buffer interamente riempito. */
	int lastbuff;
/* Flag indicante che e` stata completata la scrittura di uno dei buffer. */
	int completed;
/* Dimensione di un (sotto)buffer. */
	int dimbuff;
/* Dimensione complessiva del buffer. */
	int dimtot;
/* Posizione del buffer corrente. */
	int currbuff;
/* Numero di record immagazzinati nel (sotto)buffer corrente. */
	int relpos;
} old_trc_status_t;

typedef struct {
	old_trc_status_t status;
/* Tabella dei puntatori alle variabili sorvegliate. */
	char * ptab[MAX_OLD_TRACE_VAR];
/* Buffer di trace. */
	trc_record_t buffer[MAX_OLD_TRACE_SIZE];
} old_trc_statics_t;


/* Variabili associate al trace. */

/* Struttura di stato del trace. */

static old_trc_statics_t trc_statics = {
	{
	/* Flag di trace attivo. */
		0, /* active; */
	/* Numero di variabili sorvegliate. */
		0, /* nvar; */
	/* Posizione di scrittura nel buffer. */
		0, /* position; */
	/* Posizione dell'ultimo buffer interamente riempito. */
		0, /* lastbuff; */
	/* Flag di scrittura di uno dei buffer completata. */
		0, /* completed; */
	/* Dimensione di un (sotto)buffer. */
		MAX_OLD_TRACE_SIZE / 2, /* dimbuff; */
	/* Dimensione complessiva del buffer. */
		MAX_OLD_TRACE_SIZE, /* dimtot; */
	/* Posizione del buffer corrente. */
		0, /* currbuff; */
	/* Numero di record immagazzinati nel (sotto)buffer corrente. */
		0, /* relpos; */
	}, /* status */
/* Tabella dei puntatori alle variabili sorvegliate. */
	{ (char *) 0, /* ... */ }, /* ptab; */
/* Buffer di trace. */
	{ 0, /* ... */ }, /* buffer; */
};

#define trc_status trc_statics.status

/*
* Definizione delle variabili mantenute per compatibilita`
* con le vecchie applicazioni.
*/

unsigned char * NVRAM = (unsigned char *) 0;
unsigned char * NVRAM_pub = (unsigned char *) 0;
unsigned char * NVRAM_priv = (unsigned char *) 0;
char *pchMem = (unsigned char *) 0;
shvar_t *pKeyTab = (shvar_t *) 0;
char *MData = (char *) 0;
int iStandAlone = 0;
char *pImageBool = (char *) 0;
char **pEdgeBool = (char **) 0;
int nEdgeBool = 0;
int nMaxEdgeBool = 0;
long *pImageLong = (long *) 0;
long **pEdgeLong = (long **) 0;
int nEdgeLong = 0;
int nMaxEdgeLong = 0;
unsigned char **CNI_ios[MAX_IOS_BOARD];

/* Tabella di associazione tra segnali di scambio relativi ai tasti
 e ingressi del driver di tastiera. */

static
#ifdef __STDC__
	const
#endif
struct {
	int key;                /* Chiave base del gruppo. */
	unsigned char k_size;   /* Numero di elementi del gruppo. */
	unsigned char i_pos;    /* indice del primo input. */
	unsigned char i_size;   /* Dimensione in bit dell'input. */
	unsigned char shadow;   /* Flag indicante che la variabile */
                                /* deve essere protetta da azzeramenti */
                                /* accidentali da parte del programma PLC. */
	char * alias;           /* Segnale di scambio alternativo, per */
	                        /* le varsioni piu` recenti di "hatsh.oo". */
} init_keybvar_tab[] = {
{ plc_TASTI,    32,  0,  1, 0, NULL, },
{ plc_OVR,       4,  0, 32, 0, NULL, },
{ plc_RIPCIC,    1, 32,  1, 1, "$START", },/* Sarebbe per il centro 0. */
{ plc_SOSPCIC,   1, 33,  1, 1, "$STOP", }, /* Anche questo (ma gli alias no) */
{ plc_CLEAR_PLC, 1, 34,  1, 1, "$CLEAR", },
{ plc_RESET_PLC, 1, 35,  1, 1, "$RESET", },
};

/* Tabella delle "ombre" per i segnali di scambio che non devono essere
 azzerati dal programma PLC. */

static struct shadow_t {
	unsigned char *target;	/* Indirizzo della variabile. */
	unsigned char backup;	/* Valore di backup. */
} shadow[MAX_COMPAT_SHADOW];

/* Numero di "ombre" attive. */
static int n_shadows = 0;


/* TABELLA DI CORRISPONDENZA PER I SEGNALI DI SCAMBIO PREDEFINITI */

/* Parte generata in modo automatico. */

#define SHV_TYPE_BOOL 0
#define SHV_TYPE_LONG 1
#define SHV_TYPE_FLOAT 2

#define _PN_(x) #x,plc_##x

struct initkeys_t {
#ifdef __STDC__
	const
#endif
	char *name;
	short key;
	unsigned char dim1;
	unsigned char dim2;
	unsigned short mode;
/* Mappa di bit, che indica di quali "pacchetti" faccia parte il segnale
 di scambio. */
	unsigned char package;
};

static
#ifdef __STDC__
const
#endif
struct initkeys_t predefs[] = {
{_PN_(EMERGENZA)       ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(RESET_PLC)       ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XIO},
{_PN_(CLEAR_PLC)       ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XIO},
{_PN_(ASSICAL)         ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(RESPLC)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(CNOK)            ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_ALL},
{_PN_(CPE)             ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STOP)            ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AZZERAMENTO)     ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(MDI)             ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AUTOMATICO)      ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(MANUALE)         ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(LPL)             ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(EXIT_VAL)        ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(PAESE)           ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(RICFIN)          ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(RIPCIC)          ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XIO},
{_PN_(MODO)            ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(ABINV)           ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(COMINV)          ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(SOSPCIC)         ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XIO},
{_PN_(SELVEL)          ,  40,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(MENO)            ,  40,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(PIU)             ,  40,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(HOLDAX)          ,  40,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(ABIAX)           ,  40,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(TS)              ,   6,  47, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(TP)              ,   6,  47, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(TH)              ,   6,  47, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(T)               ,   6, 119, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(INVERR)          ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(INVES)           ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(PRNEW)           ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(DATVAL)          ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE)          ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(PIC)             ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AXPOS)           ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(ASSCOM)          ,  40,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AZZOK)           ,  40,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(S)               ,   6,   1, SHV_MODE_TYPE_FLOAT              , PKG_XNC},
{_PN_(M)               ,   6,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(KA)              ,   6,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(ORIG_ATT)        ,   6,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(ORIG)            ,   6,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(GENERAL)         ,  64,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(QUOTA)           ,  16,   1, SHV_MODE_TYPE_FLOAT|SHV_MODE_RESET,PKG_XNC},
{_PN_(TASTI)           ,  32,   1, SHV_MODE_TYPE_BOOL               , PKG_XIO},
{_PN_(OVR)             ,   4,   1, SHV_MODE_TYPE_LONG               , PKG_XIO},
{_PN_(RIC_DAT_ANT)     ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(DAT_ERR)         ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(DATVAL_ANT)      ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(KA_ANT)          ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(LZ_ANT)          ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(LY_ANT)          ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(LX_ANT)          ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(ACK_GRU)         ,  16,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STOP_GRU)        ,  16,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(START_GRU)       ,  16,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(DEFGR15)         ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR14)         ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR13)         ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR12)         ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR11)         ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR10)         ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR9)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR8)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR7)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR6)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR5)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR4)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR3)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR2)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR1)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(DEFGR0)          ,   8,   2, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(INV)             ,   6,   7, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(LZ)              ,   6,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(LY)              ,   6,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(LX)              ,   6,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(FLAGS)           ,  16,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(ANT)             ,  16,   1, SHV_MODE_TYPE_FLOAT|SHV_MODE_RESET,PKG_XNC},
{_PN_(ACCEL)           ,  16,   1, SHV_MODE_TYPE_FLOAT|SHV_MODE_RESET,PKG_XNC},
{_PN_(VEL)             ,  16,   1, SHV_MODE_TYPE_FLOAT|SHV_MODE_RESET,PKG_XNC},
{_PN_(LOOPLC)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(OUT_RANGE)       ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(CAMMA)           ,  40,   5, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(ACK_CAR)         ,  16,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STOP_CAR)        ,  16,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(START_CAR)       ,  16,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(SIMULAZIONE)     ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_WPLC)     ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_TS)       ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_TP)       ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_TH)       ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_T)        ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_S)        ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_M)        ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(ASSMOT)          ,  40,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AZZMAN)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AZZCEN)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(AZZSIN)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STOPINV)         ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(CLEAR)           ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(QUOX_FUT)        ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(TIPINT)          ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(KB3)             ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(KB2)             ,   6,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(KB_KO)           ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(KB_OK)           ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(STROBE_COM)      ,   6,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(COMMAND)         ,   6,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(RIC_OP)          ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(ERR_OP)          ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(ACK_OP)          ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(BITINT)          ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(COD_OP)          ,   1,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(OVERR)           ,  40,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(OVERRIDE)        ,   1,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(AZZ)             ,  40,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(I_MIN)           ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(I_MAX)           ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(I_VUOTO)         ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(I_BLOCCO)        ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(CONTAM)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(DEC_US)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(INC_US)          ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(ACK_CMD)         ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(ERR_CMD)         ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET, PKG_XNC},
{_PN_(RIC_CMD)         ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(EXE)             ,   1,   1, SHV_MODE_TYPE_BOOL               , PKG_XNC},
{_PN_(COD_CMD)         ,   1,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(DATA_CMD)        ,   4,   1, SHV_MODE_TYPE_LONG               , PKG_XNC},
{_PN_(DATA_OP)         ,   4,   1, SHV_MODE_TYPE_LONG|SHV_MODE_RESET, PKG_XNC},
{_PN_(EXEPLC)          ,   1,   1, SHV_MODE_TYPE_BOOL|SHV_MODE_RESET|SHV_MODE_EDGE , PKG_XNC},
{_PN_(CORR)            ,  40,   1, SHV_MODE_TYPE_FLOAT              , PKG_XNC},
};

#undef _PN_

/* Fine della parte generata in modo automatico. */

/*
* Questa funzione crea le variabili condivise predefinite.
*/

static void compat_init_predef_shvar(void)
{
int i,k,chg;
shv_t v;

/* Prima di "buttarsi" a crearne di nuove, "ripassa" quelle gia`
 presenti nella shared memory. */

	shvInit();

/* E ora definisce cio` che manca. */

	for (i = 0; i < sizeof(predefs)/sizeof(predefs[0]); ++i) {
		if (! (predefs[i].package & main_status.package)) {
		/* Il segnale di scambio non e` previsto dalla
		 configurazione corrente. */
			continue;
		}
		strcpy(v.name,predefs[i].name);
		v.mode = predefs[i].mode;
		v.dim1 = predefs[i].dim1;
		v.dim2 = predefs[i].dim2;
		v.key = predefs[i].key;
		k = shvCreate(&v,&chg);
		if (k == -1) {
		/* ERRORE. E` successo qualcosa di grave. */
			plcError(105,v.name);
		}
		else if (chg) {
		/* ERRORE. L'oggetto non ha esattamente le
		 caratteristiche richieste. */
			plcError(104,v.name);
		}
	}

/* Registra l'indirizzo del segnale di scambio "CNOK". */

	main_status.p_cnok = (unsigned char *) shvKeyToAddr(plc_CNOK,0,0);
}

/*
* Inizializzazione del colloquio "vecchio stile" col debugger.
*/

static void compat_comm_init(void)
{
	main_status.osys -> question_ker = 0;
	main_status.osys -> answer_ker = 0;
	main_status.osys -> question_ready = 0;
	main_status.osys -> answer_size = 0;
	main_status.osys -> client = 0;
}

/*
* Funzioni "compat_init_1" e "compat_init_2"
* ------------------------------------------
*
* Queste funzioni inizializzano parte del sottosistema di compatibilita`
* con isaker.
*/

void compat_init_1(void)
{
int i;
driver_t *d;
dr_ios_t *dd;
char *p;

/* Vecchio puntatore alla base della shared memory. */

	pchMem = (char *)main_status.shm_addr;

/* Vecchio puntatore alla tabella delle variabili condivise. */

	pKeyTab = main_status.shvk;

/* Vecchio handle dei dati macchina. */

	MData = main_status.m_data;

/* Vecchio flag di modalita` stand-alone. */

	iStandAlone = main_status.stand_alone;

/* Inizializza le variabili NVRAM, NVRAM_priv e CNI_ios, avvalendoosi
 dei servizi del driver IOS. */

	for (i = 0; i < MAX_IOS_BOARD; ++i) {
		d = ioGetInstance("ios",i);
		if (d) {
			dd = (dr_ios_t *)(d -> device);
			if (dd -> nvram) {

			/* Sono lasciati "occupati" quattro byte all'inizio
			 dell'area. Per ora non me ne faccio nulla (salvo
			 evitare che certe variabili RETAIN abbiano offset
			 nullo), ma non escludo di usarli in futuro. */

				if (! main_status.nvram1.addr
				 && AttachNvram(main_status.nvram1_name,
				                &main_status.nvram1,
				                dd -> nvram,i)==0)
					main_status.nvram1_free
					        = (NVRAM_priv
					           = main_status.nvram1.addr)
					          + sizeof(long);
				if (! main_status.nvram2.addr
				 && AttachNvram(main_status.nvram2_name,
				                &main_status.nvram2,
				                dd -> nvram,i)==0)
					main_status.nvram2_free
					        = (NVRAM
					           = NVRAM_pub
					           = main_status.nvram2.addr)
					          + sizeof(long);
			}
			CNI_ios[i] = dd -> cpu;
		}
		else
			CNI_ios[i] = (unsigned char **) 0;
	}

/* Alloca lo spazio per la tabella dei segnali con rilevamento di fronte
 e per la memorizzazione del loro stato precedente. */

	nMaxEdgeBool = main_status.n_max_edge_bool;
	nMaxEdgeLong = main_status.n_max_edge_long;

	p = (char *) malloc((sizeof(char *) + sizeof(char)) * nMaxEdgeBool
	                    + (sizeof(long *) + sizeof(long)) * nMaxEdgeLong);
	if (! p) {
	/* ERRORE. Memoria esaurita. */
		plcError(108,"1");
	}
	else {

	/* Assegna i valori ottenuti alle variabili "vecchio stile" ed ai
	 campi appositi della struttura di controllo del programma. */

		nEdgeBool    = main_status.n_edge_bool     = 0;
		pEdgeBool    = main_status.p_edge_bool     = (char **) p;
		p += sizeof(char *) * nMaxEdgeBool;
		pImageBool   = main_status.p_image_bool    = (char *) p;
		p += sizeof(char) * nMaxEdgeBool;
		nEdgeLong    = main_status.n_edge_long     = 0;
		pEdgeLong    = main_status.p_edge_long     = (long **) p;
		p += sizeof(long *) * nMaxEdgeLong;
		pImageLong   = main_status.p_image_long    = (long *) p;
	}

/* Crea le variabili condivise predefinite. */

	compat_init_predef_shvar();

/* Inizializza il colloquio col debugger "vecchia maniera". */

	compat_comm_init();
}

void compat_init_2(void)
{
/* Puo` servire. */
}

/*
* Funzione "compat_start"
* -----------------------
*
*  Questa funzione esegue ad ogni "START" le azioni richieste per
* compatibilita` con isaker.
*/

void compat_start(void)
{
int i,j,k,ak;
unsigned char * addr;
char s[30];
shv_t v;
char *alias;
int errf;

/* Azzera le variabili con l'attributo "RESET". */
	shvRestart();

/* Annulla gli eventuali fronti "pendenti". */
	compat_restart_edges();

/* Aggancia i segnali di scambio legati ai tasti agli opportuni ingressi
 della tastiera speciale. */
/* L'associazione e` decisa in base al contenuto della tabella
 "init_keybvar_tab", ogni elemento della quale descrive un gruppo
 di variabili condivise ed il corrispondente gruppo di ingressi. */

/* Vuota la tabella dei segnali di scambio non azzerabili. */

	n_shadows = 0;
#if 0
	errf = 0;
#else
/* ...ma niente, non interessa a nessuno se la tastiera non va
 (o, detto meglio, a nessuno interessa che sia il plc a comunicarlo...). */
	errf = 1;
#endif

/* Modifica per PLC non collegati al CN (picking): i segnali non devono
 essere agganciatio ai tasti se non e` attivata la loro generazione
 automatica. */

	if (main_status.package & PKG_XIO) {

	for (i = 0;
	     i < sizeof(init_keybvar_tab)/sizeof(init_keybvar_tab[0]);
	     ++i) {

		k = init_keybvar_tab[i].key;
		alias = init_keybvar_tab[i].alias;

	/* Controlla se la variabile possiede un "alias".
	 Se si`, utilizza quest'ultimo. */
		if (alias) {
			strcpy(v.name, alias);
			v.mode = SHV_MODE_TEST;
			ak = shvDefine(&v,(int *) NULL);
			if (ak != -1)
				k = ak;
			else
				alias = NULL; /* Come non detto,
				               niente alias */
		}

		for (j = 0; j < init_keybvar_tab[i].k_size; ++j) {
			util_safe_sprintf(s,sizeof(s),"keyb.0.%d.%d",
				init_keybvar_tab[i].i_size,
				init_keybvar_tab[i].i_pos + j);
			addr = (unsigned char *) shvKeyToAddr(k,j,0);
			if (! addr) {
			/* Segnale di scambio non definito. */
			}
			else {
			/* Se richiesto, occupa un posto nella tabella
			 delle variabili non azzerabili (a meno che non
			 sia stato trovato l'alias). */
				if (init_keybvar_tab[i].shadow
				    && n_shadows < MAX_COMPAT_SHADOW
				    && !alias) {
					shadow[n_shadows].backup = *addr;
					shadow[n_shadows].target = addr;
					addr = &shadow[n_shadows].backup;
					++n_shadows;
				}
			/* Aggancia la variabile (o la sua ombra) all'ingresso
			 specificato. */
				if (! ioAttach(s,(void *)addr,
				               init_keybvar_tab[i].i_size,
				               DRIVER_MODE_INPUT)) {
				/* ERRORE. Il driver ha rifiutato
				 l'associazione. */
					if (! errf) {
				/* Piccola pezza: si evita di
				 produrre maree di errori per una cosa
				 "da nulla". */
						plcError(5,s);
						errf = 1;
					}
				}
			}
		}
	}
	}
}

/*
* Funzione "compat_add_edge"
* --------------------------
*
* Riserva "n" celle per variabili a rilevamento di fronte di dimensione "size".
* "n" e "size" sono campi della struttura "*edge", pre-caricata.
* Restituisce il puntatore alla struttura stessa, oppure
* NULL se non c'e` piu` posto. In caso di successo, la struttura contiene
* campi che puntano alla lista degli indirizzi delle "n" variabili e alla
* lista delle immagini delle stesse. Si usi la funzione "compat_register_edge"
* per riempire queste liste.
*/

compat_edge_t * compat_add_edge(compat_edge_t * edge)
{
char ** p;
char *q;

	switch (edge -> size) {
	case sizeof(char):
		if (nEdgeBool + edge -> n > nMaxEdgeBool) {
		/* ERRORE. Non c'e` piu` spazio. */
			return (compat_edge_t *)0;
		}

		p = &pEdgeBool[nEdgeBool];
		q = (char *)&pImageBool[nEdgeBool];
		memset(q,0,sizeof(char)*edge->n);

		main_status.n_edge_bool = nEdgeBool += edge -> n;

		break;
	case sizeof(long):
		if (nEdgeLong + edge -> n > nMaxEdgeLong) {
		/* ERRORE. Non c'e` piu` spazio. */
			return (compat_edge_t *)0;
		}

		p = (char **)&pEdgeLong[nEdgeLong];
		q = (char *)&pImageLong[nEdgeLong];
		memset(q,0,sizeof(long)*edge->n);

		main_status.n_edge_long = nEdgeLong += edge -> n;

		break;
	default:
		return (compat_edge_t *)0;
	}

	edge -> varlist = p;
	edge -> imglist = q;

	return edge;
}

/*
* Funzione "compat_remove_edge"
* -----------------------------
*
*  Questa funzione ritira le ultime "n" celle a
* rilevamento di fronte della dimensione "size". "n" e"size" sono
* campi della struttura "*edge".
*/

void compat_remove_edge(compat_edge_t * edge)
{
	switch (edge -> size) {
	case sizeof(char):
		main_status.n_edge_bool = nEdgeBool -= edge -> n;
		break;
	case sizeof(long):
		main_status.n_edge_long = nEdgeLong -= edge -> n;
		break;
	default:
		break;
	}
}

/*
* Funzione "compat_register_edge"
* -------------------------------
*
*  Questa funzione registra nella lista di variabili a rilevamento di fronte
* "edge" la variabile di indirizzo "addr" nella posizione "n".
*/

void compat_register_edge(compat_edge_t * edge, int n, char *addr)
{
	switch (edge -> size) {
	case sizeof(char):
		edge -> varlist[n] = addr;
		edge -> imglist[n] = *addr;
		break;
	case sizeof(long):
		((long **)(edge -> varlist))[n] = (long *)addr;
		((long *)(edge -> imglist))[n] = *(long *)addr;
		break;
	default:
		break;
	}
}

/*
* Funzione "compat_restart_edges"
* -------------------------------
*
*  Questa funzione inizializza le immagini dei fronti per evitare che
* ad una sequenza HALT-GO siano rilevati erroneamente eventi.
*/

void compat_restart_edges(void)
{
int i;

	for (i = 0; i < main_status.n_edge_bool; ++i)
		pImageBool[i] = *pEdgeBool[i];
	for (i = 0; i < main_status.n_edge_long; ++i)
		pImageLong[i] = *pEdgeLong[i];
}

/*
* Funzione "compat_init_spaces"
* -----------------------------
*
*  Creazione dell partizioni della shared memory secondo uno schema
* compatibile con IsaGraf.
*/

/* Stringa contenente l'immagine dello spazio dei simbolo IsaGraf. */

static const char compat_ik_symbols[] =
"@QPLC_SYMBOLS,0\r\n"
"\r\n"
"@BOOLEANS,0\r\n"
"\r\n"
"@ANALOGS,0\r\n"
"\r\n"
"@TIMERS,0\r\n"
"\r\n"
"@MESSAGES,0\r\n"
"\r\n"
"@END_SYMBOLS,0\r\n";

/* Tabella descrivente la dimensione da dare ad ogni partizione. */

static long compat_ik_sizes[] = {
((sizeof(ik_system_t) + sizeof(int) - 1)/sizeof(int)) * sizeof(int),
0,
0,
((sizeof(compat_ik_symbols) + sizeof(int) - 1)/sizeof(int)) * sizeof(int),
0,
MAX_OLD_COMM_REQ * 2,
0,
0,
0,
0,
0x2000, /* Modificato run-time : dipende dalla configurazione. */
0,
128 * (sizeof(void *) + sizeof(int)),
0,
0,
0,
0,
128 * sizeof(*main_status.shvh), /* Dipende dalla configurazione. */
sizeof(shv_t), /* Ma poi cambia... */
0, /* Modificato run-time : l'ultimo e` tutto lo spazio libero rimanente. */
};

void compat_init_spaces(void)
{
int i;
long off,size;

/* Calcola l'inizio dello spazio disponibile per le partizioni. */

	off = sizeof(*main_status.shm_addr);

/* Modifica i dati dipendenti dalla configurazione. */

	compat_ik_sizes[_IK_SPC_SHVK]
	 = main_status.shvk_tab_len * sizeof(shvar_t);
	compat_ik_sizes[_IK_SPC_SHVH]
	 = main_status.shvh_tab_len * sizeof(*main_status.shvh);
	compat_ik_sizes[_IK_SPC_FREE]
	 = main_status.shm_size - off;

/* Alloca gli spazi. */

	for (i = 0;
	     i < sizeof(compat_ik_sizes)/sizeof(compat_ik_sizes[0]);
	     ++i) {
		size = compat_ik_sizes[i];
		main_status.shm_addr -> isaker_spc_dir[i].size = size;
		main_status.shm_addr -> isaker_spc_dir[i].off = off;
		off += size;
		compat_ik_sizes[_IK_SPC_FREE] -= size;
	}

/* Inizializza lo spazio dei simboli IsaGraf con qualcosa di riconoscibile. */

	strcpy((char *)(main_status.shm_addr)
	        + main_status.shm_addr -> isaker_spc_dir[_IK_SPC_OSYM].off,
	       compat_ik_symbols);
}

/*
* Funzione "compat_begin_cycle"
* -----------------------------
*
*  Questa funzione deve essere chiamata all'inizio del ciclo ordinario
* del PLC, dopo "hat".
*  Attualmente, registra lo stato di alcuni segnali di scambio (se esistono)
* associati a tasti "importanti", il valore finale dei quali dovra`
* essere l'OR logico tra quello iniziale e quello impostato dal programma.
*/

void compat_begin_cycle(void)
{
register int i;
register struct shadow_t *p;

/* Copia il valore corrente delle variabili non azzerabili. */

	for (i = n_shadows, p = shadow; --i >= 0; ++p) {
		*(p -> target) = p -> backup;
	}
}

/*
* Funzione "compat_trace"
* -----------------------
*
*  Questa funzione memorizza lo stato delle variabili di trace selezionate
* in un apposito buffer interno, ed eventualmente comunica la fine dell'ope-
* razione al debugger (trace vecchio stile).
*/

void compat_trace(void)
{
register trc_record_t m,v;
register int i;
register struct shadow_t *p;

/* Provvede a ripristinare il valore delle variabili non azzerabili. */
/* (Si`, va bene, non c'entra niente col trace. Ma chi se ne frega ?). */

	for (i = n_shadows, p = shadow; --i >= 0; ++p) {
		if (p -> backup)
			*(p -> target) = 1;
	}

/*****************/
/* Trace, trace. */
/*****************/

/* Trace "nuovo stile" (vedi remote.c). */
	cb_call(QPLC_CB_TRACE,(void *)NULL);

/* Trace "vecchio stile. */
	if (! trc_status.active)
		return;

	for (i = 0, m = 0, v = 1; i < trc_status.nvar; ++i, v = v << 1) {
		if (*trc_statics.ptab[i])
			m |= v;
	}

	trc_statics.buffer[trc_status.position] = m;

	trc_status.position = (trc_status.position + 1) % trc_status.dimtot;

	if (++trc_status.relpos == trc_status.dimbuff) {
		trc_status.completed = 1;
		trc_status.lastbuff = trc_status.currbuff;
		trc_status.currbuff = trc_status.position;
		trc_status.relpos = 0;
	}
}

/*
* Funzione "old_trace_signal"
* ---------------------------
*
*  Questa funzione spegne il flag di "caricamento completato", che la
* "old_trace_" accende quando e` riempito uno dei due buffer di trace.
*  Dovrebbe essere chiamata dal client alla fine del download di un buffer,
* prima di mettersi in attesa di un nuovo caricamento.
*/

static void old_trace_signal(void)
{
	trc_status.completed = 0;
}

/*
* Funzione "old_trace_start"
* --------------------------
*
*  Questa funzione predispone il plc ad iniziare il trace.
*  Lo stato del trace viene resettato.
*/

static void old_trace_start(void)
{
	trc_status.active = 1;
	trc_status.position = 0;
	trc_status.lastbuff = 0;
	trc_status.currbuff = 0;
	trc_status.relpos = 0;
	old_trace_signal();
}

/*
* Funzione "old_trace_stop"
* -------------------------
*
*  Questa funzione interrompe il trace del plc.
*  Lo stato del trace (buffer, in particolare) non e` alterato
* da quest'operazione.
*/

static void old_trace_stop(void)
{
	trc_status.active = 0;
}

/*
* Funzione "old_trace_add_var"
* ----------------------------
*
*  Questa funzione aggiunge l'indirizzo "p" alla lista di variabili
* da tracciare.
*  La funzione vale 1 se tutto e` andato bene, 0 altrimenti.
*/

static int old_trace_add_var(char *p)
{
	if (trc_status.nvar < MAX_OLD_TRACE_VAR) {
		trc_statics.ptab[trc_status.nvar++] = p;
		return 1;
	}
	return 0;
}

/*
* Funzione "old_trace_clear_list"
* -------------------------------
*
*  Questa funzione vuota la lista di trace.
*/

static void old_trace_clear_list(void)
{
	trc_status.nvar = 0;
}

/*
* Funzione "old_trace_status"
* ---------------------------
*
*  Questa funzione copia lo stato del trace all'indirizzo "p".
*  Restituisce il numero di byte copiati.
*/

static int old_trace_status(char * p)
{
	*(old_trc_status_t *)p = trc_status;
	return sizeof(trc_status);
}

/*
* Funzione "old_trace_copy"
* -------------------------
*
*  Questa funzione copia n record dalla posizione di trace "pos"
* all'indirizzo "p".
*  Restituisce la dimensione dell'area copiata in byte.
*/

static int old_trace_copy(trc_record_t * p,int pos,int n)
{
int rv;

	rv = n * sizeof(trc_record_t);

	while (n--) {
		*(p++) = trc_statics.buffer[pos];
		pos = (pos + 1) % MAX_OLD_TRACE_SIZE;
	}
	return rv;
}

/*
* Funzione "old_trace_set_buff"
* -----------------------------
*
*  Questa funzione imposta la dimensione dei buffer.
*  Vale 1 se tutto va bene, 0 in caso di errore.
*/

static int old_trace_set_buff(int n)
{
	if (n > trc_status.dimtot / 2 || trc_status.dimtot / n < 10)
		return 0;

	trc_status.dimbuff = n;

	return 1;
}

/*
* Riconoscimento e risposta ad un comando "vecchio stile"
*/

/* Tipo associato all'operazione di copia multipla di aree
 da parte del debugger. */
typedef struct {
	char *addr;	/* Indirizzo dell'area. */
	int len;	/* Lunghezza dell'area (0 = area non valida) */
} old_kmchk_t;

/*
* Questa due macro simulano un cambio di contesto, agli effetti
* dell'esecuzione della funzione "compat_exec_old_command".
* La prima macro dovrebbe essere chiamata dall'esecutore. Essa, se
* chiamata dall'esecutore, ha l'effetto di comunicare al supervisore
* di prendere il controllo
* del colloquio, inibire ulteriori chiamate della funzione da
* parte dell'esecutore ed uscire senza alterare le strutture legate
* al colloquio. Se chiamata dal supervisore, non ha alcun effetto.
* La seconda macro ripristina la situazione normale, nella quale
* l'esecutore controlla il colloquio.
* E` cura delle funzioni chiamanti assicurare che non siano violate le
* ipotesi che stanno alla base del meccanismo. In breve bisogna che
*
* - l'esecutore non chiami la funzione "compat_..." se non abilitato,
* - il supervisore non controlli il colloquio se non su richiesta
*   dell'esecutore,
* - la funzione "compat_..." non deve alterare i dati tra una chiamata e
*   l'altra. Solo cosi` il supervisore trovera` i dati integri e potra`
*   portare avanti il colloquio in stile isaker.
*/

#define SCHEDULE_SUPERVISOR { if (plcGiveOldComm()) return; }
#define SCHEDULE_EXECUTOR plcCatchOldComm()

static void compat_exec_old_command(unsigned char * ques, unsigned char * ans)
{
short size,cr;
/* Lista delle aree di copia multipla, vecchio stile. Scope rigorosamente
 ridottissimo. */
static int n_aree = 0;
static old_kmchk_t aree[MAX_OLD_COMM_REQ/sizeof(old_kmchk_t) + 2];

	cr = 0;
	size = 3;

	switch (ques[0]) {

/********************************/
/* Codici riguardanti il trace. */
/********************************/

	case R_TRACE_START:

	/* Lancia il trace. */

		old_trace_start();
		break;

	case R_TRACE_STOP:

	/* Interrompe il trace. */

		old_trace_stop();
		break;

	case R_TRACE_STATUS:

	/* Riporta lo stato del trace. */

		ans[size++] = 0;
		size += old_trace_status(ans + size);
	/* Risposta : un byte nullo, seguito da sizeof(tracests_t) byte. */
		break;

	case R_TRACE_LOAD:

	/* Copia una parte del buffer di trace. */

	/* Domanda : *(int *)(ques + 1) = posizione nel buffer di trace,
	  *(int *)(ques + sizeof(int)) = numero di record da copiare. */
		ans[size++] = 0;
		size += old_trace_copy((trc_record_t *)(ans + size),
				  *(int *)(ques + 1),
				  *(int *)(ques + 1 + sizeof(int)));
	/* Risposta : un byte nullo, seguito dal contenuto della parte
	 di buffer richiesta. */
		break;

	case R_TRACE_ADD:

	/* Aggiunge una variabile alla lista. */

	/* Domanda : *(char *)(ques + 1) = indirizzo della
	 variabile da tracciare. */
		if (! util_check_addr_rd(*(char **)(ques + 1),sizeof(char))
		  || ! old_trace_add_var(*(char **)(ques + 1)))
			cr = -1;
		break;

	case R_TRACE_CLEAR:

	/* Vuota la lista di trace. */

		old_trace_stop();
		old_trace_clear_list();
		break;

	case R_TRACE_SIGNAL:

	/* Spegne il flag di "buffer caricato".  Dovrebbe essere usato
	 dopo la copia di un intero buffer, per potere riconoscere il
	 successivo caricamento. */

		old_trace_signal();
		break;

	case R_TRACE_SETBUF:

	/* Imposta la dimensione dei sottobuffer. */

	/* Domanda : *(int *)(ques + 1) = dimensione. */

		if (! old_trace_set_buff(*(int *)(ques + 1)))
			cr = -1;
		break;

/*****************************************/
/* Codici riguardanti la copia di aree . */
/*****************************************/

	case R_GET_EXENAME:

	/* Ottiene il path dell'eseguibile PLC */

		strncpy(&ans[size],
		        main_status.exe_path,
		        MAX_OLD_COMM_REQ - size - 10);
		ans[MAX_OLD_COMM_REQ - 1 - 10] = '\0';
		size = MAX_OLD_COMM_REQ - 10;

	/* Risposta : il path richiesto. */

		break;

	case R_KERMEM_COPY:

	/* Copia il contenuto del blocco di memoria
	 di indirizzo e lunghezza dati. */

	/* Domanda : *(char **)(ques + 1) = indirizzo,
	             *(int *)(ques + 5) = lunghezza. */

		if (util_check_addr_rd(*(char **)(ques + 1),*(int *)(ques + 5)))
			memcpy(&ans[size],
			       *(char **)(ques + 1),*(int *)(ques + 5));
		size += *(int *)(ques + 5);

	/* Risposta : i byte richiesti. */

		break;

	case R_KERMEM_MCHK:

	/* Controlla e registra la lista di aree per la copia multipla */

	/* Domanda : *(int *)(ques + 4) = numero di record,
	             *(struct _plc_kmchk *)(ques + 8) = lista. */
		{
		int i,tot;
		old_kmchk_t *l;

#define _TMP_SZ (MAX_OLD_COMM_REQ - 2*sizeof(long))

			n_aree = *(int *)(ques + 4);
			l = (old_kmchk_t *)(ques + 4 + sizeof(int));
			if (n_aree < 0)
				n_aree = 0;
			if (n_aree > _TMP_SZ / sizeof(old_kmchk_t))
				n_aree = _TMP_SZ / sizeof(old_kmchk_t);
			for (i = tot = 0; i < n_aree; ++i,++l) {
				if (l -> len > 0
				    && l -> len + tot < _TMP_SZ
				    && util_check_addr_rd(l->addr,l->len)) {
					aree[i].addr = l -> addr;
					aree[i].len = l -> len;
					tot += l -> len;
					ans[size] = 1;
				}
				else {
					aree[i].len = 0;
					ans[size] = 0;
				}
				++size;
			}
		}

	/* Risposta : i byte richiesti. */

		break;

	case R_KERMEM_MCOPY:

	/* Riporta il contenuto delle aree di copia multipla */

	/* Domanda : *(int *)(ques + 4) = numero di record,
	             *(struct _plc_kmchk *)(ques + 8) = lista. */


		{
		register int i,j,l,ptr;

			ptr = size;
			size = _TMP_SZ;
			for (i = 0; i < n_aree ; ++i) {
				l = aree[i].len;
				for (j = 0; j < l; ++j) {
					ans[ptr++] = aree[i].addr[j];
				}
			}
		}

	/* Risposta : i byte richiesti. */

		break;

#undef _TMP_SZ /* Non serve piu` */

/***********************************************/
/* Codici riguardanti la statistica dei tempi. */
/***********************************************/

	case R_GET_TCYSTAT:

	/* Copia il contenuto delle variabili di statistica dei tempi. */

		ans[size++] = 0;
		*(long *)(&ans[size]) = exec_status.curr_usec;
		size += sizeof(long);
		*(long *)(&ans[size]) = exec_status.min_usec;
		size += sizeof(long);
		*(long *)(&ans[size]) = exec_status.max_usec;
		size += sizeof(long);

	/* Risposta : un byte nullo, seguito da 3 long. */

		break;

	case R_IF_ENSTAT:

	/* Stato della statistica. */

		ans[size++] = main_status.soft_tm;

	/* Risposta : un byte contenente lo stato del flag di
	 abilitazione della statistica. */

		break;

	case R_START_STAT:

	/* Abilitazione-disabilitazione statistica. */

	/* Domanda : *(char *)(ques + 1) = valore del flag di abilitazione */

		if (*(char *)(ques + 1))
			plcStartMeasurement();
		else
			plcStopMeasurement();

		break;

	case R_CLEAR_STAT:

	/* Riazzeramento della statistica. */

		plcClearMeasurement();

		break;


/***********************************************/
/* Codici riguardanti il caricamento dinamico. */
/***********************************************/

	case R_DYNLD_LOAD:

	/* Carica dinamicamente un modulo oggetto. */

	/* Domanda : *(char *)(ques + 1) = nome del file. */

		cr = plcLoadModule((char *)(ques + 1));

		break;

	case R_DYNLD_UNLOAD:

	/* Scarica dinamicamente un modulo oggetto. */

		cr = plcUnloadModule();

		break;

	case R_DYNLD_STATUS:

	/* Stato del caricamento. */

		ans[size++] = 0;
		size += dyStatus((int *)(ans + size));

	/* Risposta : un byte nullo, seguito da sizeof(int) byte contenenti
	 un codice di stato. */

		break;

	case R_DYNLD_PROP:

	/* Ottiene i dati relativi ad un modulo. */

	/* Domanda : *(char *)(ques + 1) = nome del modulo. */

	/* Questo comando puo` essere eseguito solo dal supervisore. */

		SCHEDULE_SUPERVISOR;

		{
		struct module_t * mod;

			if (! ( (mod = dyFindModule((char *)(ques + 1))) ) ) {
				cr = -1;
			}
			else {
				ans[size++] = 0;
				size += dyModProp(mod,
					(modprop_t *)(ans + size));

	/* Risposta : un byte nullo, seguito da sizeof(struct modprop_t) byte
	 contenenti un codice di stato. */

			}
		}

		break;

	case R_CREATE_WATCH:
	/* Crea una lista di watch (ottiene l'ID). */
	/* Domanda : *(watch_descr_t *)(ques + 1) = caratteristiche. */

		cr = brkCreate((watch_descr_t *)(ques + 1));

		ans[size++] = 0;
		*(int *)(&ans[size]) = cr;
		cr = cr < 0 ? cr : 0;
		size += sizeof(int);
	/* Risposta : un byte nullo, seguito da sizeof(int) byte contenenti
	 un codice del watchpoint. */
		break;

	case R_DELETE_WATCH:
	/* Cancella una lista di watch dato l'ID. */
	/* Domanda : *(int *)(ques + 1) = id. */
		cr = brkDestroy(*(int *)(ques + 1));
		break;

	case R_ADD_BREAK:
	/* Aggiunge un punto di break al watch. */
	/* Domanda : *(int *)(ques + 1) = id. */
	/*           *(watch_t *)(ques + 1 + sizeof(int)) = caratteristiche. */
		cr = brkAdd(*(int *)(ques + 1),(watch_t *)(ques+1+sizeof(int)));
		ans[size++] = 0;
		*(int *)(&ans[size]) = cr;
		cr = cr < 0 ? cr : 0;
		size += sizeof(int);
	/* Risposta : un byte nullo, seguito da sizeof(int) byte contenenti
	 la posizione del breakpoint. */
		break;

	case R_ENABLE_WATCH:
	/* Abilita la lista di watch dato l'ID. */
	/* Domanda : *(int *)(ques + 1) = id. */
		cr = brkEnable(*(int *)(ques + 1));
		break;

	case R_DISABLE_WATCH:
	/* Disabilita la lista di watch dato l'ID. */
		cr = brkDisable(*(int *)(ques + 1));
		break;

	case R_CLEARALL_WATCH:
	/* Cancella tutto e reinizializza. */
		{
		int i;
			for (i = 0; i < MAX_WATCHPOINT; ++i)
				brkDestroy(i);
		}
		brkReinit();
		break;

	default:
		cr = 0;
		break;
	}

/* Fine del colloquio. */

	ans[0] = ques[0];
	*(short *)(ans + 1) = cr;

	main_status.osys -> answer_size = size;
	main_status.osys -> question_ready = 0;

/* Cede il controllo all'esecutore (se non ce l'ha gia` !) */

	SCHEDULE_EXECUTOR;
}

/*
* Funzione "compat_comm"
* ----------------------
*
*  Questa funzione realizza parte del colloquio con i debugger
* "isaker-compatibili". "isaker" gestiva completamente i comandi e
* le risposte all'interno del ciclo di esecuzione. Ora non e` piu` cosi` :
* i comandi piu` pesanti sono realizzati dal thread supervisore, mentre
* all'esecutore resta il compito di riconoscere i comandi piu` semplici
* e di arbitrare il colloquio.
*/

void compat_comm(void)
{
	if (main_status.osys -> question_ready) {
		compat_exec_old_command(main_status.osys -> question_ker,
		                        main_status.osys -> answer_ker);
	}
}

/*
* Funzioni "compat_chk...", "compat_go", "compat_halt"
* -------------------------------------------------------
*
*  Queste funzioni controllano lo stato "vecchio stile", impostato dai
* debugger "isaker-compatibili".
*/

#define IK_START_INIT 0
#define IK_START_APL 1
#define IK_START_OK 2
#define IK_START_STOP 3

void compat_chkgo(void)
{
	if (main_status.osys -> start == IK_START_APL) {
		plcGo();
		compat_go();
	}
}

void compat_chkhalt(void)
{
	if (main_status.osys -> start != IK_START_OK) {
		plcHalt();
	}
}

void compat_go(void)
{
	main_status.osys -> start = IK_START_OK;
}

void compat_halt(void)
{
	main_status.osys -> start = IK_START_INIT;
}

/*
* Funzione "sys_err"
* ------------------
*
* Mantenuta per compatibilita` con "isaker". Obsoleta.
*/

void sys_err(int n, char *s)
{
	plcError(n, "%x", (long)s);
}


/* Codice di validazione della struttura registrata in ram non volatile. */

#define WD_NVRAM_CHECK_V1 ((unsigned long)(0xbe191061))

/* Struttura immagazzinata nella zona privata della RAM non volatile.
 In previsione di futuri sconvolgimenti, il primo elemento della struttura
 e` un codice che fa sia da firma che da versione, e permette di decidere
 come interpretare la struttura stessa. */

union nvram_ioconf_t {

/* VERSIONE 1 */
struct {
/* Se questo non vale WD_NVRAM_CHECK_V1, la struttura non e` da considerare
 valida. */
	unsigned long check;
/* Tabella dei flag di presenza dei controllori : 8 bit per 8 schede (ed e`
 anche troppo !). Se una cpu e` presente, dovrebbe esistere (compatibilmente
 con lo spazio disponibile nella ram non volatile) l'immagine dei byte
 di stato del controllore. */
	unsigned char board[8];
/* Tabella delle immagini dei controllori. Una per ogni bit acceso
 in "board" */
	unsigned char images[1][32];
/* NON AGGIUNGERE CAMPI DI SEGUITO ! */
} v1;

/* VERSIONE 2, se arriva */
struct {
/* Se questo non vale WD_NVRAM_CHECK_V2, la struttura non e` da considerare
 valida. */
	unsigned long check;
} v2;

/* ecc... */

};

/*
* Funzione compat_check_io()
* --------------------------
*
*  Questa funzione controlla che la configurazione registrata in NVRAM
* sia compatibile con la situazione reale dei moduli.
*/

int compat_check_io(void)
{
union nvram_ioconf_t * io;
int i,j,k,m,rv,n;

	if (! NVRAM_priv) {
		plcError(190,"NVRAM");
		return 0;
	}

	io = (union nvram_ioconf_t *) NVRAM_priv;

	if (io -> v1.check != WD_NVRAM_CHECK_V1) {
		plcError(191,"%x",(long)io -> v1.check);
		return 0;
	}

	rv = 1;
	n = 0;

	for (i = 0; i < sizeof(io -> v1.board); ++i) {
		if (n >= (main_status.nvram1.size - sizeof(io->v1))
		         / sizeof(io -> v1.images[0]) + 1)
			break;
		for (j = 0, m = 1; j < 8; ++j, m <<= 1) {
			if (io -> v1.board[i] & m) {
				if (! CNI_ios[i]) {
					plcError(192,"brd %d",i);
					rv = 0;
					break;
				}
				if (! CNI_ios[i][j]) {
					plcError(192,"brd %d, cpu %d",
					         (long)i,(long)j);
					rv = 0;
					break;
				}
				for (k = 0; k < sizeof(io -> v1.images[0]); ++k) {
					if (! (io -> v1.images[n][k] & IOS_STS_M_DIS)) {
						if (io -> v1.images[n][k]
						 != CNI_ios[i][j][IOS_IOSTATUS + k]) {
							plcError(193,"brd %d, cpu %d, mod %d",(long)i,(long)j,(long)k);
							rv = 0;
						}
					}
				}
				++n;
			}
		}
	}

	return rv;
}

/*
* Funzione compat_save_io()
* -------------------------
*
*  Questa funzione salva che la configurazione in NVRAM.
*/

void compat_save_io(void)
{
union nvram_ioconf_t * io;
int i,j,k,m,n,lim;

	if (! NVRAM_priv) {
		return;
	}

	io = (union nvram_ioconf_t *) NVRAM_priv;

	io -> v1.check = WD_NVRAM_CHECK_V1;

	n = 0;
	lim = (main_status.nvram1.size - sizeof(*io))
	      / sizeof(io -> v1.images[0]) + 1;

	for (i = 0; i < sizeof(io -> v1.board); ++i)
		io -> v1.board[i] = 0;
	for (i = 0; i < sizeof(io -> v1.board); ++i) {
		if (CNI_ios[i]) {
			for (j = 0, m = 1; j < 8; ++j, m <<= 1) {
				if (CNI_ios[i][j]) {
					if (n >= lim)
						return;
					io -> v1.board[i] |= m;
					for (k = 0; k < sizeof(io -> v1.images[0]); ++k) {
						io->v1.images[n][k] =
						     CNI_ios[i][j][IOS_IOSTATUS + k]
						   & (IOS_STS_M_SPEC | IOS_STS_M_DIS | IOS_STS_M_NBTX);
					}
					++n;
				}
			}
		}
	}
}

