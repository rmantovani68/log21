/*
* @(#) dr_ser.c 2.2 Fri Jul 28 16:51:54 MET DST 2000
*
* Driver delle comunicazioni seriali per QPLC.
*
* Forma dei parametri specifici :
*
*   accesso al dispositivo:
*    <dispositivo>.<velocità><parità><dato><stop>[<controllo di flusso>]
*    velocita`: 50...38400
*    parita`: n,o,e
*    dato: 5,6,7,8
*    stop: 1,2
*    controllo di flusso: n,x,r
*   
*
* Esempio di nome di variabile completo:
*
*       ser.COM1.9600n81
*        |    |    |
*        |    |    |
*        |    |    |
*        |    |    parametri di comunicazione per...
*        |    dispositivo 
*        di tipo SER.
*
* 09/07/97 GG 1.0 Prima stesura.
* 22/07/97 GG 1.1 Aggiunta la chiusura del file descriptor in caso di errori
*             (inimmaginabili).
* 31/07/97 GG 1.2 Corretto un bug nell'interpretazione della dimensione
*             della parola.
* 11/03/98 GG 1.3 Aggiunto il metodo "detach".
* 24/03/98 GG 1.4 Aggiunto il metodo "list".
* 01/04/98 GG 2.0 Rivoluzione dovuta a modifiche profonde in "drivers.h".
* 03/04/98 GG 2.1 Aggiunto il metodo "show" (vuoto).
* 28/07/00 GG 2.2 Aggiustamenti per compilazione su Linux.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#ifdef Linux
#include <sys/sem.h>
#else
#include <sem.h>
#endif
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/ioctl.h>

int close(int);
#ifndef B1050
#define B1050 9
#endif
#ifndef B2000
#define B2000 12
#endif

#include "qplc.h"

#include "util.h"

#include "glue.h"

#define MAX_SER 4
#define MAXSERNAME 32
#define MAXSERPARAM 12

#define DEFAULT_SER_TXQ_SIZE 512
#define DEFAULT_SER_RXQ_SIZE 512

/*
* Struttura di stato del driver.
*/

typedef struct _dr_serline {
/* Nome del dispositivo. */
	char name[MAXSERNAME];
/* File descriptor della linea seriale. */
	int fd;
/* Puntatori all'inizio ed alla fine coda di trasmissione. */
	char * txq;
	char * txq_end;
/* Posizioni di lettura e scrittura della coda di trasmissione. */
	char * txq_head;
	char * txq_tail;
/* Puntatori all'inizio ed alla fine coda di ricezione. */
	char * rxq;
	char * rxq_end;
/* Posizioni di lettura e scrittura della coda di ricezione. */
	char * rxq_head;
	char * rxq_tail;
/* Descrittore di estensione. */
	long ex;
} dr_serline_t;

typedef struct _dr_ser {
/* TID del thread di trasmissione. */
	pthread_t tid_rx;
/* TID del thread di ricezione. */
	pthread_t tid_tx;
/* Semaforo di controllo del thread di trasmissione. */
	csem_t tx_sem;
/* Dimensione delle code di trasmissione. */
	int txq_size;
/* Dimensione delle code di ricezione. */
	int rxq_size;

/* Strutture ad uso del thread ricevente. */
/* Numero di file descriptor utilizzati. */
	int nfds;
/* Tabella dei file descriptor. */
	int fds[MAX_SER];
/* Tabella degli indici di file descriptor nella tabella precedente
 pronti a fornire dati. */
	int ifds[MAX_SER];

/* Strutture "per dispositivo". */
	dr_serline_t lines[MAX_SER];

/* Parametri per il metodo "attach". */
	char name[MAXSERNAME];
	int speed,parity,cstop,csize,hwflow,swflow;
	
} dr_ser_t;

static dr_ser_t ser_status;

/*
* Funzione ser_chkparam()
* -----------------------
*
* Imposta i parametri di comunicazione.
*/

static int ser_chkparam(char * param, dr_ser_t *dr)
{
int speed,parity,cstop,csize,hwflow,swflow;
int i;
char *p,c;
static int tabella[] = {
50, 75, 110, 134, 150, 200, 300, 600, 1050, 1200,
1800, 2000, 2400, 4800, 9600, 19200, 38400, };
static int codice[] = {
B50, B75, B110, B134, B150, B200, B300, B600, B1050, B1200,
B1800, B2000, B2400, B4800, B9600, B19200, B38400, };
static int cs[] = { CS5, CS6, CS7, CS8, };


	for (i = speed = 0, p = param; isdigit(*p); ++p) {
		speed = speed * 10 + (*p - '0');
	}
	for (i = 0; i < sizeof(tabella)/sizeof(tabella[0]); ++i) {
		if (tabella[i] == speed)
			break;
	}
	if (i >= sizeof(codice)/sizeof(codice[0]))
		return 0;

	speed = codice[i];

	switch (toupper(*p)) {
	case 'N':
		parity = 0;
		break;
	case 'O':
		parity = PARENB | PARODD;
		break;
	case 'E':
		parity = PARENB;
		break;
	default:
		return 0;
	}

	c  = toupper(*(++p));
	if (c < '5' || c > '8')
		return 0;
	csize = cs[c - '5'];

	c  = toupper(*(++p));
	if (c == '2')
		cstop = CSTOPB;
	else if (c == '1')
		cstop = 0;
	else
		return 0;

	switch (toupper(*(++p))) {
	case 'N':
	case '\0':
		hwflow = CLOCAL;
		swflow = 0;
		break;
	case 'X':
		hwflow = CLOCAL;
		swflow = IXON | IXOFF;
		break;
	case 'R':
		hwflow = 0;
		swflow = 0;
		break;
	default:
		return 0;
	}

	dr -> swflow = swflow;
	dr -> hwflow = hwflow;
	dr -> parity = parity;
	dr -> cstop = cstop;
	dr -> csize = csize;
	dr -> speed = speed;

	return 1;
}

/*
* Funzione ser_main_rx()
* ----------------------
*
*  Questo e` il "main" del thread di ricezione.
*/

static void * ser_main_rx(void * arg)
{
int i,n;
register dr_serline_t *pl;
char *q;

	for (;;) {
		n = test_nfd(ser_status.fds,ser_status.ifds,
		             ser_status.nfds,(long)-1);
		for (i = 0; i < n; ++i) {
			pl = &ser_status.lines[ser_status.ifds[i]];
			q = pl -> rxq_tail;
			if (read(pl -> fd,q,1) == 1) {
				if (++q == pl -> rxq_end)
					pl -> rxq_tail = pl -> rxq;
				else
					pl -> rxq_tail = q;
			}
		}
	}
}

/*
* Funzione ser_main_tx()
* ----------------------
*
*  Questo e` il "main" del thread di trasmissione.
*/

static void * ser_main_tx(void * arg)
{
int i,n;
dr_serline_t *pl;
char *q;

	for (;;) {
		csem_wait(ser_status.tx_sem, (struct timeval *) 0);

		for (i = 0, pl = ser_status.lines;
		     i < ser_status.nfds;
		     ++i, ++pl) {

			q = pl -> txq_head;
			while (q != pl -> txq_tail) {

				n = pl -> txq_tail - q;
				if (n < 0)
					n = pl -> txq_end - q;

				n = write(pl -> fd, q, n);
				if (n <= 0)
					break;

				q += n;
				if (q == pl -> txq_end)
					q = pl -> txq;
				pl -> txq_head = q;
			}
		}
	}
}

static int ser_install(driver_t * d, int conf)
{
int i,txq,rxq;
char *p;
dr_serline_t *pl;

	if (d -> instance) {
	/* ERRORE. Troppe istanze del driver. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	txq = DEFAULT_SER_TXQ_SIZE;
	rxq = DEFAULT_SER_RXQ_SIZE;

	for (ioStartResource(d); (p = ioNextResource(d)); ) {
		if (util_strneq(p,"txq=",4)) {
			p += 4;
			txq = ioGetNum(&p);
		}
		else if (util_strneq(p,"rxq=",4)) {
			p += 4;
			rxq = ioGetNum(&p);
		}
#if 0
		else if (util_strneq(p,"params=",7)) {
			p += 7;
		}
#endif
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}

/* I thread sono inizialmente marcati "non esistenti". Saranno creati
 solo se richiesto. */
	ser_status.tid_rx = ser_status.tid_tx = -1;
/* Creazione del semaforo di controllo del thread di trasmissione. */
	ser_status.tx_sem = csem_create_val(0);
	ser_status.txq_size = txq;
	ser_status.rxq_size = rxq;
	ser_status.nfds = 0;
/* Inizializzazione delle strutture "per linea". */
	for (i = 0, pl = ser_status.lines; i < MAX_SER; ++i, ++pl) {
		pl -> name[0] = '\0';
		pl -> fd = -1;
		pl -> txq = pl -> txq_end
		 = pl -> txq_head = pl -> txq_tail
		 = pl -> rxq = pl -> rxq_end
		 = pl -> rxq_head = pl -> rxq_tail
		 = NULL;
	}

	d -> device = (void *) &ser_status;
	return 1;
}

static void ser_clear(driver_t * d)
{
register dr_ser_t *dev = (dr_ser_t *) d -> device;
int i;
dr_serline_t *pl;

/* Chiusura dei file descriptor. */
	for (i = 0, pl = dev -> lines; i < dev -> nfds; ++i, ++pl) {
		pl -> name[0] = '\0';
		close(pl -> fd);
		dev -> fds[i] = pl -> fd = -1;
	}
}

static void ser_up(driver_t * d)
{
register dr_ser_t *dev = (dr_ser_t *) d -> device;

/* Se c'e` almeno un file descriptor, lancia i thread. */

	if (dev -> nfds) {
	/* Lancio del thread di ricezione. */
		if (dev -> tid_rx == -1)
			dev -> tid_rx
			 = util_start_thread(ser_main_rx,util_execprio() - 1);
	/* Lancio del thread di trasmissione. */
		if (dev -> tid_tx == -1)
			dev -> tid_tx
			 = util_start_thread(ser_main_tx,util_execprio() - 1);
	}
}

static void ser_down(driver_t * d)
{
/* Non so che farmene, ma lo lascio. */
}

static void ser_restart(driver_t * d)
{
dr_ser_t *dev = (dr_ser_t *) d -> device;

	dev -> nfds = 0;
}

static void ser_close(driver_t * d)
{
dr_ser_t *dev = (dr_ser_t *) d -> device;

/* Cancellazione dei thread di trasmissione e ricezione. */
	util_stop_thread(dev -> tid_tx);
	util_stop_thread(dev -> tid_rx);
	dev -> tid_tx = dev -> tid_rx = -1;
/* Chiusura di tutto. */
	ser_clear(d);
}

static int ser_parse(driver_t * d, int i_off, devnode_t *l)
{
dr_ser_t *dev = (dr_ser_t *) d -> device;
dr_serline_t *pl;
int i;
char c,*p,*name;
char nm[MAXSERNAME];

	name = l -> pname;

/* Il driver SER prevede una sola istanza, ma non vedo perche` dovrei
 faticare per non scrivere uno "0". */

	if (i_off != 0 || !(l -> flags & DRIVER_MODE_INPUT)) {
	/* ERRORE. Istanza inesistente. */
		return 0;
	}

	for (p = nm, i = 0; (c = name[i]) && c != '.'; ++i) {
		if (i < MAXSERNAME - 1) {
			*(p++) = tolower(c);
		}
	}
	*p = '\0';
	name += i;
	if (c == '.')
		++name;
	if (dev -> nfds >= MAX_SER) {
	/* ERRORE. Non ci sono piu` linee libere. */
		return 0;
	}

/* Controllo di congruenza: linea gia` utilizzata ? */
	for (i = 0, pl = dev -> lines; i < dev -> nfds; ++i, ++pl) {
		if (util_streq(pl -> name, nm)) {
		/* ERRORE. Linea gia` aperta. */
			return 0;
		}
	}

	if (! ser_chkparam(name, dev)) {
	/* ERRORE. Parametri insensati. */
		return 0;
	}

/* ATTENZIONE ! Non e` proprio il massimo della correttezza alterare
 qui la struttura di stato. In teoria, bisognerebbe codificare il
 valore in l -> next.spec, e farlo elaborare a ser_attach. */
	strcpy(dev -> name, nm);

	l -> ideep = 2;

	return (l -> nbit = sizeof(dev -> lines[dev -> nfds].ex) * 8);
}

static int ser_attach(driver_t * d, devnode_t *l, void * var)
{
dr_ser_t *dev = (dr_ser_t *) d -> device;
char path[MAXPATHLEN];
int fd,nfds;
dr_serline_t *pl;
struct termios t;
long ex;

	if (dev -> nfds >= MAX_SER) {
	/* ERRORE. Non ci sono piu` linee libere. */
		return 0;
	}

	nfds = dev -> nfds;

	pl = &(dev -> lines[nfds]);

	if (pl -> fd != -1) {
		close(pl -> fd);
		pl -> fd = -1;
	}

	util_safe_sprintf(path,sizeof(path),"/dev/%s",dev -> name);

	fd = open(path,O_RDWR | O_NDELAY);
	if (fd == -1) {
	/* ERRORE. Problemi di accesso al dispositivo. */
		return 0;
	}
		
	if (tcgetattr(fd,&t) < 0) {
	/* ERRORE. Problemi di accesso al dispositivo. */
		close(fd);
		return 0;
	}

	t.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO | ECHOE | ECHOK | ECHONL);
	t.c_iflag &= ~(ISTRIP  | INLCR | IGNCR | ICRNL | IUCLC
	                | IXON | IXOFF | IXANY);
	t.c_iflag |= (IGNBRK | IGNPAR | dev -> swflow);
	t.c_oflag &= ~(OPOST);
	t.c_cflag &= ~(CBAUD | CSIZE | PARENB | PARODD | CLOCAL | CSTOPB);
	t.c_cflag |= dev -> speed | dev -> csize | dev -> parity
	            | dev -> hwflow | dev -> cstop | CREAD;
	t.c_line = 0;
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;

	if (tcsetattr(fd,TCSANOW,&t) < 0) {
	/* ERRORE. Problemi di accesso al dispositivo. */
		close(fd);
		return 0;
	}

/* Allocazione dei buffer. */

	if (! pl -> txq) {
		pl -> txq = (char *) malloc(dev -> txq_size);
		if (! pl -> txq) {
		/* ERRORE. Non c'e` memoria. */
			close(fd);
			return 0;
		}
		pl -> txq_end = pl -> txq + dev -> txq_size;
	}
	pl -> txq_head = pl -> txq_tail = pl -> txq;
	if (! pl -> rxq) {
		pl -> rxq = (char *) malloc(dev -> rxq_size);
		if (! pl -> rxq) {
		/* ERRORE. Non c'e` memoria. */
			close(fd);
			return 0;
		}
		pl -> rxq_end = pl -> rxq + dev -> rxq_size;
	}
	pl -> rxq_head = pl -> rxq_tail = pl -> rxq;

/* Registrazione del file descriptor e definitiva occupazione del
 descrittore di linea. */

	pl -> fd = dev -> fds[nfds] = fd;
	strcpy(pl -> name, dev -> name);

	dev -> nfds = nfds + 1;

/* Registrazione del servizio aggiunto. */

	ex = ioRegisterExtension(d, (void *) pl);

/* Alla variabile agganciata e` assegnato un valore che permettera`
 di risalire a driver,istanza,parametri... */

	*((long *)var) = pl -> ex  = ex;

	if (! ex) {
	/* ERRORE. La tabella dei servizi estesi e` probabilmente piena. */
		return 0;
	}

	return 1;
}

static int ser_send(dr_ser_t *dev, dr_serline_t *pl, stream_rxtx_t *st)
{
char * txq_end = pl -> txq_end;
char * txq_head = pl -> txq_head;
register char * txq_tail = pl -> txq_tail;
int i;
int n = st -> msg_size;
char * p;

	for (i = st -> msg_curr_pos, p = st -> message + i; i < n; ++i,++p) {
		*txq_tail = *p;
		if (++txq_tail == txq_end)
			txq_tail = pl -> txq;
		if (txq_tail == txq_head) {
			if (txq_tail == pl -> txq)
				txq_tail = txq_end - 1;
			else
				--txq_tail;
			break;
		}
	}

	pl -> txq_tail = txq_tail;

	csem_signal(dev -> tx_sem);

	st -> msg_curr_pos = i;
	st -> status = (i == n) ? DREXTOP_STREAM_F_DONE : 0;

	return 1;
}

static int ser_recv(dr_ser_t *dev, dr_serline_t *pl, stream_rxtx_t *st)
{
char * rxq_end = pl -> rxq_end;
register char * rxq_head = pl -> rxq_head;
char * rxq_tail = pl -> rxq_tail;
int i;
int n = st -> msg_size;
char * p;

	for (i = st -> msg_curr_pos, p = st -> message + i; i < n; ++i,++p) {
		if (rxq_head == rxq_tail)
			break;
		*p = *rxq_head;
		if (++rxq_head == rxq_end)
			rxq_head = pl -> rxq;
	}

	pl -> rxq_head = rxq_head;

	st -> msg_curr_pos = i;
	st -> status = (i == n) ? DREXTOP_STREAM_F_DONE : 0;

	return 1;
}

static int ser_extension(driver_t * d, void *sys_p, int op, void *user_p)
{
	switch (op) {
	case DREXTOP_STREAM_OP_SEND:
		return ser_send((dr_ser_t *) d -> device,
		                (dr_serline_t *) sys_p,
		                (stream_rxtx_t *) user_p);
	case DREXTOP_STREAM_OP_RECV:
		return ser_recv((dr_ser_t *) d -> device,
		                (dr_serline_t *) sys_p,
		                (stream_rxtx_t *) user_p);
	default:
		return 0;
	}

	return 1;
}

driver_op_t driver_ser = {
	"ser",
	ser_install,
	ser_restart,
	ser_parse,
	(int (*)(driver_t *, devnode_t *)) 0, /* ser_list */
	ser_attach,
	ser_restart, /* ser_detach */
	(int (*)(driver_t *)) 0, /* ser_check */
	(void (*)(driver_t *)) 0, /* ser_trigger */
	ser_up,
	ser_down,
	ser_close,
	ser_clear, /* ser_fatal */
	(void (*)(driver_t *)) 0, /* ser_read */
	(void (*)(driver_t *)) 0, /* ser_write */
	(int (*)(driver_t *,devnode_t *,void *)) 0, /* ser_show */
	ser_extension,
};

