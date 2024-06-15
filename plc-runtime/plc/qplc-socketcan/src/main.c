/*
* @(#) main.c 1.43 Tue Nov  7 17:58:27 MET 2000
* @(#) Rif. Guerrini
*
* "main" del plc CNi (qplc).
*
* 14/05/96 GG 1.0 Prima stesura.
* 30/08/96 GG 1.1 Aggiunta l'opzione "-timer-bug", per correggere il problema
*             dell'arrotondamento errato del periodo in Lynx 2.3.0.
*             Aggiunta la chiamata del metodo "exit" del modulo "hatsh".
*             Modificata la logica di assegnamento di hatsh.ok.
* 03/09/96 GG 1.2 Aggiunto un meccanismo per la creazione selettiva di
*             segnali di scambio standard.
* 04/09/96 GG 1.3 Aggiunta la chiamata di "ioDown" ed "ioRestart" all'uscita,
*             in modo che gli output siano posti in uno stato sicuro, ed
*             eventuali risorse occupate dai driver siano liberate.
* 17/09/96 GG 1.4 Aggiunto il flag "no_fault", per evitare l'entrata in fault.
*             Aggiunto l'azzeramento del flag di GO alla ricezione del segnale
*             di terminazione.
* 18/09/96 GG 1.5 Resa piu` solida la gestione dei segnali d'uscita.
* 20/09/96 GG 1.6 Aggiunte le opzioni "-path-hatsh", "-path-prog40" e
*             "-path-user", che permettono di specificare il nome dei file
*             da caricare come moduli "hatsh", "prog480" e "user".
* 15/10/96 GG 1.7 Aggiunta l'opzione "-mdata", che forza l'aggancio
*             al server dei dati macchina anche in modalita` "stand-alone".
* 22/10/96 GG 1.8 Aggiunta l'opzione  "-save-io", per eseguire un salvataggio
*             in NVRAM della configurazione hardware, per compatibilita` con
*             isaker.
* 30/10/96 GG 1.9 Aggiunta la possibilita` di utilizzare "poll" o "select"
*             a scelta.
* 26/11/96 GG 1.10 Aggiunti alcuni campi alla struttura di stato, utili
*             per la gestione delle variabili in RAM non volatile.
* 28/11/96 GG 1.11 Piccolissima modifica per la versione monolitica di QPLC.
*             In modalita` "stand-alone", "hat" e "shoes" dono rediretti su
*             due funzioni nulle.
* 12/12/96 GG 1.12 Aggiunte due opzioni per configurare il debugger.
* 19/12/96 GG 1.13 Modifiche per distinguere le sigle delle versioni delle
*             varie componenti.
* 15/01/97 GG 1.14 Aggiunti campi per la gestione dei moduli IL conteneti
*             piu` di un programma, e l'opzione " per modificare il numero
*             massimo di programmi gestibili (20).
* 29/01/97 GG 1.15 Aggiunta l'emissione di un errore in caso di problemi di
*             colloquio con altre parti del CN. Inoltre, in questi casi,
*             il PLC resta in HALT alla partenza.
* 04/02/97 GG 1.16 Aggiunta un'opzione per disattivare alcuni controlli,
*             probabilmente lenti, che il Plc esegue sulle richieste del
*             debugger.
* 07/02/97 GG 1.17 Aggiunta l'inizializzazione delle funzioni di test
*             di validita` degli indirizzi di debug. Questo dovrebbe
*             ridurre la probabilita` di rallentamenti. In ogni caso,
*             resta l'opzione "-chkaddr 0".
* 07/02/97 GG 1.18 Modificata lievissimamente l'inizializzazione degli
*             errori: la funzione "initXerror" veniva chiamata in ogni
*             caso (unico effetto: la comparsa di "/tmp/pipe_xerror". Poca
*             cosa, in effetti...).
* 27/02/97 GG 1.19 Aggiunta un'opzione per intervenire sulla priorita`
*             associata ai canali TCP/IP (sperimentale).
* 06/03/97 GG 1.20 Eliminato l'incremento di priorita` introdotto nella
*             versione precedente. Ci pensera` chi scrive "hatsh".
* 15/04/97 GG 1.21 Aggiunta l'opzione "-drvconf", che serve a memorizzare una
*             stringa di configurazione ad uso del sistema di I/O (si veda
*             "drivers.c" e "dr_*.c").
*             Corretto un errore nella funzione "opt_setstring", che provocava
*             la scrittura oltre la fine della stringa.
* 23/05/97 GG 1.22 Aggiunta l'opzione "-max-halt", che imposta il limite
*             di sistema sul numero di ripetizioni della fase di entrata in
*             HALT.
* 03/06/97 GG 1.23 Aggiunta una tabella di conversione chiave --> descrittore
*             esteso per i segnali di scambio.
* 02/07/97 GG 1.24 Aggiunta l'opzione "-go-anyway", che permette l'entrata in
*             GO anche se l'MSD in stile IL richiede segnali di I/O
*             inesistenti.
* 14/07/97 GG 1.25 Aggiunto il lancio del driver "SER" (linee seriali).
* 16/07/97 GG 1.26 Aggiunta l'opzione "-max-dr-ext", che configura il numero
*             massimo di funzioni estese di driver utilizzabili.
* 05/09/97 GG 1.27 Aggiunta l'opzione "-blocking-pipe {0|1}", che controlla
*             la proprieta` delle pipe degli errori e dei messaggi di XNC.
*             Per ora, comunque, le pipe non sono bloccanti per default.
* 23/09/97 GG 1.28 Aggiunto l'azzeramento incondizionato del flag di
*             shared memory inizializzata. Piu` che altro, per superstizione.
* 22/10/97 GG 1.29 Aggiunto il lancio del gestore del colloquio remoto.
*             Sperimentale.
* 19/11/97 GG 1.30 Aggiunta l'opzione "-remote-debug", che abilita il servizio
*             di debug remoto.
* 09/02/98 GG 1.31 Aggiunta l'installazione del driver "BGAGE" (Bobbio).
* 24/07/98 GG 1.32 Aggiunte le opzioni "-use-inax" (abilitazione uso
*             inax in modalita` master o slave) e "-inax-wd-period n"
*             (impostazione per periodo con cui si controlla il
*             funzionamento del timer).
* 01/09/98 GG 1.33 Aggiunta l'opzione "-min-overrun", che serve a
*             specificare il numero di cicli in overrun concessi al
*             programma senza che venga emesso l'errore di ciclo troppo
*             lungo.
* 03/09/98 GG 1.34 Portato a 0 il valore di default dell'opzione
*             "-inax-wd-period", cosi` il meccanismo di sorveglianza
*             dell'attivita` della INAX e` disattivato.
* 04/09/98 GG 1.35 Aggiunta la registrazione della priorita` iniziale
*             del processo in un aposito campo della struttura di stato
*             principale. Questo evita tutti i problemi legati all'uso
*             di "getprio".
*             Riportato a 30 il valore di default dell'opzione
*             "-inax-wd-period": ora funziona.
* 24/09/98 GG 1.36 Aggiunta l'opzione "-simul", che serve a simulare
*             la presenza di dispositivi senza in realta` accedervi.
*             Tutte le associazioni di I/O hanno successo, ma non si
*             hanno accessi all'hardware.
* 01/10/98 GG 1.37 Aggiunti i campi adatti a gestire un file di configurazione
*             degli I/O, e l'opzione "-path-ioconf" per ridefinire il default.
*             Aggiunta la lettura del file di configurazione.
* 09/11/98 GG 1.38 Aggiunta l'opzione "-defaultio {R|O|D|S}", che specifica
*             la proprieta` di accesso per i segnali non citati nel file di
*             configurazione. La proprieta` di default e` R, il che significa
*             che i segnali non previsti nella configurazione sono da
*             considerare obbligatori.
* 24/03/99 GG 1.39 Eliminata per default la mascheratura del segnale SIGPRIO,
*             che era stata introdotta, a dispetto delle riserve espresse
*             dalla Lynx, per evitare il sorpasso di priorita` (arbitrario,
*             a mio giudizio) operato dal sistema TCP/IP ai danni del thread
*             esecutore. L'opzione "-tcp-priority n", finora non documentata
*             (e priva di effetti, invero) serve ora a scegliere tra il nuovo
*             ed il vecchio comportamento: ponendo il parametro a 0 si ha
*             il vecchio comportamento (inutile, tanto sui CN c'e` gia`
*             intx che che sorpassa ed e` sorpassato da TCP/IP), altrimenti
*             si ha il nuovo (default). A parte questo, il valore numerico
*             non ha attualmente significato.
*             Aggiunta l'opzione "-v", abbreviazione di "-verify".
* 15/12/99 GG 1.40 Raddoppiata la dimensione delle tabelle dei segnali
*             a rilevamento di fronte (1000 per il  booleani e 200 per i long).
*             Sono anche state aggiunte due opzioni
*             per modificare questi limiti (-max-edge-bool e -max-edge-long).
*             Portata a 500 la lunghezza della coda dei messaggi da thread
*             veloce a thread lento, e segnalata la cosa nell'help.
* 28/07/00 GG 1.41 Aggiustamenti per compilazione su Linux.
* 02/11/00 GG 1.42 Modificata leggermente la logica di interpretazione
*             delle opzioni "-stand-alone" e "-no-hatsh", in modo che
*             l'inizializzatore del modulo "hat" venga invocato solo
*             se strettamente richiesto.
* 07/11/00 GG 1.43 Aggiunta l'inizializzazione del logger degli errori.
*/

#include <stdio.h>
#include <string.h>
#ifdef Linux
#include <fcntl.h>
#include <sys/mman.h> 
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <types.h>
#include <signal.h>
#include <ipc.h>
#include <msg.h>
#endif

#ifndef Linux
/* Specifici CNi */
#include <services.h>
#endif

#include "qplc.h"
#include "compat.h"
#include "util.h"
#include "shvar.h"
#include "superv.h"
#include "glue.h"
#include "debugger.h"
#include "remote.h"
#include "ioconf.h"

extern char Versione[];
extern unsigned char * NVRAM;
extern unsigned char * NVRAM_pub;

/*
* Struttura di stato del programma (stato generale : file eseguibile,
* opzioni di lancio...).
*/

qplc_statics main_status;

/*
* Struttura descrivente un'opzione della linea di comando.
*
* Pero ogni opzione riconosciuta, sono dati
* - la stringa corrispondente,
* - la stringa da visualizzare nell'help,
* - il puntatore alla funzione di gestione,
* - due parametri generici per la funzione.
*/

struct option_t {
	char * string;
	char * help;
	int (*handler)(char **argv, void * param1, void *param2);
	void * param1;
	void * param2;
};

/* Due gestori per opzioni. */
/* Gestore per opzioni a flag (attivatori). Il parametro e`
 il puntatore al flag (char) da porre ad 1. */
static int opt_setflag(char **argv, void *param1, void *param2);
/* Gestore di opzioni a stringa. Il parametro e` un puntatore
 a carattere, destinazione della stringa. */
static int opt_setstring(char **argv, void *param1, void *param2);
/* Gestore di opzioni a intero. */
static int opt_setint(char **argv, void *param1, void *param2);

static struct option_t options[] = 
{
{ "-blocking-pipe",
  "{0|1}: Rende bloccanti (1) o non (0, default) le pipe con XNC",
  opt_setint,
  (void *)(&main_status.blocking_pipe),
  (void *) 0,
},
{ "-chkaddr",
  "{0|1}: Abilita alcuni controlli su certe richieste del debugger",
  opt_setint,
  (void *)(&main_status.chkaddr),
  (void *) 0,
},
{ "-debug",
  ": Imposta la modalita` di debug",
  opt_setflag,
  (void *)(&main_status.debug),
  (void *) 0,
},
{ "-debug-buffer",
  "n : Lunghezza dell'area per i dati di debug",
  opt_setint,
  (void *)(&main_status.debug_buffer_size),
  (void *) 0,
},
{ "-debug-cycles",
  "n : Numero massimo di cicli memorizzati in debug",
  opt_setint,
  (void *)(&main_status.n_debug_events),
  (void *) 0,
},
{ "-defaultio",
  "str : Proprieta` dei segnali non citati nel file di configurazione (O,S,D o R(default))",
  opt_setstring,
  (void *)(&main_status.defaultio),
  (void *) sizeof(main_status.defaultio),
},
{ "-devconf",
  "str : Imposta una stringa di configurazione per i driver di I/O",
  opt_setstring,
  (void *)(&main_status.devconf),
  (void *) sizeof(main_status.devconf),
},
{ "-exec-queue-len",
  "n : Lunghezza della coda di messaggi interna (default 500)",
  opt_setint,
  (void *)(&main_status.qlen_exec_to_sv),
  (void *) 0,
},
{ "-go-anyway",
  ": Entra in GO anche se ci sono I/O inesistenti nel programma IL",
  opt_setflag,
  (void *)(&main_status.go_anyway),
  (void *) 0,
},
{ "-halt",
  ": Rimane in HALT al lancio",
  opt_setflag,
  (void *)(&main_status.halt),
  (void *) 0,
},
{ "-inax-wd-period",
  "n : Imposta il periodo con cui e` controllato il timer INAX",
  opt_setint,
  (void *)(&main_status.inax_wd_period),
  (void *) 0,
},
{ "-max-dev-ext",
  "n : numero massimo di associazioni di I/O con linee speciali",
  opt_setint,
  (void *)(&main_status.max_extensions),
  (void *) 0,
},
{ "-max-edge-bool",
  "n : numero massimo di variabili BOOL a rilevamento di fronte (def. 1000)",
  opt_setint,
  (void *)(&main_status.n_max_edge_bool),
  (void *) 0,
},
{ "-max-edge-long",
  "n : numero massimo di variabili LONG a rilevamento di fronte (def. 200)",
  opt_setint,
  (void *)(&main_status.n_max_edge_long),
  (void *) 0,
},
{ "-max-halt",
  "n : numero massimo di chiamate consecutive dei metodi di HALT",
  opt_setint,
  (void *)(&main_status.max_halt_cycle),
  (void *) 0,
},
{ "-max-prog",
  "n : numero massimo di programmi IL in un modulo",
  opt_setint,
  (void *)(&main_status.max_il_prog_mod),
  (void *) 0,
},
{ "-mdata",
  ": Forza il collegamento al server dei dati macchina",
  opt_setflag,
  (void *)(&main_status.mdata),
  (void *) 0,
},
{ "-min-overrun",
  "n : numero di cicli in overrun concessi al programma (default=0)",
  opt_setint,
  (void *)(&main_status.min_overrun),
  (void *) 0,
},
{ "-no-fault",
  ": Evita in ogni caso lo stato di FAULT",
  opt_setflag,
  (void *)(&main_status.no_fault),
  (void *) 0,
},
{ "-no-hatsh",
  ": Inibisce il caricamento automatico del modulo \"hatsh\"",
  opt_setflag,
  (void *)(&main_status.no_mod_hatsh),
  (void *) 0,
},
{ "-no-prog480",
  ": Inibisce il caricamento automatico del modulo \"prog480\"",
  opt_setflag,
  (void *)(&main_status.no_mod_prog480),
  (void *) 0,
},
{ "-no-user",
  ": Inibisce il caricamento automatico del modulo \"user\"",
  opt_setflag,
  (void *)(&main_status.no_mod_user),
  (void *) 0,
},
{ "-nvram-path",
  ": Specifica la dimensione della parte utile di NVRAM su file",
  opt_setstring,
  (void *)(&main_status.nvram_path),
  (void *) sizeof(main_status.nvram_path),
},
{ "-nvram-refresh",
  ": Specifica il periodo di riscrittura del file di NVRAM",
  opt_setint,
  (void *)(&main_status.nvram_refresh),
  (void *) 0,
},
{ "-nvram-size",
  ": Specifica il file da usare come NVRAM",
  opt_setint,
  (void *)(&main_status.nvram_size),
  (void *) 0,
},
{ "-nvram-totsize",
  ": Specifica la dimensione totale del file NVRAM",
  opt_setint,
  (void *)(&main_status.nvram_totsize),
  (void *) 0,
},
{ "-overrun",
  "n : Imposta la durata massima di un ciclo, in unita` di clock",
  opt_setint,
  (void *)(&main_status.max_overrun),
  (void *) 0,
},
{ "-path-hatsh",
  "file : Specifica il file da caricare come modulo \"hatsh\"",
  opt_setstring,
  (void *)(&main_status.mod_hatsh_path),
  (void *) sizeof(main_status.mod_hatsh_path),
},
{ "-path-prog480",
  "file : Specifica il file da caricare come modulo \"prog480\"",
  opt_setstring,
  (void *)(&main_status.mod_prog480_path),
  (void *) sizeof(main_status.mod_prog480_path),
},
{ "-path-user",
  "file : Specifica il file da caricare come modulo \"user\"",
  opt_setstring,
  (void *)(&main_status.mod_user_path),
  (void *) sizeof(main_status.mod_user_path),
},
{ "-path-ioconf",
  "file : Specifica il file di configurazione degli I/O",
  opt_setstring,
  (void *)(&main_status.ioconf_path),
  (void *) sizeof(main_status.ioconf_path),
},
{ "-period",
  "n : Imposta il periodo, o clock, del Plc (millisecondi)",
  opt_setint,
  (void *)(&main_status.period),
  (void *) 0,
},
{ "-posix-poll",
  ": Utilizza la primitiva \"poll\" anziche` \"select\". Vuole STREAMS",
  opt_setflag,
  (void *)(&main_status.posix_poll),
  (void *) 0,
},
{ "-priority",
  "n : Imposta l'incremento di priorita` per l'esecutore",
  opt_setint,
  (void *)(&main_status.prio_exec),
  (void *) 0,
},
{ "-quiet",
  ": Inibisce l'emissione di messaggi",
  opt_setflag,
  (void *)(&main_status.quiet),
  (void *) 0,
},
{ "-remote-debug",
  ": Abilita il servizio di debug remoto",
  opt_setflag,
  (void *)(&main_status.remote_debug),
  (void *) 0,
},
{ "-reset",
  ": Riporta in uno stato consistente la memoria condivisa",
  opt_setflag,
  (void *)(&main_status.reset_shm),
  (void *) 0,
},
{ "-save-io",
  ": Salva in RAM di backup la configurazione hardware",
  opt_setflag,
  (void *)(&main_status.old_save_io),
  (void *) 0,
},
{ "-shvinit",
  ": Azzera i segnali di scambio senza rimuoverli",
  opt_setflag,
  (void *)(&main_status.shv_init),
  (void *) 0,
},
{ "-shvmask",
  "n : Maschera di creazione dei segnali di scambio",
  opt_setint,
  (void *)(&main_status.package),
  (void *) 0,
},
{ "-sigla",
  "stringa : Definisce la sigla per la visualizzazione degli errori",
  opt_setstring,
  (void *)(main_status.err_sigla),
  (void *) NSIG_XERR,
},
{ "-simul",
  ": Evita di accedere all'hardware",
  opt_setflag,
  (void *)(&main_status.simul),
  (void *) 0,
},
{ "-soft-tm",
  ": Abilita la statistica del tempo di ciclo al lancio",
  opt_setflag,
  (void *)(&main_status.soft_tm),
  (void *) 0,
},
{ "-stand-alone",
  ": Lancia il PLC in assenza di altre parti del CN",
  opt_setflag,
  (void *)(&main_status.stand_alone),
  (void *) 0,
},
{ "-stdout",
  ": Non redirige stdout ed stderr",
  opt_setflag,
  (void *)(&main_status.nstdout),
  (void *) 0,
},
{ "-tcp-priority",
  "n : Imposta l'incremento di priorita` per il colloquio su TCP/IP",
  opt_setint,
  (void *)(&main_status.tcp_prio),
  (void *) 0,
},
{ "-timer-bug",
  ": Corregge il problema dei timer di LynxOS 2.3.0",
  opt_setflag,
  (void *)(&main_status.timer_bug),
  (void *) 0,
},
{ "-use-inax",
  ": Usa la scheda INAX come sorgente di temporizzazione",
  opt_setflag,
  (void *)(&main_status.use_inax),
  (void *) 0,
},
{ "-v",
  ": Visualizza la versione ed esce",
  opt_setflag,
  (void *)(&main_status.verify),
  (void *)(&main_status.stand_alone),
},
{ "-verify",
  ": Visualizza la versione ed esce",
  opt_setflag,
  (void *)(&main_status.verify),
  (void *)(&main_status.stand_alone),
},
};

/*
* Formattazione del nome del programma.
*
* Questa funzione ricava il path completo ed il nome base del file
* eseguibile "qplc".
*/

static void format_exe_name(char * argv0)
{
char *p;

	if (argv0[0] != '/') {
		getwd(main_status.exe_path);
		strcat(main_status.exe_path,"/");
		strcat(main_status.exe_path,argv0);
		main_status.exe_base = strrchr(main_status.exe_path, '/') + 1;
	}
	else {
		strcpy(main_status.exe_path, argv0);
		p = strrchr(argv0, '/');
		p = p ? p + 1 : argv0;
		main_status.exe_base = p;
	}
	main_status.ver_sig[VER_IDX_KERNEL] = main_status.exe_base;
}

/*
* Imposta una ragionevole configurazione di default.
* E` bene ricordarsi di aggiungere qui un'inizializzazione
* per ogni nuovo campo definito nella struttura "qplc_statics".
*/

static void load_default_conf(void)
{
int i;

	memset((char *)&main_status, 0, sizeof(main_status));

	strcpy(main_status.exe_path,"qplc");
	strcpy(main_status.devconf,"");
	main_status.drivers.ndrvrs = 0;
	main_status.max_extensions = 50;
	main_status.exe_base = main_status.exe_path;
	strcpy(main_status.err_sigla,"PLC");
	main_status.stand_alone = 0;
	main_status.mdata = 0;
	main_status.chkaddr = 1;
	main_status.blocking_pipe = 0;
	main_status.quiet = 0;
	main_status.posix_poll = 0;
	main_status.verify = 0;
	main_status.old_save_io = 0;
	main_status.debug = 0;
	main_status.simul = 0;
	main_status.no_fault = 0;
	main_status.package = PKG_ALL;
	main_status.nstdout = 0;
	main_status.start_prio = getprio(0);
	main_status.tcp_prio = 7;
	main_status.timer_bug = 0;
	main_status.use_inax = 0;
	main_status.reset_shm = 0;
	main_status.prio_exec = 10;
	main_status.min_overrun = 0;
	main_status.max_overrun = 5;
	main_status.period = 20;
	main_status.inax_wd_period = 30;
	main_status.go_anyway = 0;
	main_status.halt = 0;
	main_status.remote_debug = 0;
	main_status.shv_init = 0;
	main_status.shv_mkedge = 1;
	main_status.soft_tm = 0;
	main_status.qlen_exec_to_sv = 500;
	main_status.max_il_prog_mod = 20;
	main_status.curr_prog_id = 0;
	main_status.ver_kernel = Versione + 5; /* Salta "@(.) " */
	main_status.m_data = (char *) 0;
	main_status.isaker_sysv_sem_key = 1919;
	main_status.isaker_sysv_sem_id = -1;
	main_status.sysv_shm_key = 1919;
	main_status.sysv_shm_id = -1;
	main_status.nvram1_name = NVRAM_LBL_PLC;
	main_status.nvram1.addr = (unsigned char *)0;
	main_status.nvram1.size = 0;
	main_status.nvram1_free = (unsigned char *)0;
	main_status.nvram2_name = NVRAM_LBL_PLC480;
	main_status.nvram2.addr = (unsigned char *)0;
	main_status.nvram2.size = 0;
	main_status.nvram2_free = (unsigned char *)0;
	main_status.shm_addr = (qplc_shared *) 0;
	main_status.shm_size = 0x30000;
	main_status.osys = (ik_system_t *) 0;
	main_status.shvn = (shv_t *) 0;
	main_status.shvh = (long *) 0;
	main_status.p_shvn = (long *) 0;
	main_status.p_shvn_size = (long *) 0;
	main_status.p_shheap = (long *) 0;
	main_status.p_shheap_size = (long *) 0;
	main_status.shvk = (shvar_t *) 0;
	main_status.shvk_tab_len = 0x2000;
	main_status.shvh_tab_len = 128;
	main_status.mod_hatsh = (module_t *) 0;
	main_status.shvkey_to_descr = (shv_t **) 0;
	memset((char *)&main_status.mod_hatsh_op, 0,
	       sizeof(main_status.mod_hatsh_op));
	main_status.mod_user = (module_t *) 0;
	memset((char *)&main_status.mod_user_op, 0,
	       sizeof(main_status.mod_user_op));
	main_status.mod_plc480 = (module_t *) 0;
	memset((char *)&main_status.mod_plc480_op, 0,
	       sizeof(main_status.mod_plc480_op));
	main_status.drivers.driver_list = (driver_t *) 0;
	main_status.drivers.in_list = (driver_t *) 0;
	main_status.drivers.out_list = (driver_t *) 0;
	main_status.drivers.check_list = (driver_t *) 0;
	main_status.drivers.trigger_list = (driver_t *) 0;
	strcpy(main_status.mod_hatsh_path,"plc-objs/hatsh.oo");
	strcpy(main_status.mod_user_path,"plc-objs/user.oo");
	strcpy(main_status.mod_prog480_path,"plc-objs/prog.oo");
	strcpy(main_status.ioconf_path,"ioconf.qplc");
	strcpy(main_status.defaultio,"R");
	main_status.no_mod_user = 0;
	main_status.no_mod_hatsh = 0;
	main_status.no_mod_prog480 = 0;
	main_status.n_edge_bool     = 0;
	main_status.n_max_edge_bool = 1000;
	main_status.p_edge_bool     = (char **)0;
	main_status.p_image_bool    = (char *)0;
	main_status.n_edge_long     = 0;
	main_status.n_max_edge_long = 200;
	main_status.p_edge_long     = (long **)0;
	main_status.p_image_long    = (long *)0;
	main_status.p_cnok = (unsigned char *)0;
	main_status.p_exec_status = &exec_status;
	main_status.p_superv_status = &superv_status;
	main_status.debug_buffer_size = EVENT_BUFFER_SIZE;
	main_status.n_debug_events = MAX_LOGGED_EVENTS;
	for (i = 0; i < MAX_VER_SIG; ++i)
		main_status.ver_sig[i] = "";
	main_status.ver_sig[VER_IDX_KERNEL] = main_status.exe_path;
	main_status.ver_sig[OLD_MOD_IDX_HATSH] = "hatsh";
	main_status.ver_sig[OLD_MOD_IDX_USER] = "PLC";
	main_status.ver_sig[OLD_MOD_IDX_480] = "CPLC";
	main_status.ver_sig[OLD_IDX_BUILTIN_USER] = "ld480";
/* Path del file di NVRAM "dei poveri". */
	strcpy(main_status.nvram_path,"");
/* Periodo di rinfresco del file di NVRAM in millisecondi. */
	main_status.nvram_refresh = 0;
/* Dimensione della parte utile di NVRAM su file. */
	main_status.nvram_size = 0;
/* Dimensione totale del file di NVRAM. */
	main_status.nvram_totsize = 0;
/* Puntatore alla mappa della parte utile del file di NVRAM. */
	main_status.nvram_filemap = (void *)0;
/* Puntatore all'area immagine dell'NVRAM (memoria locale). */
	main_status.nvram_image = (void *)0;
}

/*
* Lettura del file di configurazione.
*/

static void read_config_file(void)
{
	cioInit(main_status.ioconf_path);
}

/*
* Visualizza l'help ed esce.
*/

static void usagerr(void)
{
int i;

	fprintf(stderr, "%s : uso :\n%s\n",
		main_status.exe_base,
		main_status.exe_base );
	for (i = 0; i < sizeof(options)/sizeof(options[0]); ++i)
		fprintf(stderr," %s %s.\n",options[i].string,options[i].help);
	fprintf(stderr,"\n");
	exit(1);
}

/*
* Analisi della linea di comando.
* -1 in caso di errore.
*/

/* Gestore per opzioni a flag (attivatori). Il parametro e`
 il puntatore al flag (char) da porre ad 1. */

static int opt_setflag(char **argv, void * param1, void * param2)
{
	if (param1)
		*(char *)param1 = 1;
	if (param2)
		*(char *)param2 = 1;
	return 0;
}

/* Gestore di opzioni a stringa. Il parametro e` un puntatore
 a carattere, destinazione della stringa. */

static int opt_setstring(char **argv, void * param1, void * param2)
{
	if (! argv[1])
		return -1;
	strncpy((char *)param1,argv[1],(int) param2 - 1);
	((char *)param1)[(int) param2 - 1] = '\0';
	return 1;
}

/* Gestore di opzioni a interi. Il parametro e` un puntatore
 a intero, destinazione del valore. */

static int opt_setint(char **argv, void * param1, void * param2)
{
int v;
char *p;

	if (! argv[1])
		return -1;
	for (v = 0, p = argv[1]; *p; ++p) {
		if (*p < '0' || *p > '9')
			return -1;
		v = v * 10 + *p - '0';
	}
	*(int *)param1 = v;
	return 1;
}

/* Analizzatore della linea di comando. */

static int scan_command_line(int argc, char **argv)
{
int i,j,rv,ok;

	for (i = 1; i < argc; ++i) {
		ok = 0;
		for (j = 0; j < sizeof(options)/sizeof(options[0]); ++j) {
			if (strcmp(options[j].string,argv[i]) == 0) {
				rv = options[j].handler(&argv[i],
				                        options[j].param1,
				                        options[j].param2);
				if (rv >= 0) {
					ok = 1;
					i += rv;
				}
				break;
			}
		}
		if (! ok)
			return -1;
	}

	return 0;
}
 
/*
* Inizializza i canali dei messaggi e degli errori (contesto CNI).
*/

static void init_cn_msgs(void)
{
	if (! main_status.stand_alone) {
#ifndef Linux
		initXerror();
		attrXerror(main_status.blocking_pipe
		           ? XER_BLOCCO : XER_NO_BLOCCO);
		if (! main_status.nstdout) {
			initStdout();
			attrStdout(main_status.blocking_pipe
				   ? XER_BLOCCO : XER_NO_BLOCCO);
			initStderr();
			attrStderr(main_status.blocking_pipe
				   ? XER_BLOCCO : XER_NO_BLOCCO);
		}
#endif
	}
}

/*
* Collega il PLC al server dei dati macchina.
*/

static void init_cn_mdata(void)
{
	if (main_status.mdata || ! main_status.stand_alone) {
#ifndef Linux
		main_status.m_data = AttachServerDm(ID_SDM_CN);
#endif
	}
}

/*
* Definisce l'azione da compiere alla ricezione di un segnale d'uscita.
*/

static void terminate(int s)
{

/* Chiama il metodo di blocco e restart dei dispositivi. */

	ioDown();
	ioRestart();

/* Chiama il metodo di chiusura del modulo "hatsh". */

	main_status.mod_hatsh_op.exit();

/* Segnala l'entrata in HALT. */
 
	main_status.shm_addr -> go = 0;

/* ...e "fine del palo". */

	fprintf(stderr,"%s : terminated", main_status.exe_base);
	if (s)
		fprintf(stderr," (signal %d)",s);
	fprintf(stderr,"\n");
	if (s == 0 || s == SIGINT || s == SIGTERM || s == SIGHUP)
		exit(0);
	else
		exit(1);
}

static void sigexit_intercept(void)
{
#ifdef Linux
#if 1
struct sigaction act;

	act.sa_handler = terminate;
	sigemptyset(&act.sa_mask);
	sigfillset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_restorer = NULL;
	sigaction(SIGINT,&act,NULL);
	sigaction(SIGHUP,&act,NULL);
	sigaction(SIGQUIT,&act,NULL);
	sigaction(SIGTERM,&act,NULL);
	sigaction(SIGPIPE,&act,NULL);
#else
struct sigvec vec;

	vec.sv_handler = terminate;
	vec.sv_mask = ~0; /* Maschera tutto alla ricezione del segnale. */
	vec.sv_onstack = 0;
	sigvec(SIGINT,&vec,(struct sigvec *)0);
	sigvec(SIGHUP,&vec,(struct sigvec *)0);
	sigvec(SIGQUIT,&vec,(struct sigvec *)0);
	sigvec(SIGTERM,&vec,(struct sigvec *)0);
	sigvec(SIGPIPE,&vec,(struct sigvec *)0);
#endif

#else
struct sigvec vec;

	vec.sv_handler = terminate;
	vec.sv_mask = ~0; /* Maschera tutto alla ricezione del segnale. */
	vec.sv_onstack = 0;
	sigvec(SIGINT,&vec,(struct sigvec *)0);
	sigvec(SIGHUP,&vec,(struct sigvec *)0);
	sigvec(SIGQUIT,&vec,(struct sigvec *)0);
	sigvec(SIGTERM,&vec,(struct sigvec *)0);
	sigvec(SIGSYS,&vec,(struct sigvec *)0);
	sigvec(SIGPIPE,&vec,(struct sigvec *)0);

	if (main_status.tcp_prio) {
		vec.sv_handler = SIG_IGN;
		sigvec(SIGPRIO,&vec,(struct sigvec *)0);
	}
#endif
}

/*
* Creazione dell'NVRAM basata su file.
*/
static void init_nvram_file(void)
{
int fd;
void *p;

/* Path del file di NVRAM "dei poveri". */
	if (main_status.nvram_path[0] == '\0') {
	/* Non richiesto. Meglio. */
		return;
	}
/* Periodo di rinfresco del file di NVRAM in millisecondi. */
	if (main_status.nvram_refresh == 0) {
		main_status.nvram_refresh = 3000;
	}
/* Dimensione della parte utile di NVRAM su file. */
	if (main_status.nvram_size == 0) {
		main_status.nvram_size = 512;
	}
	main_status.nvram_size
	 += (sizeof(long) - main_status.nvram_size%sizeof(long));
/* Dimensione totale del file di NVRAM. */
	if (main_status.nvram_totsize == 0) {
		main_status.nvram_totsize = 0x10000;
	}
	main_status.nvram_totsize
	 = ((main_status.nvram_totsize + 0x3FFF) / 0x8000) * 0x8000;
	if (main_status.nvram_totsize == 0) {
		main_status.nvram_totsize
		 += ((main_status.nvram_size) / 0x4000) * 0x4000;
	}
/* Puntatore alla mappa della parte utile del file di NVRAM. */
	main_status.nvram_filemap = (void *)0;
/* Puntatore all'area immagine dell'NVRAM (memoria locale). */
	main_status.nvram_image = malloc(main_status.nvram_size);
	if (main_status.nvram_image == (void *)0) {
		util_fatal(main_status.nvram_path);
	}
	memset(main_status.nvram_image,0,main_status.nvram_size);

	fd = open(main_status.nvram_path, O_RDWR);
	if (fd < 0) {
	FILE *f;
	int i;

		f = fopen(main_status.nvram_path, "w");
		if (f == NULL) {
			util_fatal(main_status.nvram_path);
		}
		for (i = 0; i < main_status.nvram_totsize; ++i) {
			fputc(0,f);
		}
		fclose(f);
		fd = open(main_status.nvram_path, O_RDWR);
		if (fd < 0) {
			util_fatal(main_status.nvram_path);
		}
	}

	p = mmap(NULL,main_status.nvram_size,
	         PROT_READ|PROT_WRITE,MAP_SHARED,
	         fd,main_status.nvram_totsize/2);
	if (p == NULL || p == ((void *)-1)) {
		util_fatal(main_status.nvram_path);
	}

	main_status.nvram_filemap = p;
	memcpy(main_status.nvram_image, p,main_status.nvram_size);

	main_status.nvram2.addr = (unsigned char *) main_status.nvram_image;
	main_status.nvram2.size = main_status.nvram_size;
/* TODO: Questo e' codice duplicato (vedi compat.c). Porta male... */
	main_status.nvram2_free = (NVRAM
	                         = NVRAM_pub
	                         = main_status.nvram2.addr)
	                        + sizeof(long);
}

/*
* Creazione degli oggetti condivisi.
*/

static void create_shm(void)
{
union { int val; struct semid_ds *buf; unsigned short *array; } arg;
struct sembuf op;

/* Creazione ed inizializzazione dei semafori (compatibilita`...). */

	main_status.isaker_sysv_sem_id
	 	 = semget(main_status.isaker_sysv_sem_key, 2, IPC_CREAT | 0777);
	if (main_status.isaker_sysv_sem_id < 0)
		util_fatal("semget");
	arg.val = 0;
	if (semctl(main_status.isaker_sysv_sem_id, 0, SETVAL, arg) < 0)
		util_fatal("semctl(0)");
	arg.val = 1;
	if (semctl(main_status.isaker_sysv_sem_id, 1, SETVAL, arg) < 0)
		util_fatal("semctl(1)");

/* Creazione della shared memory. Forza un reset se la
 shared memory non esisteva gia`. */

	main_status.sysv_shm_id = shmget(main_status.sysv_shm_key,
	                                 main_status.shm_size,
	                                 0666);
	if (main_status.sysv_shm_id < 0) {
		main_status.reset_shm = 1;
		main_status.sysv_shm_id = shmget(main_status.sysv_shm_key,
						 main_status.shm_size,
						 IPC_CREAT | 0777);
	}
	if (main_status.sysv_shm_id < 0)
		util_fatal("shmget");
	main_status.shm_addr = (qplc_shared *) shmat(main_status.sysv_shm_id,
	                                             (char *) 0, 0);
	if (main_status.shm_addr == (qplc_shared *) -1)
		util_fatal("shmat");

/* Inizializzazione (se richiesto) della shared memory. */

/* Forza in ogni caso lo stato di "HALT". */

	main_status.shm_addr -> go = 0;

/* Azzera in ogni caso il flag di shared mamory inizializzata. */

	main_status.shm_addr -> shm_ok = 0;

	if (main_status.reset_shm) {

	/* Azzera tutto. */

		memset((char *)main_status.shm_addr, 0, main_status.shm_size);

	/* Ricrea il direttorio degli spazi. */

		compat_init_spaces();

	}

/* Riassegna il puntatore alla base della memoria condivisa. */

	main_status.shm_addr -> vmembase = (char *) main_status.shm_addr;

/* Registra in un posto di piu` facile accesso il puntatore
 agli spazi allocati (OSYS, SHVK, SHVH, SHVN e FREE). */

	main_status.osys
	 = (ik_system_t *)
	    ((char *) main_status.shm_addr
	     + main_status.shm_addr -> isaker_spc_dir[_IK_SPC_OSYS].off);

	main_status.shvk
	 = (shvar_t *)
	    ((char *) main_status.shm_addr
	     + main_status.shm_addr -> isaker_spc_dir[_IK_SPC_SHVK].off);

	main_status.shvh
	 = (long *)
	    ((char *) main_status.shm_addr
	     + main_status.shm_addr -> isaker_spc_dir[_IK_SPC_SHVH].off);

	main_status.shvn
	 = (shv_t *)
	    ((char *) main_status.shm_addr
	     + main_status.shm_addr -> isaker_spc_dir[_IK_SPC_SHVN].off);

	main_status.p_shvn
	 = & main_status.shm_addr -> isaker_spc_dir[_IK_SPC_SHVN].off;
	main_status.p_shvn_size
	 = & main_status.shm_addr -> isaker_spc_dir[_IK_SPC_SHVN].size;

	main_status.p_shheap
	 = & main_status.shm_addr -> isaker_spc_dir[_IK_SPC_FREE].off;
	main_status.p_shheap_size
	 = & main_status.shm_addr -> isaker_spc_dir[_IK_SPC_FREE].size;

/* Liberazione del semaforo della shared memory. */

	op.sem_num = 0;
	op.sem_op = 1;
	op.sem_flg = 0;
	if (semop(main_status.isaker_sysv_sem_id, &op, 1) < 0)
		util_fatal("semop");

}

/*
* Installazione dei driver.
*/
/*
extern driver_op_t driver_ios;
extern driver_op_t driver_keyb;
extern driver_op_t driver_cpax;
extern driver_op_t driver_ser;
extern driver_op_t driver_bgage;
*/
static void install_drivers_1(void)
{
/*
	ioInstall(&driver_ios,0);
	ioInstall(&driver_ios,1);
#ifndef Linux
	ioInstall(&driver_keyb,0);
#endif
	ioInstall(&driver_ser,0);
#ifndef Linux
	ioInstall(&driver_bgage,0);
#endif
*/
}

static void install_drivers_2(void)
{
/*
#ifndef Linux
	ioInstall(&driver_cpax,0);
#endif
*/
}

/*
* Inizializzazione del CN.
*/

static void cn_init(void)
{
#if 1
struct sigaction act,oact;

#else
struct sigvec vec,ovec;

#endif

#ifndef Linux
	if (main_status.tcp_prio) {
		vec.sv_mask = ~0;
		vec.sv_onstack = 0;
		vec.sv_handler = SIG_DFL;
		sigvec(SIGPRIO,&vec,&ovec);
	}
#endif
	if (main_status.stand_alone
	 || main_status.no_mod_hatsh
	 || main_status.mod_hatsh_op.cninit())
		*main_status.mod_hatsh_op.ok = 0;
	else
		*main_status.mod_hatsh_op.ok = 1;
#ifndef Linux
	if (main_status.tcp_prio) {
		sigvec(SIGPRIO,&ovec,(struct sigvec *)0);
	}
#endif
}

/*
* Due funzioncine vuote per "rappezzare" hatsh imbizzarriti.
*/

static void null_hat(void) { }
static void null_shoes(int x) { }

/********/
/* MAIN */
/********/

void main(int argc, char ** argv)
{
/* Carica la configurazione di default. */
	load_default_conf();
/* Ricava il nome dell'eseguibile. */
	format_exe_name(argv[0]);
/* Analizza la linea di comando. Se errore, mostra l'help ed esce. */
	if (scan_command_line(argc,argv) < 0) {
		usagerr();
	}
/* Aggiusta alcuni valori di configurazione (dettagliucci). */
	/* Assicura che sia sempre creato il minimo indispensabile
	 dei segnali di scambio. */
	main_status.package |= PKG_MIN;
/* Imposta il pontatore alla funzione da utilizzare come select-poll. */
	glue_init(main_status.posix_poll);
/* Inizializza la parte relativa ai messaggi/errori del CN ed al
 server dei dati macchina, a meno che il programma non sia stato
 lanciato solo per per leggere la versione. */
	if (! main_status.verify) {
		init_cn_msgs();
		init_cn_mdata();
	}
/* Definizione dei moduli a caricamento dinamico (ora, per poter leggere
 le versioni). */
	create_modules();
/* Visualizza la versione delle varie parti. */
	if (! main_status.quiet) {
		util_show_versions();
	}
/* Legge la configurazione dei segnali di I/O. */
	read_config_file();
/* Esce se si voleva solo leggere la versione. */
	if (main_status.verify) {
		exit(0);
	}

/* ... e, finalmente, cominciamo con le cose serie. */

/* Inizializza la alcune funzioni di utilita`. */
	util_check_addr_init();

/* Intercettazione dei segnali d'uscita. */
	sigexit_intercept();

/* Gestione "particolare" dell'NVRAM basata su file. */
	init_nvram_file();

/* Creazione dello spazio condiviso e dei semafori associati. */
	create_shm();

/* Installazione dei driver (prima puntata). */
	install_drivers_1();

/* Inizializzazione della parte di compatibilita` con isaker (prima parte). */
	compat_init_1();

/* Salvataggio della configurazione di I/O in stile isaker ed uscita,
 se richiesto. */
	if (main_status.old_save_io) {
		compat_save_io();
		exit(0);
	}

/* Inizializzazione del logger degli errori. */
	debErrorInit();

/* Carica dinamicamente i moduli "vecchio stile". */
	load_modules();

/* Chiama l'inizializzatore del CN. */
	cn_init();

/* Installazione dei driver (seconda puntata). */
	install_drivers_2();

/* Completa l'inizializzazione del modulo di compatibilita`. */
	compat_init_2();

/* Se il modulo "HATSH" e` disabilitato, utilizza due funzioni fittizie. */
	if (! *main_status.mod_hatsh_op.ok) {

		if (! main_status.stand_alone) {
		/* Un errore, probabilmente. E conviene anche lasciare
		 l'esecutore in HALT. */
			main_status.halt = 1;
			plcError(195,"");
		}
		main_status.mod_hatsh_op.hat = null_hat;
		main_status.mod_hatsh_op.shoes = null_shoes;
	}

/* Lancia il thread supervisore. */
	superv_start();

/* Alza inizialmente il segnale "CNOK", se definito. */
	if (main_status.p_cnok)
		*main_status.p_cnok = 1;

/* Lancia il gestore del colloquio remoto. */
	dsv_start();

/* Segnala che la shared memory e` inizializzata. */
	++(main_status.shm_addr -> isaker_dbver);
	main_status.shm_addr -> shm_ok = 1;

/* Entra nel ciclo di esecuzione (e non uscira` mai piu`). */
	exec_main();
}

