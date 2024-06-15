/*
* @(#) exec.c 1.43 Tue Sep  9 01:21:25 CEST 2008
*
*  Questo file contiene il cuore del thread esecutore del Plc.
*
* 11/06/96 GG 1.0 Prima stesura.
* 30/08/96 GG 1.1 Aggiunta le funzione "plcPeriod". Aggiunta la gestione
*             dell'opzione "-timer-bug" per Lynx2.3.0. Aggiunte anche le
*             funzioni "plcNCycle" e "plcTime".
* 04/09/96 GG 1.2 Aggiunta una "memlk" all'entrata nel ciclo principale,
*             anche se non dovrebbbe essercene bisogno.
* 05/09/96 GG 1.3 Corretta la procedura di "halt", che non chiamava il
*             metodo di chiusura del modulo "user".
*             Corretta la procedura di "go", che, dopo la prima volta,
*             non chiamava piu` il metodo "start" del modulo "user",
*             ne` eseguiva la reinizializzazione dei driver.
* 06/09/96 GG 1.4 Corretto il meccanismo di accodamento dei messaggi da
*             esecutore a supervisore : la lista circolare non veniva
*             aggiornata correttamente.
* 16/09/96 GG 1.5 Spostato il trace per vedere evolvere gli autoresettanti.
* 17/09/96 GG 1.6 Aggiunta la possibilita` di inibire l'entrata in fault,.
* 18/09/96 GG 1.7 Gestito il segnale di errore matematico. Provoca l'emissione
*             di un errore e l'entrata in HALT.
* 19/09/96 GG 1.8 Aggiunto un'elaborazione dei parametri in stile "printf"
*             alle funzioni "plcError", "plcWarning" e "plcMessage".
*             Spostato il controllo sull'overrun, per poter indicare il
*             numero di cicli perduti.
*             Cambiati i codici di errore.
* 25/09/96 GG 1.9 Migliorata la gestione del caso di "coda piena". Ora
*             la funzione "exec_enqueue" fornisce un valore che riflette
*             il successo dell'operazione, inoltre favorisce l'inserimento
*             di messaggi ad alta priorita` (come i comandi), a scapito di
*             quelli a priorita` minore (come gli errori). Queste contromisure
*             dovrebbero rendere meno probabile la perdita di messaggi
*             importanti in situazioni in cui l'esecutore emette continuamente
*             richieste di visualizzazione di errori. In ogni caso, dovrebbero
*             risolvere il problema del blocco del colloquio col debugger.
*             Corretta un'imperfezione nell'impostazione della priorita`
*             dell'esecutore. Dando fiducia al manuale, tutti i thread
*             avrebbero dovuto acquisire la stessa priorita` (ma non pare
*             che questo risponda a verita`. Comunque la cosa e` stata
*             corretta).
*             Iniziata la gestione delle informazioni aggiuntive associate
*             al segnale di errore matematico.
* 26/09/96 GG 1.10 Completata la gestione dell'errore matematico (anche
*             perche`, meglio di cosi`, con Lynx, non si fa).
*             Aumentata la lunghezza massimo di messaggi ed errori.
* 27/09/96 GG 1.11 Aggiunta la funzione "plcRuntimeError", che emette errori
*             associati a valori del program counter, espressi in forma
*             comprensibile.
* 10/10/96 GG 1.12 Aggiunta la chiamata alla funzione "compat_begin_cycle" in
*             tutti i cicli che chiamano gia` "compat_trace". Questa nuova
*             funzione, chiamata all'inizio del ciclo dopo "hat", serve ad
*             eseguire il backup dei segnali di scambio non azzerabili.
* 17/10/96 GG 1.13 Aggiunta la richiesta di cambio della politica di
*             schedule.
* 30/10/96 GG 1.14 Resa un po' piu` solida la chiamata di "memlk".
* 11/12/96 GG 1.15 Aggiunte le chiamate alle funzioni di inizio e fine debug.
* 15/01/97 GG 1.16 Modificata la funzione "plcRuntimeError", in modo che
*             visualizzi anche l'indice del programma corrente (se noto).
* 05/02/97 GG 1.17 Spostato un tratto di codice che usa sleep in una posizione
*             in cui non e` ancora stata alterata la maschera del segnale
*             SIGALRM. C'era la possibilita` teorica di incastri.
* 06/02/97 GG 1.18 Aggiunti altri 32 bit al contatore di tempo trascorso.
*             Di conseguenza, e` stata definita la funzione plcTimeH, che
*             restituisce questa nuova informazione.
* 07/02/97 GG 1.19 Aggiunta l'inizializzazione esplicita della variabile
*             che contiene la parte alta del tempo trascorso. Si tratta solo
*             di paranoia.
* 23/05/97 GG 1.20 Aggiunta la possibilita` di ripetere la chiamata delle
*             procedure di HALT (quelle di utente ed i metodi "down" dei
*             driver). Sono poi state aggiunte le funzioni "plcHaltCycle",
*             che fornisce il numero di ripetizioni della fase di halt (0
*             indica il primo ciclo), e "plcRepeatHalt", che richiede la
*             ripetaizione della procedura di entrata in HALT. Esiste poi
*             un limite nel numero di ripetizioni, superato il quale si
*             ha un errore, e nessuna ulteriore ripetizione. Tutte le
*             procedure di HALT dovrebbero prevedere la possibilita` di
*             essere richiamate "controvoglia" a seguito di una richiesta
*             di ripetizione da parte di qualche altra componente. La
*             funzione "plcHaltCycle" puo` tornare utile per questo.
* 11/06/97 GG 1.21 Aggiunta la funzione "plcUpdateMessage", che aggiorna
*             un messaggio anziche` accodarlo a quelli gia` presenti
*             (ma ci vuole la versione 2.6.0.15 di quote).
* 11/11/97 GG 1.22 Aggiunta la chiamata della callback apposita alla
*             ricezione del comando SV_COMM_CBEXEC. Aggiunti i comandi di
*             gestione della statistica su richiesta del supervisore.
*             Aggiunta la funzione "plcEventNotify".
*             Aggiunta la segnalazione di entrata in GO, HALT e FAULT.
* 12/11/97 GG 1.23 Aggiunti comandi per la gestione delle liste
*             di watch.
* 26/11/97 GG 1.24 Riportata (e migliorata) la correzione eseguita in BiEsse
*             sulla versione 1.4.2.3 (divenuta 1.4.2.4). Consiste nello
*             spostamento della procedura di sorveglianza dei dispositivi
*             prima della chiamata del programma utente, in modo che sia
*             eseguita ad intervalli regolari anziche` dipendenti dalla
*             lunghezza del programma. Nelle precedenti versioni poteva
*             accadere che cicli occasionalmente prossimi al periodo nominale
*             seguiti da cicli molto brevi provocassero una segnalazione
*             erronea di dispositivo (tipicamente IOS) in timeout.
* 02/12/97 GG 1.25 Aggiunto un parametro a "plcEventNotify".
* 12/03/98 GG 1.26 Aggiunto un semaforo di sincronizzazione per sbloccare
*             il supervisore nelle operazioni che richiedono piu` cicli.
*             Aggiunto un comando per manipolare temporaneamente gli I/O
*             (ad uso del debugger).
*             Aggiunta la funzione "plcIsHalt", che vale 1 solo se lo stato
*             corrente e` "HALT ordinario e stabilizzato". Solo per uso
*             interno.
* 02/04/98 GG 1.27 Fatto sparire un warning.
* 10/06/98 GG 1.28 Aggiunta la funzione "plcSaveAx", per il salvataggio dei
*             dati asse.
* 24/07/98 GG 1.29 Profonde modifiche nella gestione delle sorgenti di
*             temporizzazione, per poter sincronizzare l'esecutore ad INTX
*             o per pilotare direttamente la INAX e superare il limite di
*             10 millisecondi di tempo di ciclo.
* 01/09/98 GG 1.30 Aggiunti punti di test (normalmente commentati) per
*             misure di velocita`.
*             Aggiunta la possibilita` di non segnelare l'errore di ciclo
*             troppo lungo se il numero di cicli in overrun non supera
*             una quantita` imposta da linea di comando.
* 02/09/98 GG 1.31 Corretta una piccola incongruenza formale nell'impostazione
*             della maschera di attesa dei segnali.
*             Ammesso il valore 0 come periodo del timer POSIX. Significa
*             "timer disabilitato". Serve ad inibire il meccanismo di
*             sorveglianza sull'attivita` del gestore della scheda INAX.
* 04/09/98 GG 1.32 Affidato ad un thread separato il meccanismo di
*             sorveglianza nella modalita` sincronizzata ad intx. Questo
*             dovrebbe risolvere il (presunto) difetto di "sigwait".
*             Aggiunta l'inizializzazione di alcuni campi (paranoia).
*             Migliorata la gestione del divisore.
* 18/02/99 GG 1.33 Aggiunta la funzione "plcSaveGenericTable", per il
*             salvataggio di una tabella utente dato il nome.
*             Raccolto un po' di codice di colloquio col supervisore.
*             Cambiata la priorita` della primitiva di salvataggio assi.
* 28/07/00 GG 1.34 Aggiustamenti per compilazione su Linux.
* 03/11/00 GG 1.35 Aggiunti quattro comandi per la gestione avanzata
*             del buffer di debug (vedi "debugger.*" e "remote.*").
* 08/11/00 GG 1.36 Eliminata la "brkHistoryMode(0)" dal comando di
*             reset delle liste, perche` inutile.
* 13/11/00 GG 1.37 Corretto un errore nell'impostazione della priorita`
*             dell'esecutore su Linux. Sintomi: sfighe varie coi comandi
*             da debugger.
* 07/12/00 GG 1.38 Aggiunta nella routine di start del PLC
*             la chiamata di una funzione per ricalcolare
*             i dati utilizzati internamente dalle funzioni di validazione
*             degli indirizzi. Si e` resa necessaria su Linux, dove
*             il calcolo al volo e` lento, ed e` necessaria percio`
*             una "cache". Su LynxOS la funzione non fa nulla.
* 12/12/00 GG 1.39 Aggiunta la funzione "plcSaveOrig", che permette di
*             salvare i dati delle origini.
* 08/02/01 GG 1.40 Aggiunta la funzione "plcSaveTable", che permette di
*             salvare qualsiasi tabella dati il nome ed il centro.
* 07/05/01 GG 1.41 Piccola modifica per Linux, dove plcTime puo` dare
*             risulatati poco realistici a causa delle latenze di sistema.
* 07/05/01 GG 1.42 Aggiunto un flag di "HALT in corso" per evitare
*             "raffiche" di "plcHalt". Evita che errori hardware
*             provochino un ciclo infinito se e` attivata l'opzione
*             "-no-fault".
* 08/09/08 GG 1.43 Evitata l'uscita in caso di mancanza di privilegi.
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <setjmp.h>
#include <pthread.h>
#ifdef Linux
#include <sys/types.h>
#include <sys/mman.h>
#include <sched.h>
#else
#include <types.h>
#include <sys/timers.h>
#include <sys/events.h>
#include <memlk.h>

#include <inax-3.h>
#endif

#include "qplc.h"
#include "cback.h"
#include "util.h"
#include "debugger.h"
#include "remote.h"

#ifdef HARD_DEBUG
#define HARD_DEBUG_PORT 0x378
static int hard_debug_port_val = 0;
#define HTPSET(x) util_outb((hard_debug_port_val|=(x)),HARD_DEBUG_PORT)
#define HTPRES(x) util_outb((hard_debug_port_val&=~(x)),HARD_DEBUG_PORT)
#else
#define HTPSET(x)
#define HTPRES(x)
#endif

/* Punto di rientro in caso di blocco del programma PLC. */

#ifdef Linux
static sigjmp_buf emergency;
#else
static jmp_buf emergency;
#endif

/*
* Struttura di stato dell'esecutore.
*/

exec_status_t exec_status;

/*
* Funzione di gestione dell'errore matematico.
*
* Grezzo. Un errore, l'entrata in HALT, e basta.
*/

static void exec_fperr(int s,
#ifdef Linux
/* Boh? Sembra che sigcontext non sia ben allineato, e che SA_SIGINFO
 non funzioni. */
#else
                       int code,
#endif
                       struct sigcontext context,
                       struct fp_context *fpc
                       )
{
unsigned long eip = context.eip;

	plcRuntimeError(1, eip);
/* ERRORE. Divisione per zero o altro errore matematico. */


/* HALT ! */
	plcHalt();

/* Su LynxOS 2.3.0 bisogna farlo. */

	util_fpu_reset();

/* Si torma al ciclo principale. */

#ifdef Linux
	siglongjmp(emergency,2);
#else
	longjmp(emergency,2);
#endif
}

/* Per far sparire un warning. */
static int exec_enqueue(sv_msg_t * msg, int prio);

/*
* Aggiornamento del contatore di clock per la modalita` sincronizzata
* ad intx. Vale 0 se il contatore non e` arrivato in fondo, 1 se
* si e` a fine conteggio oppure non si e` nella modalita` sincronizzata.
* Rinfresca anche il flag di segnale ricevuto.
*/

static int exec_handle_clock(void)
{
/* n_inax_ticks vale 0 se non si e` in modalita` sincrona. In questo
 caso si restituisce comunque "1". */
	if (exec_status.n_inax_ticks) {
	/* Indica "segnale ricevuto". */
		exec_status.inax_sig_flag = 1;
	/* Aggiorna il contatore. */
		if (++exec_status.curr_inax_tick >= exec_status.n_inax_ticks) {
			exec_status.curr_inax_tick = 0;
			return 1;
		}
		return 0;
	}
	return 1;
}

/*
* Funzione di timeout dell'esecutore.
*
* L'esecutore utilizza normalmente il segnale SIGALRM (o SIGUSR1 se
* e` sincronizzato con intx)  prodotto dal timer
* per essere attivato con periodo costante. Il thread si pone percio`
* in attesa del segnale chiamando la system call "sigwait". Se il segnale
* arriva prima che il thread entri nella "sigwait", e` chiamata questa
* funzione (la "sigwait" inibisce l'attivazione dell'handler), che
* provvede a registrare il fatto in una variabile e, se il numero di
* cicli "persi" supera il limite, forza l'HALT del Plc.
*/

static void exec_timeout(int s)
{
	HTPSET(0x80);
	HTPSET(0x80); /* Tanto per tenerla un po' lunga. */
	HTPSET(0x80);
	HTPSET(0x80);
	HTPSET(0x80);

/* Se il contatore di clock non e` arrivato alla fine,
  non e` ancora il momento di segnalare errore di ciclo
 troppo lungo. Ovviamente, se non si e` nella modalita` sincrona, il
 clock e` sempre a fine ciclo (quindi, errore sicuro). */

	if (exec_handle_clock()
	/* Pero` niente errore se si e` in debug. */
	 && (! main_status.debug)) {
		if (exec_status.curr_overrun >= main_status.max_overrun) {
		/* ERRORE. Ciclo oltre i limiti ammessi. */
			plcError(62,"%d",(long)exec_status.curr_overrun);
			exec_status.curr_overrun = 0;
			plcHalt();
#ifdef Linux
			siglongjmp(emergency,1);
#else
			longjmp(emergency,1);
#endif
		}
		else {
			++exec_status.curr_overrun;
		}
	}

	HTPRES(0x80);
}

/*
* FUNZIONI DI MISURA DEL TEMPO DI CICLO
* -------------------------------------
*/

/* Funzione vuota, ma utile. */
static void exec_null(void) { }

/*
* Funzione chiamata all'inizio di ogni ciclo, eccetto la prima volta.
*/

static void exec_tm_begin(void)
{
struct timezone zone;

	gettimeofday(&exec_status.c_begin_time,&zone);
}

/*
* Funzione chiamata alla fine di ogni ciclo.
*/

static void exec_tm_end(void)
{
struct timezone zone;

	gettimeofday(&exec_status.c_end_time,&zone);
	exec_status.curr_usec
	 = (long) exec_status.c_end_time.tv_usec
	   - (long) exec_status.c_begin_time.tv_usec
	   + ((long) exec_status.c_end_time.tv_sec
	       - (long) exec_status.c_begin_time.tv_sec) * 1000000L;

	if (exec_status.curr_usec > exec_status.max_usec)
		exec_status.max_usec = exec_status.curr_usec;
	if (exec_status.curr_usec < exec_status.min_usec)
		exec_status.min_usec = exec_status.curr_usec;
}

/*
* Funzione chiamata la prima volta che si inizia una misura.
*/

static void exec_tm_start(void)
{
	exec_status.tm_begin = exec_tm_begin;
	exec_status.tm_end = exec_tm_end;
	exec_tm_begin();
}

/*
* FASI DELL'ESECUTORE
* -------------------
*
*  Ogni fase e` codificata in una diversa funzione. Lo stato dell'esecutore
* e` rappresentato dal puntatore alla funzione che corrisponde alla fase
* corrente. Si e` fatto questo per ridurre la necessita` di eseguire test
* ciclici sullo stato, in cambio di una piccola crescita di codice.
*/

/*
* Questa funzione esegue il controllo dell'hardware comune a tutte
* le fasi tranne FAULT.
*/

static int exec_testhw(void)
{
	if (main_status.debug)
		return 1;
	ioTrigger();
	if (! ioCheck()) {
		plcFault();
	/* ERRORE. Problema hardware. */
	/*
		plcError(130,"hardware");
	*/
		return 0;
	}
	return 1;
}

/*
* Fase "RUN" : Ciclo ordinario di esecuzione.
*/

static void exec_run(void)
{
HTPSET(0x02);
	compat_chkhalt();
	ioRead();
	if (! exec_testhw())
		return;
	exec_status.deb_begin();
HTPRES(0x02);
HTPSET(0x04);
	main_status.mod_hatsh_op.hat();
	compat_begin_cycle();
HTPRES(0x04);
HTPSET(0x08);
	main_status.mod_user_op.main(0);
HTPRES(0x08);
HTPSET(0x10);
	compat_trace();
	main_status.mod_hatsh_op.shoes(0);
HTPRES(0x10);
HTPSET(0x20);
	exec_status.deb_end();
	exec_status.old_comm();
	ioWrite();
HTPRES(0x20);
}

/*
* Fase "START" : Primo ciclo attivo dopo il "GO".
*/

static void exec_start(void)
{
/* Azzeriamoli, va` la` */
	exec_status.repeat_start_halt = 0;
	exec_status.halt_cycle = 0;
/* Flag di HALT in corso, ovviamente falso. */
	exec_status.halt = 0;
/* Azzera gli errore di I/O pregressi. */
	ioClearErrors();

	compat_chkhalt();
	if (main_status.p_cnok)
		*main_status.p_cnok = 1;
	exec_status.cycle = exec_run;
	ioRead();
	if (! exec_testhw())
		return;
	exec_status.deb_begin();
	main_status.mod_hatsh_op.hat();
	compat_begin_cycle();
	main_status.mod_user_op.main(1);	/* 1 = "primo giro" */
	compat_trace();
	main_status.mod_hatsh_op.shoes(1);	/* 1 = "primo giro" */
	exec_status.deb_end();
	exec_status.old_comm();
	ioWrite();
/* Comunica l'entrata in go ai debugger collegati. */
	plcEventNotify(DSV_EVENT_GO,NULL,0);
}

/*
* Nucleo della fase "HALT".
*/

static void exec_halt_core(void)
{
	compat_chkgo();
	compat_begin_cycle();
	compat_trace();
	exec_status.old_comm();
	exec_testhw();
}

/*
* Fase "HALT" : Plc in HALT.
*/

static void exec_halt(void)
{
	exec_halt_core();
}

/*
* Fase "FINISH_HALT" : Esecuzione della parte asincrona dell'entrata in HALT.
*/

static void exec_finish_halt(void)
{
sv_msg_t msg;

/* Invia il comando di esecuzione della parte asincrona dell'entrata
 in HALT. */

	msg.cmd = SV_COMM_FINISH_HALT;
	msg.len = 0;
	exec_enqueue(&msg, CMDPRIO_COMMANDS);

/* D'ora in poi si esegue il ciclo HALT definitivo. Il supervisore
 dovrebbe inviare la conferma di entrata in HALT una volta eseguita
 la procedura asincrona. */

	exec_status.cycle = exec_halt;

	exec_halt_core();
}

/*
* Fase "inizio HALT con richiesta di ripetizione".
*/

static void exec_start_halt_repeated(void)
{
/* Azzera il flag di richiesta di ripetizione della fase iniziale di HALT. */
	exec_status.repeat_start_halt = 0;
/* Incrementa il contatore di richieste di ripetizione della fase iniziale
 della procedura di HALT. Se supera il limite imposto, errore. */
	if (++exec_status.halt_cycle <= main_status.max_halt_cycle
	    || main_status.max_halt_cycle == 0) {
	/* Esegue la procedura di stop del modulo "user defined". */
		main_status.mod_user_op.stop();
	/* Azzera (o finisce di azzerare) le uscite. */
		ioDown();
	/* Halt definitivo se non ci sono altre richieste di ripetizione. */
		if (! exec_status.repeat_start_halt)
			exec_status.cycle = exec_finish_halt;
	}
	else {
	/* Ricicla l'errore 63 (mi scoccia inventare un nuovo codice
	 per un caso del genere). */
		plcError(63,"halt");
	/* Forza l'halt definitivo. */
		exec_status.cycle = exec_finish_halt;
	}
	exec_halt_core();
}

/*
* Fase "inizio HALT".
*/

static void exec_start_halt(void)
{
/* Flag di HALT in corso, ovviamente vero. */
	exec_status.halt = 1;
/* Azzera il flag di richiesta di ripetizione della fase iniziale di HALT. */
	exec_status.repeat_start_halt = 0;
/* Azzera il contatore di richieste di ripetizione della fase iniziale
 della procedura di halt. */
	exec_status.halt_cycle = 0;
/* Esegue la procedura di stop del modulo "user defined" (vecchio stile). */
	main_status.mod_user_op.stop();
/* Azzera (o continua ad azzerare) le uscite. */
	ioDown();
/* Halt definitivo se non ci sono richieste di ripetizione. */
	if (exec_status.repeat_start_halt)
		exec_status.cycle = exec_start_halt_repeated;
	else
		exec_status.cycle = exec_finish_halt;
	exec_halt_core();
/* Comunica l'entrata in halt ai debugger collegati. */
	plcEventNotify(DSV_EVENT_HALT,NULL,0);
}

/*
* Fase "HALT con accesso temporaneo agli I/O".
*/

static void exec_halt_with_io(void)
{
	compat_chkgo();
	exec_status.cycle = exec_halt;
	ioRead();
	if (! exec_testhw())
		return;
	compat_begin_cycle();
	compat_trace();
	exec_status.old_comm();
	ioWrite();
	csem_signal(exec_status.sync_superv_sem);
}

/*
* Fase "FAULT" : Plc in HALT, e con problemi hardware.
*/

static void exec_fault(void)
{
	exec_status.old_comm();
}

/*
* Fase di inizio fault.
*/

static void exec_start_fault(void)
{
	compat_halt();
	ioFatal();
	exec_fault();
	exec_status.cycle = exec_fault;
/* Comunica l'entrata in fault ai debugger collegati. */
	plcEventNotify(DSV_EVENT_FAULT,NULL,0);
}

/*
* Comunicazione col supervisore
* -----------------------------
*/

/*
* Gestione dei comandi ricevuti dal supervisore.
*/

static void exec_from_sv(void)
{
	if (! exec_status.msg)
		return;

	switch (exec_status.msg -> cmd) {
	case SV_COMM_GO:
		plcStart();
		break;
	case SV_COMM_HALT:
		plcHalt();
		break;
	case SV_COMM_FAULT:
		plcFault();
		break;
	case SV_COMM_SYNC:
		break;
	case SV_COMM_FINISH_HALT:
		main_status.shm_addr -> go = 0;
		break;
	case SV_COMM_CBEXEC:
		cb_call(QPLC_CB_EXECUTOR,(void *)exec_status.msg -> data);
		break;
	case SV_COMM_ENMEAS:
		plcStartMeasurement();
		break;
	case SV_COMM_DISMEAS:
		plcStopMeasurement();
		break;
	case SV_COMM_CLRMEAS:
		plcClearMeasurement();
		break;
	case SV_COMM_MEAS:
		((long *)(exec_status.msg -> data))[0] = exec_status.curr_usec;
		((long *)(exec_status.msg -> data))[1] = exec_status.min_usec;
		((long *)(exec_status.msg -> data))[2] = exec_status.max_usec;
		break;
/* Creazione di una lista di watch. */
	case SV_COMM_WCREATE:
		*((int *)(exec_status.msg -> data))
		 = brkCreate((watch_descr_t *)exec_status.msg -> data);
		break;
/* Distruzione di una lista di watch. */
	case SV_COMM_WDELETE:
		*((int *)(exec_status.msg -> data))
		 = brkDestroy(*(int *)exec_status.msg -> data);
		break;
/* Aggiunta di un break ad una lista. */
	case SV_COMM_WADD:
		*((int *)(exec_status.msg -> data))
		 = brkAdd(*(int *)exec_status.msg -> data,
		          (watch_t *)(exec_status.msg -> data + sizeof(int)));
		break;
/* Abilitazione di una lista. */
	case SV_COMM_WENABLE:
		*((int *)(exec_status.msg -> data))
		 = brkEnable(*(int *)exec_status.msg -> data);
		break;
/* Disabilitazione di una lista. */
	case SV_COMM_WDISABLE:
		*((int *)(exec_status.msg -> data))
		 = brkDisable(*(int *)exec_status.msg -> data);
		break;
	case SV_COMM_WLONGHISTORY:
		brkHistoryMode(0);
		break;
	case SV_COMM_WSHORTHISTORY:
		brkHistoryMode(1);
		break;
	case SV_COMM_WLOCKHISTORY:
		brkHistoryLock();
		break;
	case SV_COMM_WUNLOCKHISTORY:
		brkHistoryUnlock();
		break;
/* Distruzione di tutte le liste. */
	case SV_COMM_WRESET:
		{
		int i;

			for (i = 0; i < MAX_WATCHPOINT; ++i)
				brkDestroy(i);
			brkReinit();
		}
		break;
	case SV_COMM_FORCEIO:
		{
		sv_comm_forceio_t *p
		 = (sv_comm_forceio_t*)(exec_status.msg -> data);

		if (exec_status.cycle == exec_halt) {
			exec_status.cycle = exec_halt_with_io;
			exec_status.sync_superv_sem = p -> sem;
			p -> rv = 1;
		}
		else {
			csem_signal(p -> sem);
			p -> rv = 0;
		}
		}
		break;
	default:
		break;
	}

	exec_status.msg = (sv_msg_t *) 0;

	csem_signal(superv_status.sv_sync_sem);
}

/*
* Manipolazione della coda dei messaggi verso il supervisore.
*/

/*
* Questa funzione aggiunge il messaggio nella posizione indicata da "priority".
* Vale 1 se il messaggio e` stato accodato, -1 se e` stato accodato a scapito
* di qualcun altro, 0 se non e` stato accodato affatto.
*/

static int exec_enqueue(sv_msg_t * msg, int prio)
{
exec_qmsg_t *p;
int i,rv;

/* Normalizza la priorita`. */

/*
* Ma e` proprio necessario ?
	if (prio < 0)
		prio = 0;
	else if (prio >= MAX_QEXEC_PRIO)
		prio = MAX_QEXEC_PRIO - 1;
*/

/* Incrementa COMUNQUE il numero di messaggi in coda. Ci si accorgera`
 poi che i numeri "non tornano". */

	++exec_status.q_len;

/* Alloca un nuovo elemento. */

	if ( (p = exec_status.free_q) ) {
		exec_status.free_q = p -> next;
		rv = 1;
	}
	else {

	/* La lista degli elementi liberi e` vuota. */
	/* Tenta di rubacchiare un elemento gia` allocato, ma con priorita`
	 minore (cioe` con "prio" piu` ALTO). */

		for (i = MAX_QEXEC_PRIO-1; i > prio; --i) {
			if ( (p = exec_status.kq[i]) ) {
			/* Elimina l'ultimo elemento inserito nella coda
			 a piu` bassa priorita`. */
				if (p == p -> pred) {
					exec_status.kq[i] = (exec_qmsg_t *) 0;
				}
				else {
					p -> next -> pred = p -> pred;
					p -> pred -> next = p -> next;
					exec_status.kq[i] = p -> next;
				}
			/* Si puo` uscire dal ciclo ! */
				break;
			}
		}

		if (! p) {
		/* Niente da fare. Non c'e` piu` posto. */
		/* ERRORE. Coda piena. */
			return 0;
		}

		rv = -1;
	}

/* Copia il messaggio. */

	p -> msg.cmd = msg -> cmd;
	p -> msg.len = msg -> len;
	memcpy(p -> msg.data, msg -> data, msg -> len);

/* Aggiunge l'elemento alla lista circolare. */

	if ( (p -> next = exec_status.kq[prio]) ) {
		p -> pred = p -> next -> pred;
		p -> pred -> next = p;
		p -> next -> pred = p;
	}
	else {
		p -> next = p -> pred = p;
	}

	exec_status.kq[prio] = p;

	return rv;
}

/*
* Invio di comandi al supervisore.
*/

static void exec_to_sv(void)
{
int i;

	if (exec_status.tx) {

	/* Attesa del completamento di una trasmissione. */

		exec_status.phase = superv_talk_nonblock(&exec_status.sv_msg,
							 exec_status.phase);

		if (exec_status.phase != SUPERV_PHASE_DONE) {

		/* Il colloquio non e` terminato. */

			return;
		}

	/* Comando completato. Vuota le code di trasmissione. */

		for (i = 0; i < MAX_QEXEC_PRIO; ++i) {
			if (exec_status.kq_tx[i]) {
				exec_status.kq_tx[i] -> pred -> next
				 = exec_status.free_q;
				exec_status.free_q = exec_status.kq_tx[i];
			}
		}

	/* Segnala la fine del colloquio. */

		exec_status.tx = 0;
	}

/* Se ci sono messaggi, possono passare alle code di trasmissione per essere
 inoltrati al supervisore. */

	if (exec_status.q_len) {

	/* Controlla che non si sia superata la capacita` della coda. */

		if (exec_status.q_len > main_status.qlen_exec_to_sv) {
		/* ERRORE. Troppi messaggi. */
		}

	/* Sposta i messaggi nelle code di trasmissione. */
	/* Vuota le code interne. */

		for (i = 0; i < MAX_QEXEC_PRIO; ++i) {
			exec_status.kq_tx[i] = exec_status.kq[i];
			exec_status.kq[i] = (exec_qmsg_t *) 0;
		}

		exec_status.q_len = 0;

	/* Inoltra la richiesta al supervisore. */

		exec_status.sv_msg.cmd = SV_COMM_EXECLIST;
		exec_status.sv_msg.len = sizeof(void *);
		*(void **)exec_status.sv_msg.data = (void *)exec_status.kq_tx;

		exec_status.phase = superv_talk_nonblock(&exec_status.sv_msg,
							 exec_status.phase);

	/* Segnala che c'e` una trasmissione in corso. */

		exec_status.tx = 1;
	}
}

/*
* Funzione "exec_startmsg"
* ------------------------
*
*  Questa funzione permette di iniziare l'esplorazione delle code
* di trasmissione descritte dal parametro "p". Le successive chiamate
* della funzione "exec_getmsg" forniranno i messaggi ordinati per priorita`
* e per data di invio.
*/

void exec_startmsg(void * p)
{
	exec_status.kq_base = (exec_qmsg_t **) p;
	exec_status.kq_index = 0;
	exec_status.kq_current = (exec_qmsg_t *) 0;
}

/*
* Funzione "exec_getmsg"
* ----------------------
*
*  Questa funzione fornisce il prossimo messaggio disponibile nella coda
* di trasmissione, oppure NULL se i messaggi sono finiti. Prima di
* utilizzare questa funzione, si deve chiamare la "exec_startmsg", indicando
* nel parametro il puntatore all'array di code di messaggi.
*/

sv_msg_t * exec_getmsg(void)
{
exec_qmsg_t *p;

	while (exec_status.kq_index < MAX_QEXEC_PRIO) {
		if ( ! ( (p = exec_status.kq_current) )) {
			if ( (p=exec_status.kq_base[exec_status.kq_index]) ) {
				p = p -> pred;
			}
			else {
				++exec_status.kq_index;
				continue;
			}
		}

		if (p == exec_status.kq_base[exec_status.kq_index]) {
			++exec_status.kq_index;
			exec_status.kq_current = (exec_qmsg_t *) 0;
		}
		else {
			exec_status.kq_current = p -> pred;
		}

		return &p -> msg;
	}

	return (sv_msg_t *) 0;
}

/*
* Funzioni di attesa dello scatto del timer in differenti circostanze.
*/

/* Attesa del segnale del timer (casi ricoperti: funzionamento ordinario,
 stand-alone e non). */
static void exec_wait_signal(void)
{
#ifdef Linux
int sig;
	sigwait(&exec_status.timer_signal,&sig);
#else
	sigwait(&exec_status.timer_signal,0);
#endif
}

/* Sospensione per un ciclo (casi ricoperti: debug ordinario). */
static void exec_wait_period(void)
{
	usleep(plcPeriod() * 1000);
}

#ifdef Linux

/* Su Linux non c'e` (ancora) il supporto per INAX */

#else 
/* Attesa del segnale del timer inax slave(casi ricoperti:
 funzionamento sincronizzato con intx, debug sincronizzato con intx). */
static void exec_wait_inax_slave_signal(void)
{
	for (;;) {
		sigwait(&exec_status.timer_signal,0);
		if (exec_status.inax_dead) {
		/* Il thread di sorveglianza ha scoperto che e` successo
		 qualcosa.
		  Qualcosa di terribile. */
			plcError(195,"intx");
			plcFault();
		}
	/* Aggiorna il divisore e rinfresca il flag di segnale ricevuto
	  Se si e` raggiunto il limite di conteggio, si puo` uscire dal
	 ciclo e procedere all'esecuzione del programma PLC. */
	/* NOTA: in teoria la linea seguente dovrebbe essere protetta
	 dall'arrivo del segnale del clock. In pratica, l'eventualita`
	 che cio` accada e` abbastanza remota (puo` capitare solo in seguito
	 a sovraccarico di intx o di qualche driver), e le conseguenze
         sarebbero in ogni caso molto lievi (occasionali perdite di ritmo
	 dell'esecutore...). */
		if (exec_handle_clock()) {
			break; /* "Ma perche` non hai usato do-while?" */
			       /* "Ah...gia`...Boh?..." */
		}
	}
}
/* Attesa del segnale del timer dell'INAX master (casi ricoperti:
 stand-alone con gestione INAX). */
static void exec_wait_inax_master_signal(void)
{
	sigwait(&exec_status.timer_signal,0);
	ioctl(exec_status.inax_fd,INAXTIACK,0);
}
/* Attesa dello scatto del timer INAX in modalita` master (casi ricoperti:
 debug stand-alone con gestione INAX). */
static void exec_wait_inax(void)
{
	ioctl(exec_status.inax_fd, INAXTWAIT, 0);
}

/*
* Inizializzazione del timer.
*/

/* Inizializzazione del timer INAX master. */
static void exec_set_inax_master(void)
{
int fd;
struct inaxboard ib;

/* Apertura del dispositivo INAX in modo master. */
	fd = open("/dev/inax0",O_RDWR);
	if (fd < 0)
		util_fatal("/dev/inax0");

/* Lettura delle caratteristiche. */
	ib.struct_size = sizeof(ib);
	if (ioctl(fd,INAXGET,&ib) < 0)
		util_fatal("INAXGET");

/* Nuove impostazioni. */
	ib.period = plcPeriod();
	if (main_status.debug) {
		ib.sig_timer = 0;
		ib.wdtrig = 0;
	}
	else {
		ib.sig_timer = SIGALRM;
		ib.wdtrig = 1;
	}
	ib.intc = 0;

/* Un po' di spettacolo. */
	ib.ledflash = 50;

	if (ioctl(fd,INAXSETTIM,&ib) < 0)
		util_fatal("INAXSETTIM");

	exec_status.inax_fd = fd;
}

/* Inizializzazione del timer INAX slave. */
static void exec_set_inax_slave(void)
{
int fd;
struct inaxboard ib;
sigset_t set;
struct sigvec vec;

/* Apertura del dispositivo INAX in modo slave. */
	fd = open("/dev/inax0-slave",O_RDWR);
	if (fd < 0)
		util_fatal("/dev/inax0-slave");

/* Adesso e` un bel casino: bisogna aspettare che il processo "intx"
 abbia programmato la scheda. Inoltre conviene aspettare il primo
 scatto del timer per avare tempo per completare le operazioni ed
 entrare nel ciclo di esecuzione. Il modo piu` sicuro per sincronizzarsi
 consiste proprio nell'attendere il segnale dell'INAX. */

/* Ignora (per ora) il segnale SIGUSR1 nella modalita` asincrona. */
	vec.sv_handler = SIG_IGN;
	vec.sv_mask = 0;
	vec.sv_onstack = 0;
	sigvec(SIGUSR1,&vec,(struct sigvec *)0);

/* Prenota il segnale SIGUSR1 come segnalazione per lo scatto del timer. */
	if (ioctl(fd,INAXSIGTIMSL,SIGUSR1) < 0)
		util_fatal("INAXSIGTIMSL");

/* Attende il segnale. */
	sigemptyset(&set);
	sigaddset(&set,SIGUSR1);
	sigwait(&set,0);

/* Il segnale e` arrivato: quindi la INAX e` stata programmata. */

/* Lettura della configurazione. Serve a ricavare il periodo
 impostato sulla scheda. */
	ib.struct_size = sizeof(ib);
	if (ioctl(fd,INAXGET,&ib) < 0)
		util_fatal("INAXGET");

/* Cerca di adattare il periodo nominale a quello del timer, e calcola
 il numero di clock tick necessari a coprire un ciclo. */

	exec_status.curr_inax_tick = 0;
	exec_status.n_inax_ticks = (main_status.period + ib.period - 1)
			         / ib.period;
	main_status.period = exec_status.n_inax_ticks * ib.period;
}
#endif /* Linux */

/* Inizializzazione del timer POSIX. */
static void exec_set_posix_timer(long pr)
{
#ifdef Linux
struct itimerval v;
#else
struct itimerspec v;
#endif

	if (! pr)
		return;
	if (main_status.timer_bug)
		--pr;

#ifdef Linux
	v.it_value.tv_sec = pr / 1000L;
	v.it_value.tv_usec = (pr % 1000L) * 1000L;
	v.it_interval = v.it_value;
	exec_status.timer = setitimer(ITIMER_REAL, &v, (struct itimerval *)NULL);
#else
	v.it_value.tv_sec = pr / 1000L;
	v.it_value.tv_nsec = (pr % 1000L) * 1000000L;
	v.it_interval = v.it_value;
	exec_status.timer = mktimer(TIMEOFDAY, DELIVERY_SIGNALS, 0);
	reltimer(exec_status.timer, &v, (struct itimerspec *) 0);
#endif
}

/* Impostazione delle risposte ai segnali asincroni. */
static void exec_setup_sig(int sig, void (*fn)(int))
{
#ifdef Linux
struct sigaction act;
	act.sa_handler = fn;
	sigemptyset(&act.sa_mask);
#if 0
	sigaddset(&act.sa_mask,SIGUSR1);
#endif
	sigaddset(&act.sa_mask,SIGALRM);
	act.sa_flags = 0;
	act.sa_restorer = NULL;
	sigaction(sig,&act,NULL);
#else
struct sigvec vec;

/* Imposta la funzione da chiamare in caso di timeout.
  Per semplicita`, si bloccano tutti i segnali utilizzati dai timer. */

	vec.sv_handler = fn;
	vec.sv_mask = (1 << (SIGUSR1 - 1)) | (1 << (SIGALRM - 1));
	vec.sv_onstack = 0;
	sigvec(sig,&vec,(struct sigvec *)0);
#endif
}

/*
* Thread gestore del segnale SIGALRM prodotto dal timer quando l'esecutore
* opera in sincrono con intx. Serve a controllare periodicamnete 
* che i segnali del timer veloce arrivino con un minimo di regolarita`.
* La funzione controlla il flag di "segnale ricevuto", e provvede
* a sbloccare l'esecutore se necessario.
*/

#ifdef Linux
/* Niente INAX */
#else
static void exec_timeout_inax_wd(void *arg)
{
sigset_t set;

/* Programma il watchdog timer. */
	exec_set_posix_timer(main_status.inax_wd_period);

	sigemptyset(&set);
	sigaddset(&set,SIGALRM);

	for (;;) {

		sigwait(&set,0);

		HTPSET(0x40);
		HTPSET(0x40); /* Per tenerlo un po' lungo. */
		HTPSET(0x40);
		HTPSET(0x40);
		HTPSET(0x40);

		if (exec_status.inax_sig_flag) {
		/* Ok, e` vivo. Nulla da dire. */
			exec_status.inax_sig_flag = 0;
		}
		else {

		/* Pare che il timer della INAX non stia funzionando a dovere.
		  Forse intx e` bloccato, o morto, oppure c'e` qualche problema
		 elettrico.
		  In debug, ci si limita a "dare il tizzo" all'esecutore,
		 altrimenti bisognerebbe anche fare qualcosa di piu`... */

			if (! main_status.debug) {
				exec_status.inax_dead = 1;
			}

		/* In mancanza d'altro, si simula la ricezione del segnale.
		 Non e` un granche`, come idea, ma e` cosi` comoda... */

			kill(getpid(), SIGUSR1);
		}

		HTPRES(0x40);
	}
}

/* Lancio del thread sensibile al segnale SIGALRM, da utilizzare nel
 caso di funzionamento sincronizzato con "intx".
  Il lancio di un diverso thread e` necessario a causa dell'imperfetta
 protezione della system call "sigwait" da segnali inattesi (in breve:
 se "sigwait" e` interrotta da un segnale non atteso, e il segnale atteso
 giunge prima dell'uscta dal gestore del primo, il sistema chiama il gestore
 del secondo anziche` concludere sigwait. Forse e` un comportamento regolare,
 ma risulta terribilmente scomodo) . */

static void exec_start_alarm_thread(void)
{
struct sigvec vec;

	if (main_status.inax_wd_period) {
	/* SIGALRM e` ignorato da tutti tranne il thread apposito. */
		vec.sv_handler = SIG_IGN;
		vec.sv_mask = 0;
		vec.sv_onstack = 0;
		sigvec(SIGALRM,&vec,(struct sigvec *)0);
	/* Siccome e` un lavorino corto, si puo` fare alla stessa priorita`
	 dell'esecutore. */
		util_start_thread(exec_timeout_inax_wd,util_execprio());
	}
}
#endif /* Linux */

/* Impostazione dell'attesa e la gestione asincrona
 del il segnale di clock. */
static void exec_setup_main_sig(int sig, void (*fn)(int))
{
	exec_setup_sig(sig, fn);
	sigemptyset(&exec_status.timer_signal);
	sigaddset(&exec_status.timer_signal,sig);
}

/* Funzione principale di programmazione delle sorgenti
 di temporizzazione. */
static void exec_start_timer(void)
{
	/* 
	* Sono possibili i seguenti modi di funzionamento:
	* 
	* - Ordinario.
	* 	Usa un timer POSIX, il segnale SIGALRM e la primitiva "sigwait".
	* - Debug ordinario.
	* 	Usa la primitiva "usleep".
	* - Ordinario in modalita` stand-alone.
	* 	Usa un timer POSIX, il segnale SIGALRM e la primitiva "sigwait".
	* - Debug in modalita` stand-alone.
	* 	Usa la primitiva "usleep".
	* - Sincronizzato al processo "intx".
	* 	Usa il dispositivo "/dev/inax0-slave", il segnale SIGUSR1 e la
	* 	primitiva "sigwait". Il gestore del segnale produce il consueto
	* 	errore "ciclo troppo lungo".
	* - Debug sincronizzato al processo "intx".
	* 	Usa il dispositivo "/dev/inax0-slave", il segnale SIGUSR1 e la
	* 	primitiva "sigwait". Il gestore del segnale non fa nulla.
	* - Modalita` stand-alone con pilotaggio della scheda INAX.
	* 	Usa il dispositivo "/dev/inax0", il segnale SIGALRM e la
	* 	primitiva "sigwait".
	* - Debug modalita` stand-alone con pilotaggio della scheda INAX.
	* 	Usa il dispositivo "/dev/inax0" ed il servizio "INAXTWAIT".
	* 
	*/

	/* Inizializzazioni paranoiche. */
	exec_status.sleep = exec_null;
	exec_status.inax_fd = -1;
	exec_status.timer = -1;
	sigemptyset(&exec_status.timer_signal);

/* Distinzione dei vari casi. */

	if (main_status.use_inax) {

#ifdef Linux

/* Niente INAX per Linux */
		errno = ENODEV;
		util_fatal("/dev/inax*");

#else

	/* Casi in cui si deve utilizzare la scheda INAX (master o slave). */

		if (main_status.stand_alone) {

		/* Modalita` stand-alone: si deve usare la INAX in
		 modalita` master. La sua programmazione e` a carico di
		 questo processo. */

			if (main_status.debug) {

			/* In debug, niente segnale (rompe solo le
			 scatole). */

			/* Funzione di attesa: ioctl(INAXTWAIT). */

				exec_status.sleep = exec_wait_inax;
			}
			else {

			/* In modalita` normale, si usa il solito SIGALRM. */

			/* Funzione di attesa: sigwait + ioctl(INAXTIACK). */

				exec_status.sleep
				 = exec_wait_inax_master_signal;

			/* Imposta la funzione da chiamare in caso
			 di timeout. */

				exec_setup_main_sig(SIGALRM, exec_timeout);

			}

			exec_set_inax_master();
		}
		else {

		/* Modalita` sincronizzata ad intx: si deve usare la INAX in
		 modalita` slave. La sua programmazione e` a carico di
		 intx. */

		/* Funzione di attesa: sigwait con gestione contatore tick. */

			exec_status.sleep = exec_wait_inax_slave_signal;

		/* Aggancia intx. */

			exec_set_inax_slave();

		/* Lancia il thread di sorveglianza. Serve ad individuare
		 eventuali malfunzionamenti di intx. */

			exec_start_alarm_thread();

		/* Imposta la funzione da chiamare in caso di timeout. */

			exec_setup_main_sig(SIGUSR1, exec_timeout);
		}
#endif /* Linux */

	}
	else {

	/* Casi in cui si deve utilizzare il timer di Lynx. */

		if (main_status.debug) {

		/* Debug ordinario (stand-alone e non). */

		/* Funzione di attesa: usleep. */

			exec_status.sleep = exec_wait_period;

		}
		else {

		/* Funzionamento ordinario (stand-alone e non). */

		/* Funzione di attesa: sigwait. */

			exec_status.sleep = exec_wait_signal;

		/* Imposta la funzione da chiamare in caso di timeout. */

			exec_setup_main_sig(SIGALRM, exec_timeout);

		/* Programma il timer. */

			exec_set_posix_timer(plcPeriod());
		}
	}
}

/* Gestione particolare dell'NVRAM su file. */
static void exec_nvram_file(void)
{
	if (main_status.nvram_filemap == (void *)0) {
		return;
	}
	if (++exec_status.nvram_refresh_count
	  < exec_status.nvram_refresh_limit) {
		return;
	}
	exec_status.nvram_refresh_count = 0;
	if (memcmp(main_status.nvram_image,
	           main_status.nvram_filemap,
	           main_status.nvram_size) == 0) {
		return;
	}
	memcpy(main_status.nvram_filemap,
	       main_status.nvram_image,
	       main_status.nvram_size);
	msync(main_status.nvram_filemap,main_status.nvram_size,MS_ASYNC);
	//msync(main_status.nvram_filemap,main_status.nvram_size,MS_SYNC);
}


/*
* Cuore dell'esecutore
* --------------------
*/

void exec_main(void)
{
#ifdef Linux
struct sigaction act;
static struct timeval start_time;
#else
struct sigvec vec;
#endif
int i;

	exec_status.nvram_refresh_count = 0;
	exec_status.nvram_refresh_limit = main_status.nvram_refresh / plcPeriod();

/* Allocazione della coda di messaggi da esecutore a supervisore. */

	exec_status.free_q = (exec_qmsg_t *) util_alloc_chain(sizeof(exec_qmsg_t), GET_STRUCT_OFFSET(exec_qmsg_t,next), main_status.qlen_exec_to_sv);
	if (! exec_status.free_q) {
	/* ERRORE. Memoria esaurita. */
		plcError(108,"4");
	}

/* Guai a swappare il PLC ! Si fanno alcuni tentativi, poi si lascia
 perdere. */

	{
		int try = 0;

#ifdef Linux
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
		util_error("mlockall");
	}
#else
	while ( memlk(PROLOCK,(char *) 0, 0) == -1) {
		if (++try >= 10) {
			util_fatal("memlk");
			break;
		}
		sleep(1);
	}
#endif
	}

	/* Inizializzazione del modulo di debug. */

	exec_status.deb_begin = exec_status.deb_end = exec_null;
	brkInit(&exec_status.deb_begin, &exec_status.deb_end);

	/* Inizializzazione dei puntatori alle funzioni di misura. */

	plcClearMeasurement();

	if (main_status.soft_tm)
		plcStartMeasurement();
	else
		plcStopMeasurement();

	/* Inizializzazione dei campi legati al colloquio. */

	for (i = 0; i < MAX_QEXEC_PRIO; ++i) {
		exec_status.kq[i] = exec_status.kq_tx[i] = (exec_qmsg_t *) 0;
	}

	exec_status.tx = 0;
	exec_status.q_len = 0;
	exec_status.phase = SUPERV_PHASE_DONE;

	/* Inizializzazione del numero di ciclo corrente e tempo trascorso dal lancio di qplc (millisecondi). */

	exec_status.n_cycle = 0;
	exec_status.plc_time = 0;
	exec_status.plc_time_h = 0;

	/* Flag di richiesta di ripetizione della procedure iniziale di halt, e numero di ripetizioni della fase iniziale di halt. */

	exec_status.repeat_start_halt = 0;
	exec_status.halt_cycle = 0;

	/* Flag di HALT in corso. */
	exec_status.halt = 0;

	/* Inizializzazione della struttura di stato ("cycle" e` gia` stato inizializzato !). */

	exec_status.inax_sig_flag = 0;
	exec_status.inax_dead = 0;
	exec_status.curr_overrun = 0;
	exec_status.curr_inax_tick = 0;
	exec_status.n_inax_ticks = 0;
	exec_status.msg = (sv_msg_t *) 0;
	plcCatchOldComm();

	/* Imposta la funzione da chiamare in caso di errori matematici. Faranno comodo le informazioni in stile System V. */

#ifdef Linux
	act.sa_handler = exec_fperr;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask,SIGUSR1);
	sigaddset(&act.sa_mask,SIGALRM);
#if 0
	act.sa_handler = exec_fperr;
	act.sa_flags = SA_SIGINFO;
#else
	act.sa_flags = 0;
#endif
	act.sa_restorer = NULL;
	sigaction(SIGFPE,&act,NULL);
#else
	vec.sv_handler = exec_fperr;
	vec.sv_mask = 0;
	vec.sv_onstack = SV_SYSVCONTEXT;
	sigvec(SIGFPE,&vec,(struct sigvec *)0);
#endif

	/* Imposta la priorita` del thread esecutore, che e` anche il principale. */

	{
	pthread_t t = pthread_self();
#ifdef Linux
	struct sched_param sch;
	int policy;

		pthread_getschedparam(t,&policy,&sch);
		sch.sched_priority = util_execprio();
		if (pthread_setschedparam(t,SCHED_FIFO,&sch))
			util_error("setschedparam");
#else
		pthread_setprio(t,pthread_getprio(t)+main_status.prio_exec);
		pthread_setscheduler(t,SCHED_FIFO,pthread_getprio(t));
#endif
	}

#ifdef Linux
	/* 
	* Su Linux bisogna leggere per forza l'orologio, perche`
	* il ciclo puo` non essere regolare e prevedibile come in LynxOS.
	* Qui si registra l'ora di lancio dell'applicazione. 
	*/
	{
		struct timezone zone;

		gettimeofday(&start_time,&zone);
	}
#endif

	/* Ultimo punto utile per inserire il riempimento della cache dei dati utilizzati dalle "util_check_addr_...". */
	util_check_addr_refresh();

	/* 
	* Punto di rientro in caso di overrun. Bisogna attivare il timer solo dopo impostato il rientro, 
	* senno`, BUM ! (eventualita` remota, ma perche` rischiare ?) 
	*/

	if (!
#ifdef Linux
	  sigsetjmp(emergency,1)
#else
	  setjmp(emergency)
#endif
		) {

		/* Imposta il timer. */

		exec_start_timer();
	} else {

		/* In caso di HALT forzato, si deve ripulire il codice da eventuali breakpoint. */

		brkEnd();
	}

	/**********************************/
	/* Ciclo infinito dell'esecutore. */
	/**********************************/


	for (;;) {

		/* Indica che il thread detentore della CPU non e` piu` l'esecutore. */

		exec_status.executor = 0;

		/* 
		* INIZIO DELLA PARTE DI CODICE ALL'INTERNO DELLA QUALE 
		* NON E` CONSENTITO CHIAMARE LE FUNZIONI CHE USANO IL FLAG "executor". 
		*/

		/* Attende lo scatto del timer. */

		HTPRES(0x01);

		exec_status.sleep();

		HTPSET(0x01);

		/* Indica che il thread detentore della CPU e` l'esecutore. */

		exec_status.executor = 1;

		/* FINE DELLA PARTE DI CODICE ALL'INTERNO DELLA QUALE NON E` CONSENTITO CHIAMARE LE FUNZIONI CHE USANO IL FLAG "executor". */

		/* Controlla e azzera il flag di overrun. */

		if (exec_status.curr_overrun) {
			if (exec_status.curr_overrun
			  > main_status.min_overrun) {
			/* ERRORE. L'esecutore ha superato
			 il periodo di Plc nominale di una quantita` 
			 superiore al limite consentito. */
				plcError(63,"%d",
				         (long)exec_status.curr_overrun);
			}
			exec_status.curr_overrun = 0;
		}

		/* Azzera l'indice del programma corrente. */

		main_status.curr_prog_id = 0;

		/* Chiama la funzione di inizio misura del tempo di ciclo. */

		exec_status.tm_begin();

		/* Aggiorna numero di ciclo corrente e tempo trascorso dal lancio di qplc (millisecondi). */

		++exec_status.n_cycle;
#ifdef Linux
		/* Su Linux bisogna leggere per forza l'orologio, perchè il ciclo puo` non essere regolare e prevedibile come in LynxOS. */
		{
		struct timeval tm;
		struct timezone zone;
		unsigned long h,l,m,ah,al,ol;

			/* Lettura della data. */
			gettimeofday(&tm,&zone);

			/* Calcolo della differenza tra la data attuale e quella del lancio dell'esecutore. */
			tm.tv_sec -= start_time.tv_sec;
			if (tm.tv_usec >= start_time.tv_usec) {
				tm.tv_usec -= start_time.tv_usec;
			}
			else {
				tm.tv_usec = 1000000
				           + tm.tv_usec
				           - start_time.tv_usec;
				--tm.tv_sec;
			}

			/* Conversione del risultato in millisecondi. */

			/* 
			* Bisogna truccheggiare un po' con la rappresentazione.
		 	* Non mi piace troppo ricorrere al tipo "long long". 
			*/
			/* (in pratica, mi faccio una moltiplicazione "a mano"). */

			al = (unsigned long) tm.tv_sec;
			ah = h = l = 0;
			m = 1000;
			for (;;) {
				if (m & 1) {
					h += ah;
					ol = l;
					l += al;
					if (ol > l)
						++h;
				}
				m /= 2;
				if (m == 0)
					break;
				ah <<= 1;
				if (al & (1 << (sizeof(al)*8-1)))
					ah |= 1;
				al <<= 1;
			}

			ol = l;
			l += (unsigned long) tm.tv_usec / 1000;
			if (ol > l)
				++h;

			exec_status.plc_time = l;
			exec_status.plc_time_h = h;
		}
#else
		exec_status.plc_time += (unsigned long)main_status.period;
		if (exec_status.plc_time < (unsigned long)main_status.period)
			++exec_status.plc_time_h; /* Parte alta del tempo. */
#endif

		/* Controlla il colloquio col supervisore (da sv a ex). */

		exec_from_sv();

		/* Esegue il "nucleo" (read-hat-prog-shoes-old_comm-write). */

		exec_status.cycle();

		/* Gestione particolare dell'NVRAM su file. */
		exec_nvram_file();

		/* Controlla il colloquio col supervisore (da ex a sv). */

		exec_to_sv();

		/* Termina la misura del tempo di ciclo. */

		exec_status.tm_end();
	}
}

/*
* FUNZIONI DI MANIPOLAZIONE DELLO STATO DELL'ESECUTORE
* ----------------------------------------------------
*/

/*
* Funzione "plcStart"
* -------------------
*
*  Questa funzione mette in START l'esecutore. E` chiamata dall'esecutore
* alla ricezione di una richiesta di start dal supervisore, oppure prima
* lancio del supervisore, se non e` impostato l'HALT all'accensione.
*/

void plcStart(void)
{
	main_status.shm_addr -> go = 1;
	exec_status.cycle = exec_start;
	compat_go();
}

/*
* Funzione "plcGo"
* -------------------
*
*  Questa funzione mette in START l'esecutore. E` chiamata dall'esecutore
* per iniziare la procedura di start, che coinvolgera` anche il supervisore.
*/

void plcGo(void)
{
sv_msg_t msg;

	msg.cmd = SV_COMM_GO;
	msg.len = 0;

	exec_enqueue(&msg, CMDPRIO_COMMANDS);
}

/*
* Funzione "plcIsHalt"
* --------------------
*
*  Questa funzione vale 1 se il Plc e` nello stato di HALT ordinario
* e stabilizzato. Ad esclusivo uso interno.
*/

int plcIsHalt(void)
{
	return exec_status.cycle == exec_halt;
}

/*
* Funzione "plcHalt"
* ------------------
*
*  Questa funzione mette in HALT l'esecutore. Deve essere chiamata
* dal thread esecutore. Puo` essere utilizzata all'interno di un
* programma PLC per forzare l'HALT.
*/

void plcHalt(void)
{
	/* Evita di inoltrare richieste di HALT a raffica. */
	if (!exec_status.halt) {
		/* Flag di HALT in corso, ovviamente vero. */
		exec_status.halt = 1;
		plcError(9,"");
		/* 
		* La segnalazione dello stato di HALT e` posticipata, per permettere
	 	* al PLC di terminare la fase di entrata in HALT (chiamata dei metodi
	 	* "stop" e "down"...). Ecco perche` non la si fa piu` qui. 
		*/
		/* main_status.shm_addr -> go = 0; */
		exec_status.cycle = exec_start_halt;
		compat_halt();
	}
}

/*
* Funzione "plcFault"
* -------------------
*
*  Questa funzione mette in FAULT l'esecutore. Deve essere chiamata
* dal thread esecutore. Puo` essere utilizzata all'interno di un
* programma PLC per forzare il FAULT. L'effetto dell'entrata in FAULT
* puo` essere irreversibile. 
*/

void plcFault(void)
{
	if (main_status.no_fault) {
		plcHalt();
	}
	else {
		plcError(8,"");
		if (main_status.p_cnok)
			*main_status.p_cnok = 0;
		main_status.shm_addr -> go = 0;
		exec_status.cycle = exec_start_fault;
		compat_halt();
		ioFatal();
	}
}

/*
* Funzione per l'invio di "roba" tipo errori o messaggi.
*/

static void exec_inverror(int cmd, int code, char * text)
{
sv_msg_t msg;
sv_comm_error_t *p;

	msg.cmd = cmd;
	p = (sv_comm_error_t *)msg.data;
	p -> code = code;
	strncpy(p -> message, text, sizeof(p -> message) - 1);
	p -> message[sizeof(p -> message) - 1] = '\0';
	msg.len = strlen(p -> message) + sizeof(p -> code) + 1;

	exec_enqueue(&msg, CMDPRIO_ERRORS);
}

/*
* Funzione per l'invio di "roba" tipo long a priorita` degli errori.
*/

static void exec_invlong_e(int cmd, long l)
{
sv_msg_t msg;

	msg.cmd = cmd;
	*(long *)msg.data = l;
	msg.len = sizeof(long);

	exec_enqueue(&msg, CMDPRIO_ERRORS);
}


/* ...e a priorita` comandi. */

static void exec_invlong_c(int cmd, long l)
{
sv_msg_t msg;

	msg.cmd = cmd;
	*(long *)msg.data = l;
	msg.len = sizeof(long);

	exec_enqueue(&msg, CMDPRIO_COMMANDS);
}

/*
* Funzione per l'invio di una stringa (priorita` comandi).
*/

static void exec_invtext_c(int cmd, char * text)
{
sv_msg_t msg;

	msg.cmd = cmd;
	strncpy(msg.data, text, sizeof(msg.data) - 1);
	msg.data[sizeof(msg.data) - 1] = '\0';
	msg.len = strlen(msg.data) + 1;

	exec_enqueue(&msg, CMDPRIO_COMMANDS);
}

/*
* Funzione "plcError"
* -------------------
*
*  Questa funzione visualizza un errore di codice "code" e test "text".
*/

void plcError(long code, char * format, ...)
{
va_list args;
char err_buff[SV_MAX_MSG_LEN];

	va_start(args, format);
	util_safe_vsprintf(err_buff,sizeof(err_buff),format,args);
	if (exec_status.executor)
		exec_inverror(SV_COMM_ERROR, code, err_buff);
	else
		superv_error(code, err_buff);
	va_end(args);
}

/*
* Funzione plcRuntimeError
* ------------------------
*
* Questa funzione emette il codice di errore specificato, e aggiunge come
* commento il valore "pc" del program counter. Tale valore e` confrontato
* con gli indirizzi dei moduli caricati dinamicamente e, se appartiene
* a qualcuno di questi, e` visualizzato come "nome_modulo+offset", per
* facilitare il debug.
*/

void plcRuntimeError(long code, unsigned long pc)
{
module_t * mod[3];
int i;

	mod[0] = main_status.mod_hatsh;
	mod[1] = main_status.mod_user;
	mod[2] = main_status.mod_plc480;

/* Tenta di dare un indicazione della posizione di schianto. */

	for (i = 0; i < 3; ++i) {
		if (mod[i] -> text
		 && (unsigned long)(mod[i] -> text) <= pc
		 && (unsigned long)(mod[i] -> text) + mod[i] -> size > pc) {
			break;
		}
	}

	if (i < 3) {
		plcError(code, "%s+0x%x,%d",
		         mod[i] -> name,
		         pc - (unsigned long)(mod[i] -> text),
		         main_status.curr_prog_id);
	}
	else {
		plcError(code, "0x%x,%d", pc, main_status.curr_prog_id);
	}
}

/*
* Funzione "plcDelerr"
* ---------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*/

void plcDelerr(long code)
{
	if (exec_status.executor)
		exec_invlong_e(SV_COMM_DELERR,code);
	else
		superv_delerr(code);
}

/*
* Funzione "plcWarning"
* ---------------------
* 
*  Questa funzione visualizza un "warning" su stderr.
*/

void plcWarning(long code, char * format, ...)
{
	va_list args;
	char err_buff[SV_MAX_MSG_LEN];

	va_start(args, format);
	util_safe_vsprintf(err_buff,sizeof(err_buff),format,args);
	if (exec_status.executor)
		exec_inverror(SV_COMM_WARNING, code, err_buff);
	else
		superv_warning(code, err_buff);
	va_end(args);
}

/* Funzioncina interna per visualizzare un messaggio. */

static void plc_message_core(long code, int update, char *format, va_list args)
{
char msg_buff[SV_MAX_MSG_LEN];

	util_safe_vsprintf(msg_buff,sizeof(msg_buff),format,args);
	if (exec_status.executor)
		exec_inverror(update ? SV_COMM_MESSAGE_UPD : SV_COMM_MESSAGE,
		              code, msg_buff);
	else
		superv_message(code, msg_buff, update);
}

/*
* Funzione "plcMessage"
* ---------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text".
*/

void plcMessage(long code, char * format, ...)
{
va_list args;

	va_start(args, format);
	plc_message_core(code,0,format,args);
	va_end(args);
}

/*
* Funzione "plcUpdateMessage"
* ---------------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text",
* sovrascrivendo l'eventuale precedente messaggio con lo stesso codice.
*/

void plcUpdateMessage(long code, char * format, ...)
{
va_list args;

	va_start(args, format);
	plc_message_core(code,1,format,args);
	va_end(args);
}

/*
* Funzione "plcDelmess"
* ---------------------
*
*  Cancella un messaggio (ammesso che la cosa si possa fare) di codice "code".
*/

void plcDelmess(long code)
{
	if (exec_status.executor)
		exec_invlong_e(SV_COMM_DELMESS,code);
	else
		superv_delmess(code);
}

/*
* Funzione "plcStartMeasurement"
* ------------------------------
*
*  Questa funzione e` chiamata dall'esecutore per impostare la misura
* del tempo di ciclo.
*/

void plcStartMeasurement(void)
{
	main_status.soft_tm = 1;
	exec_status.tm_begin = exec_tm_start;
}

/*
* Funzione "plcStopMeasurement"
* -----------------------------
*
*  Questa funzione e` chiamata dall'esecutore per interrompere la misura
* del tempo di ciclo.
*/

void plcStopMeasurement(void)
{
	main_status.soft_tm = 0;
	exec_status.tm_begin = exec_status.tm_end = exec_null;
}

/*
* Funzione "plcClearMeasurement"
* ------------------------------
*
*  Questa funzione azzera la statistica del tempo di ciclo.
*/

void plcClearMeasurement(void)
{
	exec_status.curr_usec = exec_status.max_usec = 0;
	exec_status.min_usec = (long) (((unsigned long)-1) / 2);
}

/*
* Funzione "plcLoadModule"
* ------------------------
*
*  L'esecutore chiama questa funzione per richiedere il caricamento
* dinamico del file "path".
*  Vale 0 se l'operazione e` ammissibile, -1 altrimenti.
*/

int plcLoadModule(char * path)
{
	if (main_status.shm_addr -> go)
		return -1;

	exec_invtext_c(SV_COMM_DYLOAD,path);

	return 0;
}

/*
* Funzione "plcUnloadModule"
* --------------------------
*
*  L'esecutore chiama questa funzione per richiedere lo scaricamento
* dinamico dell'ultimo modulo caricato.
*  Vale 0 se l'operazione e` ammissibile, -1 altrimenti.
*/

int plcUnloadModule(void)
{
sv_msg_t msg;

	if (main_status.shm_addr -> go)
		return -1;

	msg.cmd = SV_COMM_DYUNLOAD;
	msg.len = 0;

	exec_enqueue(&msg, CMDPRIO_COMMANDS);

	return 0;
}

/*
* Funzione "plcEventNotify"
* -------------------------
*
*  L'esecutore chiama questa funzione per richiedere la segnalazione
* di un evento di interesse per i debugger. "code" e` il codice che
* individuera` l'evento. "data" e "len" sono indirizzo e dimensione
* (eventualmente 0) di in insieme di dati associati all'evento.
*/

void plcEventNotify(long code,char *data,int len)
{
sv_msg_t msg;

	msg.cmd = SV_COMM_EVNOTIFY;
	*(long *)msg.data = code;
	if (data && len)
		memcpy(msg.data + sizeof(code),data,len);
	msg.len = sizeof(code) + len;

	exec_enqueue(&msg, CMDPRIO_COMMANDS);
}

/*
* Funzione "plcGiveOldComm"
* -------------------------
*
*  Con questa funzione l'esecutore demanda al supervisore il riconoscimento
* e l'elaborazione del comando ricevuto dal debugger "isaker-compatibile".
*  L'esecutore non percorrera` la funzione che gestisce il colloquio
* finche` il supervisore non avra` finito.
*  La funzione vale 1 se il controllo era del supervisore, 0 altrimenti.
*/

int plcGiveOldComm(void)
{
	sv_msg_t msg;

	if (exec_status.old_comm == exec_null) {

		/* Solo il supervisore dovrebbe percorrere questo tratto. */

		return 0;
	} else {

		/* Solo l'esecutore dovrebbe percorrere questo tratto. */

		msg.cmd = SV_COMM_OLDCOMM;
		msg.len = 0;

		if (exec_enqueue(&msg, CMDPRIO_COMMANDS)) {

			/* 
			* La palla passa al supervisore. Sara` lui a ripristinare
			* il valore di questo puntatore. Finche` non sara` fatto,
			* l'esecutore non percorrera` questa funzione (cioe`, non
			* percorrera` alcuna funzione che la utilizzi). 
			*/

			exec_status.old_comm = exec_null;
			return 1;
		} else {

			/* Se non c'e` piu` spazio nella coda di trasmissione,
		 	* esce senza fare niente. Il chiamante dovrebbe interpretare
		 	* il fallimento di questa funzione come un invito a
		 	* ritentare al ciclo successivo. 
			*/

			return 0;

		}
	}
}

/*
* Funzione "plcCatchOldComm"
* --------------------------
*
*  Con questa funzione il supervisore restituisce all'esecutore il
* controllo del colloquio "vecchio stile".
*  La funzione puo` comunque essere chiamata anche dall'esecutore.
*/

void plcCatchOldComm(void)
{

	/* Si fa l'ipotesi che quest'azione sia atomica. */

	exec_status.old_comm = compat_comm;
}

/*
* Funzione "plcPeriod"
* --------------------
*
* Questa funzione restituisce il periodo del ciclo di esecuzione
* in millisecondi.
*/

long plcPeriod(void)
{
	return main_status.period;
}

/*
* Funzione "plcNCycle"
* --------------------
*
* Questa funzione restituisce il numero di ciclo corrente.
*/

unsigned long plcNCycle(void)
{
	return exec_status.n_cycle;
}

/*
* Funzione "plcTime"
* ------------------
*
* Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi.
*/

unsigned long plcTime(void)
{
	return exec_status.plc_time;
}

/*
* Funzione "plcTimeH"
* -------------------
*
*  Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi (32 bit piu` significativi).
*/

unsigned long plcTimeH(void)
{
	return exec_status.plc_time_h;
}

/*
* Funzione "plcHaltCycle"
* -----------------------
*
*  Questa funzione restituisce il numero di cicli dell'esecutore dal momento
* dell'entrata in HALT. La sua chiamata ha senso solo all'interno dei metodi
* "down" dei driver e dei metodi "Stop" dei programmi PLC. Il primo ciclo di
* HALT e` lo 0. Lo scopo di questa funzione e` permettere alle procedure di
* HALT di capire se siano state richiamate a seguito della richiesta di
* ripetizione di qualche altra componente.
*/

unsigned long plcHaltCycle(void)
{
	return exec_status.halt_cycle;
}

/*
* Funzione "plcRepeatHalt"
* ------------------------
*
*  Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi (32 bit piu` significativi).
*/

void plcRepeatHalt(void)
{
	exec_status.repeat_start_halt = 1;
}

/*
* Funzione "plcSaveAx"
* ---------------------
*
*  Salva i dati dell'asse indicato (numero interno).
*/

void plcSaveAx(long ax)
{
	if (exec_status.executor)
		exec_invlong_c(SV_COMM_SAVEAX,ax);
	else
		superv_saveax(ax);
}

/*
* Funzione "plcSaveGenericTable"
* ------------------------------
*
*  Salva i dati della tabella utente indicata.
*/

void plcSaveGenericTable(char *tab)
{
	if (exec_status.executor)
		exec_invtext_c(SV_COMM_SAVEGENTAB,tab);
	else {
		superv_savegentab(tab);
	}
}

/*
* Funzione "plcSaveOrig"
* ----------------------
*
*  Salva le origini.
*/

void plcSaveOrig(void)
{
	sv_msg_t msg;

	if (exec_status.executor) {
		msg.cmd = SV_COMM_SAVEORIG;
		msg.len = 0;
		exec_enqueue(&msg, CMDPRIO_COMMANDS);
	} else {
		superv_saveorig();
	}
}

/*
* Funzione "plcSaveTable"
* -----------------------
*
*  Salva una tabella qualsiasi dato nome e centro.
*/

void plcSaveTable(char *tabname,int cen)
{
	if (exec_status.executor) {
		sv_msg_t msg;
		sv_savetab_t *p;

		msg.cmd = SV_COMM_SAVETAB;
		p = (sv_savetab_t *)(msg.data);
		p -> cen = cen;
		strncpy(p -> tabname, tabname, sizeof(p -> tabname) - 1);
		msg.data[sizeof(msg.data) - 1] = '\0';
		msg.len = sizeof(*p) - sizeof(p -> tabname)
		        + strlen(p -> tabname) + 1;
		exec_enqueue(&msg, CMDPRIO_COMMANDS);
	} else {
		superv_savetab(tabname, cen);
	}
}

