/*
* @(#) util.h 1.20 Tue Sep  9 01:20:33 CEST 2008
* @(#) Rif. Guerrini
*
* Funzioni di varia utilita` per il plc CNi (qplc).
* Tutte le funzioni qui raccolte iniziono per "util_".
*
* 14/05/96 1.0 Prima stesura.
* 05/06/96 1.1 Aggiunta la funzione "util_init_chain".
* 18/06/96 1.2 Aggiunte le funzioni "util_start_thread" e "util_bit_scan".
* 19/06/96 1.3 Aggiunta la funzione "util_check_addr".
* 23/06/96 1.4 Aggiunta la funzione "util_streq".
* 26/08/96 1.5 Aggiunta la funzione "util_execprio".
* 04/09/96 1.6 Aggiunta la funzione "util_strneq".
* 06/09/96 1.7 Aggiunta la funzione "util_ltob".
* 18/09/96 1.8 Aggiunta la funzione "util_fpu_reset".
* 19/09/96 1.9 Aggiunte le funzioni "util_safe_ltob", "util_safe_gcvt",
*          "util_safe_vsprintf", "util_usleep". Eliminata la "util_ltob".
* 27/09/96 1.10 Aggiunta la funzione "util_get_pc".
* 15/01/97 GG 1.11 Aggiunta la funzione "util_call_il", per chiamare in
*             maniera sicura funzioni, blocchi o programmi IL.
* 07/02/97 GG 1.12 Appurato che in Lynx il dispositivo "/dev/null" da`
*             EOF fin dalla prima apertura, e` stata introdotta anche la
*             funzione "util_check_addr_wr". Sia questa che la
*             "util_check_addr_rd" utilizzano lo stesso file descriptor,
*             che DEVE essere inizializzato chiamando al lancio la funzione
*             "util_check_addr_init". La mancanza di inizializzazione
*             e` la possibile causa dell'occasionale rallentamento che
*             si verifica in certe situazioni.
* 18/02/97 GG 1.13 Corretto un incredibile buco nella funzione "util_strneq",
*             che finora ha funzionato per puro caso.
*             Aggiunta la funzione "util_isrootgroup", che vale 1 se il
*             processo ha il permesso di accedere direttamente alle porte
*             di I/O.
*             Aggiunta la funzione "util_null", che non fa niente, ma lo
*             fa in fretta.
* 14/07/97 GG 1.14 Aggiunta la funzione "util_safe_sprintf". Mancava.
* 05/11/97 GG 1.15 Aggiunte le funzioni "util_ahtol" e "util_adtol".
* 19/11/97 GG 1.16 Aggiunte le funzioni "util_memset" e "util_memcpy", che
*             scrivono in memoria senza utilizzare accessi a 32 bit.
* 25/03/98 GG 1.17 Aggiunte funzioni per inizializzare e aggiungere
*             stringhe a buffer di caratteri senza sforare. Sono poi state
*             utilizzate per realizzare le funzioni del tipo "util_safe_...".
*             Aggiunta la funzione "util_ahtom" per caricare buffer di
*             lunghezza arbitraria con valori espressi in esadecimale.
* 04/09/98 GG 1.18 Eliminata la chiamata di "getprio" da "util_execprio".
*             Aggiunta in sua vece la funzione "util_startprio" per ottenere
*             la priorita` base.
* 07/12/00 GG 1.19 Riscritte per Linux le funzioni "util_check_addr_...",
*             perche` su questo sistema il driver "/dev/null" non controlla
*             i confini dei buffer. Si e` dovuto usare "/proc/<pid>/maps".
*             Molto piu` lento... Percio` e` stata aggiunta la funzione
*             "util_check_addr_refresh", che serve a ricalcolare
*             i dati utilizzati internamente dalle funzioni di validazione
*             degli indirizzi.
* 08/09/08 GG 1.20 Aggiunta la funziona util_error (stampa, non esce).
*/

#ifndef _UTIL_H_

#include <stdarg.h>
#include <pthread.h>

#define _UTIL_H_

typedef struct {
	char * string;
	char * pos;
	char * top;
} bstring_t;

/*
* Macro per il calcolo del parametro "offset" delle funzioni di
* manipolazione di liste.
*/

#define GET_STRUCT_OFFSET(t,e) ((int)&(((t *)0)->e))

/*
* Funzione "util_show_version"
* ----------------------------
*
*  Questa funzione visualizza la versione "s" (sottocodice n) su stdout
* o verso il visualizzatore delle quote (contesto CN).
*/

void util_show_version(char * s, int n);

/*
* Funzione "util_show_diff_version"
* ---------------------------------
*
*  Questa funzione visualizza la versione "s" (sottocodice n) su stdout
* o verso il visualizzatore delle quote (contesto CN), purche` sia diversa
* dalla stringa "p".
*/

void util_show_diff_version(char *p, char * s, int n);

/*
* Funzione "util_show_versions"
* ----------------------------
*
*  Questa funzione visualizza le versioni su stdout o verso il
* visualizzatore dell quote (contesto CN).
*/

void util_show_versions(void);

/*
* Funzione "util_streq"
* ---------------------
*
*  Questa funzione esegue un confronto "case insensitive" (solo uguaglianza)
* tra stringhe. Vale 1 se le stringhe sono ugusli, 0 altrimenti.
*/

int util_streq(char * s1, char * s2);

/*
* Funzione "util_strneq"
* ---------------------
*
*  Questa funzione esegue un confronto "case insensitive" (solo uguaglianza)
* tra stringhe fino alla lunghezza "n". Vale 1 se le stringhe sono uguali,
* 0 altrimenti.
*/

int util_strneq(char * s1, char * s2, int n);

/*
* Funzione "util_fatal"
* ---------------------
*
*  Questa funzione emette un errore (su stderr) ed esce.
*/

void util_error(char *s);
void util_fatal(char *s);

/*
* Funzione "util_init_chain"
* ---------------------------
*
*  Questa e` una funzione di utilita` generica, che puo` essere comoda
* nello sviluppo di driver. Crea una catena di "n" elementi di dimensione
* "size", congiunti da un puntatore che si trova ad "offset" byte dall'inizio
* di ogni elemento, utilizzando l'area "dest". L'ultimo puntatore varra` NULL.
*  Il valore di ritorno e` l'indirizzo del primo elemento della catena.
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void * util_init_chain(int size, int offset, int n, void *dest);

/*
* Funzione "util_alloc_chain"
* ---------------------------
*
*  Questa e` una funzione di utilita` generica, che puo` essere comoda
* nello sviluppo di driver. Alloca una catena di "n" elementi di dimensione
* "size", congiunti da un puntatore che si trova ad "offset" byte dall'inizio
* di ogni elemento. L'ultimo puntatore varra` NULL.
*  Il valore di ritorno e` l'indirizzo del primo elemento della catena,
* oppure NULL se non c'e` memoria.
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void * util_alloc_chain(int size, int offset, int n);

/*
* Funzione "util_alloc_cell"
* --------------------------
*
*  Questa funzione alloca una struttura di dimensione "size", avente
* un elemento di collegamento ad "offset" byte dall'inizio, riferendosi
* alla lista puntata dalla variabile di indirizzo "pfree". In caso che
* non vi siano elementi liberi (*pfree = NULL), e` chiamata la funzione
* "util_alloc_chain" per allocare "n" nuove celle.
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void * util_alloc_cell(void **pfree, int size, int offset, int n);

/*
* Funzione "util_add_cell"
* ------------------------
*
*  Questa funzione aggiunge una struttura avente
* un elemento di collegamento ad "offset" byte dall'inizio, di indirizzo "a",
* alla lista puntata dalla variabile di indirizzo "pfree".
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void util_add_cell(void *a, void **pfree, int offset);

/*
* Funzione "util_add_chain"
* -------------------------
*
*  Questa funzione aggiunge una lista di strutture aventi
* un elemento di collegamento ad "offset" byte dall'inizio, di indirizzo "a",
* alla lista puntata dalla variabile di indirizzo "pfree".
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void util_add_chain(void *a, void **pfree, int offset);

/*
* Funzione "util_find_cell"
* -------------------------
*
*  Questa funzione vale 1 se la cella "a" e` presente nella lista
* puntata dalla variabile di indirizzo "pfree", altrimenti vale 0.
*  La cella ha un elemento di collegamento ad "offset" byte dall'inizio.
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

int util_find_cell(void *a, void **pfree, int offset);

/*
* Funzione "util_move_cell"
* -------------------------
*
*  Questa funzione elimina la cella successiva a quella puntata da
* "from_p" nella lista "from_l", e la inserisce dopo la cella "to_p"
* della lista "to_l". L'elemento di collogamento delle liste si trova
* ad "offset" byte dall'inizio. Se "from_p" o "to_p" sono NULL, si intende
* che la posizione di lavoro e` l'inizio della lista.
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void util_move_cell(void **from_l, void * from_p,
                    void **to_l, void * to_p, int offset);

/*
* Funzione "util_insert_cell"
* ---------------------------
*
*  Questa funzione inserisce una nuova cella dopo la posizione "ins_p"
* della lista "ins_l", prelevandola dalla lista "pfree" ed allocando
* eventualmente "n" celle di dimensione "size". Le celle sono collegate
* da un elemento che si trova ad "offset" byte dall'inizio.
*  Se "ins_p" e` NULL, si intende che la posizione di lavoro e`
* l'inizio della lista.
*
* NOTA : Per calcolare in modo semplice il valore di "offset", si puo`
* utilizzare la macro GET_STRUCT_OFFSET(t,e) , con t=tipo della struttura
* ed e=nome dell'elemento puntatore.
*/

void * util_insert_cell(void ** ins_l, void * ins_p, void ** pfree,
                        int size, int offset, int n);

/*
* Funzione "util_start_thread"
* ----------------------------
*
*  Questa funzione lancia un thread con priorita` "ipri"
* e lo aggancia alla funzione "mainfn".
*  Restituisce il TID del thread, oppure -1 in caso di errori.
*/

pthread_t util_start_thread(void *(*mainfn)(void *),int ipri);

/*
* Funzione "util_stop_thread"
* ---------------------------
*
*  Questa funzione cancella il thread "tid".
*  Restituisce 0 se tutto bene, oppure -1 in caso di errori.
*/

int util_stop_thread(pthread_t tid);

/*
* Funzione "util_bit_scan"
* ------------------------
*
*  Questa funzione restituisce l'indice del primo bit acceso del parametro "x".
*  Se "x" vale 0, il risultato e` -1.
*/

int util_bit_scan(int x);

/*
* Funzioni "util_check_addr_rd", "util_check_addr_wr", "util_check_addr_init"
* ---------------------------------------------------------------------------
*
*  Queste funzioni controllano la leggibilita` o la scrivibilita`
* dell'indirizzo "addr" per "n" byte. Valgono 1 se l'area e` leggibile o
* scrivibile, o se il controllo e` disabilitato dal flag "chkaddr",
* 0 altrimenti.
*  La funzione "util_chek_addr_init" deve essere chiamata all'inizio
* per aprire il dispositivo "/dev/null". Su Linux, dove il trucco
* di "/dev/null" non funziona, bisogna chiamare ad ogni "start PLC"
* anche la "util_check_addr_refresh", che mantiene una cache per
* accelerare le operazioni.
*/

int util_check_addr_init(void);
void util_check_addr_refresh(void);
int util_check_addr_rd(char * addr, int n);
int util_check_addr_wr(char * addr, int n);

/*
* Funzione "util_execprio"
* ------------------------
*
*  Questa funzione restituisce la priorita` dell'esecutore.
*/

int util_execprio(void);

/*
* Funzione "util_startprio"
* ------------------------
*
*  Questa funzione restituisce la priorita` di lancio del processo.
*/

int util_startprio(void);

/*
* Funzione "util_safe_ltob"
* -------------------------
*
*  Su Lynx non c'e` "ltob". Inoltre, sprintf usa malloc. E se non la usasse,
* non si sarebbe comunque sicuri di prenderci con la lunghezza della stringa.
*/

int util_safe_ltob(char *buff, int bsize, long v, int b);

/*
* Funzione "util_safe_gcvt"
* -------------------------
*
*  Versione di "gcvt" che non esce dai limiti della stringa destinazione.
*/

int util_safe_gcvt(char *buff, int bsize, double v, int b);

/*
* Funzione "util_fpu_reset"
* -------------------------
*
*  Questa funzione rappezza un buco di Lynx, che dimentica di
* riprogrammare la maschera della eccezioni del coprocessore dopo
* la ricezione del segnale SIGFPE.
*/

void util_fpu_reset(void);

/*
* Funzione "util_init_bstring"
* ---------------------------
*
*  Questa funzione prepara una struttura di tipo "bstring_t" e la
* associa al buffer "s" di dimensione "l".
*/

void util_init_bstring(bstring_t *b, char *s, int l);

/*
* Funzione "util_cat_bstring"
* ---------------------------
*
*  Questa funzione accoda la stringa "s" al buffer descritto da "b".
*/

void util_cat_bstring(bstring_t *b, char *s);

/*
* Funzione "util_addch_bstring"
* -----------------------------
*
*  Questa funzione accoda il carattere "c" al buffer descritto da "b".
*/

void util_addch_bstring(bstring_t *b, int c);

/*
* Funzione "util_gcvt_bstring"
* ----------------------------
*
*  Questa funzione accoda il risultato di "gcvt" al buffer descritto da "b".
*/

void util_gcvt_bstring(bstring_t *b, double d, int n);

/*
* Funzione "util_ltob_bstring"
* ----------------------------
*
*  Questa funzione accoda il risultato di "ltob" al buffer descritto da "b".
*/

void util_ltob_bstring(bstring_t *b, long v, int n);

/*
* Funzione "util_vsprintf_bstring"
* --------------------------------
*
*  Semplice "vsprintf" applicata al buffer "b".
*/

void util_vsprintf_bstring(bstring_t *b, char *fmt, va_list a);

/*
* Funzione "util_sprintf_bstring"
* -------------------------------
*
*  Questa funzione realizza una mini-sprintf sul buffer descritto da "b".
*  Si veda "util_safe_sprintf" per limiti e prestazioni.
*/

void util_sprintf_bstring(bstring_t *b, char *fmt, ...);

/*
* Funzione "util_safe_vsprintf"
* -----------------------------
*
*  Questa funzione realizza una mini-vsprintf "sicura", nel senso che
* evita di scrivere fuori dalla stringa data, e non chiama funzioni strane
* (malloc) come fa la "sprintf" ordinaria.
*/

char * util_safe_vsprintf(char *dest, int bsize, char *fmt, va_list args);

/*
* Funzione "util_safe_sprintf"
* ----------------------------
*
*  Questa funzione realizza una mini-sprintf "sicura", nel senso che
* evita di scrivere fuori dalla stringa data, e non chiama funzioni strane
* (malloc) come fa la "sprintf" ordinaria.
*/

char * util_safe_sprintf(char *dest, int bsize, char *fmt, ...);

/*
* Funzione "util_usleep"
* ----------------------
*
*  Questa funzione rappezza l'ennesimo buco di Lynx. Realizza una "usleep"
* che non utilizza i segnali per funzionare. Fara` schifo, ma che alternative
* ci sono ?
*/

void util_usleep(long sec, long usec);

/*
* Funzione "util_get_pc"
* ----------------------
*
*  Questa funzione restituisce il PC del punto di chiamata del chiamate.
*  "level" indica quanti livelli di chiamata si vogliono ignorare.
*/

unsigned long util_get_pc(int level);

/*
* Funzione "util_call_il"
* ----------------------
*
*  Questa funzione serve a lanciare un programma o un blocco funzionale IL.
*  "prog" e` l'indirizzo del blocco, "statics" e` l'indirizzo della
* struttura contenente le variabili di stato ed i parametri del blocco.
*/

unsigned long util_call_il(void (*prog)(void *statics), void *statics);

/*
* Funzione "util_isrootgroup"
* ---------------------------
*
*  Questa funzione vale 0 se l'utente detentore del processo fa parte del
* gruppo del super-user (che ha il permesso di accedere direttamente alle
* porte di I/O).
*/

int util_isrootgroup(void);

/*
* Funzione "util_null"
*
* Non fa niente, ma lo fa in fretta.
*/

void util_null(void);

/*
* Funzione "util_outb"
* --------------------
*
* Out ad una porta. Un classico.
*/

int util_outb(int val, int port);

/*
* Funzione "util_inb"
* --------------------
*
* IN da una porta. Un classico.
*/

int util_inb(int port);

/*
* Funzioni "util_ahtol" e "util_adtol"
* ------------------------------------
*
*  Queste funzioni convertono una stringa esadecimale o decimale in binario.
*  Se "end" non e` nullo, vi e` scritto l'indirizzo del primo carattere
* non convertibile della stringa.
*/

unsigned long util_ahtol(char *s, char **end);

unsigned long util_adtol(char *s, char **end);

/*
* Funzione "util_ahtom"
* ---------------------
*
*  Questa funzione converte una stringa esadecimale in binario.
*  Le cifre sono interpretate a gruppi di due come byte, che vengono
* copiati ad indirizzo crescenti in "buff", fino ad un limite di "n".
*/

int util_ahtom(char *s, char **end, char *buff, int n);

/*
* Funzioni "util_memset" e "util_memcpy"
* --------------------------------------
*
* Equivalgono a "memset" e "memcpy", ma non eseguono accessi a 32 bit.
* Da utilizzare per scrivere sul bus ISA.
*/

void util_memset(register char *d, register int v, register int n);
void util_memcpy(register char *d, register char *s, register int n);

#endif /* _UTIL_H_ */

