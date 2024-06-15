/*
* @(#) exec.h 1.19 Fri May 11 15:21:07 CEST 2001
*
*  Questo file contiene le definizioni relative alle funzioni di
* controllo dello stato dell'esecutore.
*
* 11/06/96 GG 1.0 Prima stesura.
* 30/08/96 GG 1.1 Aggiunta le funzione "plcPeriod". Aggiunti contatori di
*             ciclo e di tempo trascorso. Aggiunte anche le
*             funzioni "plcNCycle" e "plcTime".
* 05/09/96 GG 1.2 Aggiunta la funzione "plcGo".
* 19/09/96 GG 1.3 Aggiunto un'elaborazione dei parametri in stile "printf"
*             alle funzioni "plcError", "plcWarning" e "plcMessage".
* 20/09/96 GG 1.4 Rimossa la distinzione tra supervisore ed esecutore
*             riguardo alle primitive dei messaggi e degli errori. La cosa
*             e` resa possibile da un flag che vale 1 se il task corrente
*             e` l'esecutore, 0 altrimenti.
* 27/09/96 GG 1.5 Aggiunta la funzione "plcRuntimeError", che emette errori
*             associati a valori del program counter, espressi in forma
*             comprensibile.
* 11/12/96 GG 1.6 Aggiunti i campi dei puntatori di inizio e fine debug.
* 06/02/97 GG 1.7 Aggiunti altri 32 bit al contatore di tempo trascorso.
*             Di conseguenza, e` stata definita la funzione plcTimeH, che
*             restituisce questa nuova informazione.
* 23/05/97 GG 1.8 Aggiunta la possibilita` di ripetere la chiamata delle
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
* 11/06/97 GG 1.9 Aggiunta la funzione "plcUpdateMessage", che aggiorna
*             un messaggio anziche` accodarlo a quelli gia` presenti
*             (ma ci vuole la versione 2.6.0.15 di quote).
* 11/11/97 GG 1.10 Aggiunta la funzione "plcEventNotify".
* 02/12/97 GG 1.11 Aggiunto un parametro a "plcEventNotify".
* 12/03/98 GG 1.12 Aggiunto un semaforo di sincronizzazione per sbloccare
*             il supervisore nelle operazioni che richiedono piu` cicli.
*             Aggiunta la funzione "plcIsHalt", che vale 1 solo se lo stato
*             corrente e` "HALT ordinario e stabilizzato". Solo per uso
*             interno.
* 10/06/98 GG 1.13 Aggiunta la funzione "plcSaveAx", per il salvataggio dei
*             dati asse.
* 24/07/98 GG 1.14 Aggiunti campi per la gestione di diverse sorgenti di
*             temporizzazione.
* 04/09/98 GG 1.15 Affidato ad un thread separato il meccanismo di
*             sorveglianza nella modalita` sincronizzata ad intx. Questo
*             dovrebbe risolvere il (presunto) difetto di "sigwait".
* 18/02/99 GG 1.16 Aggiunta la funzione "plcSaveGenericTable", per il
*             salvataggio di una tabella utente dato il nome.
* 12/12/00 GG 1.17 Aggiunta la funzione "plcSaveOrig", che permette di
*             salvare i dati delle origini.
* 08/02/01 GG 1.18 Aggiunta la funzione "plcSaveTable", che permette di
*             salvare qualsiasi tabella dati il nome ed il centro.
* 07/05/01 GG 1.19 Aggiunto un flag di "HALT in corso" per evitare
*             "raffiche" di "plcHalt". Evita che errori hardware
*             provochino un ciclo infinito se e` attivata l'opzione
*             "-no-fault".
*/

#ifndef _EXEC_H_

/*
* Elemento della coda di messaggi verso il supervisore.
*/

typedef struct _exec_qmsg_t {
	sv_msg_t msg;
	struct _exec_qmsg_t * next;
	struct _exec_qmsg_t * pred;
} exec_qmsg_t;

/*
* Definizione della struttura di stato dell'esecutore.
*/

#define MAX_QEXEC_PRIO 5

typedef struct {
/* Puntatore alla funzione ciclica, dipendente dallo stato. */
	void (* cycle)(void);
/* Puntatore alla funzione di inizio misura del tempo di ciclo. */
	void (* tm_begin)(void);
/* Puntatore alla funzione di fine misura del tempo di ciclo. */
	void (* tm_end)(void);
/* Puntatore alla funzione di inizio debug. */
	void (* deb_begin)(void);
/* Puntatore alla funzione di fine debug. */
	void (* deb_end)(void);
/* Istanti di inizio e fine del ciclo. */
	struct timeval c_begin_time;
	struct timeval c_end_time;
/* Durata corrente, minima e massima del ciclo, in microsecondi. */
	long curr_usec;
	long max_usec;
	long min_usec;
/* Numero di ciclo corrente. */
	unsigned long n_cycle;
/* Tempo trascorso dal lancio di qplc (millisecondi). */
	unsigned long plc_time;
/* Tempo trascorso dal lancio di qplc (parte alta, incrementato ad ogni
 overflow del precedente campo). */
	unsigned long plc_time_h;
/* Puntatore alla funzione di controllo del colloquio "isaker-compatibile". */
	void (* old_comm)(void);
/* Numero corrente di cicli in overrun. */
	int curr_overrun;
/* Messaggio ricevuto dal supervisore. Normalmente msg == 0. */
	sv_msg_t *msg;
/* Spazio per un messaggio verso il supervisore, utilizzato per
 l'esecuzione multipla di comandi (SV_COMM_EXECLIST). */
	sv_msg_t sv_msg;
/* Lista dei nodi liberi per la creazione delle code di messaggi. */
	exec_qmsg_t *free_q;
/* Flag di colloquio col supervisore in corso. */
	int tx;
/* Stato di avanzamento del colloquio. */
	int phase;
/* Numero di messaggi in coda. */
	int q_len;
/* Lista delle code di messaggi. */
	exec_qmsg_t *kq[MAX_QEXEC_PRIO];
/* Lista delle code di messaggi in corso di trasmissione. */
	exec_qmsg_t *kq_tx[MAX_QEXEC_PRIO];
/* Campi utilizzati dal supervisore nell'esecuzione multipla. */
/* Puntatore all base dell'array di liste di messaggi. */
	exec_qmsg_t **kq_base;
/* Posizione corrente della funzione "exec_getmsg". */
	int kq_index;	/* priorita` corrente */
	exec_qmsg_t *kq_current;	/* messaggio corrente */
/* Flag indicante che il thread corrente e` l'esecutore. Permette di
 differenziare il comportamento di alcune funzioni (per esempio quelle
 degli errori) in base a chi le chiama. Il meccanismo funziona se vale
 l'ipotesi che l'esecutore non sia interrompibile dagli altri thread. */
	int executor;
/* Flag di HALT in corso. */
	int halt;
/* Flag di richiesta di ripetizione della procedure iniziale di halt. */
	int repeat_start_halt;
/* Numero di ripetizioni della fase iniziale di halt. */
	unsigned long halt_cycle;
/* Semaforo di sincronizzazione del supervisore per le operazioni
 che richiedono piu` cicli. */
	csem_t sync_superv_sem;
/* Funzione di attesa dello scatto del timer. */
	void (*sleep)(void);
/* Segnale del timer principale. */
	sigset_t timer_signal;
/* File descriptor della inax (eventualmente) utilizzata come timer. */
	int inax_fd;
/* Timer POSIX (eventualmente) utilizzato. */
	timer_t timer;
/* Numero di clock tick dell'INAX necessari per fare un ciclo di Plc. */
	int n_inax_ticks;
/* Contatore di clock tick dell'INAX. */
	int curr_inax_tick;
/* Flag di segnle INAX slave ricevuto. */
	int inax_sig_flag;
/* Flag indicante che il segnale INAX non e` stato ricevuto. */
	int inax_dead;
/* Contatore e limite per il rinfresco dell'NVRAM su file. */
	long nvram_refresh_count;
	long nvram_refresh_limit;
} exec_status_t;

extern exec_status_t exec_status;

#define CMDPRIO_COMMANDS 2
#define CMDPRIO_ERRORS 4

/*
* Funzione "exec_startmsg"
* ------------------------
*
*  Questa funzione permette di iniziare l'esplorazione delle code
* di trasmissione descritte dal parametro "p". Le successive chiamate
* della funzione "exec_getmsg" forniranno i messaggi ordinati per priorita`
* e per data di invio.
*/

void exec_startmsg(void * p);

/*
* Funzione "exec_getmsg"
* ----------------------
*
*  Questa funzione fornisce il prossimo messaggio disponibile nella coda
* di trasmissione, oppure NULL se i messaggi sono finiti. Prima di
* utilizzare questa funzione, si deve chiamare la "exec_startmsg", indicando
* nel parametro il puntatore all'array di code di messaggi.
*/

sv_msg_t * exec_getmsg(void);

/*
* Funzione "exec_main"
* --------------------
*
*  Questa funzione contiene il cuore del PLC. Deve essere chiamata dal
* thread principale del processo, perche` utilizza il segnale del timer
* per la temporizzazione.
*  Da questa funzione non si esce piu`.
*/

void exec_main(void);

/*
* FUNZIONI DI MANIPOLAZIONE DELLO STATO DELL'ESECUTORE
* ----------------------------------------------------
*/

/* LAVORI IN CORSO */

/*
* Funzione "plcIsHalt"
* --------------------
*
*  Questa funzione vale 1 se il Plc e` nello stato di HALT ordinario
* e stabilizzato. Ad esclusivo uso interno.
*/

int plcIsHalt(void);

/*
* Funzione "plcStart"
* -------------------
*
*  Questa funzione mette in START l'esecutore. E` chiamata dall'esecutore
* alla ricezione di una richiesta di start dal supervisore.
*/

void plcStart(void);

/*
* Funzione "plcHalt"
* ------------------
*
*  Questa funzione mette in HALT l'esecutore. Deve essere chiamata
* dal thread esecutore. Puo` essere utilizzata all'interno di un
* programma PLC per forzare l'HALT.
*/

void plcHalt(void);

/*
* Funzione "plcGo"
* -------------------
*
*  Questa funzione mette in START l'esecutore. E` chiamata dall'esecutore
* per iniziare la procedura di start, che coinvolgera` anche il supervisore.
*/

void plcGo(void);

/*
* Funzione "plcFault"
* -------------------
*
*  Questa funzione mette in FAULT l'esecutore. Deve essere chiamata
* dal thread esecutore. Puo` essere utilizzata all'interno di un
* programma PLC per forzareil FAULT. L'effetto dell'entrata in FAULT
* puo` essere irreversibile. 
*/

void plcFault(void);

/*
* Funzione "plcError"
* -------------------
*
*  Questa funzione visualizza un errore di codice "code" e test "text".
*/

void plcError(long code, char * format, ...);

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

void plcRuntimeError(long code, unsigned long pc);

/*
* Funzione "plcDelerr"
* ---------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*/

void plcDelerr(long code);

/*
* Funzione "plcWarning"
* ---------------------
* 
*  Questa funzione visualizza un "warning" su stderr.
*/

void plcWarning(long code, char * format, ...);

/*
* Funzione "plcMessage"
* ---------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text".
*/

void plcMessage(long code, char * format, ...);

/*
* Funzione "plcUpdateMessage"
* ---------------------------
*
*  Questa funzione visualizza un messaggio di codice "code" e testo "text",
* sovrascrivendo l'eventuale precedente messaggio con lo stesso codice.
*/

void plcUpdateMessage(long code, char * format, ...);

/*
* Funzione "plcDelmess"
* ---------------------
*
*  Cancella un errore (ammesso che la cosa si possa fare) di codice "code".
*/

void plcDelmess(long code);

/*
* Funzione "plcStartMeasurement"
* ------------------------------
*
*  Questa funzione e` chiamata dall'esecutore per impostare la misura
* del tempo di ciclo.
*/

void plcStartMeasurement(void);

/*
* Funzione "plcStopMeasurement"
* -----------------------------
*
*  Questa funzione e` chiamata dall'esecutore per interrompere la misura
* del tempo di ciclo.
*/

void plcStopMeasurement(void);

/*
* Funzione "plcClearMeasurement"
* ------------------------------
*
*  Questa funzione azzera la statistica del tempo di ciclo.
*/

void plcClearMeasurement(void);

/*
* Funzione "plcLoadModule"
* ------------------------
*
*  L'esecutore chiama questa funzione per richiedere il caricamento
* dinamico del file "path".
*  Vale 0 se l'operazione e` ammissibile, -1 altrimenti.
*/

int plcLoadModule(char * path);

/*
* Funzione "plcUnloadModule"
* --------------------------
*
*  L'esecutore chiama questa funzione per richiedere lo scaricamento
* dinamico dell'ultimo modulo caricato.
*  Vale 0 se l'operazione e` ammissibile, -1 altrimenti.
*/

int plcUnloadModule(void);

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

int plcGiveOldComm(void);

/*
* Funzione "plcCatchOldComm"
* --------------------------
*
*  Con questa funzione il supervisore restituisce all'esecutore il
* controllo del colloquio "vecchio stile".
*  La funzione puo` comunque essere chiamata anche dall'esecutore.
*/

void plcCatchOldComm(void);


/*
* Funzione "plcPeriod"
* --------------------
*
*  Questa funzione restituisce il periodo del ciclo di esecuzione
* in millisecondi.
*/

long plcPeriod(void);

/*
* Funzione "plcTime"
* ------------------
*
*  Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi.
*/

unsigned long plcTime(void);

/*
* Funzione "plcTimeH"
* -------------------
*
*  Questa funzione restituisce il tempo trascorso dal lancio dell'applicazione
* in millisecondi (32 bit piu` significativi).
*/

unsigned long plcTimeH(void);

/*
* Funzione "plcNCycle"
* --------------------
*
*  Questa funzione restituisce il numero di ciclo corrente.
*/

unsigned long plcNCycle(void);

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

unsigned long plcHaltCycle(void);

/*
* Funzione "plcRepeatHalt"
* ------------------------
*
*  Questa funzione inoltra la richiesta di riesecuzione delle procedure
* di HALT da parte dell'esecutore al ciclo successivo. E` utile quando,
* in una procedura di HALT, si debbano compiere azioni che richiedano piu`
* di un ciclo di PLC per essere eseguite.
*/

void plcRepeatHalt(void);

/*
* Funzione "plcEventNotify"
* -------------------------
*
*  L'esecutore chiama questa funzione per richiedere la segnalazione
* di un evento di interesse per i debugger. "code" e` il codice che
* individuera` l'evento. "data" e "len" sono indirizzo e dimensione
* (eventualmente 0) di in insieme di dati associati all'evento.
*/

void plcEventNotify(long code,char *data,int len);

/*
* Funzione "plcSaveAx"
* ---------------------
*
*  Salva i dati dell'asse indicato (numero interno).
*/

void plcSaveAx(long ax);

/*
* Funzione "plcSaveGenericTable"
* ------------------------------
*
*  Salva i dati della tabella utente indicata.
*/

void plcSaveGenericTable(char *tab);

/*
* Funzione "plcSaveOrig"
* ----------------------
*
*  Salva le origini.
*/

void plcSaveOrig(void);

/*
* Funzione "plcSaveTable"
* -----------------------
*
*  Salva una tabella qualsiasi dato nome e centro.
*/

void plcSaveTable(char *tabname,int cen);

#define _EXEC_H_

#endif

