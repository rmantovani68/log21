/*
* @(#) debugger.c 1.22 Wed Nov 22 10:00:59 MET 2000
*
* Definizioni legate al debugger in tempo reale del Plc.
*
* 04/12/96 GG 1.0 Prima stesura.
* 16/12/96 GG 1.1 Modifichine.
* 17/12/96 GG 1.2 Evitato lo schianto nell'abilitazione di una lista vuota.
* 18/12/96 GG 1.3 Corretta la generazione della chiamata di "brk_hook", il
*             salvataggio di valori nell'area di debug, e altre piccole sfighe.
* 19/12/96 GG 1.4 Corretta la gestione di "condact".
* 07/01/97 GG 1.5 Corrette le azioni "TOGGLE" e "RESET".
* 16/01/97 GG 1.6 Raccolto il codice di salvataggio e ripristino delle
*             istruzioni. Modificata la "brkAdd" in modo che la lista di
*             break sia sempre ordinata per indirizzi crescenti. Aggiunta la
*             gestione di break multipli sullo stesso indirizzo, purche`
*             appartenenti alla stessa lista. Ridotta la probabilita` di
*             schianto evitando l'autoricarica di liste in cui l'ultimo
*             break inizia nel corpo del primo.
* 17/01/97 GG 1.7 Iniziate le correzioni per gestire il caso di sovrapposizione
*             di liste di break. Per ora non si e` fatto altro che eliminare
*             la rimemorizzazione del codice di backup ad ogni inizio ciclo,
*             col che si evita che un break non percorso si "autosalvi".
*             Per il resto, tutto e` ancora in alto mare. Le modifiche da
*             apportare in "brkEnable", "brkDisable", "brkClear", "brkSet"
*             sono descritte in "../doc/TODO" al punto 2.3 .
*             NOTA: QUESTA VERSIONE E` SPERIMENTALE. LE MODIFICHE DESCRITTE
*             POTREBBERO NON ESSERE PUBBLICATE.
* 20/01/97 GG 1.8 Infatti. Si riprende la versione 1.6. Come descritto al
*             punto 2.3.4 di TODO, ci si limita ad ordinare per indirizzi
*             crescenti la lista delle liste (funzione "brkAdd"), aggiungervi
*             un puntatore all'indietro, ed esplolarla a rovescio nella fase
*             di salvataggio codice-impostazione break.
* 24/01/97 GG 1.9 Corretto un buco in "brkCore", che provocava il crash in
*             caso di liste vuote.
* 10/03/97 GG 1.10 Pazzesco ! Incredibile ! Erano scambiate le definizioni
*             di ESI ed EDI !
* 18/06/97 GG 1.11 Sono stati aggiunti nuovi segnali di scambio per
*             riconoscere la ricopertura degli eventi e individuare l'insieme
*             di dati ancora affidabili nel buffer. Sono anche stati aggiunti
*             appositi campi nella struttura di stato per aggiornare in fretta
*             i nuovi segnali.
*             Resa "static" una variabile utilizzata solo qui ("deb_transreg").
* 08/07/97 GG 1.12 Altri due segnali di scambio per l'individiazione rapida
*             del passaggio per una lista di break.
* 17/11/97 GG 1.13 Non si scrive TOOGGLE !
*             E`  stato aggiunto un campo ai descrittori interni di lista
*             per registrare la posizione in cui e` registrato l'insieme di
*             eventi relativi all'ultimo ciclo durante il quale sia stato
*             incontrato ciascun watchpoint.
*             Aggiunta la funzione "brkLastEvent", che fornisce gli indirizzi
*             e le lunghezze dei blocchi che costituiscono l'ultimo evento
*             registrato.
* 02/11/00 GG 1.14 Modificata la logica di allocazione delle liste di
*             break in modo che siano preferibilmente prodotti indici
*             diversi dopo creazioni/distruzioni successive. Questo dovrebbe
*             rendere meno evidenti certi difetti del pacchetto "edit_plc".
* 03/11/00 GG 1.15 Aggiunte le funzioni "brkHistoryMode", "brkHistoryLock",
*             "brkHistoryUnlock" e "brkHistory", ad uso del debugger remoto.
*             Servono a gestire in modo piu` efficiente il buffer di debug, e
*             risolvere il problema del codice percorso raramente anche
*             col debugger remoto.
* 04/11/00 GG 1.16 Eliminato il parameto "cycle" della "brkHistory",
*             perche` e` inutile.
* 05/11/00 GG 1.17 Aggiunto l'azzeramento del campo "hist_nr" dopo la
*             lettura della storia, cosi` la lettura diventa davvero
*             "distruttiva" (precauzione esagerata, ma fa pulizia).
*             Corretta l'inizializzazione del puntatore di lettura per
*             la "brkHistory", che probabilmente non "agganciava" il
*             puntatore esterno.
* 06/11/00 GG 1.18 Aggiunto un flag di "dati presenti" per le primitive
*             locali, perche` si confondevano i casi di "buffer completamente
*             pieno" e "buffer completamente vuoto" (in entrambi i casi
*             si ha "lcl_firstevp == wr_pos").
*             Spostata alla fine dell'analisi dell'ultimo punto della lista
*             di break l'accensione del bit di "evento gia` registrato".
*             Condizionato l'aggiornamento del puntatore di lettura interno
*             alla modalita` di funzionamento 0.
*             Corretto un incredibile errore nella gestione del puntatore
*             alla coda delle liste di break durante la creazione di una
*             nuova lista. Probabilmente gli effetti del bug erano mascherati
*             dal comportamento del debugger.
*             Condizionato alla non-definizione di RECYCLE_WLID il codice
*             che si preoccupa di non riciclare immediatamente gli ID delle
*             liste di break. Sembra che risolva alcuni problemi, ma ne crei
*             altri alle versioni vecchie di "edit_plc".
* 07/11/00 GG 1.19 Aggiunte le funzioni "debErrorLog" e "debErrorList",
*             che servono ad immagazzinare gli ultimi errori prodotti
*             (vedi "plcError"), e a ottenere il buffer degli errori.
*             Queste funzioni usano tre nuovi segnali di scambio per gestire
*             il solito buffer circolare... Si tratta di "$DEB_ERRORS"
*             (e` il buffer degli errori), "$DEB_FIRSTERRP" (primo errore)
*             e "$DEB_ERRLENGTH" (numero di byte significativi nel buffer).
* 08/11/00 GG 1.20 Modificato il comportamento della "brkHistoryLock",
*             che ora, nel modo 1, forza la registrazione degli eventi
*             del ciclo corrente, anche se contengono liste gia` percorse.
*             Forzata la modalita` 0 in "brkReinit", cosi` non ci sono
*             interferenze tra debugger locali e remoti chiamati in
*             tempi diversi.
* 21/11/00 GG 1.21 Aggiunta la gestione del caso particolarmente sfigato
*             di liste con inizio e fine coincidenti. Il caso e` risolto
*             se ci sono dieci NOP consecutivi in corrispondenza del break.
*             Questo permette di aggingere un break fittizio dopo
*             quello ordinario. Questo secondo break servira` a ripristinare
*             il break ordinario. 
*             Corretta un'imperfezione nella gestione della tabella delle
*             liste gia` percorse. Il flag veniva acceso anche se la
*             lista non aveva provocato traffico (per esempio perche`
*             non era verificata la condizione associata, come puo` accadere
*             durante il debug di blocchi funzionali IL).
* 22/11/00 GG 1.22 Aggiunta il ripristino del codice coperto dal break
*             speciale in "brkClear" (chiamata da "brkEnd").
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "qplc.h"
#include "util.h"
#include "debugger.h"

#if 1
#define RECYCLE_WLID
#endif

extern void brk_hook(void);

/* Macro utili per manipolare mappe di bit. */
#define _brk_def_bittab(x,n) char x[(n+7)/8]
#define _brk_clr_bittab(x) memset((x),0,sizeof(x))
#define _brk_set_bit(x,b) ((x)[(b) / 8] |= (1 << ((b) % 8)))
#define _brk_res_bit(x,b) ((x)[(b) / 8] &= ~(1 << ((b) % 8)))
#define _brk_tst_bit(x,b) (((x)[(b) / 8] & (1 << ((b) % 8))) != 0)

/* Struttura di stato del debugger. */

static struct debug_t {
/* Area contenente i descrittori di lista di watch. */
	watch_sts_t wp[MAX_WATCHPOINT];
/* Puntatore alla lista dei descrittori. */
	watch_sts_t *list;
/* Puntatore all'ultimo elemento della lista dei descrittori. */
	watch_sts_t *list_tail;
/* Puntatore alla lista dei descrittori liberi. */
	watch_sts_t *wp_free;
/* Puntatore alla coda della lista dei descrittori liberi. */
	watch_sts_t *wp_free_tail;
/* Area contenente gli elementi di una lista di watch. */
	watch_node_t wn[MAX_WATCHNODE];
/* Puntatore alla lista degli elementi liberi. */
	watch_node_t *wn_free;
/* Registro dei flag di condizione. */
	unsigned long FLAGS;
/* Numero di watchpoint attivi. */
	int n_act;
/* Puntatori alle aree dove depositare gli indirizzi delle funzioni
 che l'esecutore deve chiamare ad ogni inizio e fine ciclo, e valori
 precedenti delle stesse. */
	void (**p_begin_cycle)(void);
	void (*p_backup_begin_cycle)(void);
	void (**p_end_cycle)(void);
	void (*p_backup_end_cycle)(void);
/* Puntatori ai segnali di scambio di debug. */
	unsigned char * sh_buffer;
	unsigned long * sh_events;
	unsigned long * sh_nevt;
	unsigned long * sh_bsize;
	unsigned long * sh_cycle;
	unsigned long * sh_lastev;
	unsigned long * sh_firstevp;
	unsigned long * sh_lastevp;
	unsigned long * sh_evcount;
	unsigned long * sh_nregev;
	unsigned long * sh_maxbrk;
	unsigned long * sh_brkcyc;
	/* Log degli errori. */
	unsigned char * sh_errors;
	unsigned long * sh_firsterrp;
	unsigned long * sh_errlength;
	unsigned long * sh_errbuffsz;
/* Dimensione del buffer degli errori. */
	unsigned int error_buffer_size;
/* Dimensione residua dell'ultimo evento ricoperto da eventi successivi. */
	unsigned long lostev_size;
/* Puntatore al limite superiore del buffer. */
	unsigned char * end_buffer;
/* Posizione di aggiunta di nuovi dati nel buffer. */
	unsigned long wr_pos;
/* Posizione d'inizio dell'ultimo record generato. */
	unsigned long record_pos;
/* Numero di byte di dati aggiunti con l'ultimo evento. */
	unsigned long n_wr;
/* Posizione di lettura degli eventi con le primitive interne
 (brkHistoryLock, brkHistoryUnlock, brkHistory), ad uso del debugger remoto. */
	unsigned long lcl_firstevp;
/* Flag di dati presenti per le primitive interne. */
	unsigned int lcl_evpresent;
/* Modalita` di riempimento del buffer. */
	int history_mode;
/* Flag di buffer bloccato. */
	int history_lock;
/* Funzione da eseguire alla fine di "brkEnd", ad uso
 della "brkHistoryLock". */
	void (*end_cb)(void);
/* Mappa degli eventi gia` registrati. */
	_brk_def_bittab(got_event_map,MAX_WATCHPOINT);
/* Puntatori alla parte di buffer "bloccato" da "brkHistoryLock". */
	unsigned char *hist_b1,*hist_b2;
	unsigned long hist_n1, hist_n2;
/* Validita` dei due blocchi (0: nessuno, 1: hist_*1, 2: entrambi. */
	int hist_nr;
/* Breakpoint pendente. Serve a gestire il caso sfigatissimo di liste
 di break con inizio e fine coincidenti. Vale normalmente NULL,
 altrimenti punta al descrittore di watch la cui gestione non e` stata
 ancora completata. */
	watch_node_t *pending_breakpoint;
} debug_status;

/* Tabella di conversione degli indici di registro. */

static char deb_transreg[] = DEB_TRANSREG;

/*
* Funzione "brkReinit"
* --------------------
*
*  Inizializza (a caldo) il sistema dei breakpoint.
*/

/* Funzione che rigenera le liste da zero. */
static void _brkMakeFreeList(void)
{
int i;
watch_sts_t *p,*q;

/* Inizializza la lista dei breakpoint attivi. */
	debug_status.list = (watch_sts_t *) 0;
	debug_status.list_tail = (watch_sts_t *) 0;
/* Inizializza la lista dei breakpoint liberi. */
	p = debug_status.wp_free = &debug_status.wp[0];
	for (i = 0; i < MAX_WATCHPOINT-1; ++i) {
		p -> id = i;
		q = p;
		q -> next = ++p;
	}
	p -> id = i;
	p -> next = (watch_sts_t *) NULL;
/* Ultimo elemento della lista dei break liberi. */
	debug_status.wp_free_tail = p;
}

/* Funzione per liberare un breakpoint e aggiungerlo in coda
 alla lista degli elementi liberi. */
static void _brkFree(watch_sts_t *p)
{
	p -> next = NULL;
	if (debug_status.wp_free_tail)
		debug_status.wp_free_tail -> next = p;
	else
		debug_status.wp_free = p;
	debug_status.wp_free_tail = p;
}

void brkReinit(void)
{
int i;
int id;
int ok;
watch_sts_t *p,*q;
watch_node_t *r,*s;
_brk_def_bittab(free_id,MAX_WATCHPOINT);

/* Disattiva le funzioni di debug. */

	*debug_status.p_begin_cycle = debug_status.p_backup_begin_cycle;
	*debug_status.p_end_cycle = debug_status.p_backup_end_cycle;

#if !defined(RECYCLE_WLID)
/* NOTA: Le azioni compiute in questo ramo di compilazione sembrano
 interferire in qualche modo con certe versione di "edit_plc". In altri
 casi, invece, sembrano risolvere qualche problema. Boh! */
 
/* Rigenera la lista dei breakpoint liberi, cercando di mantenere
 l'ordine preesistente, ma anche di evitare incongruenze
 (esempio: niente elementi ripetuti). */
/* NOTA: In un mondo ideale, tutto cio` sarebbe inutile: la lista dei
 nodi liberi dovrebbe sempre essere a posto, gli elementi dovrebbero
 essere sempre integri... Ma, come dice il poeta, "e` un mondo difficile": 
 puo` darsi che un debugger locale particolarmente malfatto sia in grado
 di provocare il caos in queste strutture, percio` conviene prepararsi
 al peggio. D'altra parte, e` utile mantenere l'ordine in cui sono stati
 liberati gli elementi, in modo che vengano di preferenza allocati
 i break che sono rimasti inutilizzati da piu` tempo (fa comodo al
 programma edit_plc - a me, non me ne potrebbe fregare di meno). */
	/* Vuota le liste dei breakpoint occupati. */
	debug_status.list = (watch_sts_t *) 0;
	debug_status.list_tail = (watch_sts_t *) 0;
	/* Genera una mappa di bit con gli elementi attualmente presenti
	 nella lista. */
	_brk_clr_bittab(free_id);
	ok = 1;
	for (q = NULL, p = debug_status.wp_free; p; q = p, p = p -> next) {
		id = p - debug_status.wp;
		if (id >= MAX_WATCHPOINT || id < 0 /* Puntatore sbagliato! */
		 || p -> id != id /* Elemento "sporco"! */
		 || _brk_tst_bit(free_id,id) /* Elemento ripetuto! */
		   ) {
		/* Non sarebbe mai dovuto accadere, ma e` accaduto:
		 la lista e` alterata (contiene dati inconsistenti, o qualche
		 nodo e` stato sporcato). Conviene uscire dal ciclo e
		 rigenerarla da zero. */
			ok = 0;
			break;
		}
		_brk_set_bit(free_id,id);
	}
	if (ok) {
	/* q = ultimo elemento della lista (o NULL). */
		debug_status.wp_free_tail = q;
	/* Aggiunge in coda alla lista tutti gli elementi che
	 mancavano. */
		for (i = 0; i < MAX_WATCHPOINT; ++i) {
			p = &debug_status.wp[i];
			p -> id = i; /* Rigenera l'ID. */
			if (! _brk_tst_bit(free_id,i)) {
				_brkFree(p);
			}
		}
	}
	else {
	/* E` successo qualcosa di grave. Conviene limitare i danni
	 rigenerando la lista da zero, e sperando che non ci siano
	 altre "bombe a tempo" in giro... */
		_brkMakeFreeList();
	}
#else /* if defined(RECYCLE_WLID) */
/* Rigenera incondizionatamente la lista degli elementi liberi. */
	_brkMakeFreeList();
#endif /* !defined(RECYCLE_WLID) */


/* Inizializza le liste dei punti di break liberi. */
	r = debug_status.wn_free = &debug_status.wn[0];
	for (i = 0; i < MAX_WATCHNODE-1; ++i) {
		s = r;
		s -> next = ++r;
	}
	r -> next = (watch_node_t *) 0;
	debug_status.FLAGS = 0;
	debug_status.n_act = 0;
	debug_status.history_mode = 0;
	debug_status.history_lock = 0;
	debug_status.end_cb = NULL;
	debug_status.hist_nr = 0;
	_brk_clr_bittab(debug_status.got_event_map);
	debug_status.lcl_evpresent = 0;
	debug_status.lcl_firstevp = debug_status.wr_pos;
	debug_status.pending_breakpoint = NULL;
}

/*
* Funzione "brkInit"
* ------------------
*
*  Inizializza il sistema dei breakpoint. Oltre alle solite cose,
* registra gli indirizzi dati, e li considera
* come destinazione di puntatori a funzioni che l'esecutore chiama
* ad inizio e fine ciclo. Quando si abilita almeno un watchpoint,
* le variabili puntate sono aggiornate, ed il Plc eseguira` le funzioni
* di ipostazione e cancellazione dei breakpoint nel codice.
*  Quando nessun watchpoint e` piu` attivo, le variabili sono riportate
* al valore originale, che sara` tipicamente l'indirizzo di una funzione
* vuota.
*/

static shv_t debug_shvar[] = {
/* Questi devono stare qui. */
{ DEBSHV_BUFFER, 1, 1, EVENT_BUFFER_SIZE, SHV_MODE_TYPE_EXTENDED, -1, 0, 0, },
{ DEBSHV_EVENTS, MAX_LOGGED_EVENTS, 1, 4,
  SHV_MODE_TYPE_LONG|SHV_MODE_CONTIG, -1, 0, 0, },
{ DEBSHV_BRKCYC, MAX_WATCHPOINT, 1, 4,
  SHV_MODE_TYPE_LONG|SHV_MODE_CONTIG, -1, 0, 0, },
{ DEBSHV_NEVT,   1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_BSIZE,  1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_CYCLE,  1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_LASTEV, 1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_FIRSTEVP,1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_LASTEVP,1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_EVCOUNT,1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_NREGEV, 1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_MAXBRK, 1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
/* Capito ? */
};
static unsigned long ** deb_tg[] = {
(unsigned long **)&debug_status.sh_buffer,
&debug_status.sh_events,
&debug_status.sh_brkcyc,
&debug_status.sh_nevt,
&debug_status.sh_bsize,
&debug_status.sh_cycle,
&debug_status.sh_lastev,
&debug_status.sh_firstevp,
&debug_status.sh_lastevp,
&debug_status.sh_evcount,
&debug_status.sh_nregev,
&debug_status.sh_maxbrk,
};

static void _brkCreateShvar(shv_t sh_list[],unsigned long **var_list[], int nv)
{
int i,chg,k;
shv_t *sh;

	for (i = 0, sh = &sh_list[0]; i < nv; ++sh,++i) {
		k = shvCreate(sh,&chg);
		if (k == -1) {
		/* ERRORE. E` successo qualcosa di grave. */
			plcError(105,sh -> name);
			return;
		}
		else if (chg) {
		/* ERRORE. L'oggetto non ha esattamente le
		 caratteristiche richieste. */
			plcError(104,sh -> name);
			return;
		}
		*var_list[i] = (unsigned long *)shvKeyToAddr(k, 0, 0);
	}
}

void brkInit(void(**p_begin_cycle)(void), void(**p_end_cycle)(void))
{
int i;

/* Registra gli indirizzi dove copiare le funzioni di debug. */

	debug_status.p_begin_cycle = p_begin_cycle;
	debug_status.p_backup_begin_cycle = *p_begin_cycle;
	debug_status.p_end_cycle = p_end_cycle;
	debug_status.p_backup_end_cycle = *p_end_cycle;

/* Crea i segnali di scambio appositi. */

	debug_shvar[0].size = main_status.debug_buffer_size;
	debug_shvar[1].dim1 = main_status.n_debug_events;
	debug_shvar[2].dim1 = MAX_WATCHPOINT;

	_brkCreateShvar(debug_shvar,deb_tg,sizeof(deb_tg)/sizeof(deb_tg[0]));

	*debug_status.sh_bsize  = main_status.debug_buffer_size;
	*debug_status.sh_nevt   = main_status.n_debug_events;   
	*debug_status.sh_cycle = 0;
	*debug_status.sh_lastev = (unsigned long)-1;
	*debug_status.sh_firstevp = (unsigned long)-1;
	*debug_status.sh_lastevp = (unsigned long)-1;
	*debug_status.sh_evcount = (unsigned long)0;
	*debug_status.sh_nregev = (unsigned long)0;
	debug_status.lostev_size = *debug_status.sh_bsize;
	debug_status.end_buffer = debug_status.sh_buffer
	                          + main_status.debug_buffer_size;
	*debug_status.sh_maxbrk = MAX_WATCHPOINT;
	for (i = 0; i < main_status.n_debug_events; ++i)
		debug_status.sh_events[i] = (unsigned long)-1;
	for (i = 0; i < MAX_WATCHPOINT; ++i)
		debug_status.sh_brkcyc[i] = (unsigned long)-1;

	debug_status.lcl_evpresent = 0;
	debug_status.lcl_firstevp
	 = debug_status.wr_pos
	 = debug_status.record_pos
	 = debug_status.n_wr
	 = 0;

/* Inizializza le lista dei breakpoint liberi e attivi. */
	_brkMakeFreeList();

/* Porta in uno stato consistente il tutto. */
	brkReinit();
}

/*
* Funzione "brkCreate"
* --------------------
*
* Definisce un breakpoint (alias "lista di watch").
* Restituisce l'ID della lista, o -1 in caso di errori.
*/

int brkCreate(watch_descr_t * wd)
{
watch_sts_t *p;
int id;

	p = debug_status.wp_free;
	if (! p)
		return -1;
/* Flag di abilitazione. */
	p -> flags = WS_USED;
/* Descrizione fornita dall'esterno. */
	p -> descr = *wd;
/* Puntatore al primo elemento della catena dei watchpoint del gruppo. */
	p -> chain = (watch_node_t *)0;
	p -> chain_tail = (watch_node_t *)0;
/* Contatori di passaggio : */
	p -> ctot = 0;	/* dalla nascita del watchpoint. */
	p -> ccurr = 0;	/* dall'inizio del ciclo. */
/* Ultimo ciclo nel quale il programma e` passato dal watchpoint. */
	p -> cycle = 0;
/* Puntatore all'elemento corrente della catena. */
	p -> curr = (watch_node_t *)0;
/* Rimozione dell'elemento dalla lista dei nodi liberi. */
	debug_status.wp_free = p -> next;
	if (! debug_status.wp_free)
		debug_status.wp_free_tail = (watch_sts_t *)NULL;
/* Prossimo elemento della lista dei breakpoint. */
	p -> next = debug_status.list;
	debug_status.list = p;
/* Elemento precedente nella lista. */
	p -> pred = (watch_sts_t *) 0;
	if (! debug_status.list_tail)
		debug_status.list_tail = p;

	id = p - debug_status.wp;
/* Un truccaccio per rendere evidente che il ciclo corrispondente al break
 non e` significativo. */
	debug_status.sh_brkcyc[id] = (unsigned long)(-(long)plcNCycle());
/* Azzeramento del flag di "evento avvenuto". */
	_brk_res_bit(debug_status.got_event_map,id);
	return id;
}

static int _brkCheck(int wlid)
{
	return (wlid >= 0 && wlid <MAX_WATCHPOINT
	        && (debug_status.wp[wlid].flags & WS_USED) );
}

/*
* Funzione "brkEnable"
* --------------------
*
* Abilita un breakpoint (alias "lista di watch").
* Vale 0 se tutto e` andato bene, o -1 in caso di errori.
*/

int brkEnable(int wlid)
{
watch_sts_t *p;

/* I soliti controlli... */

	if (!_brkCheck(wlid) || !(p = &debug_status.wp[wlid]) -> chain)
		return -1;
	if (!(p -> flags & WS_ENABLED)) {
		p -> flags |= WS_ENABLED;
		if (++debug_status.n_act == 1) {
		/* Attiva le funzioni di debug. */
			*debug_status.p_begin_cycle = brkBegin;
			*debug_status.p_end_cycle = brkEnd;
		}
	}
	return 0;
}

/*
* Funzione "brkDisable"
* ---------------------
*
* Disabilita un breakpoint (alias "lista di watch").
* Vale 0 se tutto e` andato bene, o -1 in caso di errori.
*/

int brkDisable(int wlid)
{
	if (!_brkCheck(wlid))
		return -1;
	if (debug_status.wp[wlid].flags & WS_ENABLED) {
		debug_status.wp[wlid].flags &= ~WS_ENABLED;
		if (--debug_status.n_act == 0) {
		/* Disattiva le funzioni di debug. */
			*debug_status.p_begin_cycle
			 = debug_status.p_backup_begin_cycle;
			*debug_status.p_end_cycle
			 = debug_status.p_backup_end_cycle;
		}
	}
	return 0;
}

static void _brkBackupCode(watch_node_t *q)
{
register unsigned char *p;

	p = q -> prop.addr;
	q -> backup_code[0] = p[0];
	*(long *)(&q -> backup_code[1]) = *(long *)(&p[1]);
}

/* Questa funzione vale 1 se gli indirizzi dei due break sono in conflitto,
 cioe` se il primo inizia sul corpo del secondo. */

static int _brkConflict(watch_node_t *p, watch_node_t *q)
{
	return
	  ((unsigned long)(p -> prop.addr)
	   >= (unsigned long)(q -> prop.addr))
	  &&
	  ((unsigned long)(p -> prop.addr) - (unsigned long)(q -> prop.addr)
	   < WATCHPOINT_LENGTH);
}

/* Questa funzione vale 1 se gli indirizzi dei due break coincidono, e
 lo spazio di memoria puo` ospitare due break (10 NOP consecutivi). */

static int _brkSpecialCase(watch_node_t *p, watch_node_t *q)
{
register unsigned char *m = p -> prop.addr;

	return
	  (m == q -> prop.addr)
	  &&
#if 0
/* Modo pulito e lento */
	  (m[0] == 0x90)
	  &&
	  (m[1] == 0x90)
	  &&
	  (m[2] == 0x90)
	  &&
	  (m[3] == 0x90)
	  &&
	  (m[4] == 0x90)
	  &&
	  (m[5] == 0x90)
	  &&
	  (m[6] == 0x90)
	  &&
	  (m[7] == 0x90)
	  &&
	  (m[8] == 0x90)
	  &&
	  (m[9] == 0x90)
#else
/* Modo sporco e veloce */
	  (*(unsigned long *)(m) == 0x90909090)
	  &&
	  (*(unsigned long *)(m+4) == 0x90909090)
	  &&
	  (*(unsigned short *)(m+8) == 0x9090)
#endif
	;
}

static void _brkResumeCode(watch_node_t *q)
{
register unsigned char *p;

	p = q -> prop.addr;
	p[0] = q -> backup_code[0];
	*(long *)(&p[1]) = *(long *)(&q -> backup_code[1]);
}

static void _brkResumeCodeSpecial(watch_node_t *q)
{
register unsigned char *p;

	p = q -> prop.addr + WATCHPOINT_LENGTH;
	p[0] = 0x90;
	*(unsigned long *)(&p[1]) = 0x90909090;
}


static void _brkInsertBreak_core(register unsigned char *p)
{

	p[0] = 0xE8; /* CALL <relative> */
	*(long *)(&p[1]) = (long)brk_hook - (long)p - WATCHPOINT_LENGTH;
}

static void _brkInsertBreak(register watch_node_t *q)
{
	_brkInsertBreak_core(q -> prop.addr);
}

static void _brkInsertSpecialBreak(watch_node_t *q)
{
	_brkInsertBreak_core(q -> prop.addr + WATCHPOINT_LENGTH);
}


static void _brkClear(watch_sts_t *p)
{
watch_node_t *q;

	for (q = p -> curr = p -> chain; q; q = q -> next) {
		_brkResumeCode(q);
	}
}

static void _brkSetB(watch_node_t *q)
{
	_brkBackupCode(q);
	_brkInsertBreak(q);
}

static void _brkSetSpecialB(watch_node_t *q)
{
/* Registra il breakpoint nell'apposito campo della struttura di stato,
 e genera una chiamata "anomala" al gestore dei break. */
	debug_status.pending_breakpoint = q;
	_brkInsertSpecialBreak(q);
}


static void _brkSet(watch_sts_t *p)
{
	if (p -> flags & WS_ENABLED) {
		_brkSetB( (p -> curr = p -> chain) );
	}
}

/*
* Funzione "brkDestroy"
* ---------------------
*
* Distrugge un breakpoint (alias "lista di watch").
* Vale 0 se tutto e` andato bene, o -1 in caso di errori.
*/

int brkDestroy(int wlid)
{
watch_sts_t *p;

	if (brkDisable(wlid) < 0)
		return -1;
	p = &debug_status.wp[wlid];
	p -> flags = 0;
	_brkClear(p);
	if (p -> chain_tail) {
		p -> chain_tail -> next = debug_status.wn_free;
		debug_status.wn_free = p -> chain;
	}

	if (p -> pred)
		p -> pred -> next = p -> next;
	else
		debug_status.list = p -> next;
	if (p -> next)
		p -> next -> pred = p -> pred;
	else
		debug_status.list_tail = p -> pred;

	_brk_res_bit(debug_status.got_event_map,wlid);

	_brkFree(p);

	return 0;
}

/*
* Funzione "brkAdd"
* -----------------
*
* Aggiunge un watchpoint alla "lista di watch" data.
* Restituisce -1 in caso di errori, altrimenti da` la posizione del
* nuovo elemento aggiunto.
*/

int brkAdd(int wlid, watch_t * w)
{
watch_sts_t *p,*pr,*pp;
watch_node_t *q,*r,*t;
int n;

/* Controlli vari (ed inizializzazione di "p"). */

	if (!_brkCheck(wlid))
		return -1;

	if ( (p = &debug_status.wp[wlid]) -> flags & WS_ENABLED)
		return -1;

/* Alloca un nuovo descrittore di break. */

	if (!(q = debug_status.wn_free))
		return -1;
	debug_status.wn_free = q -> next;
	q -> next = (watch_node_t *)0;

	if (p -> chain) {

	/* Se ci sono gia` elementi nella lista, cerca la posizione del
	 nuovo break, per rispettare l'ordine crescente degli indirizzi.
	  Se due break capitano allo stesso indirizzo, sono posti in
	 ordine di definizione. */

	/* NOTA : Dato che la sequenza di inserimento piu` probabile e`
	 per indirizzi creascenti, sarebbe piu` giusto cominciare
	 l'esplorazione dalla fine. Pero` ci vorrebbe un puntatore
	 all'indietro... Buoni propositi per il futuro. */

		for (n = 0, t = (watch_node_t *)0, r = p -> chain;
		     r;
		     ++n, t = r, r = r -> next) {
			if ((unsigned long)(r -> prop.addr)
			  > (unsigned long)(w -> addr)) {
				break;
			}
		}
		if (! r) {

		/* Aggiunta in coda alla lista. */

			p -> chain_tail -> next = q;
			p -> chain_tail = q;
		}
		else if (t) {

		/* Aggiunta tra l'elemento "t" e l'elemento "r". */

			q -> next = r;
			t -> next = q;
		}
		else {

		/* Aggiunta in testa. */

			q -> next = r;
			p -> curr = p -> chain = q;
		}
	}
	else {

	/* Se la lista di watch era vuota, inizializza i puntatori
	 al primo elemento, all'ultimo, ed a quello corrente. */

		p -> curr = p -> chain = q;
		p -> chain_tail = q;
		n = 0;
	}

/* Registra le opzioni nel descrittore. */

	q -> prop = *w;

/* Registra il codice ricoperto dal break. */

	_brkBackupCode(q);

/* Riordina la lista delle liste per indirizzi decrescenti. */

	/* Trova l'elemento, che va probabilmente spostato. */
	for (pr = (watch_sts_t *) 0, pp = debug_status.list;
	     pp;
	     pr = pp,  pp = pp -> next) {
		if (pp == p)
			break;
	}
	/* Rimuove temporaneamente l'elemento. */
	if (pr)
		pr -> next = p -> next;
	else
		debug_status.list = p -> next;
	if (p -> next)
		p -> next -> pred = pr;
	else
		debug_status.list_tail = pr;
	/* Cerca la posizione dell'elemento in base all'indirizzo
	 del primo break. */
	for (pr = (watch_sts_t *) 0, pp = debug_status.list;
	     pp;
	     pr = pp,  pp = pp -> next) {
		if (pp -> chain
		 && (unsigned long)(pp -> chain -> prop.addr)
		    > (unsigned long)(p -> chain -> prop.addr))
			break;
	}
	/* Risistema l'elemento. */
	p -> next = pp;
	if (pp)
		pp -> pred = p;
	else
		debug_status.list_tail = p;
	p -> pred = pr;
	if (pr)
		pr -> next = p;
	else
		debug_status.list = p;

/* Fine. Restituisce la posizione del break aggiunto. */

	return n;
}

/*
* Funzione "brkClear"
* -------------------
*
* Ripristina il codice originario in tutti i punti di break.
*/

void brkClear(void)
{
watch_sts_t *p;

	if (debug_status.pending_breakpoint) {
	/* Rigenera i NOP del caso sfigato, se necessario (questo
	 dovrebbe succedere solo in casi particolarissimi, come
	 HALT forzato da ciclo troppo lungo proprio durante l'elaborazione
	 di una lista di un solo elemento. Abbastanza improbabile, direi). */
		_brkResumeCodeSpecial(debug_status.pending_breakpoint);
		debug_status.pending_breakpoint = NULL;
	}
	/* Rigenera il codice dei breakpoint ordinari. */
	for (p = debug_status.list; p; p = p -> next) {
		_brkClear(p);
	}
}

/*
* Funzione "brkSet"
* -----------------
*
* Inserisce il primo punto di break  di ogni lista nell'indirizzo
* che gli compete.
*/

void brkSet(void)
{
watch_sts_t *p;

	for (p = debug_status.list_tail; p; p = p -> pred) {
		_brkSet(p);
	}
}

/*
* Funzione "brkBegin"
* ------------------
*
*  Questa funzione e` chiamata ad ogni inizio ciclo se c'e` almeno un
* breakpoint attivo.
*/

void brkBegin(void)
{
	*debug_status.sh_cycle = plcNCycle();
	brkSet();
}

/* Funzioncina per trasferire un certo numero di byte dal buffer circolare. */

static void _brkCopyFromBuffer(register unsigned char *d,
                               unsigned long pos,
                               register int n)
{
register unsigned char *s;

	s = debug_status.sh_buffer + pos;
	do {
		if (s >= debug_status.end_buffer) {
			 /* Buffer circolare... */
			s = debug_status.sh_buffer;
		}
		*(d++) = *(s++);
	} while (--n);
}

/* Funzioncine per aggiungere byte ai dati registrati. */

static void _brkWriteVal(unsigned char *s, unsigned char *d, int size)
{
/* Scrive i "size" byte. Poco o nulla da dire. */
	while (size--) {
		if (d >= debug_status.end_buffer) /* Buffer circolare... */
			d = debug_status.sh_buffer;
		*(d++) = *(s++);
	}
}

static void _brkLogBytes(unsigned char *src, int size)
{
int sz;
unsigned long l;

/* Prima di scrivere, controlla che non sia stato raggiunto
 un vecchio evento, che dovrebbe corrispondere a quello indicato
 da $DEB_FIRSTEVP. Se si sta per sovrascrivere quest'evento, bisogna
 aggiornare $DEB_FIRSTEVP, saltando tutti gli eventi che saranno ricoperti.
 E in fretta, anche ! */

	for (sz = size; debug_status.lostev_size < sz; ) {

		sz -= debug_status.lostev_size;

	/* Se siamo entrati qui, c'e` una ricopertura. Bisogna procurarsi
	 la dimensione del nuovo evento che sta per essere ricoperto.
	 L'evento e`, per definizione, quello indicato da $DEB_FIRSTEVP. */
		
		if (*debug_status.sh_nregev == 0
		 || *debug_status.sh_firstevp == (unsigned long) -1) {

		/* Problema: siamo arrivati in fondo all'unico evento
		 registrato. L'unica cosa saggia da fare e` non registrare
		 nient'altro. */

			return;
		}

	/* Aggiorna il contatore di eventi interamente registrati. */

		--(*debug_status.sh_nregev);

	/* Copia dal buffer la dimensione dell'evento che sta per essere
	 ricoperto, ($DEB_FIRSTEVP). */

		_brkCopyFromBuffer((unsigned char *)&debug_status.lostev_size,
		                   *debug_status.sh_firstevp,
		                   sizeof(debug_status.lostev_size));

	/* Un patetico controllo di congruenza. */

		if (debug_status.lostev_size < 2*sizeof(unsigned long)
		 || debug_status.lostev_size > *debug_status.sh_bsize) {

		/* Schifezze. E` successo qualcosa di brutto. Tanto vale
		 sovrascrivere i campi scazzati. */

			debug_status.lostev_size = 2*sizeof(unsigned long);
		}

	/* Porta avanti "$DEB_FIRSTEVP", e anche il puntatore di
	 lettura per le primitive interne, se gli siamo arrivati addosso
	 e siamo in modo 0 (normalmente riflette il valore del segnale di
	 scambio, ma evolve per conto suo se si entra nel modo 1). */

		l = (*debug_status.sh_firstevp + debug_status.lostev_size)
		  % main_status.debug_buffer_size;
		if ((debug_status.history_mode == 0)
		 && (*debug_status.sh_firstevp == debug_status.lcl_firstevp)) {
			debug_status.lcl_firstevp = l;
		}
		*debug_status.sh_firstevp = l;
	}

	debug_status.lostev_size -= sz;

/* Segnala che ci sono dati presenti per le primitive locali. */

	debug_status.lcl_evpresent = 1;

/* Registra i byte. */

	_brkWriteVal(src,debug_status.sh_buffer + debug_status.wr_pos,size);

/* Aggiorna la dimensione dell'evento e la posizione di scrittura. */

	debug_status.n_wr += size;
	debug_status.wr_pos = (debug_status.wr_pos + size)
	                      % main_status.debug_buffer_size;
}

static void _brkLogVal(unsigned long x, int size)
{
	_brkLogBytes((unsigned char *)&x, size);
}

static void _brkAddShort(unsigned long pos, int x)
{
int v;
unsigned char *d = debug_status.sh_buffer + pos;

	v = *d + (unsigned char)x;
	if (v > 256)
		x += 256;
	*(d++) = (unsigned char) v;
	*((d >= debug_status.end_buffer) ? debug_status.sh_buffer : d)
	 += (unsigned char)(x / 256);
}

static cpu_sts_t *debug_cpu_ptr;

static unsigned long _brkValReg(register int reg)
{
	if (reg)
		return debug_cpu_ptr -> regs[(int)deb_transreg[reg]];
	else
		return 0;
}

static void _brkLogReg(register int reg)
{
	if (reg)
		_brkLogVal(debug_cpu_ptr -> regs[(int)deb_transreg[reg]],
		           sizeof(unsigned long));
}

/*
* Funzione "brkEnd"
* -----------------
*
*  Questa funzione e` chiamata ad ogni fine ciclo se c'e` almeno un
* breakpoint attivo.
*/

void brkEnd(void)
{
unsigned long t;

	if (debug_status.n_wr) {

/* Se sono stati scaricati dati nel buffer, aggiorna il campo "lunghezza"
 del blocco di eventi. */
		_brkWriteVal((unsigned char *) &debug_status.n_wr,
		             debug_status.sh_buffer + debug_status.record_pos,
		             sizeof(unsigned long));
/* Azzera il numero di byte scaricati, per il prossimo ciclo. */
		debug_status.n_wr = 0;
/* Riserva l'indice del nuovo blocco nella tabella degli eventi. */
		*debug_status.sh_lastev
		 = t
		 = (*debug_status.sh_lastev + 1) % main_status.n_debug_events;
/* Registra la posizione del blocco nella tabella degli eventi. */
		debug_status.sh_events[t]
		 = *debug_status.sh_lastevp
		 = debug_status.record_pos;
/* Se non e` ancora stata definita la posizione del primo evento disponibile,
 registra come tale il nuovo evento (che e` il primo in assoluto,
 evidentemente). Per sicurezza, si aggiorna anche il puntatore di
 lettura per i servizi interni, anche se dovrebbe gia` essere stato
 inizializzato a 0. */
		if (*debug_status.sh_firstevp == (unsigned long) -1)
			debug_status.lcl_firstevp
			 = *debug_status.sh_firstevp
			 = debug_status.record_pos;
/* Porta avanti la posizione di scrittura degli eventi. */
		debug_status.record_pos = debug_status.wr_pos;
/* Incrementa il contatore di cicli durante i quali sono stati registrati
 eventi. */
		++(*debug_status.sh_evcount);
/* Aggiorna il contatore di eventi interamente registrati. */
		++(*debug_status.sh_nregev);
	}

/* Ripulisce il codice dai break. */

	brkClear();

/* Esegue la callback di fine ciclo. */

	if (debug_status.end_cb) {
		(*debug_status.end_cb)();
	}
}

/*
* Funzione "brkCore"
* ------------------
*
* Questa funzione realizza il break vero e proprio.
*/

void brkCore(cpu_sts_t *cpu)
{
watch_sts_t *wp;
watch_node_t *wn,*q;
int condition, actions, apar;
unsigned long opc,tmp,old_n_wr;
int logged;

/* Controlla il caso particolarissimo della procedura di recupero
 di una lista on inizio e fine coincidenti. In questo caso,
 bisogna rigenerare il break pendente e il codice vuoto su cui
 si trovava il break speciale. */

	wn = debug_status.pending_breakpoint;
	if (wn) {
	/* GESTIONE DEL CASO SFIGATO. */
	/* Aggiusta indirizzo di ritorno e codice. Si deve
	 usare una procedura particolare (NOTA: innocente
	 ottimizzazione: e` inutile ad eseguire i NOP, percio`
	 si tornera` ad addr+10). */
		cpu -> pc = wn -> prop.addr + 2*WATCHPOINT_LENGTH;
	/* Rigenera i NOP. */
		_brkResumeCodeSpecial(wn);
	/* Ripristina il breakpoint pendente - finalmente!. */
		_brkSetB(wn);
	/* Azzera il flag ed esce. */
		debug_status.pending_breakpoint = NULL;
		return;
	}

/* So che e` una schifezza, ma mi serve velocita`. */

	debug_cpu_ptr = cpu;

/* Per prima cosa, bisogna "indovinare" in che break siamo. */

	for (wp = debug_status.list; wp; wp = wp -> next) {
		wn = wp -> curr;
		if (wn && cpu -> pc - WATCHPOINT_LENGTH == wn -> prop.addr)
			break;
	}

	if (! wp) {
	/* Che fare ? Questo non era previsto ! */
		plcRuntimeError(104,(unsigned long)(cpu -> pc));
		plcHalt();
		return;
	}

/* Aggiusta indirizzo di ritorno e codice. */

	cpu -> pc = wn -> prop.addr;
	_brkResumeCode(wn);

/* Se il watchpoint e` disabilitato, abbiamo gia` finito. Ma allora,
 cosa ci stiamo facendo qui ? */

	if ( !(wp -> flags & WS_ENABLED))
		return;

/* Ciclo per elaborare TUTTI i break della lista che si trovano allo
 stesso indirizzo. Solo il primo genera la chiamata al gestore del break,
 gli altri sono analizzati nella stessa chiamata, ed il break successivo
 impostato e` il primo ad indirizzo diverso presente nella lista. */

	for (;;) {

/*
* Analisi della condizione di break.
*/

	/* Conviene calcolarlo adesso, una volta per tutte. */
	opc = wp -> descr.opc;

	switch (wp -> descr.cond) {
	case WATCH_C_FALSE: /* Condizione sempre falsa */
		condition = 0;
		break;
	case WATCH_C_TRUE:  /* Condizione sempre vera */
		condition = 1;
		break;
	case WATCH_C_AND:   /* Condizione vera se (FLAGS & opc) == opc */
		condition = (debug_status.FLAGS & opc) == opc;
		break;
	case WATCH_C_OR:    /* Condizione vera se (FLAGS & opc) != 0 */
		condition = (debug_status.FLAGS & opc) != 0;
		break;
	case WATCH_C_NOR:   /* Condizione vera se (FLAGS & opc) == 0 */
		condition = (debug_status.FLAGS & opc) == 0;
		break;
	case WATCH_C_TGT:   /* Condizione vera se ctot > opc */
		condition = (opc > wp -> ctot);
		break;
	case WATCH_C_TLE:   /* Condizione vera se ctot <= opc */
		condition = (opc < wp -> ctot);
		break;
	case WATCH_C_TEQ:   /* Condizione vera se ctot == opc */
		condition = (opc == wp -> ctot);
		break;
	case WATCH_C_CGT:   /* Condizione vera se ccurr > opc */
		condition = (opc > wp -> ccurr);
		break;
	case WATCH_C_CLE:   /* Condizione vera se ccurr <= opc */
		condition = (opc < wp -> ccurr);
		break;
	case WATCH_C_CEQ:   /* Condizione vera se ccurr == opc */
		condition = (opc == wp -> ccurr);
		break;
	case WATCH_C_RZ:    /* Condizione vera se REG(opc) == 0 */
		condition = (cpu -> regs[(int)deb_transreg[opc]] == 0);
		break;
	case WATCH_C_NRZ:   /* Condizione vera se REG(opc) != 0 */
		condition = (cpu -> regs[(int)deb_transreg[opc]] != 0);
		break;
	case WATCH_C_MZ:    /* Condizione vera se MEM(opc) == 0 */
		condition = *(char *)opc == 0;
		break;
	case WATCH_C_NMZ:   /* Condizione vera se MEM(opc) != 0 */
		condition = *(char *)opc != 0;
		break;
	default:
		condition = 0;
		break;
	}

/*
* Analisi delle azioni associate al break.
*/

	actions = wp -> descr.act;
	if (! condition)
		actions &= ~(wp -> descr.condact);

	if (actions & WATCH_A_SET) {  /* FLAGS |= opa1 */
		debug_status.FLAGS |= wp -> descr.opa1;
	}
	if (actions & WATCH_A_RESET) {/* FLAGS &= ~opa2 */
		debug_status.FLAGS &= ~(wp -> descr.opa2);
	}
	if (actions & WATCH_A_TOGGLE) {/* FLAGS ^= opa2 */
		debug_status.FLAGS ^= wp -> descr.opa2;
	}
	if (actions & WATCH_A_INC) {  /* ++COUNT se primo brk della lista. */
		if (wn == wp -> chain) {
			++(wp -> ctot);
			++(wp -> ccurr);
		}
	}
	if (actions & WATCH_A_CYC) {  /* Registra il ciclo corrente di Plc. */
		wp -> cycle = plcNCycle();
	}
/* Gestione evoluta del buffer di debug.
 Un evento deve essere registrato se
  (ha il flag WATCH_A_LOG)
 &&
  ( ( (siamo in modo 0) && (il buffer non e` bloccato) )
   ||
    ( (siamo in modo 1) && (l'evento non e` ancora stato registrato) ) )  */
	logged = 0; /* L'informazione servira` anche piu` avanti. */
	if ((actions & WATCH_A_LOG)
	 && (((debug_status.history_mode == 0)
	   && (!debug_status.history_lock))
	  || ((debug_status.history_mode != 0)
	   && (!_brk_tst_bit(debug_status.got_event_map,wp -> id))))) {

	/* Registra dati nel buffer di debug. */

	/* Ricorda che l'evento e` stato registrato. */

		logged = 1;

	/* Registra il ciclo di passaggio nell'apposito segnale di scambio. */

		debug_status.sh_brkcyc[wp -> id] = plcNCycle();

	/* Se, per questo ciclo, non sono ancora stati registrati eventi,
	 genera un nuovo descrittore di lista di eventi. */

		if (! debug_status.n_wr) {
			debug_status.record_pos = debug_status.wr_pos;
			_brkLogVal(2 * sizeof(unsigned long), sizeof(long));
			_brkLogVal(plcNCycle(),sizeof(long));
		}

	/* Se questo e` il primo break della lista, registra la posizione
	 del nuovo record da generare. Registra anche la posizione
	 dell'intero insieme di eventi in corso di costruzione. Servira`
	 al debugger remoto per acquisire i dati relativi ad un ciclo
	 durante il quale si sia sicuramente verificato un passaggio per
	 questa lista. */

		if (wn == wp -> chain) {
			wp -> event_record = debug_status.record_pos;
			wp -> record = debug_status.wr_pos;
			_brkLogVal(sizeof(unsigned short) + sizeof(char),
			           sizeof(short));
			_brkLogVal((unsigned long)(wp -> id), sizeof(char));
		}

		old_n_wr = debug_status.n_wr;

		if ((actions & WATCH_A_LOGFL)) {  /* Registra anche FLAGS. */
			_brkLogVal(debug_status.FLAGS, sizeof(long));
		}

	/* Interpretazione del descrittore di indirizzo/registro. */

		if (wn -> prop.operand.scale == 0) {

		/* "scale == 0" significa che interessa il valore
		 del registro r1, senza calcoli di indirizzamento. */

			_brkLogReg(wn -> prop.operand.r1);
			_brkLogReg(wn -> prop.operand.r2);
		}
		else {

			tmp = _brkValReg(wn -> prop.operand.r1)
			    + _brkValReg(wn -> prop.operand.r2) 
			      * wn -> prop.operand.scale
			    + wn -> prop.operand.off;

			_brkLogBytes((unsigned char *)tmp,
			             wn -> prop.operand.size);
		}

	/* Registro aggiuntivo, che puo` fare comodo. */

		_brkLogReg(wn -> prop.operand.ar);

	/* Aggiorna la lunghezza del (sotto)record aggiungendole la
	 quantita` di dati accodati. */

		_brkAddShort(wp -> record,
		             (int)(debug_status.n_wr - old_n_wr));
	}
	if (actions & WATCH_A_BRK) {  /* Ripristina il wp "apar". */
		apar = wp -> descr.apar;
		if (brkEnable(apar) == 0)
			_brkSet(&debug_status.wp[apar]);
	}

/* Fine del ciclo di elaborazione dei break ad indirizzo coincidente. */
	/* Passa all'elemento successivo della lista, o si riposiziona
	 all'inizio. */

	q = wn -> next;
	if (q) {

		/* Passaggio all'elemento successivo. Rimane nel ciclo se
		 il nuovo break ha lo stesso indirizzo del corrente. */

		wp -> curr = q;
		if (q -> prop.addr == wn -> prop.addr)
			wn = q;
		else
			break;
	}
	else {

	/* Guinti alla fine della lista di break, bisogna marcare
	 "registrato" l'evento, se siamo in modalita` compressa
	 (lo si potrebbe fare incondizionatamente, ma ho visto che il codice
	 generato dal compilatore e` un po' cervellotico...). */
		if (logged && debug_status.history_mode != 0) {
			_brk_set_bit(debug_status.got_event_map,wp -> id);
		}

		/* L'elemento corrente e` l'ultimo. Si esce dal ciclo. */

		q = wp -> curr = wp -> chain;
		break;
	}

	} /* FINE DEL CICLO */

/* Imposta il successivo break della lista, a meno che il suo indirizzo
 non sia in conflitto con quello corrente (caso sfigato). */

	if (! _brkConflict(wn,q)) {
		_brkSetB(q);
	}
	else if (_brkSpecialCase(wn,q)) {
	/* Tentativo di gestione di un caso particolarmente sfigato:
	 lista di break contenente un solo elemento (o, comunque,
	 lista in cui l'ultimo elemento e` allo stesso indirizzo
	 del primo). Il caso puo` essere risolto se ci sono almeno cinque
	 byte di codice vuoto dopo il break. Questo avviene se il programma
	 e` stato compilato con una versione recente di ILC. In ogni
	 caso la condizione va controllata. Se c'e` spazio (cinque NOP
	 consecutivi), si scrive li` il codice di chiamata della
	 funzione "brk_special_hook", che provvedera` a ripristinare
	 il break e i cinque NOP successivi. */
		_brkSetSpecialB(q);
	}
	else {
	/* Caso disperato: i break si accavallano, e non c'e` posto per
	 il codice di recupero. L'unica cosa saggia da fare e` lasciar
	 stare il codice originario, e pazienza se per questo ciclo
	 di PLC non funzioneranno piu`. */
	}

}

/*
* Funzione "brkLastEvent"
* -----------------------
*
* Questa funzione fornisce gli indirizzi, le lunghezze ed il numero
* dei segmenti costituenti l'ultimo blocco di eventi registrato.
* Riporta anche l'indice di ciclo del blocco di eventi.
*/

int brkLastEvent(unsigned char **b1, unsigned long *n1,
                 unsigned char **b2, unsigned long *n2,
                 unsigned long *cycle)
{
unsigned long lp;
unsigned long head[2];
long diff;
unsigned char *p;

	lp = *debug_status.sh_lastevp;
	if (lp == (unsigned long)-1)
		return 0;

/* Copia dal buffer la dimensione dell'ultimo evento. */

	_brkCopyFromBuffer((unsigned char *)&head, lp, sizeof(head));

	*cycle = head[1];
	p = debug_status.sh_buffer + lp;
	diff = (p + head[0]) - debug_status.end_buffer;
	if (diff <= 0) {
		*b1 = p;
		*n1 = head[0];
		return 1;
	}
	else {
		*b1 = p;
		*n1 = head[0] - (unsigned long)diff;
		*b2 = debug_status.sh_buffer;
		*n2 = (unsigned long)diff;
		return 2;
	}
}

/*
* Funzione "brkHistoryMode"
* -------------------------
*
* Questa funzione attiva la modalita` di debug tradizionale
* (mode == 0) o compressa (mode == 1). Nella modalita` tradizionale,
* ogni evento dei debug e` registrato incondizionatamente nel buffer
* di debug. Nella modalita` compressa, e` registrato un solo evento
* per ogni lista di break, e solo la chiamata della "brkHistoryLock"
* permette di registrare ulteriori eventi.
*/

void brkHistoryMode(int mode)
{
/* Se si passa dalla modalita` normale a quella compressa, bisogna
 buttare via la storia passata, perche` i dati che contiene
 non sono conformi alla modalita` compressa (essenzialmente: sono troppi,
 e contengono ripetizioni). */
	if (debug_status.history_mode == 0 && mode == 1) {
		debug_status.lcl_evpresent = 0;
		debug_status.lcl_firstevp = debug_status.wr_pos;
	/* Azzeramento dei bit di evento gia` incontrato. */
		_brk_clr_bittab(debug_status.got_event_map);
	}
	debug_status.history_mode = mode;
}

/*
* Funzione "BrkHistoryLock"
* -------------------------
*
* questa funzione blocca l'evoluzione dell'history. in modo 0 (modo
* ordinario), il debugger cessa di registrare gli eventi.
* in modo 1, il debugger porta avanti la posizione di lettura, e
* marca "non incontrati" tutti i watchpoint.
*/

/* Questo e` il cuore della funzione, che deve essere eseguito alla
 fine della funzione "brkEnd", non all'inizio del ciclo di esecuzione. */
static void _brkHistoryLock_step2(void)
{
unsigned char *b1,*b2;
unsigned long n1, n2;
long diff;
int nr;

/* Pre-inizializzazione, utili per capirci qualcosa in debug. */
	nr = 0;
	b1 = b2 = NULL;
	n1 = n2 = 0;
	if (debug_status.lcl_evpresent) {
	/* Calcolo della posizione e della lunghezza del
	 blocco contenente la storia. */
		diff = debug_status.wr_pos - debug_status.lcl_firstevp;
		b1 = debug_status.sh_buffer
		   + debug_status.lcl_firstevp;
		nr = 1;
	/* Il solito controllo per capire se i dati si trovano
	 a cavallo della fine del buffer.
	 NOTA: "diff == 0", significa che il buffer
	 e` completamente pieno, non vuoto! */
		if (diff > 0) {
			n1 = diff;
		}
		else {
			n1 = main_status.debug_buffer_size
			   - debug_status.lcl_firstevp;
			if (debug_status.wr_pos > 0) {
				n2 = debug_status.wr_pos;
				b2 = debug_status.sh_buffer;
				nr = 2;
			}
		}
	}
/* Ufficializza il risultato. Sara` letto dalla "brkHistory". */
	debug_status.hist_b1 = b1;
	debug_status.hist_n1 = n1;
	debug_status.hist_b2 = b2;
	debug_status.hist_n2 = n2;
	debug_status.hist_nr = nr;

/* Ora che e` stata registrata la posizione e la lunghezza
 della parte utile del buffer, si puo` spostare il puntatore
 alla base. Nella modalita` 0, il buffer non verra` aggiornato
 finche` non sara` chiamata la "brkHistoryUnlock". Nella
 modalita` 1, invece, gli eventi vengono immediatamente aggiunti,
 anche se il buffer e` bloccato, perche` e` improbabile che si
 verifichino ricoperture, essendo limitato il numero di nuovi
 eventi che si possono verificare. */
	debug_status.lcl_evpresent = 0;
	debug_status.lcl_firstevp = debug_status.wr_pos;
/* Si azzerano i flag di "evento gia` accaduto". */
	_brk_clr_bittab(debug_status.got_event_map);
/* ...ovviamente, questa funzione deve essere chiamata una volta sola! */
	debug_status.end_cb = NULL;
}

void brkHistoryLock(void)
{
	if (! debug_status.history_lock) {
		debug_status.history_lock = 1;
	/* Pre-inizializzazione, utili per capirci qualcosa in debug. */
		debug_status.hist_b1 = NULL;
		debug_status.hist_n1 = 0;
		debug_status.hist_b2 = NULL;
		debug_status.hist_n2 = 0;
		debug_status.hist_nr = 0;
	/* Si azzerano i flag di "evento gia` accaduto". Questo
	 provochera` la registrazione forzata di tutti gli eventi
	 accaduti nel ciclo che sta per iniziare (RICORDA: questa
	 funzione e` eseguita all'inizio del ciclo di PLC, durante
	 l'elaborazione dei comandi da supervisore, prima di brkBegin). */
		_brk_clr_bittab(debug_status.got_event_map);
	/* Ora bisogna impostare l'azione da compiere verso la fine
	 del ciclo, dentro "brkEnd". La callback dovra` "fotografare"
	 i dati utili nei campi "hist_*" della struttura di stato, e
	 spostare il puntatore di lettura locale. */
		debug_status.end_cb = _brkHistoryLock_step2;
	}
}

/*
* Funzione "brkHistoryUnlock"
* ---------------------------
*
* Questa funzione sblocca l'evoluzione dell'history. Ha effeto
* solo in modo 0.
*/

void brkHistoryUnlock(void)
{
	if (debug_status.history_lock) {
		debug_status.history_lock = 0;
		debug_status.end_cb = NULL;
	}
}

/*
* Funzione "brkHistory"
* ---------------------
*
* Questa funzione fornisce gli indirizzi, le lunghezze ed il numero
* dei segmenti costituenti l'intera storia degli eventi registrati.
*/

int brkHistory(unsigned char **b1, unsigned long *n1,
               unsigned char **b2, unsigned long *n2)
{
int nr;

	*b1 = debug_status.hist_b1;
	*n1 = debug_status.hist_n1;
	*b2 = debug_status.hist_b2;
	*n2 = debug_status.hist_n2;
	nr = debug_status.hist_nr;
	debug_status.hist_nr = 0;
	return nr;
}


/*
* Funzione "debErrorInit"
* -----------------------
*
* Questa funzione inizializza il sistema di log degli errori.
*/

static shv_t debug_errlog_shvar[] = {
/* Questi devono stare qui. */
{ DEBSHV_ERRORS, 1, 1, ERROR_BUFFER_SIZE, SHV_MODE_TYPE_EXTENDED, -1, 0, 0, },
{ DEBSHV_FIRSTERRP,1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_ERRLENGTH,1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
{ DEBSHV_ERRBUFFSZ,1, 1, 4, SHV_MODE_TYPE_LONG, -1, 0, 0, },
/* Capito ? */
};
static unsigned long ** deb_errlog_tg[] = {
(unsigned long **)&debug_status.sh_errors,
&debug_status.sh_firsterrp,
&debug_status.sh_errlength,
&debug_status.sh_errbuffsz,
};

void debErrorInit(void)
{
	debug_status.error_buffer_size = ERROR_BUFFER_SIZE;
	debug_status.sh_firsterrp = NULL;
	debug_status.sh_errlength = NULL;
	debug_status.sh_errbuffsz = NULL;
	debug_errlog_shvar[0].size = debug_status.error_buffer_size;
	_brkCreateShvar(debug_errlog_shvar,deb_errlog_tg,
	                sizeof(deb_errlog_tg)/sizeof(deb_errlog_tg[0]));
	if (debug_status.sh_firsterrp
	 && debug_status.sh_errlength
	 && debug_status.sh_errbuffsz) {
		*debug_status.sh_firsterrp = 0;
		*debug_status.sh_errlength = 0;
		*debug_status.sh_errbuffsz = debug_status.error_buffer_size;
	}
	else {
		debug_status.sh_firsterrp = NULL;
	}
}

/*
* Funzione "debErrorLog"
* ----------------------
*
* Questa funzione copia il messaggio di errore di codice "code" e
* testo aggiuntivo "text" nel buffer degli errori.
*/

static void _debErrorLogByte(int b)
{
int sz;
unsigned long p;

	p = *debug_status.sh_firsterrp + *debug_status.sh_errlength;
	if (p >= debug_status.error_buffer_size)
		p -= debug_status.error_buffer_size;
	++*debug_status.sh_errlength;
	if (*debug_status.sh_errlength > debug_status.error_buffer_size) {
		sz = debug_status.sh_errors[*debug_status.sh_firsterrp];
		*debug_status.sh_firsterrp += sz;
		if (*debug_status.sh_firsterrp
		 >= debug_status.error_buffer_size)
			*debug_status.sh_firsterrp
			 -= debug_status.error_buffer_size;
		*debug_status.sh_errlength -= sz;
	}
	debug_status.sh_errors[p] = b;
}

void debErrorLog(int code, char *text)
{
int sz;

	if (!debug_status.sh_firsterrp)
		return;

	sz = 1 /* Un byte per la lunghezza del blocco. */
	   + 4 /* Quattro byte per il codice di errore. */
	   + strlen(text) /* Lunghezza del testo. */
	   + 1; /* '\0' finale. */
	if (sz > 0xFF)
		sz = 0xFF;
	_debErrorLogByte(sz & 0xFF);
	_debErrorLogByte(code & 0xFF);
	_debErrorLogByte((code >> 8) & 0xFF);
	_debErrorLogByte((code >> 16) & 0xFF);
	_debErrorLogByte((code >> 24) & 0xFF);
	sz -= 5;
	do {
		if (sz == 1)
			_debErrorLogByte('\0');
		else
			_debErrorLogByte(*(text++));
	} while (--sz);
}

/*
* Funzione "debErrorList"
* -----------------------
*
* Questa funzione fornisce gli indirizzi, le lunghezze ed il numero
* dei segmenti costituenti l'intera storia degli errori registrati.
*/

int debErrorList(unsigned char **b1, unsigned long *n1,
                 unsigned char **b2, unsigned long *n2)
{
int nr;

	if (!debug_status.sh_firsterrp)
		return 0;

	nr = 0;
	*b1 = *b2 = NULL;
	*n1 = *n2 = 0;
	if (*debug_status.sh_errlength != 0)  {
		*b1 = debug_status.sh_errors + *debug_status.sh_firsterrp;
		if (*debug_status.sh_firsterrp + *debug_status.sh_errlength
		  > debug_status.error_buffer_size) {
			nr = 2;
			*n1 = debug_status.error_buffer_size
			    - *debug_status.sh_firsterrp;
			*b2 = debug_status.sh_errors;
			*n2 = *debug_status.sh_errlength - *n1;
		}
		else {
			nr = 1;
			*b1 = debug_status.sh_errors
			    + *debug_status.sh_firsterrp;
			*n1 = *debug_status.sh_errlength;
		}
	}
	return nr;
}

/*
* Funzione "debErrorFindInit"
* ---------------------------
*
* Questa funzione inizializza la variabile puntata da "p" in modo che
* possa essere utilizzata per la lettura sequenziale degli errori
* registrati nel buffer.
*/

void debErrorFindInit(unsigned long *p)
{
	if (!debug_status.sh_firsterrp)
		return;

	*p = *debug_status.sh_firsterrp;
}

/*
* Funzione "debErrorFindNext"
* ---------------------------
*
* Questa funzione legge il prossimo errore immagazzinato nel buffer,
* e aggiorna la variabile puntata da "p" per la successiva ricerca.
* il codice dell'errore e` scaricato in "code", il testo e` copiato in
* "text". La funzione vale 1 se tutto e` andato bene, 0 se non ci sono
* piu` errori da leggere.
*/

static void _debErrorCopyBytes(unsigned long *pos, int nb,
                               unsigned char *dest)
{
	while (nb) {
		if (*pos >= debug_status.error_buffer_size)
			*pos -= debug_status.error_buffer_size;
		*dest = debug_status.sh_errors[*pos];
		++dest;
		++(*pos);
		--nb;
	}
}

int debErrorFindNext(unsigned long *p, int *code, char *text)
{
unsigned long rp;
unsigned char sz;

	if (!debug_status.sh_firsterrp)
		return 0;

	if (*p - *debug_status.sh_firsterrp >= *debug_status.sh_errlength)
		return 0;
	rp = *p;
	_debErrorCopyBytes(&rp, 1, &sz);
	*p += sz;
	_debErrorCopyBytes(&rp, sizeof(int), (unsigned char *)code);
	_debErrorCopyBytes(&rp, sz - sizeof(int) - 1, (unsigned char *)text);
	return 1;
}

