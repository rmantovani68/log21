
/*
* dr_autec.c 1.0 Sun Oct 14 12:08:48 CEST 2007
*
* Gestore per I/O digitali comandati da controllore Autec
*
* 14/10/07 1.0 Prima versione, derivata dal gestore CANOpen.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#if 0
#define TEST
#endif

#include "dr_autec.h"

#define AUTEC_N_DIN 32
#define AUTEC_N_DOUT 32
#define AUTEC_N_MOD 16

typedef struct autecmodule_t autecmodule_t;

struct autecmodule_t {
/* Stato del modulo: presente, non presente, non interrogato. */
	unsigned char state;
/* Numero di byte nel PDO di input. */
	unsigned char n_ib;
	unsigned int n_ibit; // Numero di bit, per comodita`
/* Numero di byte nel PDO di output. */
	unsigned char n_ob;
	unsigned int n_obit; // Numero di bit, per comodita`
/* Stato ingressi. */
	unsigned char in[AUTEC_N_IN];
/* Stato uscite. */
	unsigned char out[AUTEC_N_OUT];
};

struct autecboard_t {
/* Flag di debug. */
	int debug;
/* Linea seriale. */
	char devname[200];
/* File descriptor del canale (sdoppiato per debug). */
	int fd_rx;
	int fd_tx;
/* Velocita` di linea. */
	int speed;
/* Pipe per comandare il ricevitore. */
	int pipe_mr[2];
/* Pipe per risposte dal ricevitore. */
	int pipe_rm[2];
/* Numero di file descriptor gestiti (primo parametro di select). */
	int nfd;
/* Flag di ricevitore dormiente. */
	int rx_sleeping;
/* Thread ricevitore. */
	pthread_t rx_thread;
/* Stato dei moduli. */
	autecmodule_t module[AUTEC_N_MOD];
/* Insieme dei moduli che richiedono update. Usare ffs per la ricerca. */
	int update_set[(AUTEC_N_MOD*2) / (sizeof(int) * 8)];

/* Buffer di trasmissione. */
	unsigned char tx[MAX_FRAME_SIZE];
/* Stato di riempimento del buffer (alla fine, dimensione buffer). */
	unsigned int n_tx;
/* Checksum di trasmissione. */
	unsigned char chksum;
};


/* Primitiva di trasmissione di un comando. */

#define AUTEC_CMD_READ_BIT 0xB4
#define AUTEC_CMD_READ_BYTE 0xE1
#define AUTEC_CMD_READ_ROM_BYTE 0xF0
#define AUTEC_CMD_READ_WORD 0xD2
#define AUTEC_CMD_READ_LONG 0xC3
#define AUTEC_CMD_WRITE_BIT 0x4B
#define AUTEC_CMD_WRITE_ROM_BYTE 0x0F
#define AUTEC_CMD_WRITE_BYTE 0x1E
#define AUTEC_CMD_WRITE_WORD 0x2D
#define AUTEC_CMD_WRITE_LONG 0x3C

#define STX 0x7E

static void autec_add_tx_raw(autecboard_t *brd, unsigned int c)
{
	brd -> tx[brd -> n_tx] = c;
	++brd -> n_tx;
	brd -> chksum += (unsigned char)c;
}

static void autec_add_tx_chksum(autecboard_t *brd)
{
	autec_add_tx_raw(brd, brd -> chksum);
}

static void autec_add_tx(autecboard_t *brd, unsigned int c)
{
	autec_add_tx_raw(brd,c);
	if (c == STX) {
		autec_add_tx_raw(brd, 0x00);
	}
}
static void autec_reset_chksum(autecboard_t *brd)
{
	brd -> chksum = 0;
}

static void autec_reset_tx(autecboard_t *brd)
{
	brd -> n_tx = 0;
	brd -> chksum = 0;
}

static int autec_send(autecboard_t *brd)
{
int rv;
unsigned int n;

	n = 0;
	do {
		rv = write(brd -> fd_tx, brd -> tx + n, brd -> n_tx - n);
		if (rv < 0) {
			if (errno != EINTR) {
				return 0;
			}
		}
		else {
			n += (unsigned int) rv;
		}
	} while (n < brd -> n_tx);
	return 1;
}

int autec_send_cmd(autecboard_t *brd,
                   unsigned int slave,
                   unsigned int md,
                   unsigned int cmd,
                   unsigned int address,
                   unsigned char *data,
                   unsigned int size)
{
unsigned int i;
unsigned int nb;
unsigned int len_limit = 0;

	if (slave >= 16 || size >= MAX_FRAME_SIZE / 2 - 14) {
		return 0;
	}
#if 0
/* Schifezza, invero */
	if ((((cmd & 0x0F) + ((cmd & 0xF0) >> 4)) != 0x0F)
	 || ((cmd >= 0x50) && (cmd <= 0xB0))) {
		return 0;
	}
#else
#if 0
	if ((cmd % 15 != 0)
	 || ((cmd >= 0x50) && (cmd <= 0xB0))) {
		return 0;
	}
#endif
#endif

	nb = size;

	switch (cmd) {
	case AUTEC_CMD_READ_BIT:
	case AUTEC_CMD_WRITE_BIT:
		if (size != 2) {
			return 0;
		}
		break;
	case AUTEC_CMD_READ_BYTE:
	case AUTEC_CMD_READ_ROM_BYTE:
		len_limit = 1;
		nb = 0;
		break;
	case AUTEC_CMD_READ_WORD:
		len_limit = 2;
		nb = 0;
		break;
	case AUTEC_CMD_READ_LONG:
		len_limit = 4;
		nb = 0;
		break;
	defaut:
		break;
	}

	autec_reset_tx(brd);
	autec_add_tx_raw(brd, STX);
	autec_reset_chksum(brd);
	autec_add_tx(brd, 0x30 + slave);
	autec_add_tx(brd, md);
	autec_add_tx(brd, cmd);
	autec_add_tx(brd, address / 0x100);
	autec_add_tx(brd, address % 0x100);
	if (len_limit != 0 && size > len_limit) {
	unsigned int sz;

		sz = (size + len_limit - 1) / len_limit;
		autec_add_tx(brd, sz / 0x100);
		autec_add_tx(brd, sz % 0x100);
	}
	for (i = 0; i < nb; ++i) {
		autec_add_tx(brd, data[i]);
	}
	autec_add_tx_chksum(brd);
	autec_add_tx_raw(brd, STX);
	autec_add_tx_raw(brd, 0x0D);

	return autec_send(brd);
}

static int autec_recv_char(autecboard_t *brd)
{
struct timeval t;
fd_set set;
unsigned char c;

	t.tv_sec = 0;
	t.tv_usec = 20000;

	for (;;) {
		FD_ZERO(&set);
		FD_SET(brd -> fd_rx, &set);
		rv = select(brd -> fd_rx + 1, &set, NULL, NULL, &t);
		if (rv == -1) {
			if (errno != EINTR) {
				return -1;
			}
		}
		else if (rv == 0) {
			return -1;
		}
		else {
			rv = read(brd -> fd_rx, &c, 1);
			if (rv != 1) {
				return -1;
			}
			return c;
		}
	}
}

static void autec_reset_rx(autecboard_t *brd, unsigned char *data, size_t size)
{
	brd -> chksum = 0;
	brd -> rx_sts = 0;
	brd -> rx_data = data;
	brd -> rx_size = size;
}

static int autec_parse_char(autecboard_t *brd, int ch)
{
int rv = -1;

	switch (brd -> rx_sts) {
	case 0:
	/* Attesa codice errore. */
		if (ch == STX) {
		}
		else if (ch != ERROR_OK) {
			brd -> rx_sts = 1;
		}
		break;
	case 1:
	/* Attesa ESC in codice errore. */
		break;
	default:
		rv = -1;
		break;
	}
	return rv;
}

int autec_recv_answ(autecboard_t *brd,
                    unsigned char *data,
                    size_t size)
{
int ch;
int rv;

	autec_reset_rx(brd, data, size);
	do {
		ch = autec_recv_char(brd);
		if (ch == -1) {
			return 0;
		}
		rv = autec_parse_char(brd, ch);
	} while (rv > 0);
	return rv == 0;
}

int autec_read(autecboard_t *brd,
               unsigned int slave,
               unsigned int md,
               unsigned int address,
               unsigned char *data,
               size_t size)
{
unsigned int cmd;
unsigned int err;
int rv;

#if 0
/* Pessima idea: i byte arriveranno probabilmente rovesciati. */
	switch (size) {
	case 0:
		return 0;
	case 2:
		cmd = AUTEC_CMD_READ_WORD;
		break;
	case 4:
		cmd = AUTEC_CMD_READ_LONG;
		break;
	case 1:
	default:
		cmd = AUTEC_CMD_READ_BYTE;
		break;
	}
#else
	cmd = AUTEC_CMD_READ_BYTE;
#endif
	rv = autec_send_cmd(brd, slave, md, cmd, address, NULL, size)
	if (rv == 0) {
		return 0;
	}

	rv = autec_recv_data(brd, data, size, &err);

	return rv;
}




/******************************************************************/

/**************** DERIVATO DA dr_can.c ****************************/


static void autecboard_dump(autecboard_t *cbr, const char *s, autecmsg_t *msg)
{
int i;

	if (! (cbr -> debug & 1)) {
		return;
	}
	printf("%s %d: ID=%03x f=%x l=%d d=",s,cbr -> line,
	       msg->id,msg->flags,msg->length);
	for (i = 0; i < msg->length; ++i) {
		printf(" %02x",msg->data[i]);
	}
	printf("\n");
}

static int autecboard_recv(autecboard_t *cbr, autecmsg_t *msg)
{
int rv;

	rv = read(cbr -> fd_rx, msg, cbr -> size_rdwr);
	autecboard_dump(cbr,"RX",msg);
	return rv;
}

static void autecboard_handle_msg(autecboard_t *cbr, autecmsg_t *msg)
{
autecmodule_t *cm;
int mod;
int cmd;
int i;
int off;

	cmd = CANOPEN_GET_CMD(msg -> id);
	if (cmd == CANOPEN_GET_CMD(CANOPEN_COBID_PDO1TX(0))
	 || cmd == CANOPEN_GET_CMD(CANOPEN_COBID_PDO2TX(0))) {
		off = 8 * CANOPEN_COBID_PDOTXINDEX(cmd);
		mod = CANOPEN_GET_ID(msg -> id);
		cm = &(cbr -> module[mod]);
		if (cm -> state == CANMOD_STATE_PRESENT) {
		/* Aggiorniamo l'immagine degli ingressi.
		 Dobbiamo fare una copia byte per byte,
		 perche` non possiamo fidarci dei dati
		 oltre la lunghezza del messaggio. */
			for (i = 0; i < msg -> length; ++i) {
				cm -> in[i+off] = msg -> data[i];
			}
		}
	}
}

/* Thread ricevitore. */
static void *autecboard_rx_thread(void *arg)
{
autecboard_t *cbr = (autecboard_t *) arg;
autecmsg_t msg;
int rv;
fd_set set;
char cmd;

	if (cbr -> debug & 1) {
		printf("Receiver started\n");
	}

	for (;;) {

	/* Attesa di eventi (messaggi CAN o comandi di sospensione
	 della ricezione). */

		FD_ZERO(&set);
		if (! cbr -> rx_sleeping) {
			FD_SET(cbr -> fd_rx, &set);
		}
		FD_SET(cbr -> pipe_mr[0], &set);
		rv = select(cbr -> nfd,&set,NULL,NULL,NULL);
		if (rv <= 0) {
			break;
		}

		if (FD_ISSET(cbr -> pipe_mr[0],&set)) {
		/* Comando di sospensione o di ripresa della ricezione.
		 Lo decifriamo, e ci regoliamo di conseguenza. */
			if (read(cbr -> pipe_mr[0], &cmd, sizeof(cmd)) <= 0) {
				break;
			}
			if (cmd == 'd') {
			/* Richiesta di sospensione della ricezione. */
				if (cbr -> debug & 1) {
					printf("Receiver disabled\n");
				}
				cbr -> rx_sleeping = 1;
			}
			else {
			/* Richiesta di ripresa della ricezione. */
				if (cbr -> debug & 1) {
					printf("Receiver enabled\n");
				}
				cbr -> rx_sleeping = 0;
			}
			/* Risposta al richiedente. */
			write(cbr -> pipe_rm[1],"\n",1);
		}

		if ((! cbr -> rx_sleeping) && FD_ISSET(cbr -> fd_rx,&set)) {
			/* 
			* Messaggio CAN. Consideriamo solo i PDO conformi
			* a DS401, e provenienti da moduli identificati
			* correttamente. Il resto, per ora, e` ignorato. 
			*/
			rv = autecboard_recv(cbr, &msg);
			if (rv <= 0) {
				break;
			}
			autecboard_handle_msg(cbr, &msg);
		}
	}

	return NULL;
}

static void autecboard_disable_receiver(autecboard_t *cbr)
{
char c;

	write(cbr -> pipe_mr[1], "d", 1);
	read(cbr -> pipe_rm[0], &c, 1);
}
static void autecboard_enable_receiver(autecboard_t *cbr)
{
char c;

	write(cbr -> pipe_mr[1], "e", 1);
	read(cbr -> pipe_rm[0], &c, 1);
}

static void autecboard_close_core(autecboard_t *cbr)
{
	if (cbr) {
		close(cbr -> fd_rx);
		close(cbr -> fd_tx);
		close(cbr -> pipe_mr[0]);
		close(cbr -> pipe_mr[1]);
		close(cbr -> pipe_rm[0]);
		close(cbr -> pipe_rm[1]);
		free(cbr);
	}
}

/* Apertura di un gestore di bus CANOpen. */
autecboard_t *autecboard_open(int nch, int speed, int debug)
{
autecboard_t *cbr = NULL;
char dev[40];
int i;
Config_par_t  cfg;
volatile Command_par_t cmd;
pthread_attr_t thr_attr;
struct sched_param sch;
int policy;

	cbr = (autecboard_t *) malloc(sizeof(*cbr));
	if (! cbr) {
		goto autecboard_init_error;
	}

	cbr -> line = nch;
	cbr -> debug = debug;
	cbr -> speed = speed;
	cbr -> size_rdwr = 1;
	cbr -> rx_sleeping = 0;
	cbr -> fd_rx = -1;
	cbr -> fd_tx = -1;
	cbr -> pipe_mr[0] = -1;
	cbr -> pipe_mr[1] = -1;
	cbr -> pipe_rm[0] = -1;
	cbr -> pipe_rm[1] = -1;

/* Apertura del dispositivo. */
	sprintf(dev,"/dev/autec%d",nch);
	cbr -> fd_rx = open(dev,O_RDWR);
	if (cbr -> fd_rx == -1) {
		if (cbr -> debug & 2) {
			sprintf(dev,"/tmp/autec%d-rx",nch);
			mkfifo(dev,0777);
			cbr -> fd_rx = open(dev,O_RDWR);
			sprintf(dev,"/tmp/autec%d-tx",nch);
			mkfifo(dev,0777);
			cbr -> fd_tx = open(dev,O_RDWR);
			if (cbr -> fd_rx == -1 || cbr -> fd_tx == -1) {
				goto autecboard_init_error;
			}
			cbr -> nfd = cbr -> fd_rx + 1;
			if (cbr -> nfd <= cbr -> fd_tx) {
				cbr -> nfd = cbr -> fd_tx + 1;
			}
			cbr -> size_rdwr = sizeof(autecmsg_t);
		}
		else {
			goto autecboard_init_error;
		}
	}
	else {
		cbr -> fd_tx = cbr -> fd_rx;
		cbr -> nfd = cbr -> fd_rx + 1;
	}

/* Creazione della pipe per comandare il thread ricevitore. */
	if (pipe(cbr -> pipe_mr) < 0) {
		goto autecboard_init_error;
	}
	if (cbr -> nfd <= cbr -> pipe_mr[0]) {
		cbr -> nfd = cbr -> pipe_mr[0] + 1;
	}
	if (cbr -> nfd <= cbr -> pipe_mr[1]) {
		cbr -> nfd = cbr -> pipe_mr[1] + 1;
	}
/* Pipe per le risposte dal ricevitore. */
	if (pipe(cbr -> pipe_rm) < 0) {
		goto autecboard_init_error;
	}
	if (cbr -> nfd <= cbr -> pipe_rm[0]) {
		cbr -> nfd = cbr -> pipe_rm[0] + 1;
	}
	if (cbr -> nfd <= cbr -> pipe_rm[1]) {
		cbr -> nfd = cbr -> pipe_rm[1] + 1;
	}

/* Impostazione della velocita` di linea. */
	cmd.cmd = CMD_STOP;
	ioctl(cbr -> fd_rx, COMMAND, &cmd);

	cfg.target = CONF_TIMING; 
	cfg.val1 = cbr -> speed;
	ioctl(cbr -> fd_rx, CONFIG, &cfg);

	cmd.cmd = CMD_START;
	ioctl(cbr -> fd_rx, COMMAND, &cmd);

/* Inizializzazione dello stato dei moduli. */
	for (i = 0; i < 128; ++i) {
		cbr -> module[i].state = CANMOD_STATE_UNKNOWN;
		cbr -> module[i].n_ib = 0;
		cbr -> module[i].n_ob = 0;
		cbr -> module[i].n_ibit = 0;
		cbr -> module[i].n_obit = 0;
		memset(cbr -> module[i].in, 0, sizeof(cbr -> module[i].in));
		memset(cbr -> module[i].out, 0, sizeof(cbr -> module[i].out));
	}

/* Lancio del thread ricevitore. Da questo momento lo stato degli
 ingressi puo` evolvere. */
	pthread_attr_init(&thr_attr);
	pthread_attr_setinheritsched(&thr_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&thr_attr,SCHED_FIFO);

	if (pthread_create(&(cbr -> rx_thread), NULL,//&thr_attr,
	                   autecboard_rx_thread,(void *)cbr) < 0) {
		pthread_attr_destroy(&thr_attr);
		goto autecboard_init_error;
	}

	pthread_attr_destroy(&thr_attr);

	return cbr;

autecboard_init_error:
	autecboard_close_core(cbr);
	return NULL;
}

void autecboard_close(autecboard_t *cbr)
{
	autecboard_close_core(cbr);
}

static int autecboard_send(autecboard_t *cbr, autecmsg_t *msg)
{
	autecboard_dump(cbr,"TX",msg);
	return write(cbr -> fd_tx, msg, cbr -> size_rdwr);
}

/* Invio del PDO per l'aggiornamento delle uscite di un modulo. */
static void autecboard_write_output(autecboard_t *cbr, int mod, int extra)
{
autecmsg_t msg;
autecmodule_t *cm = &(cbr -> module[mod]);

/* Costruzione del primo PDO standard (DS401). */
	msg.cob = 0;
	msg.flags = 0;
	if (extra) {
		msg.id = CANOPEN_COBID_PDO2RX(mod);
		msg.length = cm -> n_ob - 8;
	/* Ottimizzazione orrenda! */
	#if 0
		memcpy(msg.data,cm -> out + 8, 8);
	#else
		*(long *)(msg.data) = *(long *)(cm -> out + 8);
		*(long *)(msg.data + 4) = *(long *)(cm -> out + 12);
	#endif
	}
	else {
		msg.id = CANOPEN_COBID_PDO1RX(mod);
		msg.length = cm -> n_ob <= 8 ? cm -> n_ob : 8;
	/* Ottimizzazione orrenda! */
	#if 0
		memcpy(msg.data,cm -> out, 8);
	#else
		*(long *)(msg.data) = *(long *)(cm -> out);
		*(long *)(msg.data + 4) = *(long *)(cm -> out + 4);
	#endif
	}
/* Emissione del messaggio. */
	autecboard_send(cbr, &msg);
}

/* Aggiornamento delle uscite, cioe` trasmissione delle
 nuove immagini delle uscite dei moduli che hanno subito
 una variazione. */
void autecboard_flush_output(autecboard_t *cbr)
{
int i;
int bit;

	for (i = 0;
	     i < sizeof(cbr -> update_set)/(2*sizeof(cbr -> update_set[0]));
	     ++i) {
		while (cbr -> update_set[i]) {
			bit = ffs(cbr -> update_set[i]) - 1;
			autecboard_write_output(cbr,bit + 32*i,0);
			cbr -> update_set[i] &= ~(1 << bit);
		}
	}
	for (i = sizeof(cbr -> update_set)/(2*sizeof(cbr -> update_set[0]));
	     i < sizeof(cbr -> update_set)/sizeof(cbr -> update_set[0]);
	     ++i) {
		while (cbr -> update_set[i]) {
			bit = ffs(cbr -> update_set[i]) - 1;
			autecboard_write_output(cbr,bit + 32*i - 128, 1);
			cbr -> update_set[i] &= ~(1 << bit);
		}
	}
}

/* Scrittura di un bit di un modulo. */
void autecboard_set_output_bit(autecboard_t *cbr, int mod, int bit, int val)
{
int m;
unsigned char *op;

/* Banale accensione di bit, un tantinello ottimizzata. */
	op = &(cbr -> module[mod].out[bit / 8]);
	m = 1 << (bit % 8);
	if (val) {
		val = m;
	}
	if ((*op & m) != val) {
		*op = (unsigned char)(val | (*op & ~m));
	/* C'e` stata una modifica: aggiungiamo il modulo all'insieme
	 di quelli che richiedono aggiornamento (cioe`: accendiamo
	 il bit corrispondente). */
		if (bit >= 8*8) {
			mod += 128;
		}
		cbr -> update_set[mod / (sizeof(cbr -> update_set[0])*8)]
		 |= 1 << (mod % (sizeof(cbr -> update_set[0])*8));
	}
}

/* Scrittura di un intero byte di un modulo. */
void autecboard_set_output_byte(autecboard_t *cbr, int mod, int b, int val)
{
unsigned char *op;

/* Banale accensione di bit, un tantinello ottimizzata. */
	op = &(cbr -> module[mod].out[b]);
	if (*op != val) {
		*op = (unsigned char) val;
	/* C'e` stata una modifica: aggiungiamo il modulo all'insieme
	 di quelli che richiedono aggiornamento (cioe`: accendiamo
	 il bit corrispondente). */
		if (b >= 8) {
			mod += 128;
		}
		cbr -> update_set[mod / (sizeof(cbr -> update_set[0])*8)]
		 |= 1 << (mod % (sizeof(cbr -> update_set[0])*8));
	}
}

/* Calcolo del puntatore all'immagine delle uscite di un modulo
 (scopo: lettura ottimizzata). */
unsigned char *autecboard_get_output_image(autecboard_t *cbr, int mod)
{
	return cbr -> module[mod].out;
}

/* Calcolo del puntatore all'immagine degli ingressi di un modulo
 (scopo: lettura ottimizzata). */
unsigned char *autecboard_get_input_image(autecboard_t *cbr, int mod)
{
	return cbr -> module[mod].in;
}

int autecboard_get_input_bit(autecboard_t *cbr, int mod, int bit)
{
	return (cbr -> module[mod].in[bit / 8] & (1 << (bit % 8))) != 0;
}

static int autecboard_recv_timeout(autecboard_t *cbr, autecmsg_t *msg, int timeout)
{
fd_set fds;
struct timeval tm;
int n;

	if (timeout < 0) {
		return autecboard_recv(cbr,msg);
	}
	FD_ZERO(&fds);
	FD_SET(cbr -> fd_rx,&fds);
	tm.tv_sec = timeout / 1000;
	tm.tv_usec = (timeout % 1000) * 1000;
	n = select(cbr -> fd_rx + 1,&fds,NULL,NULL,&tm);
	if (n < 0) {
		return -1;
	}
	else if (n == 0) {
		return 0;
	}
	return autecboard_recv(cbr,msg);
}

static int autecboard_sdo_answer(autecboard_t *cbr, autecmsg_t *msg, int id)
{
int rv;
time_t t0;

#define SDO_TIMEOUT 3

	t0 = time(NULL);
	do {
		rv = autecboard_recv_timeout(cbr,msg,SDO_TIMEOUT*1000);
		if (rv <= 0) {
			return -1;
		}
		if (time(NULL) - t0 >= 2*SDO_TIMEOUT) {
			return -1;
		}
		autecboard_handle_msg(cbr, msg);
	} while (msg -> id != CANOPEN_COBID_SDOTX(id));
	if (msg -> length != 8) {
		return -1;
	}
	return 0;
}

static int autecboard_sdo_recv(autecboard_t *cbr,
                     int id, int obj, int idx,
                     void *data, unsigned int *len)
{
autecmsg_t msg;
int rv;
unsigned char *p = (unsigned char *)data;
unsigned int tot;
int cont = 0;
int toggle = 0;
int n = 0;
int i;

	msg.id = CANOPEN_COBID_SDORX(id);
	msg.cob = 0;
	msg.length = 8;
	msg.flags = 0;
	msg.data[0] = CANOPEN_CCS_STARTUPLOAD;
	msg.data[1] = obj % 0x100;
	msg.data[2] = obj / 0x100;
	msg.data[3] = idx;
	msg.data[4] = 0;
	msg.data[5] = 0;
	msg.data[6] = 0;
	msg.data[7] = 0;
	rv = autecboard_send(cbr,&msg);
	if (rv < 0) {
		return -1;
	}
	rv = autecboard_sdo_answer(cbr,&msg,id);
	if (rv < 0) {
		return -1;
	}
	if ((msg.data[0] & CANOPEN_SCS_M) != CANOPEN_SCS_STARTUPLOAD) {
		return msg.data[7] * 0x100 + msg.data[6];
	}

	tot = 0;
	cont = 0;

	switch (msg.data[0] & 0x03) {
	case 0:
		cont = 1;
		tot = 0xFFFFFFFF;
		break;
	case 1:
		tot = msg.data[4]
		    + msg.data[5] * 0x100
		    + msg.data[6] * 0x10000
		    + msg.data[7] * 0x1000000;
		cont = 1;
		break;
	case 2:
		cont = 0;
		tot = 4;
		break;
	case 3:
		cont = 0;
		tot = 4 - ((msg.data[0] & 0x0C) >> 2);
		break;
	default:
		return -1;
	}

	if (*len < tot) {
		tot = *len;
	}

	if (! cont) {
		for (i = 0; i < tot; ++i) {
			p[i] = msg.data[i+4];
		}
		*len = tot;
		return 0;
	}

	toggle = 0;
	n = 0;
	while (cont && tot > 0) {
	int nb;

		msg.id = CANOPEN_COBID_SDORX(id);
		msg.cob = 0;
		msg.length = 8;
		msg.flags = 0;
		msg.data[0] = CANOPEN_CCS_SEGUPLOAD | toggle;
		msg.data[1] = 0;
		msg.data[2] = 0;
		msg.data[3] = 0;
		msg.data[4] = 0;
		msg.data[5] = 0;
		msg.data[6] = 0;
		msg.data[7] = 0;
		rv = autecboard_send(cbr,&msg);
		if (rv < 0) {
			return -1;
		}
		rv = autecboard_sdo_answer(cbr,&msg,id);
		if (rv < 0) {
			return -1;
		}
		if ((msg.data[0]&CANOPEN_SCS_M) != CANOPEN_SCS_SEGUPLOAD) {
			break;
		}
		toggle ^= 0x10;
		nb = 7 - ((msg.data[0] & 0x0E) >> 1);
		cont = msg.data[0] & 0x01;
		for(i = 0; i < nb && tot > 0; ++i, ++p, ++n, --tot) {
			*p = msg.data[i+1];
		}
	}

	if (cont) {
		msg.id = CANOPEN_COBID_SDORX(id);
		msg.cob = 0;
		msg.length = 8;
		msg.flags = 0;
		msg.data[0] = CANOPEN_CCS_ABORT;
		msg.data[1] = obj % 0x100;
		msg.data[2] = obj / 0x100;
		msg.data[3] = idx;
		msg.data[4] = 0x02; /* 0x05040002 : Invalid size */
		msg.data[5] = 0x00;
		msg.data[6] = 0x04;
		msg.data[7] = 0x05;
		rv = autecboard_send(cbr,&msg);
		if (rv < 0) {
		//	return -1;
		}
	}

	*len = n;

	return 0;
}

static int autecboard_sdo_send(autecboard_t *cbr,
                     int id, int obj, int idx,
                     void *data, unsigned int *len)
{
autecmsg_t msg;
int rv;
unsigned char *p = (unsigned char *)data;
unsigned int tot;
int toggle;
unsigned int n;
unsigned int i;

	if (*len < 1) {
		return -1;
	}

	tot = 0;
	n = *len;

	msg.id = CANOPEN_COBID_SDORX(id);
	msg.cob = 0;
	msg.length = 8;
	msg.flags = 0;
	msg.data[0] = CANOPEN_CCS_STARTDOWNLOAD
	            | 0x01; // CANOPEN_SDO_SIZED
	msg.data[1] = obj % 0x100;
	msg.data[2] = obj / 0x100;
	msg.data[3] = idx;
	if (*len > 4) {
		msg.data[4] = (unsigned char)(*len % 0x100);
		msg.data[5] = (unsigned char)((*len / 0x100) % 0x100);
		msg.data[6] = (unsigned char)((*len / 0x10000) % 0x100);
		msg.data[7] = (unsigned char)((*len / 0x1000000) % 0x100);
	}
	else {
		msg.data[0] |= 0x02  // CANOPEN_SDO_EXPEDITED
		             | ((4-*len) << 2);
		for (i = 0; i < *len; ++i) {
			msg.data[4+i] = p[i];
		}
		for (; i < 4; ++i) {
			msg.data[4+i] = 0;
		}
		tot = *len;
		n = 0;
	}
	rv = autecboard_send(cbr,&msg);
	if (rv < 0) {
		return -1;
	}
	rv = autecboard_sdo_answer(cbr,&msg,id);
	if (rv < 0) {
		return -1;
	}
	if ((msg.data[0] & CANOPEN_SCS_M) != CANOPEN_SCS_STARTDOWNLOAD) {
		return -1;
	}

	toggle = 0;

	while (n > 0) {
	unsigned int nb;

		if (n > 7) {
			nb = 7;
		}
		else {
			nb = n;
		}
		msg.id = CANOPEN_COBID_SDORX(id);
		msg.cob = 0;
		msg.length = 8;
		msg.flags = 0;
		msg.data[0] = CANOPEN_CCS_SEGDOWNLOAD
		            | toggle
		            | ((7 - nb) << 1);
		if (n <= 7) {
			msg.data[0] |= 0x01; // CANOPEN_SDO_NOMORE
		}
		for (i = 0; i < nb; ++i, ++p) {
			msg.data[i+1] = *p;
		}
		for (; i < 7; ++i) {
			msg.data[i+1] = 0;
		}
		rv = autecboard_send(cbr,&msg);
		if (rv < 0) {
			return -1;
		}
		rv = autecboard_sdo_answer(cbr,&msg,id);
		if (rv < 0) {
			return -1;
		}
		if ((msg.data[0]&CANOPEN_SCS_M) != CANOPEN_SCS_SEGDOWNLOAD) {
			break;
		}
		n -= nb;
		tot += nb;
		toggle ^= 0x10;
	}

	*len = tot;

	return 0;
}

int autecboard_read_dictionary(autecboard_t *cbr, int mod,
                             int idx, int subidx,
                             void *data, int size)
{
int rv;
unsigned int sz = (unsigned int) size;

	autecboard_disable_receiver(cbr);
	rv = autecboard_sdo_recv(cbr, mod, idx, subidx, data, &sz);
	autecboard_enable_receiver(cbr);

	return rv;
}

int autecboard_write_dictionary(autecboard_t *cbr, int mod,
                             int idx, int subidx,
                             void *data, int size)
{
int rv;
unsigned int sz = (unsigned int) size;

	autecboard_disable_receiver(cbr);
	rv = autecboard_sdo_send(cbr, mod, idx, subidx, data, &sz);
	autecboard_enable_receiver(cbr);

	return rv;
}

static int autecop_remap_pdo(autecboard_t *cbr, int in, int pdo, int mod, int nbit)
{
unsigned long x;
unsigned char c;
unsigned char m[4];
unsigned int v;
unsigned int map_idx, comm_idx, data_idx;
unsigned int data_subidx;
unsigned int data_len, data_bit;
unsigned int pdo_id;
unsigned int i;

	if (nbit <= 64 * pdo) {
		return 0;
	}

	if (in) {
		map_idx = 0x1A00;
		comm_idx = 0x1800;
		data_idx = 0x6000;
		pdo_id = mod + 0x80;
	}
	else {
		map_idx = 0x1600;
		comm_idx = 0x1400;
		data_idx = 0x6200;
		pdo_id = mod + 0x100;
	}

	map_idx += pdo;
	comm_idx += pdo;
	pdo_id += 0x100 * (pdo + 1);
	data_subidx = 1 + pdo * 8;
	if (nbit > 64 * (pdo + 1)) {
		data_bit = 64;
	}
	else {
		data_bit = nbit - 64 * pdo;
	}
	data_len = (data_bit + 7) / 8;

/* Inibizione del PDO. */
	x = comm_idx;
	m[0] = pdo_id % 0x100;
	m[1] = pdo_id / 0x100;
	m[2] = 0;
	m[3] = 0xC0;
	if (autecboard_write_dictionary(cbr,mod,x,1,&m,sizeof(m)) < 0) {
		return -1;
	}
/* Azzeramento della sua mappa. */
	x = map_idx;
	c = 0;
	if (autecboard_write_dictionary(cbr,mod,x,0,&c,sizeof(c)) < 0) {
		return -1;
	}
/* Mappatura di un canale. */
	for (i = 0; i < data_len; ++i) {
		m[0] = 8;
		m[1] = data_subidx + i;
		m[2] = data_idx % 0x100;
		m[3] = data_idx / 0x100;
		if (autecboard_write_dictionary(cbr,mod,x,1+i,&m,sizeof(m)) < 0){
			return -1;
		}
	}
/* Definizione della lunghezza della mappa. */
	c = data_len;
	if (autecboard_write_dictionary(cbr,mod,x,0,&c,sizeof(c)) < 0) {
		return -1;
	}
/* Impostazione dell'inhibit time sul PDO. Scegliamo un
 valore dell'ordine del tempo di ciclo. */
	x = comm_idx;
/* TODO: Parametrizzare questa grandezza. */
	v = 100; // unita` = 100 microsecondi */
	m[0] = v % 0x100;
	m[1] = v / 0x100;
	if (autecboard_write_dictionary(cbr,mod,x,3,&m,2) < 0) {
		return -1;
	}
/* Impostazione della modalita` asincrona. */
	c = 254;
	if (autecboard_write_dictionary(cbr,mod,x,2,&c,sizeof(c)) < 0) {
		return -1;
	}
/* Riattivazione del PDO. */
	m[0] = pdo_id % 0x100;
	m[1] = pdo_id / 0x100;
	m[2] = 0;
	m[3] = 0x40;
	if (autecboard_write_dictionary(cbr,mod,x,1,&m,sizeof(m)) < 0) {
		return -1;
	}
	return 0;
}

static int autecboard_check_module(autecboard_t *cbr, int mod)
{
autecmodule_t *cm;
int rv;
int n;

	if (mod < 1 || mod > 127) {
		return 0;
	}

	cm = &(cbr -> module[mod]);

	switch (cm -> state) {
	case CANMOD_STATE_PRESENT:
		return 1;
	case CANMOD_STATE_NOTPRESENT:
		return 0;
	case CANMOD_STATE_UNKNOWN:
	/* Portiamo nello stato PREOPERATIONAL il modulo.
	 Cosi` siamo sicuri di poterlo manipolare via SDO. */
		autecboard_preop_module(cbr, mod);

	/* Leggiamo le voci 0x6000:0 (numero di byte per ingressi digitali)
	 e 0x6200:0 (numero di byte per uscite digitali), come da DS401.
	 Non controlliamo invece la voce 0x1000:0, come dovremmo,
	 perche` si da` il caso che possiamo avere a che fare con
	 moduli (CNi-Informatica) che non riportano il valore
	 atteso, che sarebbe 401.
	  Se entrambe le letture falliscono o danno 0 come risultato,
	 deduciamo che il modulo non e` un dispositivo di I/O digitale,
	 oppure non esiste o non funziona. In ogni caso lo marchiamo
	 come "inesistente". */

		rv = autecboard_read_dictionary(cbr, mod,
		                              0x6000, 0,
		                              &n, sizeof(n));
		if (rv != 0) {
			cm -> n_ib = 0;
		}
		else {
			if (n > 16) {
				n = 16;
			}
			cm -> n_ib = n;
		}
		cm -> n_ibit = cm -> n_ib * 8;

		rv = autecboard_read_dictionary(cbr, mod,
		                              0x6200, 0,
		                              &n, sizeof(n));
		if (rv != 0) {
			cm -> n_ob = 0;
		}
		else {
			if (n > 16) {
				n = 16;
			}
			cm -> n_ob = n;
		}
		cm -> n_obit = cm -> n_ob * 8;

		if (cbr -> debug & 1) {
			printf("Module %d: n_in=%d, n_out=%d\n",
			       mod, cm -> n_ibit, cm -> n_obit);
		}

		if (cm -> n_ib == 0 && cm -> n_ob == 0) {
		/* E` andata male. */
			cm -> state = CANMOD_STATE_NOTPRESENT;
		}
		else {
		/* Il modulo esiste! */
		/* Rimappiamo tutti i PDO che sono
		 necessari per ricoprire i segnali disponibili. */
			//autecop_remap_pdo(cbr, 1, 0, mod, cm -> n_ibit);
			autecop_remap_pdo(cbr, 1, 1, mod, cm -> n_ibit);
			autecop_remap_pdo(cbr, 1, 2, mod, cm -> n_ibit);
			autecop_remap_pdo(cbr, 1, 3, mod, cm -> n_ibit);
			//autecop_remap_pdo(cbr, 0, 0, mod, cm -> n_obit);
			autecop_remap_pdo(cbr, 0, 1, mod, cm -> n_obit);
			autecop_remap_pdo(cbr, 0, 2, mod, cm -> n_obit);
			autecop_remap_pdo(cbr, 0, 3, mod, cm -> n_obit);
			cm -> state = CANMOD_STATE_PRESENT;
		}
		break;
	default:
		break;
	}

	return (cm -> state == CANMOD_STATE_PRESENT);
}

int autecboard_get_n_input(autecboard_t *cbr, int mod)
{
	if (! autecboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_ib * 8;
}

int autecboard_get_n_output(autecboard_t *cbr, int mod)
{
	if (! autecboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_ob * 8;
}

/* Lettura (lenta) dello stato corrente degli ingressi. */
int autecboard_read_input(autecboard_t *cbr, int mod)
{
autecmodule_t *cm;
int rv, rrv;
int i;
unsigned char data;

	cm = &(cbr -> module[mod]);
	rrv = 0;
	for (i = 0; i < cm -> n_ib; ++i) {
		rv = autecboard_read_dictionary(cbr, mod,
					      0x6000, i+1,
					      &data, sizeof(data));
		if (rv == 0) {
			cm -> in[i] = data;
		}
		else {
			rrv = -1;
		}
	}
	return rrv;
}

/* Attivazione (cioe` entrata in modo "operational") ed eventaule
 interrogazione di un modulo. */
int autecboard_activate_module(autecboard_t *cbr, int mod)
{
autecmsg_t msg;

/* Controlliamo la presenza del modulo (e anche la correttezza
 dell'ID). */
	if (! autecboard_check_module(cbr, mod)) {
		return 0;
	}

#if 1
/* Leggiamo lo stato iniziale degli ingressi. */
	autecboard_read_input(cbr, mod);
#endif

/* Portiamo il modulo nello stato "operational". */
	msg.id = CANOPEN_COBID_NMT;
	msg.cob = 0;
	msg.length = 2;
	msg.flags = 0;
	msg.data[0] = 0x01; /* START REMOTE NODE */
	msg.data[1] = mod;
	autecboard_send(cbr, &msg);

	return 1;
}

/* Entrata in PREOPERATIONAL di un modulo (anche inesistente). */
int autecboard_preop_module(autecboard_t *cbr, int mod)
{
autecmsg_t msg;

/* Portiamo il modulo nello stato "boot". */
	msg.id = CANOPEN_COBID_NMT;
	msg.cob = 0;
	msg.length = 2;
	msg.flags = 0;
	msg.data[0] = 0x80; /* PREOP REMOTE NODE */
	msg.data[1] = mod;
	autecboard_send(cbr, &msg);

	return 1;
}

/* Reset di un modulo (anche inesistente). */
int autecboard_reset_module(autecboard_t *cbr, int mod)
{
autecmsg_t msg;

/* Portiamo il modulo nello stato "boot". */
	msg.id = CANOPEN_COBID_NMT;
	msg.cob = 0;
	msg.length = 2;
	msg.flags = 0;
	msg.data[0] = 0x81; /* RESET REMOTE NODE */
	msg.data[1] = mod;
	autecboard_send(cbr, &msg);

	return 1;
}

#ifdef TEST
int main(int argc, char **argv)
{
autecboard_t *cbr;
int board = 0;
int speed = 1000;
int period = 500;
int debug = 1;
int module_in = 1;
int module_out = 1;
int n_in = 8;
int n_out = 8;
int rv;
int out[128];
int in[128];
int tmp;
int i;

	for (i = 1; i < argc; ++i) {
		if ((strcmp(argv[i],"-d") == 0) && argv[i+1]) {
			debug = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-s") == 0) && argv[i+1]) {
			speed = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-b") == 0) && argv[i+1]) {
			board = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-mi") == 0) && argv[i+1]) {
			module_in = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-mo") == 0) && argv[i+1]) {
			module_out = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-ni") == 0) && argv[i+1]) {
			n_in = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-no") == 0) && argv[i+1]) {
			n_out = atoi(argv[++i]);
		}
		else if ((strcmp(argv[i],"-p") == 0) && argv[i+1]) {
			period = atoi(argv[++i]);
		}
	}

	cbr = autecboard_open(board,speed,debug);
	if (! cbr) {
		perror("autecboard_open");
		return 1;
	}

	rv = autecboard_activate_module(cbr, module_in);
	if (! rv) {
		fprintf(stderr,"Module %d not found.\n",module_in);
		return 1;
	}
	if (module_in != module_out) {
		rv = autecboard_activate_module(cbr, module_out);
		if (! rv) {
			fprintf(stderr,"Module %d not found.\n",module_out);
			return 1;
		}
	}

	for (i = 0; i < sizeof(in)/sizeof(in[0]); ++i) {
		in[i] = 0;
	}
	for (i = 0; i < sizeof(out)/sizeof(out[0]); ++i) {
		out[i] = 0;
	}
	out[0] = 1;

	for (;;) {
		for (i = 0; i < n_in; ++i) {
			tmp = autecboard_get_input_bit(cbr, module_in, i);
			if (tmp != in[i]) {
				in[i] = tmp;
				printf("Input %d = %d\n",i,tmp);
			}
		}

		tmp = out[0];
		for (i = 1; i < n_out; ++i) {
			out[i-1] = out[i];
		}
		out[n_out-1] = tmp;

		for (i = 0; i < n_out; ++i) {
			autecboard_set_output_bit(cbr, module_out, i, out[i]);
		}

		autecboard_flush_output(cbr);

		usleep(period * 1000);

	}

	return 0;
}
#endif

