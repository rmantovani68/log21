
/*
* @(#) util.c 1.29-ael Tue Sep  9 01:19:20 CEST 2008
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
* 29/11/96 1.11 Modificata la stampa delle versioni per gestire il caso
*          della versione nel programma IL.
* 19/12/96 GG 1.12 Modifiche per distinguere la sigla delle versioni delle
*             varie componenti.
* 15/01/97 GG 1.13 Aggiunta la funzione "util_call_il", per chiamare in
*             maniera sicura funzioni, blocchi o programmi IL.
*             Spostate tutte le funzioni contenenti assembler in "util2.s".
* 04/02/97 GG 1.14 Aggiunta un'opzione per disattivare il controllo
*             eseguito dalla funzione "util_check_addr_rd". Emergenza.
* 07/02/97 GG 1.15 Appurato che in Lynx il dispositivo "/dev/null" da`
*             EOF fin dalla prima apertura, e` stata introdotta anche la
*             funzione "util_check_addr_wr". Sia questa che la
*             "util_check_addr_rd" utilizzano lo stesso file descriptor,
*             che DEVE essere inizializzato chiamando al lancio la funzione
*             "util_check_addr_init". La mancanza di inizializzazione
*             e` la possibile causa dell'occasionale rallentamento che
*             si verifica in certe situazioni.
* 18/02/97 GG 1.16 Corretto un incredibile buco nella funzione "util_strneq",
*             che finora ha funzionato per puro caso.
*             Aggiunta la funzione "util_isrootgroup", che vale 1 se il
*             processo ha il permesso di accedere direttamente alle porte
*             di I/O.
* 14/07/97 GG 1.17 Aggiunta "util_safe_sprintf"... che mancava !
* 16/07/97 GG 1.18 Aggiunta la funzione "util_stop_thread". Modificata
*             la modalita` di cancellazione dei thread.
* 05/11/97 GG 1.19 Aggiunte le funzioni "util_ahtol" e "util_adtol".
* 19/11/97 GG 1.20 Aggiunte le funzioni "util_memset" e "util_memcpy", che
*             scrivono in memoria senza utilizzare accessi a 32 bit.
* 09/02/98 GG 1.21 Corretto un bug nella funzione "util_safe_gcvt": restituiva
*             la lunghezza sbagliata.
* 25/03/98 GG 1.22 Aggiunte funzioni per inizializzare e aggiungere
*             stringhe a buffer di caratteri senza sforare. Sono poi state
*             utilizzate per realizzare le funzioni del tipo "util_safe_...".
*             Aggiunta la funzione "util_ahtom" per caricare buffer di
*             lunghezza arbitraria con valori espressi in esadecimale.
* 04/09/98 GG 1.23 Eliminata la chiamata di "getprio" da "util_execprio".
*             Aggiunta in sua vece la funzione "util_startprio" per ottenere
*             la priorita` base.
* 30/11/98 GG 1.24 Evitata la chiamata di "pthread_cancel"
*             in "util_stop_thared" se il valroe di "tid" e` -1. Questo
*             risolve un buco curioso (buco? forse...) della system call,
*             e un comportamento anomalo all'entrata in HALT.
* 28/07/00 GG 1.25 Aggiustamenti per compilazione su Linux.
* 05/12/00 GG 1.26 Correzione della funzione "util_isrootgroup", che
*             non funziona (per cause non addebitabili alla sua logica)
*             sui microXNC basati su kernel 2.3.0 (il kernel restituisce
*             una lista di gruppi vuota. E` evidentemente una sfiga di
*             kernel. La soluzione e` controllare se l'UID e` 0 - il che
*             e` sempre vero sul micro).
* 07/12/00 GG 1.27 Riscritte per Linux le funzioni "util_check_addr_...",
*             perche` su questo sistema il driver "/dev/null" non controlla
*             i confini dei buffer. Si e` dovuto usare "/proc/<pid>/maps".
*             Molto piu` lento... Percio` e` stata aggiunta la funzione
*             "util_check_addr_refresh", che serve a ricalcolare
*             i dati utilizzati internamente dalle funzioni di validazione
*             degli indirizzi.
* 20/07/06 GG 1.28-ael Intercettato l'errore EINTR in sem_wait,
*             perche' le versioni recenti di pthread non lo
*             intercettano automaticamente (NOTA: non documentato
*             trattasi forse di bug?).
* 08/09/08 GG 1.29-ael Aggiunta la funziona util_error (stampa, non esce).
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#ifdef Linux
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <semaphore.h>
#else
#include <types.h>
#include <msg.h>
#include <sem.h>
#endif
#include <errno.h>
#include <pthread.h>
#ifndef Linux
#include <proc.h>
#endif

#ifdef Linux
#include "lynx-a.out.h"
#include "lynx-nlist.h"
#else
/* Specifici CNi */
#include <services.h>
#endif

#include "qplc.h"

#include "util.h"

/*
* Funzione "util_show_version"
* ----------------------------
*
*  Questa funzione visualizza la versione "s" (sottocodice n) su stdout
* o verso il visualizzatore delle quote (contesto CN).
*/

void util_show_version(char * s, int n)
{
	if (!s)
		s = "unknown";
	else if (!s[0])
		return;
	if (main_status.stand_alone)
		printf("%s [%s] version %s\n",
		       main_status.exe_base, main_status.ver_sig[n], s);
	else
#ifdef Linux
		;
#else
		invVersione(main_status.ver_sig[n],1,s);
#endif
}

/*
* Funzione "util_show_diff_version"
* ---------------------------------
*
*  Questa funzione visualizza la versione "s" (sottocodice n) su stdout
* o verso il visualizzatore delle quote (contesto CN), purche` sia diversa
* dalla stringa "p".
*/

void util_show_diff_version(char *p, char * s, int n)
{
	util_show_version(strcmp(p,s) ? s : "", n);
}

/*
* Funzione "util_show_versions"
* ----------------------------
*
*  Questa funzione visualizza le versioni su stdout o verso il
* visualizzatore delle quote (contesto CN).
*/

void util_show_versions(void)
{
	util_show_version(main_status.ver_kernel,VER_IDX_KERNEL);
	util_show_version(main_status.mod_hatsh_op.version,OLD_MOD_IDX_HATSH);
	if (main_status.mod_user_op.ver_il)
		util_show_version(main_status.mod_user_op.ver_il,
		                  OLD_MOD_IDX_USER);
	else {
		util_show_version(main_status.mod_user_op.version,
		                  main_status.mod_user -> text
		                       ? OLD_MOD_IDX_USER
		                       : OLD_IDX_BUILTIN_USER);
	}
	util_show_version(main_status.mod_plc480_op.version,OLD_MOD_IDX_480);
}

/*
* Funzione "util_streq"
* ---------------------
*
*  Questa funzione esegue un confronto "case insensitive" (solo uguaglianza)
* tra stringhe. Vale 1 se le stringhe sono ugusli, 0 altrimenti.
*/

int util_streq(char * s1, char * s2)
{
	while (tolower(*s1) == tolower(*s2)) {
		if (! *s1)
			return 1;
		++s1;
		++s2;
	}
	return 0;
}

/*
* Funzione "util_strneq"
* ---------------------
*
*  Questa funzione esegue un confronto "case insensitive" (solo uguaglianza)
* tra stringhe fino alla lunghezza "n". Vale 1 se le stringhe sono uguali,
* 0 altrimenti.
*/

int util_strneq(char * s1, char * s2, int n)
{
	if (n == 0)
		return 1;
	while (tolower(*s1) == tolower(*s2)) {
		if (! *s1 || ! --n)
			return 1;
		++s1;
		++s2;
	}
	return 0;
}

/*
* Funzione "util_fatal"
* ---------------------
*
*  Questa funzione emette un errore (su stderr) ed esce.
*/

void util_error(char *s)
{
	fprintf(stderr,"%s : %s : %s\n", main_status.exe_base, s, strerror(errno));
}

void util_fatal(char *s)
{
	util_error(s);
	exit(1);
}

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

void * util_init_chain(int size, int offset, int n, void *dest)
{
char *p = (char *) dest;
int i;

	if (! n || ! size || !dest)
		return (void *) 0;

	for (i = 1; i < n; ++i)
		*(char **)(p + (i - 1)*size + offset) = p + i * size;

	*(char **)(p + (n - 1)*size + offset) = (char *) 0;

	return (void *) p;
}

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

void * util_alloc_chain(int size, int offset, int n)
{
char *p;

	if (! n || ! size)
		return (void *) 0;

	p = (char *) malloc(size * n);
	if (! p)
		return (void *) 0;

	return util_init_chain(size, offset, n, (void *) p);
}

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

void * util_alloc_cell(void **pfree, int size, int offset, int n)
{
void * p;

	if (! *pfree) {
		p = util_alloc_chain(size, offset, n);
		if (! p)
			return (void *) 0;
		*pfree = p;
	}

	p = *pfree;
	*pfree = *(void **)((char *)p + offset);

	return p;
}

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

void util_add_cell(void *a, void **pfree, int offset)
{
	if (a) {
		*(void **)((char *)a + offset) = *pfree;
		*pfree = a;
	}
}

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

void util_add_chain(void *a, void **pfree, int offset)
{
void *p,*q;

	for (p = a, q = (void *)0; p; q = p, p = *(void **)((char *)p+offset))
		;

	if (q) {
		*(void **)((char *)q + offset) = *pfree;
		*pfree = a;
	}
}

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

int util_find_cell(void *a, void **pfree, int offset)
{
void *p;

	for (p = *pfree; p; p = *(void **)((char *)p+offset)) {
		if (p == a)
			return 1;
	}
	return 0;
}

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
                    void **to_l, void * to_p, int offset)
{
void *p;

	if (from_p) {
		from_l = (void **)((char *)from_p + offset);
	}

	p = *from_l;
	if (! p)
		return;
	*from_l = *(void **)((char *)p + offset);

	util_add_cell(p,
	              to_p
	               ? (void **) ((char *)to_p + offset)
	               : to_l,
	              offset);
}

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
                        int size, int offset, int n)
{
void * p;

	p = util_alloc_cell(pfree, size, offset, n);
	if (! p)
		return p;

	util_add_cell(p,
	              ins_p
	               ? (void **) ((char *)ins_p + offset)
	               : ins_l,
	              offset);

	return p;
}

/*
* Funzione "util_start_thread"
* ----------------------------
*
*  Questa funzione lancia un thread con priorita` "ipri"
* e lo aggancia alla funzione "mainfn".
*  Restituisce il TID del thread, oppure -1 in caso di errori.
*/

/* Lanciatore della routine. E` un trucco per convincere Lynx ad
 eseguire thread su funzioni sia in spazio codice che in spazio dati (per
 caricamento dinamico). E serve anche a far eseguire al thread qualche
 utile sciocchezzuola. */
static void * _util_thread_main(void * fn)
{
#ifdef Linux
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
#else
	pthread_setcancel(CANCEL_ON);
	pthread_setasynccancel(CANCEL_ON);
#endif
	return (*(void *(*)(void *))fn)((void *) 0);
}

pthread_t util_start_thread(void *(*mainfn)(void *),int ipri)
{
pthread_attr_t thread;
pthread_t tid;

/* Inizializza la struttura descrivente il thread. */

#ifdef Linux
	pthread_attr_init(&thread);
#else
	pthread_attr_create(&thread);
#endif

/* Modifica la priorita` del thread da generare. */

#ifdef Linux
	pthread_attr_setinheritsched(&thread,PTHREAD_EXPLICIT_SCHED);
{
struct sched_param sch;
int policy;

	pthread_attr_setschedpolicy(&thread,SCHED_FIFO);
	pthread_attr_getschedparam(&thread,&sch);
	sch.sched_priority = ipri;
	if (pthread_attr_setschedparam(&thread,&sch)) {
		util_error("setschedparam");
	}
}
#else
	pthread_attr_setinheritsched(&thread,0);
	pthread_attr_setprio(&thread,ipri);
#endif

/* Crea l'istanza del thread. */

/*
*  Nelle intenzioni, il codice doveva essere cosi` :
*
*	if (pthread_create(& tid,thread,mainfn,(void *) 0) < 0) {
*		tid = (pthread_t) -1;
*	}
*
* Sembra pero` che Lynx non accetti puntatori a funzione fuori dallo
* spazio di codice ("text") del processo. Allora si deve usare un trucco :
* una routine di lancio nello spazio codice, e l'argomento a puntare
* alla routine definitiva.
*/
	if (pthread_create(&tid,
#ifdef Linux
	&
#endif
		thread
		,_util_thread_main,(void *)mainfn) < 0) {
		tid = (pthread_t) -1;
	}

/* Libera il descrittore utilizzato per il lancio del thread. */

#ifdef Linux
	pthread_attr_destroy(&thread);
#else
	pthread_attr_delete(&thread);
#endif

	return tid;
}

/*
* Funzione "util_stop_thread"
* ---------------------------
*
*  Questa funzione cancella il thread "tid".
*  Restituisce 0 se tutto bene, oppure -1 in caso di errori.
*/

int util_stop_thread(pthread_t tid)
{
int rv;
void *sts;

	if (tid == -1)
		return -1;
	rv = pthread_cancel(tid);
	if (rv != -1) {
		pthread_join(tid,&sts);
		pthread_detach(&tid);
	}
	return rv;
}

/*
* Funzione "util_bit_scan"
* ------------------------
*
*  Questa funzione restituisce l'indice del primo bit acceso del parametro "x".
*  Se "x" vale 0, il risultato e` -1.
*/

/*
int util_bit_scan(int x)
{
register int y;
#ifdef __GNUC__
	__asm__ ("xorl %0,%0; decl %0; bsf %1,%0" : "=r" (y) : "" (x));
#else
	y = -1;
	while (((unsigned int)x) % 2 == 0) {
		x /= 2;
		++y;
	}
#endif
	return y;
}
*/

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

#ifdef Linux

#if 0
void util_check_addr_refresh(void)
{
}

int util_check_addr_init(void)
{
	return 1;
}

int util_check_addr_rd(char * addr, int n)
{
	return 1;
}

int util_check_addr_wr(char * addr, int n)
{
	return 1;
}
#else
/* Su Linux il driver "/dev/null" non controlla la validita`
 dei buffer, quindi il trucco non funziona. Bisogna ricorrere
 a "/proc/<pid>/maps". */

typedef struct {
	char *base;
	char *end;
} util_chkaddr_area_t;
typedef struct {
#define MAX_UTIL_CHKADDR_TABLE 128
	util_chkaddr_area_t t_area[MAX_UTIL_CHKADDR_TABLE];
	int n_area;
} util_chkaddr_tarea_t;
typedef struct {
	util_chkaddr_area_t area;
	char rd;
	char wr;
} util_chkaddr_record_t;

static struct {
/* Aree statiche, perche` non possiamo influire sulla configurazione
 dello spazio di indirizzamento con delle allocazioni dinamiche. */
	util_chkaddr_tarea_t table_r;
	util_chkaddr_tarea_t table_w;
	util_chkaddr_record_t table[MAX_UTIL_CHKADDR_TABLE];
} _util_check_addr_st;

static int _util_check_addr_generic(char *addr, int n, int wr)
{
util_chkaddr_tarea_t *t;
int i;
int rv;

	if (wr)
		t = &_util_check_addr_st.table_w;
	else
		t = &_util_check_addr_st.table_r;

	rv = 0;
	for (i = 0; i < t -> n_area; ++i) {
		if (addr >= t -> t_area[i].base
		 && addr < t -> t_area[i].end
		 && addr+n >= t -> t_area[i].base
		 && addr+n <= t -> t_area[i].end) {
			rv = 1;
			break;
		}
	}

	return rv;
}

static void _util_check_addr_addarea(util_chkaddr_record_t *r,
                                     util_chkaddr_tarea_t *t)
{
	if ((t -> n_area > 0)
	 && (r -> area.base == t -> t_area[t -> n_area - 1].end)) {
	/* Unisce due aree contigue in una sola. */
		t -> t_area[t -> n_area - 1].end = r -> area.end;
	}
	else {
	/* Crea una nuova area. */
		t -> t_area[t -> n_area] = r -> area;
		++(t -> n_area);
	}
}

void util_check_addr_refresh(void)
{
int fd,rv;
char procpath[80],line[80];
unsigned long base,end;
char rp,wp;
char c;
int i,n_lines;

	if (! main_status.chkaddr)
		return;

	util_safe_sprintf(procpath,sizeof(procpath),"/proc/%d/maps",getpid());

/* Primo giro: conteggio delle linee. */

	fd = open(procpath,O_RDONLY);
	if (fd == -1) {
		util_fatal(procpath);
		return;
	}

	for (n_lines = 0, rv = 1; rv == 1; ) {
		rv = read(fd,&c,1);
		if (rv == 1 && c == '\n')
			++n_lines;
	}

	close(fd);

	if (n_lines <= 0)
		return;
	else if (n_lines > MAX_UTIL_CHKADDR_TABLE)
		n_lines = MAX_UTIL_CHKADDR_TABLE;

/* Secondo giro: riempimento della tabella. */

	fd = open(procpath,O_RDONLY);
	if (fd == -1) {
		util_fatal(procpath);
		return 0;
	}

	for (i = 0, rv = 1; rv == 1 && i < n_lines; ++i) {
	/* Le informazioni utili sono nei primi quattro campi:
	 start(8ch)-end(8ch) r(1)w(1).... */
#define USEFUL_FIELDS (8+1+8+1+2)
		rv = read(fd,line,USEFUL_FIELDS);
		if (rv != USEFUL_FIELDS) {
			break;
		}
	/* Si isolano i campi utili. */
		line[8] = '\0';
		line[8+1+8] = '\0';
		line[USEFUL_FIELDS] = '\0';
	/* Estrazione dei valori dai campi. */
		base = util_ahtol(&line[0], (char **) NULL);
		end = util_ahtol(&line[8+1], (char **) NULL);
		rp = line[8+1+8+1+0];
		wp = line[8+1+8+1+1];
	/* Riempimento della tabella. */
		_util_check_addr_st.table[i].area.base = (char *)base;
		_util_check_addr_st.table[i].area.end = (char *)end;
		_util_check_addr_st.table[i].rd = (rp == 'r');
		_util_check_addr_st.table[i].wr = (wp == 'w');
	/* Butta via il resto della linea. */
		do {
			rv = read(fd,&c,1);
		} while (rv == 1 && c != '\n');
	}

/* Nel caso fosse successo qualcosa di strano... */
	n_lines = i;

	close(fd);

	if (n_lines <= 0)
		return;

/* Adesso bisogna compattare la tabella e separare le aree leggibili
 da quelle scrivibili. Il problema e` che
 nulla impedirebbe all'esecutore di intervenire in qualsiasi momento per
 chiamare le "util_check_addr_...". Allora bisogna assicurarsi che
 l'assegnamento dei nuovi valori non provochi incongruenze. Si accetta
 allora una temporanea "sospensione del servizio" (ma il PLC e` ancora
 in HALT), che comunque non dovrebbe dare fastidio ai debugger.
  Se tutto cio` non dovesse bastare,
 si potrebbe aggiungere quella famosa righina in
 /usr/src/linux/drivers/char/mem.c, e tornare ad usare /dev/null, come
 si fa con LynxOS... */

	/* Azzeramento temporaneo della tabella. */
	_util_check_addr_st.table_r.n_area = 0;
	_util_check_addr_st.table_w.n_area = 0;

	for (i = 0; i < n_lines; ++i) {
#if 0
printf("area %08lx-%08lx %c%c\n",
		(unsigned long)_util_check_addr_st.table[i].area.base,
		(unsigned long)_util_check_addr_st.table[i].area.end,
		_util_check_addr_st.table[i].rd ? 'r' : '-',
		_util_check_addr_st.table[i].wr ? 'w' : '-');
#endif
		if (_util_check_addr_st.table[i].rd) {
			_util_check_addr_addarea(&_util_check_addr_st.table[i],
					&_util_check_addr_st.table_r);
		}
		if (_util_check_addr_st.table[i].wr) {
			_util_check_addr_addarea(&_util_check_addr_st.table[i],
					&_util_check_addr_st.table_w);
		}
	}

#if 0
printf("Readable sections:\n");
for (i = 0; i < _util_check_addr_st.table_r.n_area; ++i) {
printf(" %08lx-%08lx\n",
		(unsigned long)_util_check_addr_st.table_r.t_area[i].base,
		(unsigned long)_util_check_addr_st.table_r.t_area[i].end);
}
printf("Writable sections:\n");
for (i = 0; i < _util_check_addr_st.table_w.n_area; ++i) {
printf(" %08lx-%08lx\n",
		(unsigned long)_util_check_addr_st.table_w.t_area[i].base,
		(unsigned long)_util_check_addr_st.table_w.t_area[i].end);
}
#endif
}

int util_check_addr_init(void)
{
	util_check_addr_refresh();
	return 1;
}

int util_check_addr_rd(char * addr, int n)
{
	return (! main_status.chkaddr
                || _util_check_addr_generic(addr,n,0));
}

int util_check_addr_wr(char * addr, int n)
{
	return (! main_status.chkaddr
                || _util_check_addr_generic(addr,n,1));
}
#endif
#else
static int _util_check_addr_null_fd = -1;

void util_check_addr_refresh(void)
{
}

int util_check_addr_init(void)
{
	if (main_status.chkaddr) {
		_util_check_addr_null_fd = open("/dev/null",O_RDWR);
		if (_util_check_addr_null_fd < 0) {
			return 0;
		}
		util_check_addr_refresh();
	}
	return 1;
}

int util_check_addr_rd(char * addr, int n)
{
	return (! main_status.chkaddr
	        || write(_util_check_addr_null_fd,addr,n) >= 0);
}

int util_check_addr_wr(char * addr, int n)
{
	return (! main_status.chkaddr
                || read(_util_check_addr_null_fd,addr,n) >= 0);
}
#endif

/*
* Funzione "util_execprio"
* ------------------------
*
*  Questa funzione restituisce la priorita` dell'esecutore.
*/

int util_execprio(void)
{
    return main_status.start_prio + main_status.prio_exec;
}

/*
* Funzione "util_startprio"
* ------------------------
*
*  Questa funzione restituisce la priorita` di lancio del processo.
*/

int util_startprio(void)
{
    return main_status.start_prio;
}

/*
* Funzione "util_safe_ltob"
* -------------------------
*
*  Su Lynx non c'e` "ltob". Inoltre, sprintf usa malloc. E se non la usasse,
* non si sarebbe comunque sicuri di prenderci con la lunghezza della stringa.
*/

int util_safe_ltob(char *dest, int bsize, long v, int b)
{
int sign,neg,n,len;
char buff[40];
char *p;

	p = buff + sizeof(buff) - 1;
	*p = '\0';

	if ( (sign = b < 0) ) {
		b = -b;
		if (v < 0) {
			v = -v;
			neg = 1;
			sign = 0;
		}
		else {
			neg = 0;
		}
	}
	else {
		sign = 0;
		neg = 0;
	}

	do {
		if (sign) {
			n = (int)(v % b);
			v = v / b;
		}
		else {
			n = (int)((unsigned long)v % b);
			v = (long)((unsigned long)v / b);
		}
		*(--p) = n + (n > 9 ? 'a' - 10 : '0');
	} while (v);
	if (neg)
		*--p = '-';

	len = sizeof(buff) - (p - buff); 
	if (len > bsize) {
		len = bsize;
	}
	if (len)
		memcpy(dest,p,len);

	return len - 1;
}

/*
* Funzione "util_safe_gcvt"
* -------------------------
*
*  Versione di "gcvt" che non esce dai limiti della stringa destinazione.
*/

int util_safe_gcvt(char *buff, int bsize, double v, int b)
{
char tmp[64];
int len;

	if (b > sizeof(tmp) - 5)
		b = sizeof(tmp) - 5;
	gcvt(v, b, tmp);
	len = strlen(tmp);
	if (len >= bsize)
		len = bsize - 1;
	if (len >= 0)
		memcpy(buff,tmp,len + 1);
	return len;
}

/*
* Funzione "util_fpu_reset"
* -------------------------
*
*  Questa funzione rappezza un buco di Lynx, che dimentica di
* riprogrammare la maschera della eccezioni del coprocessore dopo
* la ricezione del segnale SIGFPE.
*/

/*
void util_fpu_reset(void)
{
#ifdef __GNUC__
__asm__ volatile (
	"finit;"
	"xorl	%%eax,%%eax;"
	"pushl	%%eax;"
	"fstcw	(%%esp);"
	"popl	%%eax;"
	"andl	$0xFFF3,%%eax;"
	"pushl	%%eax;"
	"fldcw	(%%esp);"
	"popl	%%eax;"
	: : );
#else
/-* Ma come fai senza gcc ? *-/
#endif
}
*/

/*
* Funzione "util_init_bstring"
* ---------------------------
*
*  Questa funzione prepara una struttura di tipo "bstring_t" e la
* associa al buffer "s" di dimensione "l".
*/

void util_init_bstring(bstring_t *b, char *s, int l)
{
	b -> string = s;
	b -> pos = s;
	b -> top = s + l - 1;
	*(b -> pos) = '\0';
	*(b -> top) = '\0';
}

/*
* Funzione "util_cat_bstring"
* ---------------------------
*
*  Questa funzione accoda la stringa "s" al buffer descritto da "b".
*/

void util_cat_bstring(bstring_t *b, char *s)
{
	while (*s && b -> pos < b -> top) {
		*(b -> pos) = *s;
		++(b -> pos);
		++s;
	}
	*(b -> pos) = '\0';
}

/*
* Funzione "util_addch_bstring"
* -----------------------------
*
*  Questa funzione accoda il carattere "c" al buffer descritto da "b".
*/

void util_addch_bstring(bstring_t *b, int c)
{
	if (b -> pos < b -> top) {
		*(b -> pos) = (char) c;
		++(b -> pos);
	}
	*(b -> pos) = '\0';
}

/*
* Funzione "util_gcvt_bstring"
* ----------------------------
*
*  Questa funzione accoda il risultato di "gcvt" al buffer descritto da "b".
*/

void util_gcvt_bstring(bstring_t *b, double d, int n)
{
int l;

	l = util_safe_gcvt(b -> pos, b -> top - b -> pos, d, n);
	if (l > 0)
		b -> pos += l;
	*(b -> pos) = '\0';
}

/*
* Funzione "util_ltob_bstring"
* ----------------------------
*
*  Questa funzione accoda il risultato di "ltob" al buffer descritto da "b".
*/

void util_ltob_bstring(bstring_t *b, long v, int n)
{
int l;

	l = util_safe_ltob(b -> pos, b -> top - b -> pos, v, n);
	if (l > 0)
		b -> pos += l;
	*(b -> pos) = '\0';
}

/*
* Funzione "util_vsprintf_bstring"
* --------------------------------
*
*  Semplice "vsprintf" applicata al buffer "b".
*/

void util_vsprintf_bstring(bstring_t *b, char *fmt, va_list a)
{
char c;
int base;

	while ( (c = *fmt) ) {
		if (c == '%') {
			c = *(++fmt);
			switch (c) {
			case 's':
				util_cat_bstring(b,va_arg(a,char *));
				break;
			case 'c':
				util_addch_bstring(b,va_arg(a,int));
				break;
			case 'b':
				base = 2;
				goto DO_LTOB;
			case 'o':
				base = 8;
				goto DO_LTOB;
			case 'x':
				base = 16;
				goto DO_LTOB;
			case 'd':
				base = -10;
				goto DO_LTOB;
			case 'u':
				base = 10;
			DO_LTOB:
				util_ltob_bstring(b, va_arg(a,long), base);
				break;
			case 'f':
				util_gcvt_bstring(b, va_arg(a,double), 6);
				break;
			case '0':
				c = '%';
				--fmt;
			default:
				util_addch_bstring(b,c);
				break;
			}
		}
		else {
			util_addch_bstring(b,c);
		}
		++fmt;
	}
}

/*
* Funzione "util_sprintf_bstring"
* -------------------------------
*
*  Questa funzione realizza una mini-sprintf sul buffer descritto da "b".
*  Si veda "util_safe_sprintf" per limiti e prestazioni.
*/

void util_sprintf_bstring(bstring_t *b, char *fmt, ...)
{
va_list args;

	va_start(args, fmt);
	util_vsprintf_bstring(b,fmt,args);
	va_end(args);
}

/*
* Funzione "util_safe_vsprintf"
* -----------------------------
*
*  Questa funzione realizza una mini-vsprintf "sicura", nel senso che
* evita di scrivere fuori dalla stringa data, e non chiama funzioni strane
* (malloc) come fa la "sprintf" ordinaria.
*/

char * util_safe_vsprintf(char *dest, int bsize, char *fmt, va_list a)
{
bstring_t b;

	util_init_bstring(&b, dest, bsize);
	util_vsprintf_bstring(&b, fmt, a);
	return dest;
}

/*
* Funzione "util_safe_sprintf"
* ----------------------------
*
*  Questa funzione realizza una mini-sprintf "sicura", nel senso che
* evita di scrivere fuori dalla stringa data, e non chiama funzioni strane
* (malloc) come fa la "sprintf" ordinaria.
*/

char * util_safe_sprintf(char *dest, int bsize, char *fmt, ...)
{
va_list args;

	va_start(args, fmt);
	util_safe_vsprintf(dest,bsize,fmt,args);
	va_end(args);
	return dest;
}

/*
* Funzione "util_usleep"
* ----------------------
*
*  Questa funzione rappezza l'ennesimo buco di Lynx. Realizza una "usleep"
* che non utilizza i segnali per funzionare. Fara` schifo, ma che alternative
* ci sono ?
*/

void util_usleep(long sec, long usec)
{
#ifdef Linux
long v = sec*1000000L+usec;
	usleep(v);
#else
static csem_t sem = (csem_t) 0;
struct timeval tm;

/* Piccolo problema di concorrenza. Ma molto piccolo. Basta chiamare
 questa funzione la prima volta in condizioni sicure. */
	if (! sem) {
		sem = csem_create_val(0);
	}

	tm.tv_sec = sec;
	tm.tv_usec = usec;

	csem_wait(sem, &tm);
#endif
}


/*
* Funzione "util_get_pc"
* ----------------------
*
*  Questa funzione Restituisce il PC del punto di chiamata del chiamate.
*/

/*
unsigned long util_get_pc(int level)
{
#ifdef __GNUC__
__asm__ volatile (
	"movl	%%ebp,%%eax;"
	"movl	(%%eax),%%eax;"
	"movl	4(%%eax),%%eax;"
	: : );
#else
/-* Ma come fai senza gcc ? *-/
#endif
}
*/

/*
* Funzione "util_call_il"
* ----------------------
*
*  Questa funzione serve a lanciare un programma o un blocco funzionale IL.
*  "prog" e` l'indirizzo del blocco, "statics" e` l'indirizzo della
* struttura contenente le variabili di stato ed i parametri del blocco.
*/

/*
unsigned long util_call_il(void (*prog)(void *statics), void *statics)
{
#ifdef __GNUC__
__asm__ volatile (
	"pushl	%%esi;"
	"pushl	%%edi;"
	"pushl	%%ebx;"
	"movl	12(%%ebp),%%eax;"
	"pushl	%%eax;"
	"movl	8(%%ebp),%%eax;"
	"call	%%eax;"
	"addl	$4,%%esp;"
	"popl	%%ebx;"
	"popl	%%edi;"
	"popl	%%esi;"
	: : );
#else
/-* Ma come fai senza gcc ? *-/
#endif
}
*/

/*
* Funzione "util_isrootgroup"
* ---------------------------
*
*  Questa funzione vale 0 se l'utente detentore del processo fa parte del
* gruppo del super-user (che ha il permesso di accedere direttamente alle
* porte di I/O).
*/

int util_isrootgroup(void)
{
#ifdef Linux
  	return 0;
#else
int n;
int gr[NGROUPS];

	if (getuid() == 0)
		return 1;

	n = NGROUPS;
	if (getgroups(&n,gr) == -1)
		return 0;
	while (n--) {
		if (gr[n] == 0)
			return 1;
	}
	return 0;
#endif
}

/*
* Funzioni "util_ahtol" e "util_adtol"
* ------------------------------------
*
*  Queste funzioni convertono una stringa esadecimale  o decimale in binario.
*/

static int util_htob(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - ('A' - 10);
	else if (c >= 'a' && c <= 'f')
		return c - ('a' - 10);
	else
		return -1;
}

unsigned long util_ahtol(char *s, char **end)
{
unsigned long u = 0;
register char c;

	for (;;) {
		c = util_htob(*s);
		if (c < 0)
			break;
		u = u * 16 + c;
		++s;
	}
	if (end)
		*end = s;
	return u;
}

unsigned long util_adtol(char *s, char **end)
{
unsigned long u = 0;
register char c;

	for (;;) {
		c = *s;
		if (c >= '0' && c <= '9')
			u = u * 10 + c - '0';
		else
			break;
		++s;
	}
	if (end)
		*end = s;
	return u;
}

/*
* Funzione "util_ahtom"
* ---------------------
*
*  Questa funzione converte una stringa esadecimale in binario.
*  Le cifre sono interpretate a gruppi di due come byte, che vengono
* copiati ad indirizzo crescenti in "buff", fino ad un limite di "n".
*/

int util_ahtom(char *s, char **end, char *buff, int n)
{
int i;
int c,u;

	if (n <= 0)
		return 0;
	memset(buff,0,n);
	for (i = 0; i < n; ) {
		u = util_htob(*s);
		if (u < 0)
			break;
		*buff = u;
		++i;
		++s;
		c = util_htob(*s);
		if (c < 0)
			break;
		*buff = u * 16 + c;
		++s;
		++buff;
	}
	if (end)
		*end = s;
	return i;
}

/*
* Funzioni "util_memset" e "util_memcpy"
* --------------------------------------
*
* Equivalgono a "memset" e "memcpy", ma non eseguono accessi a 32 bit.
* Da utilizzare per scrivere sul bus ISA.
*/

void util_memset(register char *d, register int v, register int n)
{
	while (n--)
		*(d++) = (char) v;
}

void util_memcpy(register char *d, register char *s, register int n)
{
	while (n--)
		*(d++) = *(s++);
}


/*
* Roba per quel cazzone di Linus Torvald
*/

#ifdef Linux

#if 1
#define N_SEMAFORI 16
static sem_t semafori[N_SEMAFORI];
static n_semafori = 0;
static pthread_once_t semaforo_allocato = PTHREAD_ONCE_INIT;
static void csem_alloc_master_sem(void)
{
	if (sem_init(&semafori[0],0,1) < 0) {
		util_fatal("sem_init(0)");
	}
	++n_semafori;
}
int csem_create_val(int ival)
{
int rv;
	if (n_semafori >= N_SEMAFORI) {
		errno = ENOSYS;
		rv = -1;
	}
	else {
		pthread_once(&semaforo_allocato,csem_alloc_master_sem);
		for (;;) {
			rv = sem_wait(&semafori[0]);
			if (rv == 0) {
				break;
			}
		}
		if (sem_init(&semafori[n_semafori],0,ival) == -1) {
			rv = -1;
		}
		else {
			rv = n_semafori;
			++n_semafori;
		}
		sem_post(&semafori[0]);
	}
	return rv;
}
int csem_create(void)
{
	return csem_create_val(1);
}
int csem_wait(int sem, struct timeval *tm)
{
int rv;

	if (tm) {
		util_fatal("wait, wait,...");
	}
	for (;;) {
		rv = sem_wait(&semafori[sem]);
		if (rv == 0) {
			break;
		}
		if (errno != EINTR) {
			util_fatal("sem_wait");
		}
	}
	return rv;
}
int csem_signal(int sem)
{
	return sem_post(&semafori[sem]);
}
#else
#define N_SEMAFORI 16
static int id_semafori = -1;
static int semafori[N_SEMAFORI] = { 0, };

static int cerca_sem(void)
{
int i;

	for (i = 0; i < N_SEMAFORI; ++i)
		if (semafori[i] == 0) {
			semafori[i] = 1;
			return i;
		}
	return -1;
}

int csem_create_val(int ival)
{
int rv;
union { int val; struct semid_ds *buf; unsigned short *array; } arg;
int i;

	if (id_semafori == -1) {
		id_semafori = semget(IPC_PRIVATE, N_SEMAFORI, IPC_CREAT | 0777);
		if (id_semafori == -1)
			util_fatal("semget");
		for (i = 0; i < N_SEMAFORI; ++i) {
			arg.val = 0;
			if (semctl(id_semafori, i, SETVAL, arg) < 0)
				util_fatal("semctl(2)");
		}
	}
	rv = cerca_sem();
	if (rv == -1)
		return 0;
	arg.val = ival;
	if (semctl(id_semafori, rv, SETVAL, arg) < 0)
		util_fatal("semctl(3)");
	return rv;
}

int csem_create(void)
{
	return csem_create_val(1);
}

int csem_wait(int sem, struct timeval *tm)
{
struct sembuf sops[1];

	if (tm) {
		util_fatal("wait, wait,...");
	}
	sops[0].sem_num = sem;
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;
	return semop(id_semafori,sops,1);
}

int csem_signal(int sem)
{
struct sembuf sops[1];

	sops[0].sem_num = sem;
	sops[0].sem_op = 1;
	sops[0].sem_flg = 0;
	return semop(id_semafori,sops,1);
}
#endif

int getprio(int n)
{
	return 40 - getpriority(PRIO_PROCESS,0);
}

#endif


