/*
* @(#) superv.h 1.14 Thu Feb  8 15:04:38 MET 2001
*
*  Questo file contiene il supervisore del PLC. Il supervisore e`
* un thread a priorita` ordinaria che controlla l'esecutore e
* gestisce il colloquio col debugger.
*
* 13/06/96 GG 1.0 Prima stesura.
* 11/06/97 GG 1.1 Aggiunto il parametro "update" alla funzione
*             "superv_message", per poter sfruttare le nuove prestazioni
*             di quote introdotte con la versione 2.6.0.15 . Di conseguenza,
*             e` stato aggiunto il comando "SV_COMM_MESSAGE_UPD" ad uso
*             dell'esecutore.
* 16/07/97 GG 1.2 Aggiunto il comando "SV_COMM_FINISH_HALT",
*             per l'esecuzione di azioni lente (ma che terminano sicuramente)
*             da parte del supervisore al termine dell'entrata in HALT.
* 05/11/97 GG 1.3 Aggiunto il comando SV_COMM_CALLBACK, che serve ad attivare
*             la callback QPLC_CB_SUPERV.
* 07/11/97 GG 1.4 Resa pubblica la funzione "superv_exec".
* 10/11/97 GG 1.5 Resa pubblica la funzione "superv_to_exec".
*             Aggiunta la definizione della struttura utilizzata dalla callback
*             degli errori.
* 11/11/97 GG 1.6 Aggiunto il comando di attivazione della callback per
*             l'esecutore. Aggiunti comandi per la gestione della statistica
*             e per la segnalazione di eventi asincroni.
* 12/03/98 GG 1.7 Aggiunto il comando SV_COMM_FORCEIO, per la forzatura
*             degli I/O da debugger (usare con estrema cautela).
*             Aggiunto un semaforo per bloccare l'acceso all'esecutore
*             durante le operazioni lunghe.
* 10/06/98 GG 1.8 Aggiunto il comando "SV_COMM_SAVEAX" per il salvataggio
*             dei dati di un asse dato l'indice assoluto dell'asse.
* 18/02/99 GG 1.9 Aggiunto il comando "SV_COMM_SAVEGENTAB" per il salvataggio
*             dei dati di una tabella utente noto il nome.
* 28/07/00 GG 1.10 Aggiustamenti per compilazione su Linux.
* 28/07/00 GG 1.11 Aggiunti eventi asincroni per cancellazione errori
*             e gestione messaggi.
* 03/11/00 GG 1.12 Aggiunti quattro comandi per la gestione avanzata
*             del buffer di debug (vedi "debugger.*" e "remote.*").
* 12/12/00 GG 1.13 Aggiunta la funzione "superv_saveorig", che salva
*             le tabelle origini, e il comando "SV_COMM_SAVEORIG", per
*             permettere la chiamata della funzione da esecutore.
* 08/02/01 GG 1.14 Aggiunta la funzione "superv_savetab", che salva
*             qualsiasi tabella, e il comando "SV_COMM_SAVETAB", per
*             permettere la chiamata della funzione da esecutore.
*/

#ifndef _SUPERV_H_

#ifndef Linux
#include <sem.h>
#endif
#include <pthread.h>

#define SV_MAX_MSG_LEN 100

typedef struct {
/* Lunghezza della parte dati. */
	int len;
/* Messaggio - parte di comando. */
	char cmd;
/* Messaggio - parte dati. */
	char data[SV_MAX_MSG_LEN];
} sv_msg_t;

typedef struct {
/* TID del thread supervisore. */
	pthread_t tid;
/* Semaforo di attivazione del thread supervisore. */
	csem_t sv_act_sem;
/* Semaforo di sincronizzazione col thread esecutore. */
	csem_t sv_sync_sem;
/* Semaforo di sincronizzazione coll'esecutore per
 operazioni che richiedono piu` di un ciclo. */
	csem_t sv_lockexec_sem;

/* Flag che indica che il task che ha inviato il comando non utilizza
 il semaforo. Questo flag sara` riportato a 0 dal supervisore. */
	char sync;
/* Semaforo di risposta, per la riattivazione del richiedente (eccetto
 quando sync == 1). */
	csem_t sv_answ_sem;
/* Semaforo di accesso al comando-messaggio. */
	pthread_mutex_t m_mutex;
/* Puntatore al messaggio. */
	sv_msg_t *msg;
/* Puntatore all'ultimo modulo caricato. */
	module_t *last_module;
} sv_status_t;

/* Dichiarazione della struttura di stato del supervisore, ad uso degli
 altri moduli. */
extern sv_status_t superv_status;

/* Tipi di messaggi. Questi codici sono utilizzati anche dall'esecutore.
  Alcuni codici hanno senso solo per l'esecutore, altri solo per il
 supervisore. Benche`, a rigore, i due ambiti non siano confrontabili,
 si e` preferito mantenere lo stesso formato dei messaggi e dei comandi
 sia per il supervisore che per l'interprete per ragioni di semplicita`. */

enum {
/* Fa niente. */
	SV_COMM_NULL = 0,
/* Plc in GO. */
	SV_COMM_GO,
/* Plc in HALT. */
	SV_COMM_HALT,
/* Plc in FAULT. */
	SV_COMM_FAULT,
/* Sincronizza il supervisore con l'esecutore (cioe`, fa quasi niente). */
	SV_COMM_SYNC,
/* Esegue una lista di comandi descritta dal campo "data" del messaggio.
  Il campo "data" deve essere interpretato come un "void *", da passare
 alla funzione "exec_explore_queue". */
	SV_COMM_EXECLIST,
/* Visualizza un errore. "data" deve essere interpretato
 come "sv_comm_error_t". */
	SV_COMM_ERROR,
/* Cancella un errore dalla coda degli errori. "data" e` interpretato
 come "long" */
	SV_COMM_DELERR,
/* Visualizza un warning su stderr. Stessa convenzione utilizzata per
 gli errori. */
	SV_COMM_WARNING,
/* Visualizza un messaggio. Stessa convenzione utilizzata per
 gli errori. */
	SV_COMM_MESSAGE,
/* Cancella un messaggio. "data" e` interpretato
 come "long" */
	SV_COMM_DELMESS,
/* Caricamento dinamico del file "data". */
	SV_COMM_DYLOAD,
/* Scaricamento dell'ultimo modulo caricato dinamicamente. */
	SV_COMM_DYUNLOAD,
/* L'esecutore ha richiesto l'aiuto del supervisore per
 rispondere ad una richiesta impegnativa da parte di un
 debugger vecchio stile. */
	SV_COMM_OLDCOMM,
/* Visualizza un messaggio sovrascrivendo quelllo precedente.
 Stessa convenzione utilizzata per gli errori. */
	SV_COMM_MESSAGE_UPD,
/* Esecuzione della fase finale dell'entrata in HALT (richiesta da
 parte dell'esecutore). */
	SV_COMM_FINISH_HALT,
/* Attivazione della callback (supervisore). */
	SV_COMM_CALLBACK,
/* Attivazione della callback (esecutore). */
	SV_COMM_CBEXEC,
/* Attivazione della statistica. */
	SV_COMM_ENMEAS,
/* Disattivazione della statistica. */
	SV_COMM_DISMEAS,
/* Azzeramento della statistica. */
	SV_COMM_CLRMEAS,
/* Ottenimento della statistica. */
	SV_COMM_MEAS,
/* Comunicazione di un evento asincrono su richiesta dell'esecutore. */
	SV_COMM_EVNOTIFY,
/* Creazione di una lista di watch. */
	SV_COMM_WCREATE,
/* Distruzione di una lista di watch. */
	SV_COMM_WDELETE,
/* Aggiunta di un break ad una lista. */
	SV_COMM_WADD,
/* Abilitazione di una lista. */
	SV_COMM_WENABLE,
/* Disabilitazione di una lista. */
	SV_COMM_WDISABLE,
/* Distruzione di tutte le liste. */
	SV_COMM_WRESET,
/* Forzatura degli I/O. */
	SV_COMM_FORCEIO,
/* Salvataggio delle tabelle assi del centro a cui appartiene l'asse
 di numero interno dato. */
	SV_COMM_SAVEAX,
/* Salvataggio di una tabella utente dato il nome. */
	SV_COMM_SAVEGENTAB,
/* Attivazione della modalita` di watch "lunga" (registrazione
 di tutti gli eventi). */
	SV_COMM_WLONGHISTORY,
/* Attivazione della modalita` di watch "compressa" (registrazione
 di un solo evento per lista di watch). */
	SV_COMM_WSHORTHISTORY,
/* Blocco dell'accesso alla storia di watch. */
	SV_COMM_WLOCKHISTORY,
/* Sblocco dell'accesso alla storia di watch. */
	SV_COMM_WUNLOCKHISTORY,
/* Salvataggio delle origini. */
	SV_COMM_SAVEORIG,
/* Salvataggio dei una tabella qualunque. */
	SV_COMM_SAVETAB,
/* ... */
};

/* Tipo associato al comando SV_COMM_ERROR. */
typedef struct {
	long code;
	char message[SV_MAX_MSG_LEN - sizeof(long)];
} sv_comm_error_t;

/* Possibili stati dell'elaborazione di un comando. */
#define SUPERV_PHASE_DONE 0
#define SUPERV_PHASE_BUSY 1
#define SUPERV_PHASE_WORKING 2

/* Struttura utilizzata dalla callback degli errori. */
typedef struct {
	long code;
	char *text;
} sv_err_t;

/* Struttura utilizzata dalla callback dei messaggi. */
typedef struct {
	long code;
	char action;
/* Tipi di azioni ammessi. */
#define	SV_MSG_ACTION_DELETE 0
#define	SV_MSG_ACTION_ADD 1
#define	SV_MSG_ACTION_UPDATE 2
	char *text;
} sv_cbmsg_t;

/* Tipo associato al comando SV_COMM_FORCEIO. */
typedef struct {
/* Flag di successo (1: OK, 0: errore). */
	int rv;
/* Valore da assegnare all'uscita o letto dall'ingresso. */
	union { char ch; short sh; long lo; float fl; double db; } val;
/* Semaforo per l'esecutore. */
	csem_t sem;
/* Modo di accesso (0: INPUT, 1: OUTPUT). */
	int mode;
/* Numero di bit del segnale. */
	int bit;
/* Stringa descrivente il segnale. */
	char path[SV_MAX_MSG_LEN-3*sizeof(int)-sizeof(double)-sizeof(csem_t)];
} sv_comm_forceio_t;

/* Tipo associato al comando SV_COMM_SAVETAB. */
typedef struct {
	int cen;
	char tabname[SV_MAX_MSG_LEN - sizeof(int)];
} sv_savetab_t;

/*
* Funzione "superv_start"
* -----------------------
*
*  Questa funzione lancia il thread supervisore.
*/

void superv_start(void);


/*
* Funzione "superv_talk" e "superv_talk_nonblock"
* -----------------------------------------------
*
*  Questa funzione trasmette al supervisore il messaggio "msg". All'uscita,
* l'area puntata da "msg" conterra` la risposta, se il comando la prevede.
*  Tale area deve percio` essere statica, o comunque disponibile e non
* alterara per tutta la durata del colloquio tra supervisore e chiamante.
*
*  L'esecutore del programma PLC deve sempre utilizzare la versione "nonblock"
* della funzione, che, anziche` essere "void", e` "int", e vale
*  SUPERV_PHASE_DONE : l'operazione e` stata completata,
*  SUPERV_PHASE_BUSY : risorsa bloccata, altro colloquio in corso;
*  SUPERV_PHASE_WORKING : lock impostato, attesa di risposta.
*  E` inoltre presente un altro parametro, "phase", che deve contenere
* il valore di ritorno della funzione alla chiamata precedente. La variabile
* che il task utilizzera` per memorizzarlo e` di fatto una variabile di
* stato che rappresenta la fase di avanzamento dell'elaborazione di un
* comando da parte del supervisore. Il parametro "phase" deve valere 0
* quando un task chiama per la prima volta questa funzione.
*/

void superv_talk(sv_msg_t *msg);
int superv_talk_nonblock(sv_msg_t *msg, int phase);

/*
* Funzione "superv_error"
* -----------------------
*
*  Questa funzione visualizza un errore di codice "code" e testo "text".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_error(long code, char * text);

/*
* Funzione "superv_delerr"
* ------------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_delerr(long code);

/*
* Funzione "superv_warning"
* -------------------------
* 
*  Questa funzione visualizza un "warning" su stderr.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_warning(long code, char * text);

/*
* Funzione "superv_message"
* -------------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text".
*  Il messaggio e` accodato o sovrascritto a seconda che "update" valga 0
* o 1.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_message(long code, char * text, int update);

/*
* Funzione "superv_delmess"
* ------------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_delmess(long code);

/*
* Interprete dei comandi del supervisore. Puo` essere eseguita solo
* dal thread supervisore.
*/

void superv_exec(sv_msg_t *msg);

/*
* Invio di un comando all'esecutore. Puo` essere eseguita solo dal thread
* supervisore.
*/

void superv_to_exec(sv_msg_t *msg);

/*
* Funzione "superv_saveax"
* ------------------------
*
*  Salva i dati asse dato l'indice assoluto "ax".
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_saveax(long ax);

/*
* Funzione "superv_savegentab"
* ----------------------------
*
*  Salva la tabella utente dato il nome.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_savegentab(char *tab);

/*
* Funzione "superv_saveorig"
* --------------------------
*
*  Salva i dati origine.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_saveorig(void);

/*
* Funzione "superv_savetab"
* -------------------------
*
*  Salva la tabella qualsiasi dato il nome ed il centro.
*  Puo` essere chiamata solo dal supervisore o dal thread principale
* se il supervisore non e` ancora partito.
*/

void superv_savetab(char *tabname, int cen);

#define _SUPERV_H_

#endif

