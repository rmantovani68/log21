/*
* @(#) qplc.h 1.27 Thu Apr  3 00:30:09 CEST 2008
* @(#) Rif. Guerrini
*
* Definizioni di interesse generale inirenti il plc CNi.
*
* 14/05/96 GG 1.0 Prima stesura.
* 30/08/96 GG 1.1 Aggiunta l'opzione "-timer-bug", per correggere il problema
*             dell'arrotondamento errato del periodo in Lynx 2.3.0.
* 03/09/96 GG 1.2 Aggiunto un meccanismo per la creazione selettiva di
*             segnali di scambio standard.
* 17/09/96 GG 1.3 Aggiunto il flag "no_fault", per evitare l'entrata in fault.
* 22/10/96 GG 1.4 Aggiunto il flag "old_save_io", per eseguire un salvataggio
*             in NVRAM della configurazione hardware, per compatibilita` con
*             isaker.
* 30/10/96 GG 1.5 Aggiunto il flag "posix_poll" per poter scegliere se
*             fare a meno di System V o di STREAMS.
* 26/11/96 GG 1.6 Aggiunti alcuni campi alla struttura di stato, utili
*             per la gestione delle variabili in RAM non volatile.
* 12/12/96 GG 1.7 Aggiunte due opzioni per configurare il debugger.
* 19/12/96 GG 1.8 Modifichine per visualizzare diverse sigle di versione
*             per le varie componenti.
* 15/01/97 GG 1.9 Aggiunti campi per la gestione dei moduli IL conteneti
*             piu` di un programma.
* 04/02/97 GG 1.10 Aggiunta un'opzione per disattivare alcuni controlli,
*             probabilmente lenti, che il Plc esegue sulle richieste del
*             debugger.
* 27/02/97 GG 1.11 Aggiunta un'opzione per intervenire sulla priorita`
*             associata ai canali TCP/IP (sperimentale).
* 15/04/97 GG 1.12 Aggiunta l'opzione "-drvconf", che serve a memorizzare una
*             stringa di configurazione ad uso del sistema di I/O (si veda
*             "drivers.c" e "dr_*.c").
* 23/05/97 GG 1.13 Aggiunto un campo contenente il numero massimo di cicli
*             di entrata in halt ammissibili.
* 03/06/97 GG 1.14 Aggiunta una tabella di conversione chiave --> descrittore
*             esteso per i segnali di scambio.
* 02/07/97 GG 1.15 Aggiunto un flag che permette l'entrata in GO se l'MSD in
*             stile IL richiede segnali di I/O inesistenti.
* 16/07/97 GG 1.16 Aggiunta la dimensione massima della tabella delle
*             estensioni.
* 05/09/97 GG 1.17 Aggiunto un flag per agire sulla proprieta` "blocking"
*             delle pipe degli errori e dei messaggi.
* 19/11/97 GG 1.18 Aggiunto il flag "remote_debug", che abilita il servizio
*             di debug remoto.
* 24/07/98 GG 1.19 Piccola modifica per variazioni ad "exec.h". Aggiunti
*             alcuni campi alla struttura principale (nuove opzioni).
* 01/09/98 GG 1.20 Aggiunto il campo "min_overrun", che specifica
*             il numero di cicli in overrun concessi al programma senza
*             che venga emesso l'errore di ciclo troppo lungo.
* 04/09/98 GG 1.21 Aggiunta la registrazione della priorita` iniziale
*             del processo in un aposito campo della struttura di stato
*             principale. Questo evita tutti i problemi legati all'uso
*             di "getprio".
* 24/09/98 GG 1.22 Aggiunto il campo "simul", che serve a simulare
*             la presenza di dispositivi senza in realta` accedervi.
*             Tutte le associazioni di I/O hanno successo, ma non si
*             hanno accessi all'hardware.
* 01/10/98 GG 1.23 Aggiunti i campi adatti a gestire un file di configurazione
*             degli I/O, e l'opzione "-path-ioconf" per ridefinire il default.
* 09/11/98 GG 1.24 Aggiunto il campo "defaultio", che specifica
*             la proprieta` di accesso per i segnali non citati nel file di
*             configurazione. La proprieta` di default e` R, il che significa
*             che i segnali non previsti nella configurazione sono da
*             considerare obbligatori.
* 28/07/00 GG 1.25 Aggiustamenti per compilazione su Linux.
* 17/09/02 RM 1.26 Eliminata la definizione di tipo timer_t se gia' definito 
*             in /usr/include/time.h
* 02/04/08 GG 1.27 Allargato lo spazio per le opzioni -devconf.
*/

#ifndef _QPLC_H_

#define _QPLC_H_

/* Sfiga... */
#include <signal.h>

#ifdef Linux
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
typedef void * csem_t;
#if !defined __timer_t_defined
typedef void * timer_t;
#endif
#define NSIG_XERR 6 /*caratteri della sigla processo*/

#else
#include <libxerr.h>
#include <hserax.h>
#endif
#include <nvram.h>
#include "modules.h"
#include "dynaload.h"
#include "drivers.h"
#include "shvar.h"
#include "superv.h"
#include "exec.h"
#include "compat.h"

#define MODULE_LABEL "module_name"

#define VER_IDX_KERNEL 0
#define OLD_MOD_IDX_HATSH 1
#define OLD_MOD_IDX_USER 2
#define OLD_MOD_IDX_480 3
#define OLD_IDX_BUILTIN_USER 4

#define MAXDRVCONF 500

/*
* Definizione della struttura rappresentante l'entry di una variabile
* condivisa (alias "segnale di scambio").
*/

	#pragma pack(1)
typedef struct {

/* Offset della variabile rispetto all'inizio del blocco di shared memory
 del PLC. E` espresso in byte per ragioni di efficienza. Se l'elemento
 non e` ancora assegnato, questo campo vale 0. */

	long off;

/* Non utilizzato. */

	unsigned char filler;

/* Dimensione (numero di colonne) della matrice di cui e` la base.
 Significativo SOLO per l'elemento [0][0] di una variabile matriciale. */

	unsigned short dim;

} shvar_t;
	#pragma pack()

/*
* Definizionde della struttura condivisa. Rappresenta lo stato
* del Plc per i processi esterni. Contiene molti campi non utilizzati,
* per compatibilita` con ISAGRAF.
*/

typedef struct {

/* Tabella dei descrittori degli "spazi" occupati in stile Isagraf. */

	struct { long size; long off; } isaker_spc_dir[40];

/* Tabella dei descrittori degli "spazi" liberi in stile Isagraf. */

	struct { long size; long off; } isaker_spc_free[40];

/*
* Stato del Plc (grezzo)
*/
/* Flag di PLC in GO. */

	int go;

/* Indirizzo di base dell'area condivisa nello spazio di indirizzamento
 del PLC. Serve agli altri programmi per "tradurre" i puntatori prodotti
 dal PLC. */

	char * vmembase;

/* Flag indicante che l'inizializzazione delle aree condivise controllate
 dal PLC e` stata completata. */

	int shm_ok;

/* Versione del database dei simboli.
  Questa variabile e` mantenuta solo per compatibilita`. */

	long isaker_dbver;

/* Flag indicante aggiornamento dei simboli. E` mantenuta solo per
 compatibilita`. */

	int isaker_ldsym;

} qplc_shared;

/* Struttura di stato (interno) del programma. */

typedef struct {

/* Nome completo dell'eseguibile. */

	char exe_path[MAXPATHLEN + 1];

/* Nome "base" dell'eseguibile. */

	char * exe_base;

/* Versione del kernel PLC. */

	char * ver_kernel;

/* Tabella delle sigle di versione. */

#define MAX_VER_SIG 8

	char * ver_sig[MAX_VER_SIG];

/* Sigla ad uso del visualizzatore degli errori. */

	char err_sigla[NSIG_XERR + 2];

/* Flag di modalita` "stand alone". */

	char stand_alone;

/* Flag che forza il ricorso ai dati macchina anche in "stand alone". */

	char mdata;

/* Flag che abilita alcuni controlli su certe richieste del debugger. */

	int chkaddr;

/* Flag che controlla l'entrata in modalita` bloccante delle scritture sulle
 pipe dei messaggi e degli errori. */

	int blocking_pipe;

/* Flag per l'inibizione dei messaggi. */

	char quiet;

/* Flag per l'uso della primitiva POSIX "poll" invece di quella System V
 "select". "poll" vuole "streams" installato. */

	char posix_poll;

/* Flag per la visualizzazione della versione. */

	char verify;

/* Flag per il salvataggio della configurazione hardware in NVRAM. */

	char old_save_io;

/* Flag per il reset della memoria condivisa alla partenza. */

	char reset_shm;

/* Flag di debug. */

	char debug;

/* Flag di simulazione. */

	char simul;

/* Flag che inibisce il fault. */

	char no_fault;

/* Flag che permette l'entrata in GO se l'MSD in stile IL richiede
 segnali di I/O inesistenti. */

	char go_anyway;

/* Flag per la non redirezione di stdout ed stderr. */

	char nstdout;

/* Flag per la correzione del problema dell'arrotondamento del periodo
 in Lynx 2.3.0. */

	char timer_bug;

/* Flag di uso della scheda INAX come sorgente di temporizzazione. */

	char use_inax;

/* Flag di HALT al lancio. */

	char halt;

/* Flag per l'abilitazione del servizio di debug remoto. */

	char remote_debug;

/* Flag di abilitazione della misura software del tempo di ciclo. */

	char soft_tm;

/* Flag per evitare il caricamento dei moduli al lancio. */

	char no_mod_hatsh;
	char no_mod_user;
	char no_mod_prog480;

/* Flag di reinizializzazione dei segnali di scambio. */

	char shv_init;

/* Flag di reinizializzazione della lista delle variabili a rilevamento
 di fronte. */

	char shv_mkedge;

/* Maschera di creazione dei segnali di scambio (vedi "compat.h"). */

	int package;

/* Dimensione della coda di messaggi da esecutore a supervisore. */

	int qlen_exec_to_sv;
 
/* Dimensione del buffer di debug. */

	int debug_buffer_size;

/* Numero massimo di cicli memorizzati in debug. */

	int n_debug_events;   

/* Numero massimo di programmi (tipicamente IL) contenuti in un modulo. */

	int max_il_prog_mod;

/* Indice del programma (tipicamente IL) corrente. */

	int curr_prog_id;

/* Numero massimo di ripetizioni della fase iniziale di halt. */

	unsigned long max_halt_cycle;

/* Handle del server dei dati macchina. */

	char * m_data;

/* Chiave dei semafori SystemV utilizzati per compatibilita` con Isagraf. */

	int isaker_sysv_sem_key;

/* ID dei semafori. */

	int isaker_sysv_sem_id;

/* Chiave della shered memory SystemV utilizzata. */

	int sysv_shm_key;

/* ID della shared memory. */

	int sysv_shm_id;

/* Descrittori delle aree di RAM non volatile utilizzate dal PLC. */

	char * nvram1_name;
	nvram_t nvram1;
	unsigned char * nvram1_free;
	char * nvram2_name;
	nvram_t nvram2;
	unsigned char * nvram2_free;

/* Indirizzo della shared memory. */

	qplc_shared * shm_addr;

/* Dimensione della shared memory. */

	int shm_size;

/* Dimensione della tabella delle variabili condivise. */

	int shvk_tab_len;

/* Dimensione della tabella di hash. */

	int shvh_tab_len;

/* Puntatore all'area di controllo "isaker-compatibile". */

	ik_system_t * osys;

/* Puntatore alla tabella delle variabili condivise (per chiave). */

	shvar_t * shvk;

/* Puntatore alla tabella di hash. */

	long * shvh;

/* Puntatore alla tabella delle variabili condivise (per nome). */

	shv_t * shvn;

/* Puntatore alla tabella inversa chiave / descrittore. */

	shv_t ** shvkey_to_descr;

/* Puntatore alla cella che contiene l'offset della tabella dei nomi. */

	long * p_shvn;

/* Puntatore alla cella che contiene la dimensione della tabella dei nomi. */

	long * p_shvn_size;

/* Puntatore alla cella che contiene l'offset dello pseudo-heap condiviso. */

	long * p_shheap;

/* Puntatore alla cella che contiene la dimensione dello pseudo-heap
 condiviso. */

	long * p_shheap_size;

/*
* Variabili relative ai moduli caricati dinamicamente.
*/

	module_t * mod_hatsh;
	mod_hatsh_t mod_hatsh_op;
	module_t * mod_user;
	mod_user_t mod_user_op;
	module_t * mod_plc480;
	mod_plc480_t mod_plc480_op;

/* Struttura di controllo del sistema dei driver. */

	driver_control_t drivers;

/* Dimensione massima della tabella delle estensioni. */

	int max_extensions;

/* Stringa di configurazione ad uso del sistema dei driver. */

	char devconf[MAXDRVCONF + 1];

/* Proprieta` di default dei segnali non previsti nel file di
 configurazione. */

	char defaultio[2];

/* Nomi dei file associati ai moduli caricati al lancio. "" se non si
 desidera il caricamento automatico. */

	char mod_hatsh_path[MAXPATHLEN + 1];
	char mod_user_path[MAXPATHLEN + 1];
	char mod_prog480_path[MAXPATHLEN + 1];

/* Nome del file di configurazione degli I/O. */

	char ioconf_path[MAXPATHLEN + 1];

/* Variabili legate alla gestione "vecchio stile" delle variabili
 a rilevamento di fronte. */

	int n_edge_bool;	/* Numero di variabili BOOL. */
	int n_max_edge_bool;	/* Numero massimo di variabili BOOL. */
	char **p_edge_bool;	/* Tabella dei puntatori BOOL. */
	char *p_image_bool;	/* Immagini delle variabili BOOL. */
	int n_edge_long;	/* Idem per i LONG... */
	int n_max_edge_long;
	long **p_edge_long;
	long *p_image_long;

/* Priorita` del processo al momento del lancio. */

	int start_prio;

/* Incremento di priorita` fittizio, ad uso del gestore di TCP/IP
 di esecuzione. */

	int tcp_prio;

/* Incremento di priorita` del thread principale all'entrata nel ciclo
 di esecuzione. */

	int prio_exec;

/* Numero di cicli in overrun ammessi prima della segnalazione. */

	int min_overrun;

/* Numero massimo di cicli in overrun ammessi, dopodiche`, BUM !. */

	int max_overrun;

/* Periodo del Plc, in millisecondi. */

	int period;

/* Periodo con cui e` controllato il funzionamento del timer INAX. */

	int inax_wd_period;

/* Puntatore al flag "CNOK". */

	unsigned char * p_cnok;

/* Puntatori alle strutture di stato delle componenti "esecutore" e
 "supervisore". */

	exec_status_t *p_exec_status;
	sv_status_t *p_superv_status;

/* Path del file di NVRAM "dei poveri". */
	char nvram_path[MAXPATHLEN + 1];
/* Periodo di rinfresco del file di NVRAM in millisecondi. */
	int nvram_refresh;
/* Dimensione della parte utile di NVRAM su file. */
	int nvram_size;
/* Dimensione totale del file di NVRAM. */
	int nvram_totsize;
/* Puntatore alla mappa della parte utile del file di NVRAM. */
	void *nvram_filemap;
/* Puntatore all'area immagine dell'NVRAM (memoria locale). */
	void *nvram_image;

} qplc_statics;

/*
* Struttura di stato del programma (stato generale : file eseguibile,
* opzioni di lancio...).
*/

extern qplc_statics main_status;


#endif /* _QPLC_H_ */


