
/*
* @(#) remote.c 1.24 Thu Jan 24 16:07:53 MET 2002
*
*  Modulo server per controllo remoto di QPLC.
*
* 20/10/97 GG 1.0 Prima stesura.
* 20/11/97 GG 1.1 Prima versione stabilizzata. L'allocazione dei descrittori
*             di sessione e` ora dinamica, per risparmiare.
* 21/11/97 GG 1.2 Aggiunto il comando "ipcs", per ottenere le caratteristiche
*             degli oggetti IPC per il colloquio locale. 
* 24/11/97 GG 1.3 Utilizzata la funzione "shvKeyToAddr" per ottenere
*             l'indirizzo di un segnale di scambio data la chiave.
* 01/12/97 GG 1.4 Aggiunto il comando "info", che restituisce alcune utili
*             informazioni. Aggiunta la possibilita` di dimensionare il buffer
*             di trace col comando "trace m <numero di cicli>". Aggiunto il
*             comando "trace t" che serve a condizionare l'inizio del trace
*             al verificarsi di un'apposita condizione di trigger.
*             Aggiunta la gestione dei trigger (comando "trigger a|c|d ...").
*             "info" ora restituisce anche il munero di trigger massimo per
*             sessione.
* 09/12/97 GG 1.5 Corretto un errore nella lettura dei parametri di "wcreate".
* 13/03/98 GG 1.6 Aggiunti i comandi "in <ingresso> <numero di bit>" e
*             "out <uscita> <numero di bit> <valore>" per leggere o scrivere
*             valori sul segnale di I/O specificato.
*             Spezzato in alcune sottofunzioni il comando "VAR". Le
*             sottofunzioni di risoluzione die nomi potranno servire piu`
*             avanti.
* 24/03/98 GG 1.7 Aggiunto il comando "list", sperimentalissimo.
* 03/04/98 GG 1.8 Aggiunto il comando "iostat", per ottenere lo stato di
*             un segnale di I/O in modo non invasivo, anche con esecutore
*             in GO.
* 04/09/98 GG 1.9 Utilizzata la nuova funzione "util_startprio" al posto
*             "getprio(0)".
* 22/09/98 GG 1.10 Corretto un buco nella risoluzione degli indirizzi di
*             simboli appartenenti alla sezione COMMON.
*             Aggiunto il comando "set", che assegna un valore ad una
*             variabile.
* 24/09/98 GG 1.11 Aggiunto il parametro "p" al comando list. Visualizza
*             la lista delle associazioni di I/O correnti.
*             Aggiunto il comando "setin" per simulare il valore di un
*             input.
* 28/09/98 GG 1.12 Generalizzata la procedura di traduzione delle variabili.
*             Come risultato, ora e` disponibile una funzione generica di
*             esplorazione sequenziale della symbol table di un file oggetto.
*             Eliminati alcuni parametri inutili in alcune funzioni interne.
*             Aggiunto il comando "sym" (lavori in corso).
* 10/11/98 GG 1.13 Modificata la procedura di ricerca di un nome dato
*             l'indirizzo in modo da privilegiare i nomi che non iniziano
*             per "." (come ".data").
* 13/07/00 GG 1.14 Corretto un incredibile errore nella ricerca del comando
*             nella tabelle di associazione nome-funzione. Provocava
*             schianti se si dava una stringa maggiore dell'ultimo nome
*             presente.
* 28/07/00 GG 1.15 Aggiustamenti per compilazione su Linux.
* 28/07/00 GG 1.16 Aggiunti eventi asincroni per cancellazione errori
*             e gestione messaggi, e il comando "messages", che funziona come
*             "errs" ma e` relativo ai messaggi.
* 02/11/00 GG 1.17 Corretto un errore nel comando "errs", introdotto nella
*             versione precedente.
* 03/11/00 GG 1.18 Aggiunto, per sicurezza, l'azzeramento della struttura
*             usata come indirizzo IP per la system call "bind". Solo
*             per pulizia formale.
*             Aggiunta l'abilitazione del "KEEP ALIVE" sulle connessioni,
*             utile per teleassistenza.
*             Tolto in controllo su "ECONNRESET" nella via di errore
*             di ricezione. Se arriva quell'errore, si chiude.
*             Aggiunto il comando "whistory", che serve ad ottenere
*             il contenuto del buffer di debug e a cambiare la
*             modalita` di funzionamento del debug da "normale"
*             (registrazione degli eventi di ogni ciclo) a "compressa"
*             (registrazione di un solo evento per ogni lista).
* 04/11/00 GG 1.19 Eliminato un parametro inutile di "brkHistory".
* 07/11/00 GG 1.20 Aggiunte le opzioni "v" ed "l" al comando "errs".
*             Servono ad avere il dump degli errori in forma binaria ("v")
*             o umanamente leggibile ("l", buona solo per debug).
* 22/01/01 GG 1.21 Portata a 1024 byte la massima lunghezza di un comando,
*             e a 64 il massimo numero di celle gestibili dal comando memchk.
* 13/02/01 GG 1.22 Realizzata REALMENTE la modifica che credevo di avere
*             fatto nella versione precedente.
* 24/01/02 GG 1.23 Eliminati alcuni warning aggiungendo la proprieta`
*             "const" a vari puntatori.
* 17/09/02 RM 1.24 Aumentati i limiti per la comunicazione via socket:
*             #define MAX_CMD 2048        
*             #define MAX_DSV_D_LINE 2048
*             #define MAX_MEMS 512
*/

/* La solita pezza schifosa per far convivere socket e posix. */
#ifdef Linux
#else
#ifdef _POSIX_SOURCE
#undef _POSIX_SOURCE
#define _posix_
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#ifdef Linux
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#else
#include <types.h>
#include <time.h>
#include <socket.h>
#endif
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>

#ifdef Linux
#include "lynx-a.out.h"
#else
#include <a.out.h>
#endif

#include "qplc.h"
#include "superv.h"
#include "debugger.h"
#include "cback.h"
#include "util.h"

#ifdef _posix_
#undef _posix_
#define _POSIX_SOURCE
#endif

#include <pthread.h>
#ifdef Linux
#include <sys/sem.h>
#else
#include <sem.h>
#endif

#include "remote.h"

#if 0
#define UN_BYTE_ALLA_VOLTA
#endif

#define SERVICE_NAME PLCREM_SERVICE_NAME
#define SERVICE_NUMBER PLCREM_SERVICE_NUMBER
#define MAX_CLIENT 8
#define MAX_CMD 2048
#define MAX_DSV_D_LINE 2048
#define MAX_MEMS 512
#define MAX_TRACE_BUFFER (5 * 128)
#define TRACE_BUFFER_SIZE (2 * MAX_TRACE_BUFFER)
#define MAX_TRACE_VAR (sizeof(trace_event_t) * 8)
#define MAX_TRIGGER_VAR 4

typedef unsigned long trace_event_t;

/* Descrittore di un'area da sorvegliare. */
typedef struct {
	unsigned char *addr;
	int len;
} dsv_mem_t;

/* Descrittore di una variabile di trigger. */
typedef union {
	signed char sch;
	unsigned char uch;
	signed short ssh;
	unsigned short ush;
	signed long sl;
	unsigned long ul;
	float f;
} dsv_anytype_t;
typedef struct _dsv_trig_t {
	dsv_anytype_t *a;
	dsv_anytype_t v;
	int id;
	int act;
	int (*cmp)(struct _dsv_trig_t *);
} dsv_trig_t;
typedef struct {
	int id;
	struct session_st *t;
	unsigned long v;
} dsv_trig_event_t;

#if MAX_CLIENT <= 256
typedef unsigned char client_index_t;
#else
typedef unsigned short client_index_t;
#endif

/* Descrizione della struttura di stato di una sessione. */

typedef struct session_st {
/* Indirizzo del prossimo descrittore. */
	struct session_st * next;
/* Indice del descrittore. */
	int index;
/* File descriptor. */
	int fd;
/* Numero di campi (stringhe) contenuti nel comando. */
	int argc;
/* Puntatore agli argomenti (comando escluso). */
	char *args;
/* Comando in corso di costruzione. */
	char line[MAX_CMD];
/* Posizione di inserimento del prossimo carattere ricevuto. */
	int lastch;
/* Maschera di sensibilita` agli eventi asincroni. */
	int flags;
/* Aree impostate con memchk e loro numero. */
	dsv_mem_t mems[MAX_MEMS];
	int nmems;
/* Buffer di trace. */
	trace_event_t tracebuf[TRACE_BUFFER_SIZE];
/* Tabella delle variabili sottoposte a trace. */
	char *tracetab[MAX_TRACE_VAR];
/* Numero di cicli immagazzinati per ogni evento di trace. */
	int trace_len;
/* Numero di variabili sottoposte a trace. */
	int ntrace;
/* Posizione di inserimento del prossimo evento di trace. */
	int trace_pos;
/* Numero di cicli immagazzinati nel buffer corrente. */
	int trace_count;
/* Posizione del prossimo blocco di trace. */
	int trace_bnext;
/* Flag di (semi)buffer di trace leggibile. */
	int trace_done;
/* Condizione di attivazione del trace (ad uso dei trigger). */
	int trace_cond;
/* Prossimo elemento nella lista di trace. */
	struct session_st *trace_next;
/* Ciclo dell'ultimo evento caricato col comando "wshow". */
	unsigned long last_watch_cycle;
/* Ultimo trigger ID utilizzato. */
	int trigger_id;
/* Numero di trigger definiti. */
	int ntrigger;
/* Tabella dei descrittori di trigger. */
	dsv_trig_t trigtab[MAX_TRIGGER_VAR];
/* Prossimo elemento nella lista di trigger. */
	struct session_st *trigger_next;
/* Ultimo nodo dell'albero dei dispositivi ottenuto col comando "list". */
	devnode_t ionode;
} session_t;

/*
* Significato dei flag di sessione.
*/
/* Il client vuole essere informato sugli errori di Plc. */
#define DSV_SESSION_F_ERRORS 0x01

/*
* Tipo da utilizzare nella costruzione di una risposta di tipo "D".
*/

typedef struct {
	session_t *t;
	int len;
	char data[MAX_DSV_D_LINE + 2]; /* caratteri + \n + \0 */
} dsv_answ_t;

/* Descrizione della struttura di stato di questo modulo. */

typedef struct {
/* Insieme dei file descriptor canali aperti. */
	fd_set open_chans;
/* Insieme dei file descriptor pronti per la ricezione. */
	fd_set ready_chans;
/* Massimo indice di file descriptor attivo. */
	int maxfd;
/* File descriptor per l'accoglimento di richieste di connessione. */
	int server_fd;
/* TID del thread incaricato della gestione del colloquio. */
	pthread_t tid;
/* Tabella di conversione file descriptor/indice nella tabella dei servizi
 (largamente sovradimensionata). */
	client_index_t client_index[FD_SETSIZE];
/* Descrittori di stato delle connessioni attive. */
	session_t *trans[MAX_CLIENT];
/* Numero di descrittori (liberi o occupati) gia` allocati. */
	int n_alloc_trans;
/* Indirizzo del primo descrittore libero. */
	session_t * free;
/* Indirizzo del primo descrittore occupato. */
	session_t * tlist;
/* Semaforo di accesso alle liste. */
	csem_t sem;
/* Lista dei client interessati al trace. */
	session_t * trace_list;
/* Lista dei client interessati al trigger. */
	session_t * trigger_list;
} remote_sts_t;

/**************************************\
* Struttura di stato di questo modulo. *
\**************************************/

static remote_sts_t dsv_sts;

/*
* Aggiunta del file descriptor "fd" alla lista di quelli attivi.
*/

static void dsv_add_fd(int fd)
{
	FD_SET(fd, &dsv_sts.open_chans);
	if (dsv_sts.maxfd < fd)
		dsv_sts.maxfd = fd;
}

/*
* Chiusura e rimozione del file descriptor "fd" dalla lista di quelli attivi.
*/

static void dsv_remove_fd(int fd)
{
int i,m;

	close(fd);
	FD_CLR(fd, &dsv_sts.open_chans);
	for (i = m = 0; i <= dsv_sts.maxfd; ++i) {
		if (FD_ISSET(i,&dsv_sts.open_chans))
			m = i;
	}
	dsv_sts.maxfd = m;
}

/*
* Apertura del canale server. Vale 0 se tutto e` andato bene, -1
* in caso di errori.
*/

static int dsv_open_socket(void)
{
int on,rv;
struct sockaddr_in s_in;
struct servent *service;
int service_number;
int e;

/* Ottenimento del numero di servizio. */
	service = getservbyname((char *)SERVICE_NAME,(char *)"tcp");
	if (service)
		service_number = service -> s_port;
	else
		service_number = htons(SERVICE_NUMBER);

/* Creazione del canale server. */
	dsv_sts.server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (dsv_sts.server_fd < 0) { e = __LINE__-1; goto open_socket_ERROR; }

	on = 1;
	rv = setsockopt(dsv_sts.server_fd,SOL_SOCKET,SO_REUSEADDR,
	                (char *)&on,sizeof(on));
	if (rv < 0) { e = __LINE__-1; goto open_socket_ERROR; }

	memset((char *)&s_in,0,sizeof(s_in));
	s_in.sin_family = AF_INET;
	s_in.sin_addr.s_addr = INADDR_ANY;
	s_in.sin_port = service_number;

	rv = bind(dsv_sts.server_fd,(struct sockaddr *)&s_in,sizeof(s_in));
	if (rv < 0) { e = __LINE__-1; goto open_socket_ERROR; }

/* Configurazione della coda di connessione. */
	rv = listen(dsv_sts.server_fd,MAX_CLIENT);
	if (rv < 0) { e = __LINE__-1; goto open_socket_ERROR; }

/* Aggiunta del file descriptor alla lista. */
	dsv_add_fd(dsv_sts.server_fd);

	return 0;

/* Errori vari. Manda su STDERR un messaggio che contiene il
 numero di linea ed il codice di errore. */
open_socket_ERROR:

	fprintf(stderr,"%s: %s:%d: %s\n", main_status.err_sigla,__FILE__,e,strerror(errno));

	close(dsv_sts.server_fd);

	return -1;
}

/*
* Trasmissione di dati al client.
*/

static void dsv_send(int fd, const char *s, int nb)
{
int rv;

	while (nb) {
#if 0
		rv = send(fd,s,(unsigned int)nb,0);
#else
		rv = write(fd,s,(unsigned int)nb);
#endif
		if (rv < 0) {
			perror("write(socket)");
			break;
		}
		nb -= rv;
		s += rv;
	}
}

/*
* Invio di una risposta in caso di errore.
* Il messaggio ha la forma "E <numero><LF>".
*/

static void dsv_error(session_t *t, int code)
{
char msg[16];

	util_safe_sprintf(msg,sizeof(msg)-1,"E %x\n",code);
	dsv_send(t -> fd, msg, (int)strlen(msg));
}

/*
* Inizializza una stringa per una risposta di tipo "c".
*/

static void dsv_answ_init(dsv_answ_t *a, session_t *t, int c)
{
	a -> data[0] = c;
	a -> data[1] = ' ';
	a -> data[2] = '\0';
	a -> len = 2;
	a -> t = t;
}

/*
* Inizializza una stringa per una risposta di tipo "D" (dati sincroni).
*/

static void dsv_answ_init_data(dsv_answ_t *a, session_t *t)
{
	dsv_answ_init(a, t, 'D');
}

/*
* Trasmette una risposta e reinizializza il descrittore in modo
* che sia pronto per un'eventuale continuazione.
*/

static void dsv_answ_flush(dsv_answ_t *a)
{
	if (a -> len < 3)
		return;
	a -> data[a -> len] = '\n';
	a -> data[++(a -> len)] = '\0';
	dsv_send(a -> t -> fd, (unsigned char *) a -> data, a -> len);
	dsv_answ_init(a, a -> t, a -> data[0]);
}

/*
* Trasmette una risposta col carattere di continuazione impostato.
*/

static void dsv_answ_cflush(dsv_answ_t *a)
{
	a -> data[1] = '-'; /* Carattere di continuazione. */
	dsv_answ_flush(a);
}

/*
* Aggiunge "n" caratteri ad una risposta in corso di costruzione.
* Se non c'e` piu` spazio, trasemette la risposta e la reinizializza.
*/

static void dsv_answ_addchrs(dsv_answ_t *a, const char *s, int l)
{
int m;

	if (l <= 0)
		return;
	m = MAX_DSV_D_LINE - a -> len;
	if (l > m) {
		dsv_answ_cflush(a);
		m = MAX_DSV_D_LINE - a -> len;
		if (l > m)
			return;
	}
	memcpy(&a -> data[a -> len], s, (unsigned int) l);
	a -> len += l;
}

/*
* Aggiunge i caratteri "0 " o "1 " a seconda che "f" valga 0 o no.
*/

static void dsv_answ_addflag(dsv_answ_t *a, int f)
{
	dsv_answ_addchrs(a,f ? "1 " : "0 ",2);
}

/*
* Aggiunge una stringa ad una risposta in corso di costruzione.
* Se non c'e` piu` spazio, trasemette la risposta e la reinizializza.
*/

static void dsv_answ_addstr(dsv_answ_t *a, const char *s)
{
	dsv_answ_addchrs(a,s,(int)strlen(s));
}

/*
* Aggiunge una stringa esadecimale rappresentante il valore puntato da "p"
* di "n" byte. Se "b" vale 0, i byte sono letti nell'ordine in cui sono
* memorizzati, e la linea puo` venire spezzata in qualsiasi punto per
* continuare sulla successiva.
*/

static void dsv_answ_addhv(dsv_answ_t *a, unsigned char *p, int n, int b)
{
int l,m,izero;
char *s;
unsigned char c,h;
static char htab[] = "0123456789ABCDEF";

	l = n*2;
	m = MAX_DSV_D_LINE - a -> len;
	if (l > m && b) {
		dsv_answ_cflush(a);
		m = MAX_DSV_D_LINE - a -> len;
		if (l > m)
			return;
	}
	if (b) {
		p += n;
		izero = 1;
	}
	else {
		--p;
		izero = 0;
	}
	for (s = &a -> data[a -> len]; n--; ) {
		if (b) {
			c = *(--p);
			if ((! c) && izero)
				continue;
		}
		else {
			if (m <= 0) {
				dsv_answ_cflush(a);
				m = MAX_DSV_D_LINE - a -> len;
				s = &a -> data[a -> len];
			}
			m -= 2;
			c = *(++p);
		}
		if ((h = (c & 0xF0) >> 4) || !izero) {
			*(s++) = htab[h];
			++(a -> len);
		}
		*(s++) = htab[c & 0x0F];
		++(a -> len);
		izero = 0;
	}
	if (izero) {
		*s = '0';
		++(a -> len);
	}
}

/*
* Emette una risposta di tipo "L" e
* inizializza una una risposta di tipo "D" (dati sincroni).
*/

static void dsv_answ_init_long_data(dsv_answ_t *a, session_t *t, long len)
{
	dsv_answ_init(a, t, 'L');
	dsv_answ_addhv(a, (unsigned char *) &len, sizeof(len), 1);
	dsv_answ_flush(a);
	dsv_answ_init_data(a, t);
}

/*
* Aggiunge alla risposta i valori secondo il formato dato.
* "%s" indica una stringa, "%x" un valore esadecimale ad n byte, "%b" una
* sequenza di n byte esadecimali. "%%" e` il carattere "%" singolo.
*/

static void dsv_answ_addfmt(dsv_answ_t *a,const char *fmt,...)
{
char c;
const char *s;
unsigned char *p;
int n,spec;
va_list args;

	va_start(args,fmt);
	n = 0;
	s = fmt;
	spec = 0;
	while ( (c = *fmt) ) {
		if (spec) {
			n = 0;
			spec = 0;
			s = fmt + 1;
			switch (c) {
			case '%':
				--s;
				++n;
				break;
			case 'x':
				p = va_arg(args,unsigned char *);
				dsv_answ_addhv(a,p,va_arg(args,int),1);
				break;
			case 'b':
				p = va_arg(args,unsigned char *);
				dsv_answ_addhv(a,p,va_arg(args,int),0);
				break;
			case 's':
				dsv_answ_addstr(a,va_arg(args,char *));
				break;
			}
		}
		else if (c == '%') {
			dsv_answ_addchrs(a,s,n);
			n = 0;
			spec = 1;
		}
		else {
			++n;
		}
		++fmt;
	}
	dsv_answ_addchrs(a,s,n);
	va_end(args);
}

/*
* Inizializza una stringa per una risposta di tipo "A" (eventi asincroni).
*/

static void dsv_answ_init_event(dsv_answ_t *a, session_t *t, int code)
{
	dsv_answ_init(a, t, 'A');
	dsv_answ_addhv(a,(unsigned char *)&code , sizeof(code), 1);
}

/* Emette un record dati completo contenente un valore long. */
static void dsv_answ_long(session_t *t, long val)
{
dsv_answ_t a;

	dsv_answ_init_data(&a, t);
	dsv_answ_addhv(&a,(unsigned char *)&val , sizeof(val), 1);
	dsv_answ_flush(&a);
}

/*
* Invio del prompt. Se "code" < 0, non fa nulla,
* se = 0, il messaggio sara` "OK<LF>",
* se maggiore di zero, sara` un messaggio di errore.
*/

static void dsv_prompt(session_t *t, int code)
{
	if (code < 0)
		;
	else if (code)
		dsv_error(t,code);
	else
		dsv_send(t -> fd, "OK\n", 3);
}

/*
* Cancellazione della linea di comando relativa alla sessione "t".
* I caratteri successivi faranno parte di una nuova linea.
*/

static void dsv_clear_line(session_t *t)
{
	t -> line[0] = '\0';
	t -> lastch = 0;
	t -> argc = 0;
}

/*
* Inizio di una nuova sessione. "fd" e` il file descriptor del
* nuovo canale client. Viene riservato un descrittore di sessione
* associato al file descriptor.
* La funzione vale 1 in caso di successo, 0 se non ci sono piu`
* descrittori liberi.
*/

static int dsv_start_trans(int fd)
{
session_t *t;
int rv;

	csem_wait(dsv_sts.sem,(struct timeval *)NULL);

/* Prende un descrittore dalla lista di quelli liberi. */
	t = dsv_sts.free;
	if ((! t) && dsv_sts.n_alloc_trans < MAX_CLIENT) {
	/* Se non ci sono descrittori liberi, tenta di allocarne dinamicamente
	 uno nuovo. */
		t = (session_t *) malloc(sizeof(*t));
		if (t) {
		/* Aggiunge un nuovo elemento alla tabella dei
		 descrittori allocati. */
			t -> next = (session_t *) 0;
			t -> index = dsv_sts.n_alloc_trans;
			dsv_sts.trans[dsv_sts.n_alloc_trans] = t;
			++dsv_sts.n_alloc_trans;
		}
	}
	if (t) {
	/* Sposta l'elemento dalla lista dei liberi a quella
	 delle sessioni aperte. Se il descrittore e` stato allocato
	 ex novo, il campo "next" punta a NULL, valore che viene copiato
	 in "free". */
		dsv_sts.free = t -> next;
		t -> next = dsv_sts.tlist;
		dsv_sts.tlist = t;
	/* Registra il file descriptor. */
		t -> fd = fd;
		dsv_add_fd(fd);
	/* Aggiorna la tabella di conversione file descriptor/sessione. */
		dsv_sts.client_index[fd] = t -> index;
	/* Inizializza lo stato della sessione. */
		t -> flags = (1 << DSV_EVENT_HALT)
		           | (1 << DSV_EVENT_FAULT)
		           | (1 << DSV_EVENT_GO);
		t -> nmems = 0;
		t -> trigger_id = 0;
		t -> ntrigger = 0;
		t -> trace_next = (session_t *) NULL;
		t -> ntrace = 0;
		t -> trace_pos = 0;
		t -> trace_count = 0;
		t -> trace_bnext = 0;
		t -> trace_done = 0;
		t -> trace_cond = 0;
		t -> trace_len = MAX_TRACE_BUFFER;
		t -> last_watch_cycle = plcNCycle();
		t -> ionode.ideep = 0;
		dsv_clear_line(t);
	/* Emette un "OK" beneaugurante. */
		dsv_prompt(t,0);
		rv = 1;
	}
	else {
/* Errore se non si e` trovato un descrittore libero. */
		rv = 0;
	}

	csem_signal(dsv_sts.sem);

	return rv;
}

/* Abilitazione trace, non protetta da semaforo. */
static int dsv_enable_trace(session_t *t)
{
	if (!(t -> flags & (1 << DSV_EVENT_TRACE)) && t -> ntrace) {
		t -> trace_next = dsv_sts.trace_list;

	/* SEZIONE CRITICA ! Si fa l'ipotesi che l'assegnamento seguente
	 avvenga in modo ininterrompibile. Su piattaforma Intel, in modalita`
	 protetta a 32 bit, questo e` vero. Comunque, occhio ! */

		dsv_sts.trace_list = t;

	/* Fine della sezione critica. */

		t -> flags |= (1 << DSV_EVENT_TRACE);
		return 1;
	}
	else
		return 0;
}

/* Disabilitazione trace, non protetta da semaforo. */
static void dsv_disable_trace(session_t *t)
{
register session_t *p,**q;

	t -> flags &= ~(1 << DSV_EVENT_TRACE);

	for (q = &dsv_sts.trace_list, p = *q;
	     p;
	     q = &p -> trace_next, p = *q) {
		if (p == t) {

/* SEZIONE CRITICA ! Si fa l'ipotesi che l'assegnamento seguente
 avvenga in modo ininterrompibile. Su piattaforma Intel, in modalita`
 protetta a 32 bit, questo e` vero. Comunque, occhio ! */

			*q = t -> trace_next;

/* Fine della sezione critica. */

			return;
		}
	}
}

/* Abilitazione trigger, non protetta da semaforo. */
static int dsv_enable_trigger(session_t *t)
{
	if (!(t -> flags & (1 << DSV_EVENT_TRIGGER)) && t -> ntrigger) {
		t -> trigger_next = dsv_sts.trigger_list;

	/* SEZIONE CRITICA ! Si fa l'ipotesi che l'assegnamento seguente
	 avvenga in modo ininterrompibile. Su piattaforma Intel, in modalita`
	 protetta a 32 bit, questo e` vero. Comunque, occhio ! */

		dsv_sts.trigger_list = t;

	/* Fine della sezione critica. */

		t -> flags |= (1 << DSV_EVENT_TRIGGER);
		return 1;
	}
	else
		return 0;
}

/* Disabilitazione trigger, non protetta da semaforo. */
static void dsv_disable_trigger(session_t *t)
{
register session_t *p,**q;

	t -> flags &= ~(1 << DSV_EVENT_TRIGGER);

	t -> ntrigger = 0;

	for (q = &dsv_sts.trigger_list, p = *q;
	     p;
	     q = &p -> trigger_next, p = *q) {
		if (p == t) {

/* SEZIONE CRITICA ! Si fa l'ipotesi che l'assegnamento seguente
 avvenga in modo ininterrompibile. Su piattaforma Intel, in modalita`
 protetta a 32 bit, questo e` vero. Comunque, occhio ! */

			*q = t -> trigger_next;

/* Fine della sezione critica. */

			return;
		}
	}
}

/*
* Chiusura di una sessione.
* Il file descriptor e` chiuso e rimosso dalla lista di quelli attivi,
* il descrittore e` liberato.
*/

static void dsv_end_trans(session_t *t)
{
register session_t *p,**q;
int fd;

	if (t) {
		csem_wait(dsv_sts.sem,(struct timeval *)NULL);

	/* Questo segnalera` che il descrittore di transaizone non e` piu`
	 valido. */
		fd = t -> fd;
		t -> fd = -1;
	/* Chiusura e rimozione del canale. */
		dsv_remove_fd(fd);
	/* Ritiro di tutte le definizioni legate al canale (trace,
	 trigger, sensibilita` agli errori, ...). */
		dsv_disable_trace(t);
		dsv_disable_trigger(t);
		t -> flags = 0;
	/* Spostamento del descrittore di sessione dalla liste di
	 quelli attivi alla lista dei liberi. */
		for (q = &dsv_sts.tlist, p = *q; p; q = &p -> next, p = *q) {
			if (p == t) {
				*q = t -> next;
				break;
			}
		}
		t -> next = dsv_sts.free;
		dsv_sts.free = t;

		csem_signal(dsv_sts.sem);
	}
}

/*
* Gestione della richiesta di una nuova connessione.
* Se ci sono descrittori di sessione liberi, la richiesta viene accolta
* ed e` aperta una nuova sessione, altrimenti la connessione viene
* chiusa immediatamente.
*/

static void dsv_new_client(void)
{
int new,on;
int fromsize = sizeof(struct sockaddr_in);
struct sockaddr_in from;

	new = accept(dsv_sts.server_fd, (struct sockaddr *)&from, &fromsize);
	if (new >= 0) {

		on = 1;
		setsockopt(new,IPPROTO_TCP,TCP_NODELAY,(char *)&on,sizeof(on));
#if 1
		on = 1;
		setsockopt(new,SOL_SOCKET,SO_KEEPALIVE,(char *)&on,sizeof(on));
#endif

		if (! dsv_start_trans(new))
			close(new);
	}
}

/*
* Esecuzione di un comando specifico del supervisore (0 argomenti).
*/

static void dsv_sv_exec_0(int cmd)
{
sv_msg_t msg;

	msg.cmd = cmd;
	msg.len = 0;
	superv_exec(&msg);
}

/*
* Esecuzione di un comando specifico del supervisore (1 argomento int,
* risposta int).
*/

static int dsv_sv_exec_1(int cmd, int val)
{
sv_msg_t msg;

	msg.cmd = cmd;
	*(int *)(msg.data) = val;
	msg.len = sizeof(int);
	superv_exec(&msg);
	if (*(int *)(msg.data) < 0)
		return DSV_E_BADARG;
	else
		return 0;
}

/************ Funzioni per la risoluzione dei nomi ***************/

/*
* Risoluzione di un segnale di scambio.
*/
static int dsv_decode_shvar(char *name, int l, int i1, int i2, shv_t *pv,
                            int *pkey, long *paddr, int *ptype)
{
shv_t v;
char *p,c;
int key;

/* Se il nome contiene almeno un carattere minuscolo, o e` piu`
 lungo del limite imposto, non puo` essere un segnale di scambio. */
	for (p = name; (c = *p); ++p)
		if (c >= 'a' && c <= 'z')
			break;
	if (*p || l > MAX_SHV_NAME)
		return 0;

/* Cerca il descrittore del segnale di scambio, se esiste. */
	strcpy(v.name,name);
	v.mode = SHV_MODE_TEST;
	key = shvDefine(&v,(int *)NULL);
	if (key < 0)
		return 0;

/* Controlla gli indici. Errore se sono fuori range. */
	if ((unsigned int)i1 >= (unsigned int)v.dim1
	 || (unsigned int)i2 >= (unsigned int)v.dim2)
		return DSV_E_BADARG;

/* Emette il risultato. */
	*pv = v;
	*paddr = (long)shvKeyToAddr(key,i1,i2);
	*pkey = key + i2 * v.dim1 + i1;
	*ptype = v.mode & ~(SHV_MODE_TEST | SHV_MODE_EXCL);

	return -1;
}

/*
* Applicazione di una funzione generica agli elementi della symbol
* table di un oggetto.
*/

/* Struttura di controllo della funzione da applicare iterativamente
 agli elementi della symbol table. */
typedef struct {
	FILE *file;
	long str_pos;
	long sym_pos;
	int nsym;
	char *text;
	char *common;
	void *statics;
} symtab_iterator_t;

static int dsv_explore_symtab(char *file,
                              char *text, char *common,
                              int (*fn)(symtab_iterator_t *,
                                        struct nlist *,
                                        void *),
                              int *ipos,
                              void *statics)
{
struct nlist sym;
struct exec head;
int i,rv,ii;
symtab_iterator_t it;

/* Bisogna analizzare la symbol table per calcolare la
 posizione di tutte le variabili "COMMON" che la precedono. */

	it.text = text;
	it.common = common;
	it.statics = statics;

	it.file = fopen(file,"r");

	if (! it.file
	 || fread((char *)&head,sizeof(head),1,it.file) != 1
	 || ! head.a_syms)
		goto OBJ_ERROR;

/* Calcola la posizione della symbol table e della string table. */
	it.sym_pos = head.a_text + head.a_data
		   + head.a_trsize + head.a_drsize + sizeof(head);
	it.str_pos = it.sym_pos + head.a_syms;

	it.nsym = head.a_syms / sizeof(struct nlist);

	if (ipos)
		ii = *ipos;
	else
		ii = 0;

	if (ii >= it.nsym)
		goto OBJ_ERROR;

	if (fseek(it.file,(long)(it.sym_pos + ii * sizeof(struct nlist)),0))
		goto OBJ_ERROR;

/* Legge la symbol table e incrementa "common" della dimensione
 di ogni simbolo della sezione COMMON. Si ferma quando trova
 il simbolo "name". */

	for (i = ii, rv = 0; i < it.nsym; ++i) {

		if (fread(&sym, sizeof(sym), 1, it.file) != 1)
			goto OBJ_ERROR;

	/* La funzione deve valere 0 per indicare "errore",
	 "-1" per indicare "fine esplorazione", 1  per indicere "continua". */

		rv = (*fn)(&it,&sym,statics);

		if (rv == 0)
			goto OBJ_ERROR;
		else if (rv == -1)
			break;
	}

	fclose(it.file);

	if (ipos)
		*ipos = i;

	return -1;

OBJ_ERROR:
	if (it.file)
		fclose(it.file);
	return 0;
}

/* Elaborazione di un elemento della symbol table nell'assegnamento
 dell'indirizzo ad un simbolo della sezione "common". */

/* Struttura di stato della funzione iterativa. */
typedef struct {
	char *name;
	char *common;
	int found;
} ivar_common_tester_t;

static int dsv_decode_ivar_common_element(symtab_iterator_t *it,
                                          struct nlist *sym,
                                          void *sts)
{
long pos;
char c,*p;
ivar_common_tester_t *statics = (ivar_common_tester_t *)sts;

/* Se il simbolo corrente non e` "common", passa oltre. */
	if ((sym -> n_type & N_TYPE) != N_UNDF || sym -> n_value == 0)
		return 1;

/* Cerca nella string table il nome del simbolo
 e lo confronta con "name". */

	pos = ftell(it -> file);
	if (fseek(it -> file, it -> str_pos + sym -> n_un.n_strx,0))
		return 0;

	p = statics -> name - 1;
	do {
		++p;
		c = fgetc(it -> file);
	} while (*p && c == *p);

/* Trovato ! */
	if (c == *p) {
		statics -> found = 1;
		return -1;
	}

/* Incrementa del valore del simbolo la variabile di stato "common". */
	statics -> common += sym -> n_value;

/* Si riposiziona dove era prima. */
	if (fseek(it -> file,pos,0))
		return 0;

	return 1;
}

/* Risoluzione di un simbolo interno nella sezione "common". */
static int dsv_decode_ivar_common(char *name, char *file,
                                  char *text, char *common,
                                  unsigned long *paddr)
{
ivar_common_tester_t sts;
int rv;

	sts.name = name;
	sts.common = common;
	sts.found = 0;
	rv = dsv_explore_symtab(file, text, common,
	                        dsv_decode_ivar_common_element,
	                        (int *)NULL,
	                        (void *)&sts);
	*paddr = (unsigned long)sts.common;
	return rv && sts.found;
}

extern int nlist(const char *path, struct nlist *nl);

/* Risoluzione di un simbolo interno. */
static int dsv_decode_ivar(char *name, char *file,
                           char *text, char *common,
                           unsigned long *paddr)
{
struct nlist nl[3];
unsigned long addr;

	nl[0].n_un.n_name = name;
	nl[1].n_un.n_name = (char *)"";
	nl[2].n_un.n_name = NULL;

	nlist(file,nl);

	addr = (unsigned long)text + nl[0].n_value;

	if ((nl[0].n_type & N_TYPE) == N_UNDF) {

		if (nl[0].n_value == 0)
			return 0;

	/* UFFA ! E` una variabile nella sezione COMMON. */

		if (dsv_decode_ivar_common(name,file,text,common,&addr) == 0)
			return 0;
	}

	*paddr = addr;
	return -1;
}

/*
* Risoluzione di un simbolo interno con ricerca del modulo di appartenenza.
*/
static int dsv_decode_ivar_glob(char *name, char **file, long *paddr)
{
int rv;
module_t *m;

/* Prova a cercare il nome nei moduli caricati. */
	for (m = Modules; m; m = m -> next) {
		if (m -> text) {
			rv = dsv_decode_ivar(name,m -> file,
			                     m -> text,m -> common,paddr);
			if (rv) {
				*file = m -> file;
				return rv;
			}
		}
	}

/* Estremo tentativo: cerca il nome nell'eseguibile di qplc. */

	rv = dsv_decode_ivar(name,main_status.exe_path,
	                        (char *)0,(char *)0,paddr);
	if (rv)
		*file = main_status.exe_path;

	return rv;
}

/*******************************************\
* FUNZIONI ASSOCIATE AI VARI COMANDI REMOTI *
\*******************************************/

#if 0 /* Scopiazzare da qui. */
static int dsv_cmd_(session_t *t)
{
	return 0;
}
#endif

/* Richiede o sospende l'invio degli errori di Plc. */
static int dsv_cmd_errs(session_t *t)
{
int rv = 0;
unsigned char *b1,*b2;
unsigned long n1,n2;
long n;
dsv_answ_t a;

	switch (t -> args[0]) {
	case 'd':
	/* Disattivazione dell'invio degli errori come eventi asincroni. */
		t -> flags &= ~( (1 << (DSV_EVENT_ERROR))
		               | (1 << (DSV_EVENT_DELERR)) );
		break;
	case 'e':
	/* Attivazione dell'invio degli errori come eventi asincroni. */
		t -> flags |= (1 << (DSV_EVENT_ERROR))    
		            | (1 << (DSV_EVENT_DELERR));
		break;
	case 'v':
	case 'l':
	/* Visualizzazione degli errori immagazzinati. "v" emette i dati
	 in forma numerica, "l" li emette in forma "leggibile". */
		n1 = n2 = 0;
		n = debErrorList(&b1,&n1,&b2,&n2);
		if (n == 0) {
			rv = DSV_E_BADSTS;
		}
		else {
			if (t -> args[0] == 'v') {
				n = (long)(n1 + n2);
				dsv_answ_init_long_data(&a,t,n);
				dsv_answ_addhv(&a,b1,(int)n1,0);
				if (n2)
					dsv_answ_addhv(&a,b2,(int)n2,0);
				dsv_answ_flush(&a);
			}
			else {
			int code;
			char text[256]; /* Posso permettermelo? */

				dsv_answ_init_data(&a,t);
				for (debErrorFindInit(&n1);
				     debErrorFindNext(&n1,&code,text); ) {

					dsv_answ_addfmt(&a,
					 "%x %s",
					 (unsigned char *)&code, sizeof(code),
					 (unsigned char *)&text);
					dsv_answ_cflush(&a);
				}
				dsv_answ_addfmt(&a, ".");
				dsv_answ_flush(&a);
			}
			rv = -1;
		}
		break;
	default:
		rv = DSV_E_BADARG;
		break;
	}
	return rv;
}

/* Richiede o sospende l'invio dei messaggi di Plc. */
static int dsv_cmd_messages(session_t *t)
{
int rv = 0;

	switch (t -> args[0]) {
	case 'd':
		t -> flags &= ~(1 << DSV_EVENT_MESSAGE);
		break;
	case 'e':
		t -> flags |= (1 << DSV_EVENT_MESSAGE);
		break;
	default:
		rv = DSV_E_BADARG;
	}
	return rv;
}

/* Statistica della memoria libera (area dei segnali di scambio). */
static int dsv_cmd_free(session_t *t)
{
dsv_answ_t a;
shvstat_t s;

	if (shvStat(&s) < 0)
		return DSV_E_BADSTS;
	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "%x %x %x %x",
	                (unsigned char *)&s.free_nvram, sizeof(s.free_nvram),
	                (unsigned char *)&s.free_heap, sizeof(s.free_heap),
	                (unsigned char *)&s.free_keys, sizeof(s.free_keys),
	                (unsigned char *)&s.max_free_bkey,
	                                  sizeof(s.max_free_bkey));
	dsv_answ_flush(&a);

	return -1;
}

/* Trova la variabile corrispondente ad un indirizzo. */
#if 0
static int dsv_cmd_find(session_t *t)
{
dsv_answ_t a;
shvstat_t s;
unsigned long addr;
char *p;

/* Lettura dell'indirizzo. Errore se non corrisponde almeno
 ad un'area leggibile. */
	addr = util_ahtol(t -> args, &p);
	if (! util_check_addr_rd((char *)addr,1))
		return DSV_E_BADARG;
/* Primo tentativo: ricerca nell'area dei segnali di scambio. */

#if 0
	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "%s %s %x %x %x");
	dsv_answ_flush(&a);

	return -1;
#else
	return 0;
#endif
}
#endif

/* Mette in GO. */
static int dsv_cmd_go(session_t *t)
{
	dsv_sv_exec_0(SV_COMM_GO);
	return 0;
}

/* Mette in HALT. */
static int dsv_cmd_halt(session_t *t)
{
	dsv_sv_exec_0(SV_COMM_HALT);
	return 0;
}

/* Legge un ingresso. Sintassi : in <segnale> <nbit> */
static int dsv_cmd_in(session_t *t)
{
sv_msg_t msg;
sv_comm_forceio_t *f;
char *p;
dsv_answ_t a;

	f = (sv_comm_forceio_t*)(msg.data);
	msg.cmd = SV_COMM_FORCEIO;
	msg.len = sizeof(*f);
	p = t -> args;
	strncpy(f -> path,p,sizeof(f -> path));
	f -> path[sizeof(f -> path) - 1] = '\0';
	p += strlen(p) + 1;
#if 0
	f -> bit = util_ahtol(p, &p); ++p;
#endif
	f -> mode = 0;
	superv_exec(&msg);
	if (f -> rv == 0)
		return DSV_E_BADARG;
	else if (f -> rv < 0)
		return DSV_E_BADSTS;
	else {
/* Truccaccio: si ipotizza che il valore sia "sovrapponibile"
 Esampio da non imitare. */
		dsv_answ_init_data(&a,t);
		dsv_answ_addhv(&a, (unsigned char *)&f -> val,
		               (f -> bit + 7) / 8, 1);
		dsv_answ_flush(&a);
		return -1;
	}
}

/* Informazioni su periodo PLC, massima dim. del buffer di trace, massimo
 numero di variabili tracciabili, massimo numero di variabili gestite da
 "memcopy", e chi piu` ne ha... */
static int dsv_cmd_info(session_t *t)
{
dsv_answ_t a;
long v1,v2,v3,v4,v5;

	v1 = plcPeriod();
	v2 = MAX_TRACE_BUFFER;
	v3 = MAX_TRACE_VAR;
	v4 = MAX_MEMS;
	v5 = MAX_TRIGGER_VAR;
	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "%x %x %x %x %x",
	                (unsigned char *)&v1, sizeof(v1),
	                (unsigned char *)&v2, sizeof(v2),
	                (unsigned char *)&v3, sizeof(v3),
	                (unsigned char *)&v4, sizeof(v4),
	                (unsigned char *)&v5, sizeof(v5));
	dsv_answ_flush(&a);

	return -1;
}

/* Acquisizione non invasiva dello stato di un segnale di I/O. */
static int dsv_cmd_iostat(session_t *t)
{
unsigned long val = 0;
int nbit,mode;
char *p;
dsv_answ_t a;
devnode_t l;

	p = t -> args;
	if (t -> argc < 2)
		return DSV_E_BADARG;
	if (p[0] == 'i' || p[0] == 'I')
		mode = DRIVER_MODE_INPUT;
	else if (p[0] == 'o' || p[0] == 'O')
		mode = DRIVER_MODE_OUTPUT;
	else {
		mode = util_ahtol(p, &p);
		if ((mode & (DRIVER_MODE_INPUT | DRIVER_MODE_OUTPUT)) == 0)
			return DSV_E_BADARG;
	}
	p += strlen(p) + 1;
	if (t -> argc == 2) {
		nbit = ioParse(p,mode,&l);
		if (nbit == 0)
			return DSV_E_BADARG;
	}
	else if (t -> argc != 4) {
		return DSV_E_BADARG;
	}
	else {
		l.flags = mode;
		l.ideep = util_ahtol(p, &p); ++p;
		l.next.driver = (driver_t *)util_ahtol(p, &p); ++p;
		util_ahtom(p, &p, (char *)&l.next.spec,
			   sizeof(l.next.spec)); ++p;
	}
	if (! ioShow(&l,(void *)&val))
		return DSV_E_BADARG;
	dsv_answ_init_data(&a,t);
	dsv_answ_addhv(&a, (unsigned char *)&val,(l.nbit+7)/8, 1);
	dsv_answ_flush(&a);
	return -1;
}

/* Informazioni sugli oggetti IPC. */
static int dsv_cmd_ipcs(session_t *t)
{
dsv_answ_t a;

	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "V %x %x %x %x V %x %x 2 %s %x %s %x",
	/* Shared memory (per ora solo di tipo SYSV). */
	                (unsigned char *)&main_status.sysv_shm_key,
	                sizeof(main_status.sysv_shm_key),
	                (unsigned char *)&main_status.sysv_shm_id,
	                sizeof(main_status.sysv_shm_id),
	                (unsigned char *)&main_status.shm_size,
	                sizeof(main_status.shm_size),
	                (unsigned char *)&main_status.shm_addr,
	                sizeof(main_status.shm_addr),
	/* Semafori (per ora solo di tipo SYSV). */
	                (unsigned char *)&main_status.isaker_sysv_sem_key,
	                sizeof(main_status.isaker_sysv_sem_key),
	                (unsigned char *)&main_status.isaker_sysv_sem_id,
	                sizeof(main_status.isaker_sysv_sem_id),
	/* Nomi e indirizzi delle zone di RAM non volatile. */
	                main_status.nvram1_name,
	                (unsigned char *)&main_status.nvram1.addr,
	                sizeof(main_status.nvram1.addr),
	                main_status.nvram2_name,
	                (unsigned char *)&main_status.nvram2.addr,
	                sizeof(main_status.nvram2.addr)
	                );
	dsv_answ_flush(&a);

	return -1;
}

/* Lista dei dispositivi disponibili. */
static void dsv_cmd_list_p_core(dsv_answ_t *a)
{
iobind_t *p;

	for (p = ioGetBindings(); p; p = p -> next) {
		dsv_answ_addfmt(a, "%s %x %x %x %x",
			(unsigned char *)&p -> name,
			(unsigned char *)&p -> flags, sizeof(p -> flags),
			(unsigned char *)&p -> nbit, sizeof(p -> nbit),
			(unsigned char *)&p -> addr, sizeof(p -> addr),
			(unsigned char *)&p, sizeof(p));
		dsv_answ_cflush(a);
	}
}
static void dsv_cmd_list_print(dsv_answ_t *a, devnode_t *r)
{
	if (r -> name[0])
		dsv_answ_addfmt(a, "%s %s %x %x %x %x %b %x %b",
			(unsigned char *)&r -> name,
			r -> comment[0] ? (unsigned char *)&r -> comment
			                : (unsigned char *) "?",
			(unsigned char *)&r -> flags, sizeof(r -> flags),
			(unsigned char *)&r -> nbit, sizeof(r -> nbit),
			(unsigned char *)&r -> ideep, sizeof(r -> ideep),
			(unsigned char *)&r -> tree.driver,
					 sizeof(r -> tree.driver),
			(unsigned char *)&r -> tree.spec,
					 sizeof(r -> tree.spec),
			(unsigned char *)&r -> next.driver,
					 sizeof(r -> next.driver),
			(unsigned char *)&r -> next.spec,
					 sizeof(r -> next.spec));
	else
		dsv_answ_addfmt(a, ".");
}
static void dsv_cmd_list_core(dsv_answ_t *a, devnode_t *r)
{
devnode_t l;

	while (ioList(r) == 0 && r -> name[0]) {
		dsv_cmd_list_print(a,r);
		dsv_answ_cflush(a);
		if (r -> flags & DRIVER_MODE_LIST) {
			l = *r;
			++l.ideep;
			dsv_cmd_list_core(a,&l);
		}
	/* A livello 0 c'e` solo un elemento, sempre. */
		if (r -> ideep == 0)
			break;
		r -> tree = r -> next;
	}
}
static int dsv_cmd_list(session_t *t)
{
dsv_answ_t a;
devnode_t l;
char *p;

	p = t -> args;
	if (t -> argc == 0) {
		dsv_answ_init_data(&a,t);
		l.ideep = 0;
		dsv_cmd_list_core(&a,&l);
		dsv_answ_addfmt(&a, ".");
		dsv_answ_flush(&a);
		return -1;
	}
	else if (t -> argc == 1) {
		if (*p == 'e' && (t -> ionode.flags & DRIVER_MODE_LIST))
			++(t -> ionode.ideep);
		else if (*p == 'n')
			t -> ionode.tree = t -> ionode.next;
		else if (*p == 'p') {
			dsv_answ_init_data(&a,t);
			dsv_cmd_list_p_core(&a);
			dsv_answ_addfmt(&a, ".");
			dsv_answ_flush(&a);
			return -1;
		}
		else
			return DSV_E_BADARG;
	}
	else if (t -> argc != 3)
		return DSV_E_BADARG;
	else {
		t -> ionode.ideep = util_ahtol(p, &p); ++p;
		t -> ionode.tree.driver = (driver_t *) util_ahtol(p, &p); ++p;
		util_ahtom(p, &p, (char *)t -> ionode.tree.spec,
			   sizeof(t -> ionode.tree.spec)); ++p;
	}
	if (ioList(&(t -> ionode)))
		return DSV_E_BADARG;
	dsv_answ_init_data(&a,t);
	dsv_cmd_list_print(&a,&(t -> ionode));
	dsv_answ_flush(&a);
	return -1;
}

/* Carica il programma contenuto nel file arg1. */
static int dsv_cmd_load(session_t *t)
{
sv_msg_t msg;

	if (main_status.shm_addr -> go)
		return DSV_E_BADSTS;
	msg.cmd = SV_COMM_DYLOAD;
	strncpy(msg.data, t -> args, sizeof(msg.data));
	msg.data[sizeof(msg.data) - 1] = '\0';
	msg.len = strlen(msg.data) + 1;
	superv_exec(&msg);
	return 0;
}

/* Visualizza arg2 byte all'indirizzo arg1. */
static int dsv_cmd_mem(session_t *t)
{
unsigned long addr;
int nb,rv,n;
char *p;
dsv_answ_t a;

	addr = util_ahtol(t -> args, &p);
	nb = util_ahtol(p + 1,(char **) NULL);
	n = nb ? nb : 1;
	if (nb >= 0 && util_check_addr_rd((char *)addr,n)) {
		if (nb == 0) {
			rv = 0;
		}
		else {
			dsv_answ_init_data(&a,t);
			dsv_answ_addhv(&a, (unsigned char *)addr, nb, 0);
			dsv_answ_flush(&a);
			rv = -1;
		}
	}
	else {
		rv = DSV_E_BADARG;
	}
	return rv;
}

/* Convalida e registra aree in debug. */
static int dsv_cmd_memchk(session_t *t)
{
unsigned long addr;
int nb,i,n,f;
char *p;
dsv_answ_t a;

	if (t -> argc % 2 != 0 || t -> argc > MAX_MEMS * 2)
		return DSV_E_BADARG;
	dsv_answ_init_data(&a,t);
	for (p = t -> args, i = t -> argc, n = 0; i; i -= 2) {
		addr = util_ahtol(p, &p);
		++p;
		nb = util_ahtol(p,&p);
		++p;
		f = nb > 0 && util_check_addr_rd((char *)addr,nb);
		dsv_answ_addflag(&a, f);
		if (f) {
			if (n >= MAX_MEMS)
				return DSV_E_BADARG;
			t -> mems[n].addr = (unsigned char *)addr;
			t -> mems[n].len = nb;
			++n;
		}
	}
	t -> nmems = n;
	dsv_answ_flush(&a);

	if (t -> argc == 0)
		return 0;
	else
		return -1;
}

/* Visualizza i contenuti delle aree in debug. */
static int dsv_cmd_memcopy(session_t *t)
{
dsv_answ_t a;
int i;

	if (t -> nmems == 0)
		return DSV_E_BADSTS;
	dsv_answ_init_data(&a,t);
	for (i = 0; i < t -> nmems; ++i)
		dsv_answ_addhv(&a, t -> mems[i].addr, t -> mems[i].len, 0);
	dsv_answ_flush(&a);
	return -1;
}

/* Visualizza informazioni sul modulo di nome arg1 . */
static int dsv_cmd_mod(session_t *t)
{
dsv_answ_t a;
module_t *mod;
modprop_t m;

	mod = dyFindModule(t -> args);
	if (! mod)
		return DSV_E_BADARG;
	else
		dyModProp(mod, &m);

	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "%x %x %x %x %x %s",
	                (unsigned char *)&m.text, sizeof(m.text),
	                (unsigned char *)&m.data, sizeof(m.data),
	                (unsigned char *)&m.bss, sizeof(m.bss),
	                (unsigned char *)&m.common, sizeof(m.common),
	                (unsigned char *)&m.size, sizeof(m.size),
	                m.file);
	dsv_answ_flush(&a);

	return -1;
}

/* Forza un'uscita. Sintassi : out <segnale> <nbit> <valore> */
static int dsv_cmd_out(session_t *t)
{
sv_msg_t msg;
sv_comm_forceio_t *f;
char *p;

	f = (sv_comm_forceio_t*)(msg.data);
	msg.cmd = SV_COMM_FORCEIO;
	msg.len = sizeof(*f);
	p = t -> args;
	strncpy(f -> path,p,sizeof(f -> path));
	f -> path[sizeof(f -> path) - 1] = '\0';
	p += strlen(p) + 1;
#if 0
	f -> bit = util_ahtol(p, &p); ++p;
#endif
	f -> mode = 1;
/* Truccaccio: si ipotizza che il valore sia "sovrapponibile"
 Esampio da non imitare. */
	f -> val.lo = util_ahtol(p, &p);
	superv_exec(&msg);
	if (f -> rv == 0)
		return DSV_E_BADARG;
	else if (f -> rv < 0)
		return DSV_E_BADSTS;
	else
		return 0;
}

/* Chiude la sessione. */
static int dsv_cmd_quit(session_t *t)
{
	dsv_end_trans(t);
	return -1;
}

/* Scrittura di una variabile. */
static int dsv_cmd_set(session_t *t)
{
char *name;
int l,rv;
int i1,i2;
char *p,*q;
shv_t v;
int key;
long addr;
int typecode;
char *file;
int lim,nv;
unsigned long val;

/* Parametri: un nome, seguito eventualmente da due indici e una lista di
 valori esadecimali (byte). */
	if (t -> argc < 4)
		return DSV_E_BADARG;

/* Lettura dei parametri nome ed indici. */
	name = t -> args;
	l = strlen(name);
	i1 = i2 = 0;
	p = name + l + 1;
	i1 = (int)util_ahtol(p, &p); ++p;
	i2 = (int)util_ahtol(p, &p); ++p;

/* Decodifica il segnale di scambio (ammesso che si tratti di un
 segnale di scambio). */
	rv = dsv_decode_shvar(name, l, i1, i2, &v, &key, &addr, &typecode);
	if (rv != -1) {
	/* Prova a interpretare il nome come simbolo ordinario. */
		rv = dsv_decode_ivar_glob(name,&file,&addr);
	/* D'ora in poi gli indici sono privi di senso. Saranno ignorati. */
	/* Nessun limite (arrrghhhh!) sul numero di byte. */
		lim = MAX_DSV_D_LINE;
	}
	else {
	/* Coi segnali di scambio si puo` controllare anche la dimensione
	 dei dati. Quest'informazione manca nel caso di simboli ordinari. */
		switch (typecode & SHV_MODE_TYPE) {
		case SHV_MODE_TYPE_BOOL:
		case SHV_MODE_TYPE_CHAR:
		case SHV_MODE_TYPE_UCHAR:
			lim = sizeof(char);
			break;
		case SHV_MODE_TYPE_LONG:
		case SHV_MODE_TYPE_ULONG:
		case SHV_MODE_TYPE_FLOAT:
			lim = sizeof(long);
			break;
		case SHV_MODE_TYPE_DOUBLE:
			lim = sizeof(double);
			break;
		default:
			lim = MAX_DSV_D_LINE;
			break;
		}
	}

	if (rv == -1) {

	/* Ora bisogna interpretare gli argomenti restanti. Ogni argomento
	 e` trattato come valore esadecimale. Il numero di cifre di ciascun
	 argomento decide quanti byte ricopre. Le taglie riconosciute sono
	 1,2 e 4 byte, cioe` 1-2 cifre, 3-4 cifre e 5-8 cifre
	 rispettivamente. */

		for (nv = t -> argc - 3; nv > 0; --nv) {
		/* Interpretazione dell'argomento. "l" conterra` il
		 numero di byte interessati dall'argomento. */
			q = p;
			val = util_ahtol(p, &p);
			++p;
			switch (p - q) {
			case 2: case 3: l = sizeof(char); break;
			case 4: case 5: l = sizeof(short); break;
			case 6: case 7: case 8: case 9: l=sizeof(long); break;
			default: l = 0; break;
				break;
			}
		/* Un sommario controllo di congruenza. */
			if (l == 0
			 || l > lim
			 || !util_check_addr_wr((char *)addr,l)) {
				rv = 0;
				break;
			}
			lim -= l;
		/* Si tenta, per quanto possibile, di realizzare
		 scritture atomiche. Questo dovrebbe rendere meno
		 probabili le incongruenze dovute ad interventi
		 dell'esecutore a meta` di un trasferimento. */
			switch (l) {
			case sizeof(short):
				*((short *)addr) = (short)val;
				addr += sizeof(short);
				break;
			case sizeof(char):
				*((char *)addr) = (char)val;
				addr += sizeof(char);
				break;
			default:
				*((unsigned long *)addr) = val;
				addr += sizeof(long);
				break;
			}
		}
	}

	return rv ? 0 : DSV_E_BADARG;
}

/* Assegna il valore ad un ingresso. Sintassi : setin <segnale> <valore> */
static int dsv_cmd_setin(session_t *t)
{
char *p;
iobind_t *q;
unsigned long descr;
unsigned long val;

	p = t -> args;
	descr = util_ahtol(p, &p); ++p;
	val = util_ahtol(p, &p); ++p;
	q = ioFindBinding(descr);
	if ((! q) || (! (q -> flags & DRIVER_MODE_INPUT)))
		return DSV_E_BADARG;
#if 0
	else if (! main_status.simul)
		return DSV_E_BADSTS;
#endif
	else {
		switch ((q -> nbit + 7) / 8) {
		case 4:
			*(unsigned long *)(q -> addr) = val;
			break;
		case 2:
			*(unsigned short *)(q -> addr) = (unsigned short)val;
			break;
		default:
			*(unsigned char *)(q -> addr) = (unsigned char)val;
			break;
		}
		return 0;
	}
}

/* Trova il simbolo piu` vicino all'indirizzo dato. */

/* Elaborazione di un elemento della symbol table nella ricerca della
 migliore corrispondenza indirizzo-simbolo. C'e` anche il solito
 ricalcolo degli indirizzi nella sezione "common". */

/* Struttura di stato della funzione iterativa. */
typedef struct {
	unsigned long addr;
	char *text;
	char *common;
	unsigned long best_try_diff;
	long best_try_pos;
} find_best_element_t;

static int dsv_find_best_element_test(symtab_iterator_t *it,
                                      struct nlist *sym,
                                      void *sts)
{
unsigned long ca;
find_best_element_t *statics = (find_best_element_t *)sts;

/* Controlla il tipo del simbolo ed estrae l'indirizzo. */
	if ((sym -> n_type & N_TYPE) == N_UNDF) {
		if (sym -> n_value == 0) {
		/* E` un riferimento esterno. Passa oltre. */
			return 1;
		}
		else {
		/* E` un simbolo nella sezione "common". Il suo
		 indirizzo e` quello dell'ultima posizione "common"
		 assegnata. */
			ca = (unsigned long) (statics -> common);
		/* Sposta in avanti il puntatore nell'area "common". */
			statics -> common += sym -> n_value;
		}
		
	}
	else {
	/* E` un simbolo ordinario. */
		ca = ((unsigned long)(statics -> text)) + sym -> n_value;
	}

/* Confronta l'indirizzo del simbolo con quello da trovare. */

	ca = statics -> addr - ca;
/* Truccaccio, schifezza... */
	if (((long)ca) < 0) {
	/* Il simbolo corrente si trova oltre l'indirizzo. Non va bene. */
		return 1;
	}

/* Se la differenza tra gli indirizzi e` minore di quella registrata,
 la registra, assieme alla posizione del nome nella string table. */
	if (ca < statics -> best_try_diff) {
		statics -> best_try_diff = ca;
		statics -> best_try_pos = it -> str_pos + sym -> n_un.n_strx;
	}


/* Se la differenza trovata e` zero, la ricerca termina immediatamente. */

	return ca ? 1 : -1;
}

/* Ricerca del simbolo piu` vicino ad un dato indirizzo (nucleo
 procedurale). */
static int dsv_find_best_element_core(unsigned long addr, char *file,
                                      char *text, char *common,
                                      unsigned long *diff,
                                      char *name, int name_size)
{
find_best_element_t sts;
int rv;
FILE *f;
int c;
int ipos;
int niter;
char *pn;
int nsz;
unsigned long btd;

/* Inizializza lo stato dell'iteratore. */
	sts.addr = addr;
	sts.text = text;
	sts.common = common;
	sts.best_try_diff = *diff = (unsigned long)(-1);
	sts.best_try_pos = -1;
	name[0] = '.';
	name[1] = '\0';

/* Continua la ricerca finche` non trova un nome che non inizi con ".". */

	for (ipos = 0, niter = 0; name[0] == '.'; ++niter, ++ipos) {

		btd = sts.best_try_diff;
		sts.best_try_diff = (unsigned long)(-1);
		sts.best_try_pos = -1;

	/* Applica l'iteratore alla funzione di ricerca del simbolo. */
		rv = dsv_explore_symtab(file, text, common,
					dsv_find_best_element_test,
					&ipos,
					(void *)&sts);

	/* In caso di insuccesso, uscita. */
		if (rv == 0 || sts.best_try_pos == -1)
			return 0;

	/* Esce (con successo) se non si e` trovato niente di meglio. */

		if (btd < sts.best_try_diff)
			break;
	
	/* Comunica lo scarto minimo ottenuto. */
		*diff = sts.best_try_diff;

	/* Lettura del nome: si riapre il file e ci si posiziona all'inizio
	 della stringa del nome. */

		f = fopen(file,"r");
		if (! f)
			goto OBJ_ERROR;

		if (fseek(f, sts.best_try_pos, 0))
			goto OBJ_ERROR;

		pn = name;
		nsz = name_size;

	/* Estrazione del nome. */
		do {
			c = fgetc(f);
			if (c == EOF)
				goto OBJ_ERROR;
			*(pn++) = c;
			--nsz;
		} while (c && nsz);

		fclose(f);
	}

/* Fine. */

	return -1;

OBJ_ERROR:
	if (f)
		fclose(f);
	return 0;
}

static int dsv_find_best_element(session_t *t,
                                 unsigned long addr, char *file,
                                 char *text, char *common)
{
char buf[64];
dsv_answ_t a;
unsigned long diff;
int rv;

/* Cerca il nome del simbolo piu` vicino. */
	buf[sizeof(buf)-1] = '\0';
	rv = dsv_find_best_element_core(addr, file,
	                                text,
				        common,
				        &diff,
				        buf,
				        sizeof(buf) - 1);
	if (! rv)
		return DSV_E_BADARG;
/* Emette il risultato. */
	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "I %s %x %s",
			buf,
			(unsigned char *)&diff,
			sizeof(diff),
			file);
	dsv_answ_flush(&a);

/* FINE */
	return -1;
}

static int  dsv_find_best_sh_element_core(session_t *t, unsigned long addr)
{
dsv_answ_t a;
shv_t sh;
int key,i0,i1;

	if (shvAddrToVar((void *)addr,&sh,&key) == 0) {
		dsv_answ_init_data(&a,t);
		key -= sh.key;
		i0 = key / sh.dim2;
		i1 = key % sh.dim2;
		dsv_answ_addfmt(&a, "S %s %x %x",
				sh.name,
				(unsigned char *)&i0, sizeof(i0),
				(unsigned char *)&i1, sizeof(i1));
		dsv_answ_flush(&a);
		return -1;
	}
	return DSV_E_BADARG;
}

static int dsv_cmd_sym(session_t *t)
{
char *p;
unsigned long addr;
module_t *m;
int rv;

	rv = DSV_E_BADARG;

	p = t -> args;
	addr = util_ahtol(p, &p); ++p;

/* Tenta prima coi segnali di scambio. Per accelerare la ricerca,
 si controlla prima che l'indirizzo ricada almeno in un intervallo
 ragionevole. */
	if ((addr >= ((long)main_status.shm_addr)
	  && addr < ((long)main_status.shm_addr)
	          + main_status.shm_size)
	 ||
	   ((addr >= ((long)main_status.nvram2.addr)
	  && addr < ((long)main_status.nvram2.addr)
	          + main_status.nvram2.size))) {

		rv = dsv_find_best_sh_element_core(t,addr);
	}
	else {

	/* Prova a cercare il nome nei moduli caricati. */

		for (m = Modules; m; m = m -> next) {

	/* La ricerca del modulo termina quando se ne trova uno che
	 risieda ad un indirizzo compatibile. */
	 
			if (addr >= ((unsigned long)(m -> text))
			 && addr < ((unsigned long)(m -> text)) + m -> size) {

			/* Cerca il nome del simbolo piu` vicino. */
				rv = dsv_find_best_element(t, addr,
				                           m -> file,
				                           m -> text,
				                           m -> common);
			/* FINE (comunque sia andata). */
				return rv;
			}
		}

#if 0
	/* Estremo tentativo: cerca il nome nell'eseguibile di qplc. */
	/* NOTA: Non puo` funzionare: l'eseguibile e` in formato COFF,
	 mentre la routine di esplorazione accetta solo A.OUT. */

		rv = dsv_find_best_element(t, addr,
		                           main_status.exe_path,
					   (char *) 0,
					   (char *) 0);
#endif
	}

	return rv;
}

/* Statistica: c=cancella, d=disabilita, e=abilita, v=visualizza. */
static int dsv_cmd_stat(session_t *t)
{
int rv = 0;
char *p;
dsv_answ_t a;
sv_msg_t msg;

	for (p = t -> args; *p; ++p) {
		msg.len = 0;
		switch (*p) {
		case 'c':
			msg.cmd = SV_COMM_CLRMEAS;
			break;
		case 'd':
			msg.cmd = SV_COMM_DISMEAS;
			break;
		case 'e':
			msg.cmd = SV_COMM_ENMEAS;
			break;
		case 'v':
			msg.cmd = SV_COMM_MEAS;
			break;
		default:
			return DSV_E_BADARG;
		}
		superv_exec(&msg);
		if (*p == 'v') {
			dsv_answ_init_data(&a,t);
			dsv_answ_addfmt(&a, "%x %x %x",
				(unsigned char *)&((long *)msg.data)[0],
				sizeof(long),
				(unsigned char *)&((long *)msg.data)[1],					sizeof(long),
				(unsigned char *)&((long *)msg.data)[2],
				sizeof(long));
			dsv_answ_flush(&a);
			rv = -1;
		}
	}
	return rv;
}

/* Stato del plc. */
static int dsv_cmd_status(session_t *t)
{
dsv_answ_t a;

	dsv_answ_init_data(&a,t);
	dsv_answ_addhv(&a, (unsigned char *)&(main_status.shm_addr -> go),
	               sizeof(main_status.shm_addr -> go), 1);
	dsv_answ_flush(&a);
	return -1;
}

/* Trace: a <addr...>=crea lista, c=cancella, d=disabilita,
 e=abilita, m <len>=dimensiona buffer, v=visualizza. */
static int dsv_cmd_trace(session_t *t)
{
int rv = 0;
char *p;
dsv_answ_t a;
int i,n,f;
int se,pe;
long addr;

	switch (t -> args[0]) {
	case 'a':
		if (t -> flags & (1 << DSV_EVENT_TRACE)
		 || !main_status.shm_addr -> go)
			return DSV_E_BADSTS;
		if (t -> argc < 2)
			return DSV_E_BADARG;
		dsv_answ_init_data(&a,t);
		for (p = t -> args + 2, i = t -> argc - 1, n = 0; i; --i) {
			addr = util_ahtol(p, &p);
			++p;
			f = util_check_addr_rd((char *)addr,1);
			dsv_answ_addflag(&a, f);
			if (f)
				t -> tracetab[n++] = (char *)addr;
		}
		dsv_answ_flush(&a);
		t -> ntrace = n;
	/* La condizione di trace e` normalmente vera. Viene posta a
	 FALSO solo se si specifica "trace t" (vedi). */
		t -> trace_cond = 1;
		rv = -1;
		break;
	case 'c':
		csem_wait(dsv_sts.sem,(struct timeval *)NULL);
		dsv_disable_trace(t);
		csem_signal(dsv_sts.sem);
		t -> ntrace = 0;
		t -> trace_pos = 0;
		t -> trace_bnext = 0;
		t -> trace_count = 0;
		t -> trace_done = 0;
		t -> trace_cond = 0;
		break;
	case 'd':
		csem_wait(dsv_sts.sem,(struct timeval *)NULL);
		dsv_disable_trace(t);
		csem_signal(dsv_sts.sem);
		break;
	case 'e':
		csem_wait(dsv_sts.sem,(struct timeval *)NULL);
		if (! dsv_enable_trace(t))
			rv = DSV_E_BADSTS;
		csem_signal(dsv_sts.sem);
		break;
	case 'm':
		if (t -> argc != 2)
			return DSV_E_BADARG;
		p = t -> args + 2;
		n = util_ahtol(p, &p);
		if (n < 1 || n > MAX_TRACE_BUFFER)
			return DSV_E_BADARG;
		t -> trace_len = n;
		rv = 0;
		break;
	case 't':
		t -> trace_cond = 0;
		break;
	case 'v':
		if (! (t -> trace_done)) {
			rv = DSV_E_BADSTS;
			break;
		}

		se = sizeof(trace_event_t) * t -> trace_len;
		pe = t -> trace_bnext - t -> trace_len;
		dsv_answ_init_long_data(&a,t,se);
		if (pe < 0) {
			dsv_answ_addhv(&a,
			       (unsigned char *)
				&t -> tracebuf[TRACE_BUFFER_SIZE + pe],
			       (int)(sizeof(trace_event_t) * (-pe)), 0);
			pe = 0;
			se = sizeof(trace_event_t) * t -> trace_bnext;
		}
		if (se)
			dsv_answ_addhv(&a,
			         (unsigned char *)&t -> tracebuf[pe], se, 0);
		dsv_answ_flush(&a);
		t -> trace_done = 0;
		rv = -1;
		break;
	default:
		rv = DSV_E_BADARG;
		break;
	}
	return rv;
}

/* Trigger: a <addr> <op> <act> <param>=crea trigger, d <id>=cancella. */

/* Funzioncine di verifica della condizione. */
static int dsv_trigger_null_cmp(dsv_trig_t *tr) { return 0; }
static int dsv_trigger_cmp_rtrig(dsv_trig_t *tr)
{
register unsigned char c = tr -> v.uch;

	return ((tr -> v.uch = tr -> a -> uch)) && !c;
}
static int dsv_trigger_cmp_ftrig(dsv_trig_t *tr)
{
register unsigned char c = tr -> v.uch;

	return (!((tr -> v.uch = tr -> a -> uch))) && c;
}
static int dsv_trigger_cmp_chg8(dsv_trig_t *tr)
{
register unsigned char c;

	if ((c = tr -> a -> uch) != tr -> v.uch) {
		tr -> v.uch = c;
		return 1;
	}
	return 0;
}
static int dsv_trigger_cmp_chg16(dsv_trig_t *tr)
{
register unsigned short c;

	if ((c = tr -> a -> ush) != tr -> v.ush) {
		tr -> v.ush = c;
		return 1;
	}
	return 0;
}
static int dsv_trigger_cmp_chg32(dsv_trig_t *tr)
{
register unsigned long c;

	if ((c = tr -> a -> ul) != tr -> v.ul) {
		tr -> v.ul = c;
		return 1;
	}
	return 0;
}
static int dsv_trigger_cmp_eq8(dsv_trig_t *tr)
{
	return tr -> a -> uch == tr -> v.uch;
}
static int dsv_trigger_cmp_eq16(dsv_trig_t *tr)
{
	return tr -> a -> ush == tr -> v.ush;
}
static int dsv_trigger_cmp_eq32(dsv_trig_t *tr)
{
	return tr -> a -> ul == tr -> v.ul;
}

/* Funzioncine di inizializzazione. */

static void dsv_trigger_initcmp_null(dsv_trig_t *tr) { }
static void dsv_trigger_initcmp_copy8(dsv_trig_t *tr) { tr -> v.uch = tr -> a -> uch; }
static void dsv_trigger_initcmp_copy16(dsv_trig_t *tr) { tr -> v.ush = tr -> a -> ush; }
static void dsv_trigger_initcmp_copy32(dsv_trig_t *tr) { tr -> v.ul = tr -> a -> ul; }

/* NOTA : Questa tabella deve riflettere l'ordine delle definizioni
 degli enum appositi in "plcrem.h". */

static const struct dsv_trigger_ttab_t {
	int (*f)(dsv_trig_t *);
	void (*fi)(dsv_trig_t *);
	int sz;
} dsv_tab_cmp[] = {
{ dsv_trigger_null_cmp, dsv_trigger_initcmp_null, 1 },
{ dsv_trigger_cmp_rtrig, dsv_trigger_initcmp_copy8, 1, },
{ dsv_trigger_cmp_ftrig, dsv_trigger_initcmp_copy8, 1, },
{ dsv_trigger_cmp_chg8, dsv_trigger_initcmp_copy8, 1, },
{ dsv_trigger_cmp_chg16, dsv_trigger_initcmp_copy16, 2, },
{ dsv_trigger_cmp_chg32, dsv_trigger_initcmp_copy32, 4, },
{ dsv_trigger_cmp_eq8, dsv_trigger_initcmp_null, 1, },
{ dsv_trigger_cmp_eq16, dsv_trigger_initcmp_null, 2, },
{ dsv_trigger_cmp_eq32, dsv_trigger_initcmp_null, 4, },
};

#define MAX_TRIGGER_CMP (sizeof(dsv_tab_cmp)/sizeof(dsv_tab_cmp[0]))

static int dsv_cmd_trigger(session_t *t)
{
int rv = 0;
char *p;
dsv_answ_t a;
int i,n,op,act;
long addr,param;
dsv_trig_t *tr;

	switch (t -> args[0]) {
	case 'a':

	/* Legge i parametri. */

		if (t -> argc != 5)
			return DSV_E_BADARG;
		p = t -> args + 2;
		addr = util_ahtol(p, &p); ++p;
		op = util_ahtol(p, &p); ++p;
		if (op < 0 || op >= MAX_TRIGGER_CMP)
			return DSV_E_BADARG;
		act = util_ahtol(p, &p); ++p;
		param = util_ahtol(p, &p); ++p;

		if (!util_check_addr_rd((char *)addr,dsv_tab_cmp[op].sz))
			return DSV_E_BADARG;

	/* Alloca un descrittore di trigger. */

		if (t -> ntrigger >= MAX_TRIGGER_VAR) {

		/* Cerca un descrittore marcato "libero" (contiene la
		 funzione di confronto nulla). */

			for (i = 0, tr = t -> trigtab;
			     i < t -> ntrigger; ++i, ++tr ) {
				if (tr -> cmp == dsv_trigger_null_cmp)
					break;
			}
			if (i >= t -> ntrigger)
				return DSV_E_BADARG;
		}
		else {

		/* Occupa un nuovo descrittore, ed aggiunge la sessione
		 alla liste di trigger se il descrittore e` l'unico
		 occupato. */

			tr = &t -> trigtab[t -> ntrigger];
			tr -> cmp = dsv_trigger_null_cmp;
		/* L'assegnamento del nuovo valore al contatore deve
		 essere ininterrompibile. */
			if (++(t -> ntrigger) == 1) {
				csem_wait(dsv_sts.sem,(struct timeval *)NULL);
				dsv_enable_trigger(t);
				csem_signal(dsv_sts.sem);
			}
		}

	/* Assegna i parametri finali al descrittore. Ovviamente la
	 funzione di confronto deve essere assegnata per ultima (e deve
	 valere l'ipotesi che l'assegnamento sia ininterrompibile). */

		tr -> a = (dsv_anytype_t *)addr;
		tr -> v.ul = param;
		tr -> act = act;
	/* Chiama l'inizializzatore (prima che sia assegnata la
	 funzione di test). */
		(*(dsv_tab_cmp[op].fi))(tr);
	/* Assegna infine la funzione di test. */
		tr -> cmp = dsv_tab_cmp[op].f; /* CRITICO ! */

	/* Crea un nuovo identificatore ed emette il risultato. */

		n = tr -> id = ++(t -> trigger_id);

		dsv_answ_init_data(&a,t);
		dsv_answ_addhv(&a, (unsigned char *)&n, sizeof(n), 1);
		dsv_answ_flush(&a);

		rv = -1;
		break;

	case 'c':
		csem_wait(dsv_sts.sem,(struct timeval *)NULL);
		dsv_disable_trigger(t);
		csem_signal(dsv_sts.sem);
		break;

	case 'd':

		if (t -> argc != 2)
			return DSV_E_BADARG;
		p = t -> args + 2;
		n = util_ahtol(p, &p);

	/* Cerca il trigger ID nella lista specifica di questa sessione. */

		for (i = 0, tr = t -> trigtab; i  < t -> ntrigger; ++i, ++tr ) {
			if (n == tr -> id)
				break;
		}
		if (i >= t -> ntrigger)
			return DSV_E_BADARG;

	/* Assegna una funzione di confronto nulla (sempre falsa) al
	 trigger che sta per essere rimosso. */

	/* ATTENZIONE ! Si suppone che quest'operazione avvenga in modo
	 ininterrompibile ! (Su x86 e` vero) */
		tr -> cmp = dsv_trigger_null_cmp;

	/* Cerca il descrittore nullo di indice piu` basso nel
	 nel vettore dei descrittori della sessione. */

		for (i = t -> ntrigger, tr = &t -> trigtab[i - 1];
		     i && tr -> cmp == dsv_trigger_null_cmp;
		     --tr, --i)
			;

	/* Assume come numero di descrittori attivi l'indice trovato (+1).
	  L'effetto risultante e` la rimozione di tutti i descrittori nulli
	 dal vettore. */

	/* ATTENZIONE ! Si suppone che quest'operazione avvenga in modo
	 ininterrompibile ! (Su x86 e` vero) */
		t -> ntrigger = i;

	/* Se la sessione non ha piu` descrittori attivi, e` rimossa
	 dalla lista di quelle interessate al trigger. */

		if (i == 0) {
			csem_wait(dsv_sts.sem,(struct timeval *)NULL);
			dsv_disable_trigger(t);
			csem_signal(dsv_sts.sem);
		}
		rv = 0;
		break;
	default:
		rv = DSV_E_BADARG;
		break;
	}
	return rv;
}

/* Emissione dei dati di un segnale di scambio. */
static int dsv_cmd_var_shvar(session_t *t,char *name, int l, int i1, int i2)
{
shv_t v;
int key;
long addr;
dsv_answ_t a;
int typecode;
int rv;

/* Decodifica il segnale di scambio (ammesso che si tratti di un
 segnale di scambio). */
	rv = dsv_decode_shvar(name, l, i1, i2, &v, &key, &addr, &typecode);
	if (rv != -1)
		return rv;

/* Emette il risultato. */
	dsv_answ_init_data(&a,t);
	dsv_answ_addfmt(&a, "S %x %x %x %x %x %x",
	                (unsigned char *)&addr,sizeof(addr),
	                (unsigned char *)&v.dim1,sizeof(v.dim1),
	                (unsigned char *)&v.dim2,sizeof(v.dim2),
			(unsigned char *)&typecode,sizeof(typecode),
	                (unsigned char *)&v.size,sizeof(v.size),
	                (unsigned char *)&key,sizeof(key));
	dsv_answ_flush(&a);

	return -1;
}

/* Emissione dei dati di un simbolo interno. */
static int dsv_cmd_var_ivar(session_t *t, char *name)
{
long addr;
dsv_answ_t a;
int rv;
char *file;

	rv = dsv_decode_ivar_glob(name, &file, &addr);
	if (rv) {
		dsv_answ_init_data(&a,t);
		dsv_answ_addfmt(&a, "I %x %s",
				(unsigned char *)&addr,sizeof(addr),
				file);
		dsv_answ_flush(&a);
	}

	return rv;
}

/* Risoluzione di un nome. */
static int dsv_cmd_var(session_t *t)
{
char *name;
int l,rv;
int i1,i2;
char *p;

/* Parametri: un nome, seguito eventualmente da uno o due indici. */
	if (t -> argc < 1 || t -> argc > 3)
		return DSV_E_BADARG;

/* Lettura dei parametri. */
	name = t -> args;
	l = strlen(name);
	i1 = i2 = 0;
	if (t -> argc > 1) {
		p = name + l + 1;
		i1 = (int)util_ahtol(p, &p); ++p;
		if (t -> argc == 3)
			i2 = (int)util_ahtol(p, &p);
	}

/* Prova ad interpretare il nome come segnale di scambio. */
	rv = dsv_cmd_var_shvar(t,name,l,i1,i2);
	if (rv)
		return rv;

/* Prova a interpretare il nome come simbolo ordinario. */
/* D'ora in poi gli indici sono privi di senso. Saranno ignorati. */
	rv = dsv_cmd_var_ivar(t,name);

	return rv ? -1 : DSV_E_BADARG;
}

/* Versione QPLC. */
static int dsv_cmd_ver(session_t *t)
{
dsv_answ_t a;

	dsv_answ_init_data(&a,t);
	dsv_answ_addstr(&a,main_status.ver_kernel);
	dsv_answ_flush(&a);
	return -1;
}

/* Aggiunta di un punto di break alla lista.
  Parametri: id addr r1 r2 scale size off ar. */
static int dsv_cmd_wadd(session_t *t)
{
char *p = t -> args;
sv_msg_t msg;
struct arg {
	int id;
	watch_t w;
} *a;

	a = (struct arg *)msg.data; 
	a -> id = (int)util_ahtol(p, &p); ++p;
	a -> w.addr = (unsigned char *)util_ahtol(p, &p); ++p;
	a -> w.operand.r1 = (char)util_ahtol(p, &p); ++p;
	a -> w.operand.r2 = (char)util_ahtol(p, &p); ++p;
	a -> w.operand.scale = (char)util_ahtol(p, &p); ++p;
	a -> w.operand.size = (char)util_ahtol(p, &p); ++p;
	a -> w.operand.off = (unsigned long)util_ahtol(p, &p); ++p;
	a -> w.operand.ar = (char)util_ahtol(p, &p); ++p;
	msg.cmd = SV_COMM_WADD;
	msg.len = sizeof(*a);
	superv_exec(&msg);
	if (*(int *)(msg.data) < 0)
		return DSV_E_BADARG;
	else
		return 0;
}

/* Creazione di una lista di break.
  Parametri: act cond apar opc opa1 opa2. */
static int dsv_cmd_wcreate(session_t *t)
{
char *p = t -> args;
watch_descr_t *w;
sv_msg_t msg;
int rv;

	w = (watch_descr_t *)msg.data;
	w -> act = (unsigned short)util_ahtol(p, &p); ++p;
	w -> condact = (unsigned short)util_ahtol(p, &p); ++p;
	w -> cond = (unsigned short)util_ahtol(p, &p); ++p;
	w -> apar = (unsigned short)util_ahtol(p, &p); ++p;
	w -> opc = (unsigned long)util_ahtol(p, &p); ++p;
	w -> opa1 = (unsigned long)util_ahtol(p, &p); ++p;
	w -> opa2 = (unsigned long)util_ahtol(p, &p); ++p;
	msg.cmd = SV_COMM_WCREATE;
	msg.len = sizeof(*w);
	superv_exec(&msg);
	rv = *(int *)(msg.data);
	if (rv < 0)
		return DSV_E_BADARG;
	else
		dsv_answ_long(t, (long) rv);
	return -1;
}

/* Cancellazione di una lista di break. */
static int dsv_cmd_wdel(session_t *t)
{
	dsv_sv_exec_1(SV_COMM_WDELETE,
	              (int)util_ahtol(t -> args, (char **)NULL));
	return 0;
}

/* Disabilitazione di una lista di break. */
static int dsv_cmd_wdisable(session_t *t)
{
	return dsv_sv_exec_1(SV_COMM_WDISABLE,
	                     (int)util_ahtol(t -> args, (char **)NULL));
}

/* Abilitazione di una lista di break. */
static int dsv_cmd_wenable(session_t *t)
{
	return dsv_sv_exec_1(SV_COMM_WENABLE,
	                     (int)util_ahtol(t -> args, (char **)NULL));
}

/* Funzione per scaricare gli eventi di debug in vari modi. */
static int dvs_download_events(session_t *t, int mode)
{
unsigned char *b1,*b2;
unsigned long n1,n2,cy;
long n;
dsv_answ_t a;

	n1 = n2 = 0;
	if (mode == 0) {
/* Ipotesi soggiacenti: la risposta sara` trasmessa interamente
 prima che l'esecutore ricopra il buffer di debug. */
		n = brkLastEvent(&b1,&n1,&b2,&n2,&cy);
		if (n == 0 || cy == t -> last_watch_cycle)
			return DSV_E_BADSTS;
	}
	else {
		n = brkHistory(&b1,&n1,&b2,&n2);
		if (n == 0)
			return DSV_E_BADSTS;
	}
	n = (long)(n1 + n2);
	dsv_answ_init_long_data(&a,t,n);
	dsv_answ_addhv(&a,b1,(int)n1,0);
	if (n2)
		dsv_answ_addhv(&a,b2,(int)n2,0);
	dsv_answ_flush(&a);
	return -1;
}

/* Manipolazione della storia degli eventi di break. */
static int dsv_cmd_whistory(session_t *t)
{
int rv = 0;

	switch (t -> args[0]) {
	case 'l':

	/* Abilitazione della modalita` "lunga": la lettura
	 della storia riportera` l'intero contenuto del buffer.
	  E` possibile la perdita di qualche evento di break.
	  Questa e` la modalita` di funzionamento per default. */
		dsv_sv_exec_0(SV_COMM_WLONGHISTORY);

		break;

	case 's':

	/* Abilitazione della modalita` "compressa": la lettura
	 della storia riportera` solo un evento per ogni lista di break;
	 dopo la lettura, l'esecutore registrera` di nuovo un solo evento
	 per ogni lista, finche` non si leggera` di nuovo la storia, o non
	 si riabilitera` la modalita` "l", o non si chiudera` la
	 sessione. */
		dsv_sv_exec_0(SV_COMM_WSHORTHISTORY);

		break;

	case 'v':

	/* Lettura della storia. Il risultato dipende dalla modlaita`
	 impostata. */

	/* Comunica all'esecutore che sta per iniziare una lettura
	 della storia. */
		dsv_sv_exec_0(SV_COMM_WLOCKHISTORY);

	/* Legge la storia e la trasmette come risposta dati. */
		rv = dvs_download_events(t, 1);

	/* Comunica all'esecutore che il buffer della storia e` di
	 nuovo disponibile. */
		dsv_sv_exec_0(SV_COMM_WUNLOCKHISTORY);

		break;

	default:
		rv = DSV_E_BADARG;
		break;
	}
	return rv;
}

/* Cancellazione di tutte le liste. */
static int dsv_cmd_wreset(session_t *t)
{
	dsv_sv_exec_0(SV_COMM_WRESET);
	return 0;
}

/* Visualizzazione dei dati di debug relativi all'ultimo ciclo. */
static int dsv_cmd_wshow(session_t *t)
{
	return dvs_download_events(t, 0);
}

/*
* Esecuzione della linea di comando relativa alla sessione "t".
* il campo "argc" contiene il numero di parole che compongono il comando,
* il campo "line" contiene le parole, separate tra loro da un
* carattere '\0'.
* Questa funzione restituisce il codice di errore del comando eseguito,
* oppure -1 se il comando e` vuoto.
*/

/* Puntatori alle funzioni che realizzano i vari comandi. */
struct dsv_cmd_tab_t {
/* Nome del comando. */
	const char * n;
/* Puntatore alla funzione. */
	int (*f)(session_t *);
/* Numero di argomenti richiesti (-1 = variabile). */
	int a;
};

/* NOTA : QUESTA LISTA DEVE ESSERE ORDINATA PER NOME. */
static const struct dsv_cmd_tab_t dsv_cmd_tab[] = {
/* Richiede ("e") o sospende ("d") la trasmissione degli errori di Plc. */
{ "errs", dsv_cmd_errs, 1 },
/* Statistica della memoria libera. */
{ "free", dsv_cmd_free, 0 },
/* Mette in GO. */
{ "go", dsv_cmd_go, 0 },
/* Mette in HALT. */
{ "halt", dsv_cmd_halt, 0 },
/* Legge un ingresso. */
{ "in", dsv_cmd_in, 1 },
/* Informazioni su periodo PLC, massima dim. del buffer di trace, maxximo
 numero di variabili tracciabili, massimo numero di variabili gestite da
 "memcopy", e chi piu` ne ha... */
{ "info", dsv_cmd_info, 0 },
/* Acquisizione non invasiva dello stato di un segnale di I/O. */
{ "iostat", dsv_cmd_iostat, -1 },
/* Informazioni sugli oggetti da utilizzare per la comunicazione locale. */
{ "ipcs", dsv_cmd_ipcs, 0 },
/* Elenco dispositivi. */
{ "list", dsv_cmd_list, -1 },
/* Carica il programma contenuto nel file arg1. */
{ "load", dsv_cmd_load, 1 },
/* Visualizza arg2 byte all'indirizzo arg1. */
{ "mem", dsv_cmd_mem, 2 },
/* Convalida e registra aree in debug. */
{ "memchk", dsv_cmd_memchk, -1 },
/* Visualizza i contenuti della aree in debug. */
{ "memcopy", dsv_cmd_memcopy, 0 },
/* Richiede ("e") o sospende ("d") la trasmissione di messaggi di Plc. */
{ "messages", dsv_cmd_messages, 1 },
/* Visualizza informazioni sul modulo di nome arg1 . */
{ "mod", dsv_cmd_mod, 1 },
/* Forza un'uscita. */
{ "out", dsv_cmd_out, 2 },
/* Chiude la sessione. */
{ "quit", dsv_cmd_quit, 0 },
/* Assegnamento a variabile: set varabile indice indice valore... */
{ "set", dsv_cmd_set, -1 },
/* Simulazione di valore di input: setin descrittore valore */
{ "setin", dsv_cmd_setin, 2 },
/* Statistica: c=cancella, d=disabilita, e=abilita, v=visualizza. */
{ "stat", dsv_cmd_stat, 1 },
/* Stato del plc. */
{ "status", dsv_cmd_status, 0 },
/* Conversione di un indirizzo nel piu` prossimo simbolo. */
{ "sym", dsv_cmd_sym, 1 },
/* Trace: a <addr...>=crea lista, c=cancella, d=disabilita,
 e=abilita, v=visualizza. */
{ "trace", dsv_cmd_trace, -1 },
/* Trigger: a <addr> <sz> <op> <param>=crea trigger, c=cancella tutto,
 d <id>=cancella id. */
{ "trigger", dsv_cmd_trigger, -1 },
/* Risoluzione di un nome. */
{ "var", dsv_cmd_var, -1 },
/* Versione di QPLC. */
{ "ver", dsv_cmd_ver, 0 },
/* Aggiunta di un punto di break alla lista.
  Parametri: id addr r1 r2 scale size off ar. */
{ "wadd", dsv_cmd_wadd, 8 },
/* Creazione di una lista di break.
  Parametri: act cond apar opc opa1 opa2. */
{ "wcreate", dsv_cmd_wcreate, 7 },
/* Cancellazione di una lista di break. */
{ "wdel", dsv_cmd_wdel, 1 },
/* Disabilitazione di una lista di break. */
{ "wdisable", dsv_cmd_wdisable, 1 },
/* Abilitazione di una lista di break. */
{ "wenable", dsv_cmd_wenable, 1 },
/* Gestione avanzata degli eventi di break. */
{ "whistory", dsv_cmd_whistory, 1 },
/* Cancellazione di tutte le liste. */
{ "wreset", dsv_cmd_wreset, 0 },
/* Visualizzazione dei dati di debug relativi all'ultimo ciclo. */
{ "wshow", dsv_cmd_wshow, -1 },
};

static int dsv_exec_cmd_core(session_t * t)
{
char *p;
int il,ih,i,rv;
const struct dsv_cmd_tab_t *f;

	p = t -> line;

/* Ricerca binaria del comando nella tabella. */

	il = 0;
	ih = sizeof(dsv_cmd_tab)/sizeof(dsv_cmd_tab[0]);
	for (i = (il + ih) / 2; il < ih; i = (il + ih) / 2) {
		f = &dsv_cmd_tab[i];
		rv = strcmp(f -> n,p);
		if (rv == 0) {
			if (f -> a >= 0 && f -> a != t -> argc - 1)
				return DSV_E_NARGS;
			else {
				t -> args = p + strlen(p) + 1;
				--(t -> argc);
				return (*(f -> f))(t);
			}
		}
		else if (rv < 0) {
			il = i + 1;
		}
		else {
			ih = i;
		}
	}
	return DSV_E_UNKNOWN;
}

static void dsv_exec_cmd(void *fixed_arg, void *cd)
{
int rv;
session_t *t = (session_t *)(*(void **)(((sv_msg_t *)cd) -> data));

	rv = dsv_exec_cmd_core(t);
	*(int *)(((sv_msg_t *)cd) -> data) = rv;
}

/*
* Questa funzione aggiunge il carattere "c" alla linea di comando
* in corso di costruzione per la sessione "t". Se il carattere e`
* '\0', e` aggiunto solo se non e` il primo della linea e il precedente
* carattere non era '\0'.
* La funzione incrementa il contatore di parole (campo "argc" di "t")
* ogni volta che una nuova parola e` completata con uno '\0'.
* E` restituito il valore 1 se tutto e` andato bene, 0 se si e` raggiunta
* la massima lunghezza prevista per un comando.
*/

static int dsv_add_char_to_line(session_t *t, int c)
{
int lch = t -> lastch;

	if (c == '\0') {
/* Evita di aggiungere il carattere '\0' se si e` a inizio linea
 o il precedente carattere era gia` '\0'. */
		if (lch == 0 || t -> line[lch - 1] == '\0')
			return 1;
/* Se il carattere '\0' puo` essere aggiunto, e` stata completata una
 nuova parola. Incremnta "argc" di conseguenza. */
		++(t -> argc);
	}
	if (lch < MAX_CMD) {
/* Se non si e` raggiunto il limite fisico del buffer, si puo`
 aggiungere il nuovo carattere alla linea. */
		t -> line[lch] = c;
		t -> lastch = lch + 1;

		return 1;
	}
/* Errore. Buffer pieno. */
	return 0;
}

/*
*  Questa funzione inoltra la richiesta di esecuzione del comando
* giunto dal client "t". L'esecuzione del comando e` a carico del
* thread supervisore, che esegue la funzione "dsv_exec_cmd" in
* risposta al codice "SV_COMM_CALLBACK".
*/

static int dsv_post_exec_request(session_t *t)
{
/* Area di transito per il messaggio da inviare al supervisore.
  Conterra` il puntatore al descrittore di sessione come dato iniziale,
 ed il codice di ritorno del comando all'uscita. Quest'area puo` essere
 una variabile automatica, a patto che si usi la primitiva di colloquio
 bloccante (superv_talk). In caso contrario, si deve ricorrere ad una
 variabile statica. */
sv_msg_t msg;

	if (t -> argc == 0)
		return -1;
	msg.len = sizeof(void *);
	msg.cmd = SV_COMM_CALLBACK;
	*(void **)(msg.data) = (void *) t;
	superv_talk(&msg);
	return *(int *)(msg.data);
}

/*
* Questa funzione interpreta il carattere "c" ricevuto
* dal canale associato alla sessione "t", e aggiorna di conseguenza
* lo stato della sessione. Sono eliminati gli spazi superflui,
* le parole che costituiscono il comando sono isolate e saparate dal
* carattere '\0'. Il campo "argc" di "t" conta il numero di parole
* contenute nella linea di comando. I caratteri '\r' e '\n' sono
* interpretati come terminatori di linea, la ricezone di uno qualsiasi dei
* due scatena l'interpretazione e l'esecuzione del comando.
* Le linee di comando troppo lunghe non sono eseguite, ma provocano l'errore
* DSV_E_LONGLINE. Le linee vuote sono ignorate. Eseguito il comando,
* o riconosciuto che la linea e` troppo lunga, viene trasmesso il
* prompt al client (OK o segnalazione di errore).
*/

static void dsv_handle_char(session_t *t, int c)
{
int rv;

	switch (c) {
	case '\n':
	case '\r':
	case '\0':
	/* Questi tre caratteri indicano il completamento della linea.
	 Si aggiunge un '\0' per chiudere l'ultima parola. */
		if (dsv_add_char_to_line(t,'\0')) {
	/* Se la linea non e` troppo lunga, e` interpretata ed eseguita. */
			rv = dsv_post_exec_request(t);
		}
		else {
	/* Linea troppo lunga. Si produce l'errore specifico. */
			rv = DSV_E_LONGLINE;
		}
	/* Emissione del prompt adatto al codice di errore "rv". */
		dsv_prompt(t,rv);
	/* Cancellazione della linea. */
		dsv_clear_line(t);
		break;
	case ' ':
	case '\t':
	/* I caratteri di spaziatura individuano la fine di una parola.
	 Percio`, al loro posto, e` aggiunto un '\0' alla linea. */
		dsv_add_char_to_line(t,'\0');
		break;
	default:
	/* Gli altri caratteri sono semplicemente accodati alla linea
	 in corso di costruzione. */
		dsv_add_char_to_line(t,c);
		break;
	}
}

/*
* Questa funzione gestisce gli eventi relativi ai file descriptor
* delle sessioni aperte. Gli eventi possibili sono la ricezione di
* uno i piu` caratteri, la chiusura della connessione, o un errore di
* comunicazione.
*/

static void dsv_handle_request(int fd)
{
int n;
session_t *t;
#ifdef UN_BYTE_ALLA_VOLTA
unsigned char c;
#else
static unsigned char line[MAX_CMD*2 + 1];
int i;
#endif

#ifdef UN_BYTE_ALLA_VOLTA
	n = read(fd, &c, 1);
#else
	n = recv(fd, line, sizeof(line) - 1, 0);
#endif

	t = dsv_sts.trans[dsv_sts.client_index[fd]];

	if (n <= 0) {

/* Chiusura della connessione o errore di comunicazione.
  Alcuni errori sono temporanei e possono essere ignorati, altri
 sono permanenti e costringono a chiudere la sessione. */

		if (n < 0 && (errno == EINTR))
			return;

	/* Chiusura della sessione. */

		dsv_end_trans(t);
	}
	else {

/* Carattere ricevuto dal client. */

#ifdef UN_BYTE_ALLA_VOLTA
		dsv_handle_char(t,c);
#else
		for (i = 0; i < n && t -> fd != -1; ++i)
			dsv_handle_char(t,line[i]);
#endif
	}
}

/*
* Questa funzione contiene il ciclo infinito percorso dal
* thread incaricato di gestire le sessioni. Il thread attende
* eventi sui file descriptor delle sessioni e su quello di connesisone
* al servizio, e gestisce gli eventi che man mano gli vengono
* comunicati.
*/

static void *dsv_main(void *arg)
{
int nfd,i;

	for (;;) {

		memcpy((char *)&dsv_sts.ready_chans,
		       (char *)&dsv_sts.open_chans, sizeof(fd_set));

		nfd = select(dsv_sts.maxfd+1,&dsv_sts.ready_chans,NULL,NULL,
		             (struct timeval *) NULL);

		if (nfd > 0
		 && FD_ISSET(dsv_sts.server_fd, &dsv_sts.ready_chans)) {

	/* Evento sul canale di server: richiesta di connessione. */

			--nfd;

			dsv_new_client();
		}

	/* Esamina gli eventi sui canali associati alle sessioni
	 gia` aperte, e aggiorna lo stato delle sessioni per le
	 quali ci sono novita`. */

		for (i = 0; --nfd >= 0; ++i) {
			while (!(FD_ISSET(i, &dsv_sts.ready_chans)))
				++i;
			dsv_handle_request(i);
		}
	}
	return (void *)NULL;
}

/*
* Questa funzione e` chiamata dal supervisore ogni volta che l'esecutore
* produce un evento asincrono tramite "plcEventNotify", "plcError",
* "plcMessage" e simili.
*/

static void dsv_event_cb(void *code, void *data)
{
dsv_answ_t a;
session_t *t;

	csem_wait(dsv_sts.sem,(struct timeval *)NULL);

	for (t = dsv_sts.tlist; t; t = t -> next) {
		if ((int)code >= sizeof(t -> flags)*8
		 || (t -> flags & (1 << (int)code))) {
			switch ((int)code) {
			case DSV_EVENT_TRIGGER:
				{
				dsv_trig_event_t *e = (dsv_trig_event_t *)data;

				if (t != e -> t)
					break;
				dsv_answ_init_event(&a,t,(int)code);
				dsv_answ_addfmt(&a," %x %x",
				 (unsigned char *)&e -> id, sizeof(e -> id),
				 (unsigned char *)&e -> v, sizeof(e -> v));
				dsv_answ_flush(&a);
				}
				break;

			case DSV_EVENT_ERROR:
				{
				sv_err_t *e = (sv_err_t *)data;
				dsv_answ_init_event(&a,t,(int)code);
				dsv_answ_addfmt(&a," %x %s",
						&e -> code, sizeof(e -> code),
						e -> text);
				dsv_answ_flush(&a);
				}
				break;

			case DSV_EVENT_HALT:
			case DSV_EVENT_FAULT:
			/* Sono eventi non mascherabili, e ne approfitto. */
			/* In halt, niente trace, ne` trigger. */
				t -> flags &= ~(1 << DSV_EVENT_TRACE);
				dsv_sts.trace_list = (session_t *) NULL;
				dsv_sts.trigger_list = (session_t *) NULL;
				goto DEFAULT_DSV_EVENT_HANDLER;

			case DSV_EVENT_TRACE:
				if (t != *(session_t **)data)
					break;
				goto DEFAULT_DSV_EVENT_HANDLER;

			case DSV_EVENT_DELERR:
				{
				int e = *(int *)data;
				dsv_answ_init_event(&a,t,(int)code);
				dsv_answ_addfmt(&a," %x",
						&e, sizeof(e));
				dsv_answ_flush(&a);
				}
				break;

			case DSV_EVENT_MESSAGE:
				{
				sv_cbmsg_t *m = (sv_cbmsg_t *)data;
				dsv_answ_init_event(&a,t,(int)code);
				if (m -> action == SV_MSG_ACTION_DELETE) {
					dsv_answ_addfmt(&a," %x %x",
						&m -> action,sizeof(m->action),
						&m -> code, sizeof(m -> code)
						);
				}
				else {
					dsv_answ_addfmt(&a," %x %x %s",
						&m -> action,sizeof(m->action),
						&m -> code, sizeof(m -> code),
						m -> text);
				}
				dsv_answ_flush(&a);
				}
				break;

			default:
			DEFAULT_DSV_EVENT_HANDLER:
				dsv_answ_init_event(&a,t,(int)code);
				dsv_answ_flush(&a);
				break;
			}
		}
	}

	csem_signal(dsv_sts.sem);
}

/*
* Callback chiamata dall'esecutore alla ricezione del comando
* del supervisore "SV_COMM_CBEXEC".
*/

static void dsv_exec_cb(void *f, void *cd)
{
/* Potra` servire. */
}

/*
* Callback chiamata dall'esecutore per realizzare il trace nuovo stile.
*/

static void dsv_trace_cb(void *f, void *cd)
{
session_t *t;

/* Per prima cosa, si eseguono i test e le azioni relative alle
 variabili sottoposte a trigger. Un possibile effetto e` lo sblocco di
 una o piu` sessioni di trace (vedi oltre). */

	for (t = dsv_sts.trigger_list; t; t = t -> trigger_next) {

	register int n = t -> ntrigger;
	register dsv_trig_t *p = t -> trigtab;

		while (n--) {
			if ((*(p -> cmp))(p)) {
		/* Esecuzione delle azioni associate. */
			int act = p -> act;
				if (act & DSV_TRIGGER_ACT_TRACEON)
					t -> trace_cond = 1;
				if (act & DSV_TRIGGER_ACT_TRACEOFF)
					t -> trace_cond = 0;
				if (act & DSV_TRIGGER_ACT_NOTIFY) {
				dsv_trig_event_t e;
					e.id = p -> id;
					e.t = t;
					e.v = p -> v.ul;
					plcEventNotify(DSV_EVENT_TRIGGER,
					               (char *)&e,sizeof(e));
				}
			}
			++p;
		}
	}

/* Ora controlla le operazioni di trace. */

	for (t = dsv_sts.trace_list; t; t = t -> trace_next) {

	/* Salta a pie` pari il client se la condizione non e` verificata. */

		if (! t -> trace_cond)
			continue;

		{
		register trace_event_t v = 0;
		register trace_event_t m = 1;
		register int n = t -> ntrace;
		register char **tab = t -> tracetab;

			while (n--) {
				if (**tab)
					v |= m;
				m <<= 1;
				++tab;
			}
			t -> tracebuf[t -> trace_pos] = v;
		}

		if (++(t -> trace_pos) == TRACE_BUFFER_SIZE)
			t -> trace_pos = 0;
		if (++(t -> trace_count) >= t -> trace_len) {
			t -> trace_bnext = t -> trace_pos;
			t -> trace_done = 1;
			t -> trace_count = 0;
		/* Notifica l'evento al client interessato. */
			plcEventNotify(DSV_EVENT_TRACE,(char *)&t,sizeof(t));
		}
	}
}

/*
* Questa funzione lancia il server di debug remoto.
* Deve essere chiamata alla fine della procedura di startup del PLC.
*/

void dsv_start(void)
{
	if (!main_status.remote_debug)
		return;

/* Si ignora il segnale SIGPIPE, cosi`, se qualcuno chiude la connessione
 a sorpresa, non succede niente. */
	signal(SIGPIPE,SIG_IGN);
/* Crea il semaforo di accesso alle liste. */
	dsv_sts.sem = csem_create();
/* Definisce la callback che il thread supervisore chiamera` in
 risposta al comando "SV_COMM_CALLBACK". Il thread server del debug
 remoto incarica in questo modo il supervisore di interpretare ed 
 eseguire i comandi in arrivo. */
	cb_set(QPLC_CB_SUPERV,dsv_exec_cmd,(void *)NULL,(cback_t *)NULL); 
/* Definisce la callback che il thread supervisore chiamera` in
 presenza di errori ("plcError"). */
	cb_set(QPLC_CB_PLCERR,dsv_event_cb,(void *)DSV_EVENT_ERROR,
	       (cback_t *)NULL); 
/* Definisce la callback che il thread supervisore chiamera` in
 caso di cancellazione di errori ("plcDelerr"). */
	cb_set(QPLC_CB_PLCDELERR,dsv_event_cb,(void *)DSV_EVENT_DELERR,
	       (cback_t *)NULL); 
/* Definisce la callback che il thread supervisore chiamera` in
 caso di emissione o cancellazione di messaggi ("plc*Message"). */
	cb_set(QPLC_CB_PLCMSG,dsv_event_cb,(void *)DSV_EVENT_MESSAGE,
	       (cback_t *)NULL); 
/* Definisce la callback che l'esecutore chiamera` alla ricezione del comando
 "SV_COMM_CBEXEC" del supervisore. */
	cb_set(QPLC_CB_EXECUTOR,dsv_exec_cb,(void *)NULL,(cback_t *)NULL); 
/* Definisce la callback che l'esecutore chiamera` per realizzare il trace
 "nuovo stile". */
	cb_set(QPLC_CB_TRACE,dsv_trace_cb,(void *)NULL,(cback_t *)NULL); 
/* Definisce la callback che il supervisore chiamera` per notificare
 un evento asincrono su richiesta dell'esecutore. */
	cb_set(QPLC_CB_EVNOTIFY,dsv_event_cb,(void *)NULL,(cback_t *)NULL); 
/* Inizializza la struttura di stato. */
	FD_ZERO(&dsv_sts.open_chans);
	dsv_sts.server_fd = -1;
	dsv_sts.maxfd = -1;
	dsv_sts.trace_list = (session_t *)NULL;
	dsv_sts.trigger_list = (session_t *)NULL;
/* Nessun descrittore allocato ancora. L'allocazione di descrittori di
 sessione avverra` solo quando necessario. */
	dsv_sts.n_alloc_trans = 0;
	dsv_sts.tlist = dsv_sts.free = (session_t *) 0;

/* Apre il canale server, rendendo disponibile il servizio TCP. */
	if (dsv_open_socket() < 0)
		return; /* Esce subito in caso di errori. */

/* Lancia il thread incaricato dell'effettiva gestione del servizio. */
	dsv_sts.tid = util_start_thread(dsv_main,util_startprio());
}

