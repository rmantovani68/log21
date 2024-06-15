
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <can4linux.h>

typedef struct pseudocan_t pseudocan_t;

typedef struct module_t module_t;

/* Descrizione di un modulo. */
struct module_t {
/* ID (0 se modulo inesistente). */
	int id;
/* Tipo (esempio: 401 per modulo DS401). */
	unsigned int type;
/* Stato CANOpen */
	unsigned char state;
#define STATE_NONE 0
#define STATE_PREOPERATIONAL 1
#define STATE_OPERATIONAL 2
#define STATE_STOPPED 3
#define STATE_MAX 4
/* Numero di ingressi digitali. */
	unsigned char n_din;
/* Numero di uscite digitali. */
	unsigned char n_dout;
/* Numero di ingressi digitali. */
	unsigned char n_ain;
/* Numero di uscite digitali. */
	unsigned char n_aout;
/* Numero di ingressi encoder. */
	unsigned char n_enc;
/* Stato degli ingressi digitali. */
	unsigned char d_in[32];
/* Stato delle uscite digitali. */
	unsigned char d_out[32];
/* Stato degli ingressi analogoci. */
	unsigned short a_in[32];
/* Stato delle uscite analogoche. */
	unsigned short a_out[32];
/* Gestore di riferimento. */
	pseudocan_t *psc;
};

struct pseudocan_t {
/* Flag di debug. */
	int debug;
/* Linea simulata. */
	int line;
/* FIFO per la simulazione e file descriptor associati. */
	char fifo_tx[256];
	char fifo_rx[256];
	int rxfd;
	int txfd;
/* Tabella dei moduli presenti. */
	module_t module[128];
};

typedef struct moddescr_t moddescr_t;

struct moddescr_t {
	int id;
	int type;
	int n_din;
	int n_dout;
	int n_ain;
	int n_aout;
	int n_enc;
	struct moddescr_t *next;
};

int pseudocan_send(pseudocan_t *psc, canmsg_t *msg);

void module_setout(module_t *m, unsigned char *out, int base, int n)
{
int i;
	if (memcmp(m -> d_out + base, out, n) != 0) {
		memcpy(m -> d_out + base, out, n);
		printf("Module %d.%d: out =",m->psc->line,m->id);
		for (i = 0; i < m -> n_dout; ++i) {
			printf(" %02x",m -> d_out[i]);
		}
		printf("\n");
	}
}

void module_setaout(module_t *m, unsigned short *out, int base, int n)
{
int i;
	if (memcmp(m -> a_out + base, out, sizeof(m->a_out[0])*n) != 0) {
		memcpy(m -> a_out + base, out, sizeof(m->a_out[0])*n);
		printf("Module %d.%d: aout =",m->psc->line,m->id);
		for (i = 0; i < m -> n_aout; ++i) {
			printf(" %04x",m -> a_out[i]);
		}
		printf("\n");
	}
}

int module_setstate(module_t *m, int state)
{
const static char *stsnames[] = {
"none",
"preoperational",
"operational",
"stopped",
};

	if (state <= STATE_NONE || state >= STATE_MAX) {
		return 0;
	}
	if (m -> state == STATE_NONE) {
		return 0;
	}
	if (m -> state == STATE_OPERATIONAL && state != STATE_OPERATIONAL) {
	static unsigned char null_out[32] = { 0, };
		module_setout(m, null_out, 0, 32);
	}
	m -> state = state;
	printf("Module %d.%d: state = %s\n",
	       m->psc->line,m->id,stsnames[state]);
	return 1;
}

void module_setain(module_t *m, unsigned short *in)
{
canmsg_t msg;
int ni;

	if (m -> state != STATE_OPERATIONAL) {
		return;
	}
	ni = m -> n_ain;
	if (ni > 4) {
		if (memcmp(m -> a_in, in, 8) != 0) {
			memcpy(m -> a_in, in, 8);
			msg.id = 0x380 + m -> id;
			msg.flags = 0;
			msg.length = 8;
			memcpy(msg.data, m -> a_in, 8);
			pseudocan_send(m -> psc, &msg);
		}
		if (memcmp(m -> a_in+4, in+4, m -> n_ain-4) != 0) {
			memcpy(m -> a_in+4, in+4, m -> n_ain-4);
			msg.id = 0x480 + m -> id;
			msg.flags = 0;
			msg.length = (m -> n_ain-4)*sizeof(short);
			memcpy(msg.data, m -> a_in+4,
				(m->n_ain-4)*sizeof(short));
			pseudocan_send(m -> psc, &msg);
		}
	}
	else {
		if (memcmp(m -> a_in, in, 8) != 0) {
			memcpy(m -> a_in, in, 8);
			msg.id = 0x380 + m -> id;
			msg.flags = 0;
			msg.length = (m -> n_ain)*sizeof(short);
			memcpy(msg.data, m -> a_in,
				(m->n_ain)*sizeof(short));
			pseudocan_send(m -> psc, &msg);
		}
	}
}

void module_setin(module_t *m, unsigned char *in)
{
canmsg_t msg;
int ni;

	if (m -> state != STATE_OPERATIONAL) {
		return;
	}
	ni = m -> n_din;
	if (ni > 8) {
		if (memcmp(m -> d_in, in, 8) != 0) {
			memcpy(m -> d_in, in, 8);
			msg.id = 0x180 + m -> id;
			msg.flags = 0;
			msg.length = 8;
			memcpy(msg.data, m -> d_in, 8);
			pseudocan_send(m -> psc, &msg);
		}
		if (memcmp(m -> d_in+8, in+8, m -> n_din-8) != 0) {
			memcpy(m -> d_in+8, in+8, m -> n_din-8);
			if (m -> type == 0x402) {
				msg.id = 0x400 + m -> id;
			}
			else {
				msg.id = 0x280 + m -> id;
			}
			msg.flags = 0;
			msg.length = m -> n_din-8;
			memcpy(msg.data, m -> d_in+8, m -> n_din-8);
			pseudocan_send(m -> psc, &msg);
		}
	}
	else {
		if (memcmp(m -> d_in, in, m -> n_din) != 0) {
			memcpy(m -> d_in, in, m -> n_din);
			msg.id = 0x180 + m -> id;
			msg.flags = 0;
			msg.length = m -> n_din;
			memcpy(msg.data, m -> d_in, m -> n_din);
			pseudocan_send(m -> psc, &msg);
		}
	}
}

void module_set_input_bit(module_t *m, int bit, int val)
{
unsigned char tmp[32];

	memcpy(tmp,m -> d_in,sizeof(m->d_in));
	if (val) {
		tmp[bit / 8] |= (1 << (bit % 8));
	}
	else {
		tmp[bit / 8] &= ~(1 << (bit % 8));
	}
	module_setin(m, tmp);
}

void module_set_input_word(module_t *m, int w, int val)
{
unsigned short tmp[32];

	memcpy(tmp,m -> a_in,sizeof(m->a_in));
	tmp[w] = val;
	module_setain(m, tmp);
}

int module_get_n_din(module_t *m)
{
	if ((! m) || (m -> id == 0)) {
		return -1;
	}
	return m -> n_din * 8;
}

int module_get_n_ain(module_t *m)
{
	if ((! m) || (m -> id == 0)) {
		return -1;
	}
	return m -> n_ain;
}

int module_get_n_dout(module_t *m)
{
	if ((! m) || (m -> id == 0)) {
		return -1;
	}
	return m -> n_dout * 8;
}

void module_nmt(module_t *m, int cmd)
{
canmsg_t msg;

	if (m -> state == STATE_NONE) {
		return;
	}
	switch (cmd) {
	case 0x01: /* START */
		module_setstate(m,STATE_OPERATIONAL);
		break;
	case 0x02: /* STOP */
		module_setstate(m,STATE_STOPPED);
		break;
	case 0x80: /* PREOP */
		module_setstate(m,STATE_PREOPERATIONAL);
		break;
	case 0x81: /* RESET */
	case 0x82: /* RESETCOMM */
		msg.id = 0x700 + m -> id;
		msg.length = 1;
		msg.flags = 0;
		msg.data[0] = 0;
		pseudocan_send(m -> psc, &msg);
		module_setstate(m,STATE_PREOPERATIONAL);
		break;
	default:
		break;
	}
}

unsigned char *module_getin(module_t *m)
{
	return m -> d_in;
}

void module_pdo(module_t *m, canmsg_t *msg, int pdo)
{
int l;
int base = 0;

	if (m -> state != STATE_OPERATIONAL) {
	}
	if (pdo == 0 || pdo == 1) {
		base = pdo*8;
		if (base >= m -> n_dout) {
			return;
		}
		if (msg -> length > m -> n_dout - base) {
			l = m -> n_dout - base;
		}
		else {
			l = msg -> length;
		}
		module_setout(m, msg->data, base, l);
	}
	else {
		base = (pdo - 2)*4;
		if (base >= m -> n_aout) {
			return;
		}
		if (msg -> length/sizeof(short) > m -> n_aout - base) {
			l = m -> n_dout - base;
		}
		else {
			l = msg -> length/sizeof(short);
		}
		module_setaout(m, msg->data, base, l);
	}
}

void module_sdo(module_t *m, canmsg_t *msg)
{
int idx,subidx;
canmsg_t answ;

	if (m -> state == STATE_NONE) {
		return;
	}

	if (msg -> length != 8) {
		return;
	}

	idx = msg -> data[1] + 256*msg -> data[2];
	subidx = msg -> data[3];

	answ = *msg;
	answ.data[4] = 0;
	answ.data[5] = 0;
	answ.data[6] = 0;
	answ.data[7] = 0;

	if (msg -> data[0] == 0x40) {
		answ.id = 0x580 + m -> id;
		if (idx == 0x6000 && subidx == 0) {
			answ.data[0] = 0x4F;
			answ.data[4] = m -> n_din;
		}
		else if (idx == 0x6200 && subidx == 0) {
			answ.data[0] = 0x4F;
			answ.data[4] = m -> n_dout;
		}
		else if (idx == 0x6401 && subidx == 0) {
			answ.data[0] = 0x4F;
			answ.data[4] = m -> n_ain;
		}
		else if (idx == 0x6401 && subidx == 0) {
			answ.data[0] = 0x4F;
			answ.data[4] = m -> n_ain;
		}
		else if (idx == 0x6411 && m -> type == 401 && subidx == 0) {
			answ.data[0] = 0x4F;
			answ.data[4] = m -> n_aout;
		}
		else if (idx == 0x640F && m -> type == 0x402 && subidx == 0) {
			answ.data[0] = 0x4F;
			answ.data[4] = m -> n_aout;
		}
		else if (idx == 0x2000 && subidx == 0) {
		static int conta = 0;
			answ.data[0] = 0x43;
			answ.data[4] = (conta % 0x100);
			answ.data[5] = ((conta / 0x100) % 0x100);
			answ.data[6] = ((conta / 0x10000) % 0x100);
			answ.data[7] = ((conta / 0x1000000) % 0x100);
			++conta;
		}
		else if (idx == 0x1000 && subidx == 0) {
			answ.data[0] = 0x43;
			answ.data[4] = m -> type % 0x100;
			answ.data[5] = m -> type / 0x100;
			answ.data[6] = 0x0F;
			answ.data[7] = 0x00;
		}
		else if (idx == 0x1008 && subidx == 0) {
			answ.data[0] = 0x43;
			if (m -> type == 0x402) {
				answ.data[4] = 'k';
				answ.data[5] = '5';
				answ.data[6] = '0';
				answ.data[7] = '\0';
			}
			else {
				answ.data[4] = 'F';
				answ.data[5] = 'a';
				answ.data[6] = 'k';
				answ.data[7] = 'e';
			}
		}
		else if (idx == 0x1009 && subidx == 0) {
			answ.data[0] = 0x43;
			answ.data[4] = 'H';
			answ.data[5] = 'w';
			answ.data[6] = '0';
			answ.data[7] = '1';
		}
		else if (idx == 0x100A && subidx == 0) {
			answ.data[0] = 0x43;
			answ.data[4] = 'S';
			answ.data[5] = 'w';
			answ.data[6] = '1';
			answ.data[7] = '0';
		}
		else {
			answ.data[0] = 0x80;
			answ.data[4] = 0x00;
			answ.data[5] = 0x00;
			answ.data[6] = 0x02;
			answ.data[7] = 0x06;
		}
		pseudocan_send(m -> psc, &answ);
	}
	else if ((msg -> data[0] & 0xE0) == 0x20) {
		answ.id = 0x580 + m -> id;
		if (((idx >= 0x1800 && idx <= 0x1803)
		  || (idx >= 0x1400 && idx <= 0x1403)) && subidx == 3) {
			answ.data[0] = 0x80;
			answ.data[4] = 0x00;
			answ.data[5] = 0x00;
			answ.data[6] = 0x02;
			answ.data[7] = 0x06;
		}
		else {
			answ.data[0] = 0x60;
			answ.data[4] = 0x00;
			answ.data[5] = 0x00;
			answ.data[6] = 0x00;
			answ.data[7] = 0x00;
		}
		pseudocan_send(m -> psc, &answ);
	}
}

static void dump(const char *s, pseudocan_t *psc, canmsg_t *msg)
{
int i;

	printf("%s %d: ID=%03lx f=%x l=%d d=",s,psc -> line,
	       msg->id,msg->flags,msg->length);
	for (i = 0; i < msg->length; ++i) {
		printf(" %02x",msg->data[i]);
	}
	printf("\n");
}

void pseudocan_close(pseudocan_t *psc)
{
	if (psc) {
		if (psc -> rxfd != -1) {
			close(psc -> rxfd);
		}
		if (psc -> txfd != -1) {
			close(psc -> txfd);
		}
		free(psc);
	}
}

pseudocan_t *pseudocan_open(int line, moddescr_t *md, int debug)
{
pseudocan_t *psc = NULL;
int i;

	psc = (pseudocan_t *) malloc(sizeof(*psc));
	if (! psc) {
		perror("pseudocan_open");
		goto pseudocan_open_ERROR;
	}

	psc -> line = line;
	psc -> debug = debug;
	psc -> rxfd = -1;
	psc -> txfd = -1;

	sprintf(psc -> fifo_rx,"/tmp/can%d-rx",line);
	mkfifo(psc -> fifo_rx,0777);
	psc -> rxfd = open(psc -> fifo_rx,O_RDWR);
	if (psc -> rxfd == -1) {
		perror(psc -> fifo_rx);
		goto pseudocan_open_ERROR;
	}

	sprintf(psc -> fifo_tx,"/tmp/can%d-tx",line);
	mkfifo(psc -> fifo_tx,0777);
	psc -> txfd = open(psc -> fifo_tx,O_RDWR);
	if (psc -> txfd == -1) {
		perror(psc -> fifo_tx);
		goto pseudocan_open_ERROR;
	}

	memset(psc -> module, 0, sizeof(psc -> module));
	for (i = 0; i < 128; ++i) {
		psc -> module[i].id = i;
		psc -> module[i].type = 0;
		psc -> module[i].n_din = 0;
		psc -> module[i].n_dout = 0;
		psc -> module[i].n_ain = 0;
		psc -> module[i].n_aout = 0;
		psc -> module[i].state = STATE_NONE;
		psc -> module[i].psc = psc;
	}
	while (md) {
		psc -> module[md -> id].id = md -> id;
		psc -> module[md -> id].type = md -> type;
		psc -> module[md -> id].n_din = md -> n_din;
		psc -> module[md -> id].n_dout = md -> n_dout;
		psc -> module[md -> id].n_ain = md -> n_ain;
		psc -> module[md -> id].n_aout = md -> n_aout;
		psc -> module[md -> id].state = STATE_PREOPERATIONAL;
		psc -> module[md -> id].psc = psc;
		md = md -> next;
	}
	return psc;

pseudocan_open_ERROR:
	if (psc) {
		pseudocan_close(psc);
	}
	return NULL;
}

int pseudocan_send(pseudocan_t *psc, canmsg_t *msg)
{
int rv;

	if (psc -> debug) {
		dump("TX", psc, msg);
	}
	rv = write(psc -> rxfd,msg,sizeof(*msg));
	if (rv < 0) {
		perror("write");
	}
	return rv > 0;
}

int pseudocan_event(pseudocan_t *psc)
{
int n;
canmsg_t msg;
int id;
int cmd;
module_t *m;

	n = read(psc -> txfd,&msg,sizeof(msg));
	if (n <= 0) {
		return 0;
	}
	if (psc -> debug) {
		dump("RX", psc, &msg);
	}

	if (msg.id == 0 && msg.length >= 2) {
	/* NMT */
		if (msg.data[1] == 0) {
		int i;

			for (i = 1; i < 128; ++i) {
				module_nmt(&psc -> module[i], msg.data[0]);
			}
		}
		else {
			module_nmt(&psc -> module[msg.data[1]], msg.data[0]);
		}
		return 1;
	}

	cmd = msg.id & 0x780;
	id = msg.id & 0x07F;

	m = &psc -> module[id];
	if (m -> id == 0) {
		return 1;
	}

	switch (cmd) {

	case 0x100: /* Time */ break;

	case 0x200: /* PDO1 RX */
		module_pdo(m, &msg, 0);
		break;
	case 0x300: /* PDO2 RX */
		module_pdo(m, &msg, 1);
		break;
	case 0x400: /* PDO3 RX */
		module_pdo(m, &msg, 2);
		break;
	case 0x500: /* PDO4 RX */
		if (m -> type == 0x402) {
			module_pdo(m, &msg, 1);
		}
		else {
			module_pdo(m, &msg, 3);
		}
		break;

	case 0x600: /* SDO RX */
		module_sdo(m, &msg);
		break;

	case 0x700: /* NMT (RTR?) */
		break;
	default:
		break;
	}
	return 1;
}

int pseudocan_get_fd(pseudocan_t *psc)
{
	return psc -> txfd;
}

module_t *pseudocan_get_module(pseudocan_t *psc, int id)
{
	if (id < 1 || id > 127) {
		return NULL;
	}
	return &(psc -> module[id]);
}

static void usage(void)
{
	printf("pseudocan [-l <line>] [-d <debuglevel>] [-c]"
	       " [-m <id>,<n_in>,<n_out>,<n_ain>,<n_aout>]...\n");
	exit(1);
}

static void input_err(void)
{
	printf("Bad line (expected: \"<id> <bit> <value>\")\n");
}

int main(int argc, char **argv)
{
int line = 0;
int debug = 0;
struct moddescr_t *md = NULL;
pseudocan_t *psc = NULL;
int txfd;
fd_set set;
int i;
int cniinf = 0;


	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i],"-l") == 0 && argv[i+1]) {
			if (sscanf(argv[++i],"%d",&line) != 1
			 || line < 0 || line > 1) {
				usage();
			}
		}
		else if (strcmp(argv[i],"-m") == 0 && argv[i+1]) {
		int id,ni,no,nai,nao,nenc;
		struct moddescr_t *m = NULL;
			nenc = 0;
			if (sscanf(argv[++i],"%d,%d,%d,%d,%d,%d",
			    &id,&ni,&no,&nai,&nao,&nenc) < 5
			 || id < 1 || id > 127 || ni < 0 || ni > 256
			 || no < 0 || no > 256 || nai < 0 || nai > 8
			 || nao < 0 || nao > 8 || nenc < 0 || nenc > 4) {
				usage();
			}
			m = (struct moddescr_t *)malloc(sizeof(*m));
			if (! m) {
				perror("malloc");
				return 1;
			}
			m -> id = id;
			if (cniinf) {
				m -> type = 0x402;
			}
			else {
				m -> type = 401;
			}
			m -> n_din = (ni + 7) / 8;
			m -> n_dout = (no + 7) / 8;
			m -> n_ain = nai;
			m -> n_aout = nao;
			m -> n_enc = nenc;
			m -> next = md;
			md = m;
		}
		else if (strcmp(argv[i],"-d") == 0 && argv[i+1]) {
			if (sscanf(argv[++i],"%d",&debug) != 1) {
				usage();
			}
		}
		else if (strcmp(argv[i],"-c") == 0) {
			cniinf = 1;
		}
		else {
			usage();
		}
	}

	psc = pseudocan_open(line,md,debug);
	if (! psc) {
		perror("pseudocan_open");
		return 1;
	}

	for (;;) {
		txfd = pseudocan_get_fd(psc);
		FD_ZERO(&set);
		FD_SET(0,&set);
		FD_SET(txfd,&set);
		if (select(txfd+1,&set,NULL,NULL,NULL) <= 0) {
			break;
		}
		if (FD_ISSET(0,&set)) {
		char l[100];
		int id,bit,v;
		module_t *m;

			fgets(l,sizeof(l),stdin);
			if (sscanf(l,"a %d %d %d",&id,&bit,&v) == 3
			 && id >= 1 && id <= 127 && bit >= 0) {
				m = pseudocan_get_module(psc, id);
				if ((! m) || (bit > module_get_n_ain(m))) {
					input_err();
				}
				else {
					module_set_input_word(m, bit, v);
				}
			}
			else if (sscanf(l,"%d %d %d",&id,&bit,&v) != 3
			 || id < 1 || id > 127 || bit < 0) {
				input_err();
			}
			else {
				m = pseudocan_get_module(psc, id);
				if ((! m) || (bit > module_get_n_din(m))) {
					input_err();
				}
				else {
					module_set_input_bit(m, bit, v);
				}
			}
		}
		if (FD_ISSET(txfd,&set)) {
			pseudocan_event(psc);
		}
	}

	return 0;
}

