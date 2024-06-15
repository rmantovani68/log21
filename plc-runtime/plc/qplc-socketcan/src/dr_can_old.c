/*
* dr_can.c 1.12 04/01/21
*
* Gestore CANOpen basato su can4linux.
*
* 09/06/06 1.0  gg : Prima versione documentata.
*                    Aggiunto un automatismo per acquisire
*                    via SDO lo stato iniziale degli ingressi
*                    (pare che alcuni moduli non inviino sempre il PDO iniziale
*                    all'entrata in operational per i bit oltre il 63).
*                    all'entrata in operational per i bit oltre il 63).
*                    Semplificato e raccolto il codice di gestione dei PDO
*                    Semplificato e raccolto il codice di gestione dei PDO
*                    in ricezione.
*                    in ricezione.
*                    Aggiunta la gestione del PDO in ricezione anche
*                    Aggiunta la gestione del PDO in ricezione anche
*                    durante una transazione SDO (prima, i messaggio non
*                    durante una transazione SDO (prima, i messaggio non
*                    riguardanti la transazione SDO andavano persi).
*                    riguardanti la transazione SDO andavano persi).
* 24/02/08 1.1  gg : Ripulito il codice, generalizzato il trattamento dei PDO,
*                    Aggiunta la gestione degli I/O analogici, sia DS401 che
*                    nella variante CNI Informatica. Si è dovuta aggiungere
*                    l'emissione del SYNC, perche' a quanto pare gli ingressi
*                    analogici CNI Informatica funzionano solo cosi`.
* 26/02/08 1.2  gg : Aggiunte funzioni per controllare meglio l'emissione del SYNC.
* 28/02/08 1.3  gg : Aggiunte varie altre letture da dizionario, piu` messaggi,
*                    altri flag di debug, e la mitica "fase 2" per i moduli CN104.
*                    Riepilogo dei flag di debug:
*                    Bit 0 (0x01): stampa totale, per compatibilita`.
*                    Bit 1 (0x02): usa il canale simulato, se non c'e` altro.
*                    Bit 2 (0x04): stampa le grandezze lette durante il setup.
*                    Bit 3 (0x08): stampa i messaggi CAN e l'attivita` del thread.
* 02/04/08 1.4  gg : Prima gestione encoder CN104 (solo i primi due).
*                    Da capire: le porcherie che si ricevono come PDO "strani".
* 08/07/08 1.5       Aggiunta una funzione per agire su alcune opzioni
*                    di funzionamento. Al momento e` possibile inibire
*                    la rimappatura dei PDO sui moduli CNI Informatica.
*                    Evitata l'inizializzazione degli assi sulla scheda K50.
* 04/08/08 1.6  gg : Aggiustamenti per compilazione con la nuova versione
*                    di can4linux.
* 18/08/08 1.7  gg : Modifiche per l'adattamento ai PDO premappati della K50.
*                    Evitate lunghe attese in caso di assenza del modulo
*                    (identificata con la fallita lettura dell'indice 0x1000).
* 12/09/08 1.8  gg : Pesantissime modifiche per avere gli SDO asincroni.
* 13/09/08 1.9  gg : Corretti gli immancabili bachi (SDO a piu` segmenti,
*                    maschera comandi SDO errata, flag di continuazione invertito).
* 16/12/09 1.10 gg : Filtrati i messaggi con ID non validi, flag di errore
*                    accesi o ID estesi. Troncate a 8 byte le lunghezze
*                    non corrette. Migliorata la stampa di debug.
* 11/12/10 1.11 gg : Supporto del convertitore USB-CAN della IXXAT e integrazione
*                    della relativa libreria (come modulo caricato dinamicamente,
*                    per non perdere la compatibilita' con i sistemi dove manca).
* 04/01/21 1.12 rm : Supporto del driver generico socketcan
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dlfcn.h>

#include <can4linux.h>


#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#if 1
#define BCI_DYNAMIC
#endif

#ifdef BCI_DYNAMIC
/*
* Ridefinizioni delle funzioni della libreria IXXAT per poter scrivere codice che può fare a meno di lei
* (link dinamico di libbci.so). Il codice si compila a patto che i prototipi in bci.h non siano troppo
* "strani" (es. uso non necessario di "extern").
*/
#define BCI_Init (*p_BCI_Init)
#define BCI_OpenBoard (*p_BCI_OpenBoard)
#define BCI_InitCan (*p_BCI_InitCan)
#define BCI_ResetCan (*p_BCI_ResetCan)
#define BCI_StartCan (*p_BCI_StartCan)
#define BCI_TransmitCanMsg (*p_BCI_TransmitCanMsg)
#define BCI_ReceiveCanMsg (*p_BCI_ReceiveCanMsg)
#define BCI_GetErrorString (*p_BCI_GetErrorString)
#define BCI_GetBoardInfo (*p_BCI_GetBoardInfo)
#define BCI_ConfigRxQueue (*p_BCI_ConfigRxQueue)
#endif

#include <bci.h>

#if 0
#define TEST
#endif
#if 0
#define TEST_ENCODER
#endif
#if 1
#define USE_ENCODER
#endif

#ifdef TEST_ENCODER
#ifndef USE_ENCODER
#define USE_ENCODER
#endif
#endif

#include "dr_can.h"

#define DEFAULT_RESET_PAUSE 2000000

#define DEFAULT_NOREMAP_PDO_CNIINF 1

#define SDO_POOL_SIZE 256
#define SDO_TIMEOUT 3

/*
* Attivare questo se si rilevano problemi di lettura SDO
* con dati piu` lunghi di 4 byte. Lo standard richiederebbe
* di lasciare a 0 i campi inutilizzati della richiesta di
* trasmissione di un segmento SDO, mentre i moduli CNI informatica
* si aspettano che siano ribaditi l'indice e il sottoindice.
*/
#if 0
#define STRICT_DS301
#endif

#define CANOPEN_NUM_ID 128
#define CANOPEN_NUM_PDO 4

#define CAN_ID_NBIT 11

#define CANOPEN_GET_ID(obj) ((obj) & 0x07F)
#define CANOPEN_GET_CMD(obj) ((obj) & 0x780)
#define CANOPEN_COBID_NMT    0x000
#define CANOPEN_COBID_SYNC   0x080
#define CANOPEN_COBID_EMERG(id) ((id)+0x080)
#define CANOPEN_COBID_TIME   0x100
#define CANOPEN_COBID_PDOTX(n,id) ((id)+0x180+0x100*(n))
#define CANOPEN_COBID_PDORX(n,id) ((id)+0x200+0x100*(n))
#define CANOPEN_COBID_PDO1TX(id) ((id)+0x180)
#define CANOPEN_COBID_PDO1RX(id) ((id)+0x200)
#define CANOPEN_COBID_PDO2TX(id) ((id)+0x280)
#define CANOPEN_COBID_PDO2RX(id) ((id)+0x300)
#define CANOPEN_COBID_PDO3TX(id) ((id)+0x380)
#define CANOPEN_COBID_PDO3RX(id) ((id)+0x400)
#define CANOPEN_COBID_PDO4TX(id) ((id)+0x480)
#define CANOPEN_COBID_PDO4RX(id) ((id)+0x500)
#define CANOPEN_COBID_SDOTX(id) ((id)+0x580)
#define CANOPEN_COBID_SDORX(id) ((id)+0x600)
#define CANOPEN_COBID_NMTEC(id) ((id)+0x700)
#define CANOPEN_COBID_PDOTXINDEX(id) (((id)-0x180)/0x100)
#define CANOPEN_COBID_PDORXINDEX(id) (((id)-0x200)/0x100)

#define CANOPEN_CCS_M 0xE0
#define CANOPEN_SCS_M 0xE0
#define CANOPEN_CCS_STARTDOWNLOAD 0x20
#define CANOPEN_SCS_STARTDOWNLOAD 0x60
#define CANOPEN_CCS_SEGDOWNLOAD 0x00
#define CANOPEN_SCS_SEGDOWNLOAD 0x20
#define CANOPEN_CCS_STARTUPLOAD 0x40
#define CANOPEN_SCS_STARTUPLOAD 0x40
#define CANOPEN_CCS_SEGUPLOAD 0x60
#define CANOPEN_SCS_SEGUPLOAD 0x00

#define CANOPEN_CCS_ABORT 0x80
#define CANOPEN_SCS_ABORT 0x80

/*
* Profilo CANOpen CNI Informatica: c'e` stato evidentemente
* un equivoco nell'interpretazione dello standard: il numero
* del profilo dovrebbe essere decimale, non esadecimanle!
* Inoltre il profilo per I/O digitali e` 401, non 402: quello e`
* per i motori!
* Peraltro, quelli della CNI Informatica sono oggetti ben lontani
* da qualsiasi standard... 
*/
#define PROFILE_CNIINFORMATICA 0x402

#define CNIINF_UNKNOWN 0
#define CNIINF_K50 1

typedef struct canpdo_t canpdo_t;

struct canpdo_t {
	unsigned long id;
	void *data;
	unsigned int size;
	unsigned int wordsz;
};

typedef struct canmodule_t canmodule_t;

/* Struttura di stato di una transazione SDO. */
typedef struct sdo_rw_t sdo_rw_t;

struct sdo_rw_t {
	int state;
	int wr;
	canmodule_t *cm;
	int idx;
	int subidx;
	void *data;
	unsigned int *len;
	canmsg_t msg;
	unsigned char *p;
	unsigned int tot;
	int cont;
	int toggle;
	int n;
	unsigned int abort;
	canboard_t *cbr;
	/* Parte riservata alle transazioni asincrone. */
	/* Tempo trascorso dall'ultima risposta dello slave. */
	unsigned long timer;
	/* Flag di SDO occupato. */
	int busy;
	/* Valore di completamento: 1: non terminato, 0: successo, <0: fallimento. */
	int rv;
	/* Puntatori di partecipazione alla lista degli SDO (asincroni) in corso. */
	sdo_rw_t *prev;
	sdo_rw_t *next;
};

struct sdo_t {
	/* Canale di riferimento (per il pool). */
	canboard_t *cbr;
	/* Parametri della richiesta. */
	canmodule_t *cm;
	int wr;
	int idx;
	int subidx;
	void *data;
	unsigned int len;
	/* Numero di sequenza della richiesta, per riconoscere le richieste abbandonate. */
	unsigned long seqn;
	/* Callback di terminazione e suo argomento. */
	void (*cb)(sdo_t *sdo, int err, unsigned int len, unsigned int abort, void *cb_arg);
	void *cb_arg;
	/* Lista delle richieste SDO inevase (per modulo). */
	sdo_t *next;
	/* Operazione di riferimento (pleonastico, probabilmente). */
	sdo_rw_t *sr;
};

struct canmodule_t {
	/* ID del modulo. */
	unsigned int id;
	/* Riferimento al bus contenitore. */
	canboard_t *cbr;
	/* Informazioni provenienti dalla lettura dei registri obbligatori. */
	/* Dal registro 0x1000 */
	unsigned int profile; // Dovrebbe essere 401, ma...
	unsigned int model; // Codice modello, specifico del costruttore.
	unsigned int info; // Dovrebbe contenere il tipo di segnali ma...
	/* Dal registro 0x1008 */
	char name[32];
	/* Dal registro 0x1009 */
	char hwver[32];
	/* Dal registro 0x100A */
	char swver[32];
	/* Stato del modulo: presente, non presente, non interrogato. */
	unsigned char state;
#define CANMOD_STATE_UNKNOWN 0
#define CANMOD_STATE_PRESENT 1
#define CANMOD_STATE_NOTPRESENT 2
	/* Indice delle uscite analogiche. */
	unsigned int aout_idx;
	/* Descrittori dei PDO associati al modulo. */
	canpdo_t txpdo[CANOPEN_NUM_PDO];
	canpdo_t rxpdo[CANOPEN_NUM_PDO];
	/* Numero di byte nel PDO di input (lettura dizionario 0x6000:0). */
	unsigned char n_ib;
	unsigned int n_ibit; // Numero di bit, per comodita`
	/* Numero di byte nel PDO di output (lettura dizionario 0x6200:0). */
	unsigned char n_ob;
	unsigned int n_obit; // Numero di bit, per comodita`
	/* Numero di ingressi e uscite analogiche. */
	unsigned char n_ain;
	unsigned char n_aout;
	/* Numero ingressi encoder. */
	unsigned char n_enc;
	/* Stato ingressi. */
	unsigned char in[32];
	/* Stato uscite. */
	unsigned char out[32];
	/* Stato ingressi analogici. */
	unsigned short ain[16];
	/* Stato uscite analogiche. */
	unsigned short aout[16];
	/* Stato encoder. */
	long enc[16];
	/* Stato dell'eventuale transazione SDO in corso. */
	sdo_rw_t sr;
	/* Lista delle richieste SDO inevase (testa e coda della lista). */
	sdo_t *sdo_req_head;
	sdo_t *sdo_req_tail;
};

/* Funzione di reazione ad un particolare oggetto CAN. */
typedef struct canmsgcb_t canmsgcb_t;
struct canmsgcb_t {
	void (*fn)(canboard_t *cbr, canmsg_t *msg, void *arg);
	void *arg;
};

typedef struct canlldrv_t {
	int (*open)(canboard_t *cbr, int nch, int speed, int debug);
	int (*send)(canboard_t *cbr, canmsg_t *msg);
	int (*recv)(canboard_t *cbr, canmsg_t *msg);
} canlldrv_t;

struct canboard_t {
	/* Flag di debug. */
	int debug;
	/* Linea CAN. */
	int line;
	/* File descriptor del canale CAN (sdoppiato per debug). */
	int fd_rx;
	int fd_tx;
	/* Velocita` di linea. */
	int speed;
	/* A corredo (maledetto can4linux!): terzo parametro per read/write. */
	int size_rdwr;
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
	canmodule_t module[CANOPEN_NUM_ID];
	/* Insieme dei moduli/PDO che richiedono update. Usare ffs per la ricerca. */
#define UPDATE_SET_ITEM_TYPE int
#define UPDATE_SET_ITEM_NBIT ((sizeof(UPDATE_SET_ITEM_TYPE) * 8))
#define UPDATE_SET_DIM ((CANOPEN_NUM_ID + UPDATE_SET_ITEM_NBIT - 1) / UPDATE_SET_ITEM_NBIT)
	int update_set[CANOPEN_NUM_PDO][UPDATE_SET_DIM];

	/* Opzioni di funzionamento varie: */
	/* Flag per l'inibizione della mappatura dei PDO nei dispositivi della CNI Informatica. */
	int noremap_pdo_cniinf;

	/* Tabella delle funzioni di reazione ai messaggi CAN entranti. */
	canmsgcb_t msgcb[1 << CAN_ID_NBIT];

	/* Pool delle richieste SDO libere (array preallocato e lista degli elementi non in uso). */
	sdo_t sdo_pool[SDO_POOL_SIZE];
	sdo_t *sdo_pool_free;
	/* Lista degli SDO asincroni in corso. */
	sdo_rw_t *sdo_rw_list;

	/* Puntatori alle funzioni "driver" di basso livello. */
	const canlldrv_t *lldrv;

	/* Campo riservato alle funzioni di basso livello. */
	void *llarg;

};

static void sdo_rw_init(sdo_rw_t *sr, canmodule_t *cm);
static void sdo_handler(canboard_t *cbr, canmsg_t *msg, void *arg);

static void sdo_init(sdo_t *sdo)
{
	sdo -> cm = NULL;
	sdo -> wr = 0;
	sdo -> idx = 0;
	sdo -> subidx = 0;
	sdo -> data = NULL;
	sdo -> len = 0;
	sdo -> seqn = 0;
	sdo -> cb = NULL;
	sdo -> cb_arg = NULL;
	sdo -> next = NULL;
	sdo -> sr = NULL;
}

static sdo_t *sdo_allocate(canboard_t *cbr)
{
	static sdo_t *sdo;
	static unsigned long sdo_seqn = 0;

	sdo = cbr -> sdo_pool_free;
	if (sdo != NULL) {
		cbr -> sdo_pool_free = sdo -> next;
		sdo_init(sdo);
		if (++sdo_seqn == 0) {
			sdo_seqn = 1;
		}
		sdo -> seqn = sdo_seqn;
	}
	return sdo;
}

static void sdo_free(sdo_t *sdo)
{
	if (sdo != NULL) {
		sdo_init(sdo);
		sdo -> next = sdo -> cbr -> sdo_pool_free;
		sdo -> cbr -> sdo_pool_free = sdo;
	}
}

static void canboard_dump(canboard_t *cbr, const char *s, canmsg_t *msg)
{
	int i;

	if (! (cbr -> debug & 8)) {
		return;
	}
	if (msg -> flags & MSG_ERR_MASK) {
		// Errore di comunicazione
		printf("%s %d: ERROR f=%02x",s,cbr -> line, msg->flags);
	} else {
		int l = msg->length;
		
		if (msg -> flags & MSG_EXT) {
			// Messaggio esteso
			printf("%s %d: ID=%08lx f=%02x l=%d d=",s,cbr -> line, msg->id,msg->flags,msg->length);
		} else {
			printf("%s %d: ID=%03lx f=%02x l=%d d=",s,cbr -> line, msg->id,msg->flags,msg->length);
		}
		if (msg -> flags & MSG_RTR) {
			printf("RTR");
		} else {
			if (l > 8) {
				l = 8;
			}
			for (i = 0; i < l; ++i) {
				printf(" %02x",msg->data[i]);
			}
		}
	}
	printf("\n");
}

/**********************************************************************/
/* Varianti delle primitive di apertura, trasmissione, ricezione      */
/* secondo i vari tipi di controllore CAN (PC600, IXXAT, simulato...) */
/**********************************************************************/

/**********************************************************/
/*              Interfaccia per can4linux                 */
/**********************************************************/

static int canboard_open_can4linux(canboard_t *cbr, int nch, int speed, int debug);
static int canboard_send_can4linux(canboard_t *cbr, canmsg_t *msg);
static int canboard_recv_can4linux(canboard_t *cbr, canmsg_t *msg);

static const canlldrv_t can4linux_lldrv = {
	canboard_open_can4linux,
	canboard_send_can4linux,
	canboard_recv_can4linux,
};

/* Parte "pubblica" (vale a dire, quella da usare più sotto) */
static int canboard_open_can4linux(canboard_t *cbr, int nch, int speed, int debug)
{
	char dev[40];
	volatile Command_par_t cmd;
	Config_par_t  cfg;

	/* Apertura del dispositivo. */
	sprintf(dev,"/dev/can%d",nch);
	cbr -> fd_rx = open(dev,O_RDWR);
	if (cbr -> fd_rx == -1) {
		return 0;
	}
	cbr -> fd_tx = cbr -> fd_rx;
	cbr -> size_rdwr = sizeof(canmsg_t);

	cmd.cmd = CMD_STOP;
#if defined(CAN_IOCTL_COMMAND)
	ioctl(cbr -> fd_rx, CAN_IOCTL_COMMAND, &cmd);
#else
	ioctl(cbr -> fd_rx, COMMAND, &cmd);
#endif

	cfg.target = CONF_TIMING; 
	cfg.val1 = cbr -> speed;
#if defined(CAN_IOCTL_CONFIG)
	ioctl(cbr -> fd_rx, CAN_IOCTL_CONFIG, &cfg);
#else
	ioctl(cbr -> fd_rx, CONFIG, &cfg);
#endif

	cmd.cmd = CMD_START;
#if defined(CAN_IOCTL_COMMAND)
	ioctl(cbr -> fd_rx, CAN_IOCTL_COMMAND, &cmd);
#else
	ioctl(cbr -> fd_rx, COMMAND, &cmd);
#endif

	cbr -> lldrv = &can4linux_lldrv;
	cbr -> llarg = NULL;

	return 1;
}

static int canboard_send_can4linux(canboard_t *cbr, canmsg_t *msg)
{
	return (write(cbr -> fd_tx, msg, 1) > 0);
}

static int canboard_recv_can4linux(canboard_t *cbr, canmsg_t *msg)
{
	return (read(cbr -> fd_rx, msg, 1) > 0);
}

/**********************************************************/
/*              Interfaccia per debug pipe                */
/**********************************************************/

static int canboard_open_debugpipe(canboard_t *cbr, int nch, int speed, int debug);
static int canboard_send_debugpipe(canboard_t *cbr, canmsg_t *msg);
static int canboard_recv_debugpipe(canboard_t *cbr, canmsg_t *msg);

static const canlldrv_t debugpipe_lldrv = {
	canboard_open_debugpipe,
	canboard_send_debugpipe,
	canboard_recv_debugpipe,
};

/* Parte "pubblica" (vale a dire, quella da usare più sotto) */
static int canboard_open_debugpipe(canboard_t *cbr, int nch, int speed, int debug)
{
char dev[40];

	sprintf(dev,"/tmp/can%d-rx",nch);
	mkfifo(dev,0777);
	cbr -> fd_rx = open(dev,O_RDWR);
	sprintf(dev,"/tmp/can%d-tx",nch);
	mkfifo(dev,0777);
	cbr -> fd_tx = open(dev,O_RDWR);
	if (cbr -> fd_rx == -1 || cbr -> fd_tx == -1) {
		return 0;
	}
	cbr -> size_rdwr = sizeof(canmsg_t);

	cbr -> lldrv = &debugpipe_lldrv;
	cbr -> llarg = NULL;

	return 1;
}
static int canboard_send_debugpipe(canboard_t *cbr, canmsg_t *msg)
{
	return (write(cbr -> fd_tx, msg, sizeof(canmsg_t)) > 0);
}
static int canboard_recv_debugpipe(canboard_t *cbr, canmsg_t *msg)
{
	return (read(cbr -> fd_rx, msg, sizeof(canmsg_t)) > 0);
}

/**********************************************************/
/*              Interfaccia per ixxat USB                 */
/**********************************************************/

/*
* Parte "privata" (vale a dire, quella da usare solo per realizzare
* la parte "pubblica").
*/

#ifdef BCI_DYNAMIC
/*
* Caricatore dinamico della libreria. Permette di lanciare qplc
* anche in assenza del binario della libreria IXXAT.
*/

#define IXXAT_BCI_LIB "libbci.so"

typedef struct lddlist_t {
	const char *name;
	void **pfn;
} lddlist_t;

const static lddlist_t ixxatusb_ldd_list[] = {
	{ "BCI_Init", (void **)&p_BCI_Init },
	{ "BCI_OpenBoard", (void **)&p_BCI_OpenBoard },
	{ "BCI_InitCan", (void **)&p_BCI_InitCan },
	{ "BCI_ResetCan", (void **)&p_BCI_ResetCan },
	{ "BCI_StartCan", (void **)&p_BCI_StartCan },
	{ "BCI_TransmitCanMsg", (void **)&p_BCI_TransmitCanMsg },
	{ "BCI_ReceiveCanMsg", (void **)&p_BCI_ReceiveCanMsg },
	{ "BCI_GetErrorString", (void **)&p_BCI_GetErrorString },
	{ "BCI_GetBoardInfo", (void **)&p_BCI_GetBoardInfo },
	{ "BCI_ConfigRxQueue", (void **)&p_BCI_ConfigRxQueue },
	{ NULL, NULL },
};
static int ixxatusb_ldd_done = 0;
static int ixxatusb_ldd_ok = 0;

static int ixxatusb_ldd(int debug)
{
	const static lddlist_t *l;
	void *lib;

	if (ixxatusb_ldd_done) {
		return ixxatusb_ldd_ok;
	}

	ixxatusb_ldd_done = 1;

	lib = dlopen(IXXAT_BCI_LIB, RTLD_NOW);
	if (lib == NULL) {
		if (debug & 5) {
			printf("IXXAT-USB: library \"%s\" not found (%s)\n",
			       IXXAT_BCI_LIB, dlerror());
		}
		return 0;
	}

	for (l = ixxatusb_ldd_list; l->name != NULL; ++l) {
		*(l->pfn) = dlsym(lib, l->name);
		if (*(l->pfn) == NULL) {
			if (debug & 5) {
				printf("IXXAT-USB: entry \"%s\" not found\n", l->name);
			}
			dlclose(lib);
			return 0;
		}
	}

	ixxatusb_ldd_ok = 1;
	return 1;
}
#else
static int ixxatusb_ldd(int debug)
{
	return 1;
}
#endif

typedef struct ixxatusb_t {
	BCI_BRD_HDL board;
	UINT8 channel;
	int nch;
	/* Thread ricevitore. */
	pthread_t rx_thread;
	/* Pipe per l'accodamento dei messaggi ricevuti. */
	int pipe_rx[2];
} ixxatusb_t;

#define IXXATUSB_NUM_CHAN 4

/* Canali su dispositivi IXXAT-USB e loro numero. */
static ixxatusb_t ixxatusb_channels[IXXATUSB_NUM_CHAN];
static int ixxatusb_nchan = 0;

static void ixxatusb_collect_channels(int debug)
{
	UINT16 brd = 0;
	ixxatusb_t *ix;
	BCI_ts_BrdInfo info;
	int ncan = 0;
	BCI_BRD_HDL hboard;
	int rv;
	int i;

	for (brd = 0; brd < IXXATUSB_NUM_CHAN; ++brd) {
		rv = BCI_OpenBoard(&hboard, 0, brd, 0);
		if (rv != BCI_OK) {
			if (debug & 5) {
				printf("IXXAT-USB: open_brd(%d): %s\n", brd, BCI_GetErrorString(rv));
			}
			break;
		}
		if (BCI_GetBoardInfo (hboard, &info) != BCI_OK) {
			if (debug & 5) {
				printf("IXXAT-USB: open_brd(%d): %s\n", brd, BCI_GetErrorString(rv));
			}
			break;
		}
		ncan = info.num_can;
		if (debug & 5) {
			printf("IXXAT-USB: Board %d: num_can=%d, type=%s\n", brd, ncan, info.can_type[0]);
		}
		if (ncan < 1) {
			continue;
		}
		for (i = 0; i < ncan; ++i) {
			ix = &ixxatusb_channels[ixxatusb_nchan];
			ix->board = hboard;
			ix->channel = (UINT16)i;
			ix->nch = ixxatusb_nchan;
			++ixxatusb_nchan;
			if (ixxatusb_nchan >= IXXATUSB_NUM_CHAN) {
				return;
			}
		}
	}
}

static void ixxatusb_reset(canboard_t *cbr)
{
	ixxatusb_t *ixxatusb;
	int rv;

	ixxatusb = (ixxatusb_t *) cbr -> llarg;
	rv = BCI_StartCan(ixxatusb->board, ixxatusb->channel);
	if (rv != BCI_OK) {
		if (cbr -> debug) {
			printf("IXXAT-USB: reset(%d): %s\n", ixxatusb->nch, BCI_GetErrorString(rv));
		}
	}
	usleep(500000);
}

/* Thread ricevente. */
static void *ixxatusb_rx_thread(void *arg)
{
	canboard_t *cbr = (canboard_t *) arg;
	ixxatusb_t *ixxatusb;
	BCI_ts_CanMsg bci_msg;
	canmsg_t msg;
	int rv;
	int ok;

	ixxatusb = (ixxatusb_t *) cbr -> llarg;

	for (;;) {
		rv = BCI_ReceiveCanMsg(ixxatusb->board, ixxatusb->channel,
		                       &bci_msg, BCI_WAIT_FOREVER);
		if (rv != BCI_OK) {
			if (cbr -> debug) {
				printf("IXXAT-USB: recv(%d): %s\n", ixxatusb->nch, BCI_GetErrorString(rv));
			}
			ixxatusb_reset(cbr);
		} else {
			ok = 1;
			msg.id = bci_msg.id;
			msg.length = bci_msg.dlc;
			msg.flags = 0;


			switch (bci_msg.mff) {
				case BCI_MFF_29_DAT:
					msg.flags |= MSG_EXT;
				case BCI_MFF_11_DAT:
					memcpy(msg.data, bci_msg.a_data,
							msg.length);
				break;
				case BCI_MFF_29_RMT:
					msg.flags |= MSG_EXT;
				case BCI_MFF_11_RMT:
					msg.flags |= MSG_RTR;
				break;
				case BCI_MFF_STS_MSG:
					ok = 0;
					if (cbr -> debug) {
						printf("IXXAT-USB: recv(%d): status=0x%02x\n", ixxatusb->nch, bci_msg.id);
					}
					if (bci_msg.id & BCI_CAN_BUS_OFF) {
						printf("IXXAT-USB: recv(%d): BUS OFF\n", ixxatusb->nch);
						ixxatusb_reset(cbr);
					}
				break;
				default:
					ok = 0;
					if (cbr -> debug) {
						printf("IXXAT-USB: recv(%d): mff=%d\n", ixxatusb->nch, bci_msg.mff);
					}
				break;
			}
			if (ok) {
				rv = write(ixxatusb->pipe_rx[1], &msg, sizeof(msg));
				if (rv < 0) {
					// Errore...
				}
			}
		}
	}
}

/* Variabili d'istanza specifiche del driver IXXAT-USB. */

/* Parte "pubblica" (vale a dire, quella da usare più sotto) */

static int canboard_open_ixxatusb(canboard_t *cbr, int nch, int speed, int debug);
static int canboard_send_ixxatusb(canboard_t *cbr, canmsg_t *msg);
static int canboard_recv_ixxatusb(canboard_t *cbr, canmsg_t *msg);

static const canlldrv_t ixxatusb_lldrv = {
	canboard_open_ixxatusb,
	canboard_send_ixxatusb,
	canboard_recv_ixxatusb,
};

static int canboard_open_ixxatusb(canboard_t *cbr, int nch, int speed, int debug)
{
/* Flag per evitare le inizializzazioni ripetute. */
static int init_done = 0;
/* Tabella dei valori di temporizzazione. */
static const struct bci_timings {
	UINT8 bt0;
	UINT8 bt1;
} bci_timings[] = {
	{ BCI_10KB },
	{ BCI_20KB },
	{ BCI_50KB },
	{ BCI_100KB },
	{ BCI_125KB },
	{ BCI_250KB },
	{ BCI_500KB },
	{ BCI_1000KB },
};

int rv = 0;
ixxatusb_t *ixxatusb = NULL;
UINT16 location;
int bci_timing;
pthread_attr_t thr_attr;

	if (! ixxatusb_ldd(debug)) {
		return 0;
	}
	if (! init_done) {
		rv = BCI_Init();
		if (rv != BCI_OK) {
			if (cbr -> debug) {
				printf("IXXAT-USB: open(%d): %s\n",
				       nch, BCI_GetErrorString(rv));
			}
			return 0;
		}
		ixxatusb_collect_channels(debug);
		init_done = 1;
	}

	if (nch >= ixxatusb_nchan) {
		return 0;
	}

	ixxatusb = &ixxatusb_channels[nch];

	switch (speed) {
		case 10:
			bci_timing = 0;
		break;
		case 20:
			bci_timing = 1;
		break;
		case 50:
			bci_timing = 2;
		break;
		case 100:
			bci_timing = 3;
		break;
		case 125:
			bci_timing = 4;
		break;
		case 250:
			bci_timing = 5;
		break;
		case 500:
			bci_timing = 6;
		break;
		//case 800: // Non gestito???
		case 1000:
			bci_timing = 7;
		break;
		default:
			bci_timing = -1;
		break;
	}
	if (bci_timing == -1) {
		if (cbr -> debug) {
			printf("IXXAT-USB: open(%d): speed %d not supported\n", nch, speed);
		}
		return 0;
	} else {
		rv = BCI_InitCan(ixxatusb->board, ixxatusb->channel, bci_timings[bci_timing].bt0, bci_timings[bci_timing].bt1, 0);
		if (rv != BCI_OK) {
			if (cbr -> debug) {
				printf("IXXAT-USB: open(%d): %s\n", nch, BCI_GetErrorString(rv));
			}
			return 0;
		}
	}

	rv = BCI_StartCan(ixxatusb->board, ixxatusb->channel);
	if (rv != BCI_OK) {
		if (cbr -> debug) {
			printf("IXXAT-USB: open(%d): %s\n", nch, BCI_GetErrorString(rv));
		}
		return 0;
	}

#if 0 // ??? Da' l'errore "non implementato"...
	rv = BCI_ConfigRxQueue(ixxatusb->board, ixxatusb->channel,
	                       BCI_LATENCY_MODE);
	if (rv != BCI_OK) {
		if (cbr -> debug & 1) {
			printf("IXXAT-USB: open(%d): %s\n",
			       nch, BCI_GetErrorString(rv));
		}
		return 0;
	}
#endif

	if (pipe(ixxatusb -> pipe_rx) < 0) {
		if (cbr -> debug) {
			printf("IXXAT-USB: open(%d): cannot create pipe\n", nch);
		}
		return 0;
	}

	cbr -> fd_rx = ixxatusb -> pipe_rx[0];
	cbr -> fd_tx = ixxatusb -> pipe_rx[0]; // Non usato
	cbr -> lldrv = &ixxatusb_lldrv;
	cbr -> llarg = ixxatusb;

/* Lancio del thread ricevitore. */
	pthread_attr_init(&thr_attr);
	pthread_attr_setinheritsched(&thr_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&thr_attr,SCHED_FIFO);

	if (pthread_create(&(ixxatusb -> rx_thread), &thr_attr, ixxatusb_rx_thread,(void *)cbr) != 0) {
		// Proviamo con parametri meno impegnativi...
		if (pthread_create(&(ixxatusb -> rx_thread), NULL, ixxatusb_rx_thread,(void *)cbr) < 0) {
			pthread_attr_destroy(&thr_attr);
			return 0;
		}
	}

	pthread_attr_destroy(&thr_attr);

	return 1;
}
static int canboard_send_ixxatusb(canboard_t *cbr, canmsg_t *msg)
{
BCI_ts_CanMsg bci_msg;
ixxatusb_t *ixxatusb;
int rv;

	ixxatusb = (ixxatusb_t *) cbr -> llarg;
	bci_msg.id = msg->id;
	bci_msg.dlc = msg->length;
	bci_msg.time_stamp = 0;
	if (msg->flags & MSG_RTR) {
		if (msg->flags & MSG_EXT) {
			bci_msg.mff = BCI_MFF_29_RMT;
		}
		else {
			bci_msg.mff = BCI_MFF_11_RMT;
		}
	}
	else {
		memcpy (bci_msg.a_data, msg->data, msg->length);
		if (msg->flags & MSG_EXT) {
			bci_msg.mff = BCI_MFF_29_DAT;
		}
		else {
			bci_msg.mff = BCI_MFF_11_DAT;
		}
	}
  
	rv = BCI_TransmitCanMsg(ixxatusb->board, ixxatusb->channel, &bci_msg);
	if (rv != BCI_OK) {
		if (cbr -> debug) {
			printf("IXXAT-USB: send(%d): %s\n", ixxatusb->nch, BCI_GetErrorString(rv));
		}
#if 0
		rv = BCI_StartCan(ixxatusb->board, ixxatusb->channel);
		if (rv != BCI_OK) {
			if (cbr -> debug & 1) {
				printf("IXXAT-USB: reset(%d): %s\n",
				       ixxatusb->nch, BCI_GetErrorString(rv));
			}
		}
#endif
		return 0;
	}
	return 1;
}
static int canboard_recv_ixxatusb(canboard_t *cbr, canmsg_t *msg)
{
	return (read(cbr -> fd_rx, msg, sizeof(canmsg_t)) > 0);
}

/**********************************************************************/
/*                        Interfaccia socketcan                       */
/**********************************************************************/

typedef struct socketcan_t {
	/* socket */
	int s;
	UINT8 channel;
	int nch;

	/* Thread ricevitore. */
	pthread_t rx_thread;

	/* Pipe per l'accodamento dei messaggi ricevuti. */
	int pipe_rx[2];
} socketcan_t;

#define SOCKETCAN_NUM_CHAN 4

/* Canali su dispositivi IXXAT-USB e loro numero. */
static socketcan_t socketcan_channels[SOCKETCAN_NUM_CHAN];
static int socketcan_nchan = 0;


/* Thread ricevente. */
static void *socketcan_rx_thread(void *arg)
{
	canboard_t *cbr = (canboard_t *) arg;
	socketcan_t *socketcan;
	canmsg_t msg;
	int nbytes;
	struct can_frame frame;
	int rv;
	int ok=0;
	int i;

	socketcan = (socketcan_t *) cbr -> llarg;

	for (;;) {
		nbytes = read(socketcan->s, &frame, sizeof(struct can_frame));

		if (nbytes < 0) {
			if (cbr -> debug) {
				printf("SOCKETCAN (%d) - %s\n", strerror(errno));
			}
			return 1;
		}

		/*
		if (cbr -> debug & 9) {
			printf("RX - 0x%03X [%d] ",frame.can_id, frame.can_dlc);

			for (i = 0; i < frame.can_dlc; i++){
				printf("%02X ",frame.data[i]);
			}
			printf("\n");
		}
		*/

		ok = 1;
		/* 
		* put can message in the rx pipe 
		*/
		msg.length = frame.can_dlc;
		msg.flags = 0;
		if(frame.can_id & CAN_ERR_FLAG){
			/* error */
			msg.id = frame.can_id & CAN_ERR_MASK;
			ok = 0;

			if (cbr -> debug) {
				printf("SOCKETCAN: recv(%d CANID : %02X) - %s\n", socketcan->nch, frame.can_id, "CAN_ERR");
			}

		} else if(frame.can_id & CAN_EFF_FLAG){
			/* extended can id - 29 bits  */
			msg.flags |= MSG_EXT;
			msg.id = frame.can_id & CAN_EFF_MASK;
			if (cbr -> debug) {
				printf("SOCKETCAN: recv(%d CANID : %02X) - %s\n", socketcan->nch, frame.can_id, "CAN_EFF");
			}
		} else {
			/* simple can id - 11 bits  */
			msg.id = frame.can_id & CAN_SFF_MASK;
		}

		memcpy(msg.data, frame.data, msg.length);
		
		if (ok) {
			/*
			if (cbr -> debug & 1) {
				printf("SOCKETCAN (%d) - INVIO MSG CAN in PIPE_RX\n", socketcan->nch);
			}
			*/
			rv = write(socketcan->pipe_rx[1], &msg, sizeof(msg));
			if (rv < 0) {
				if (cbr -> debug) {
					printf("SOCKETCAN (%d) - %s\n", strerror(errno));
				}
			}
		}
	}
}



static int canboard_open_socketcan(canboard_t *cbr, int nch, int speed, int debug);
static int canboard_send_socketcan(canboard_t *cbr, canmsg_t *msg);
static int canboard_recv_socketcan(canboard_t *cbr, canmsg_t *msg);

static const canlldrv_t socketcan_lldrv = {
	canboard_open_socketcan,
	canboard_send_socketcan,
	canboard_recv_socketcan,
};

/* Parte "pubblica" (vale a dire, quella da usare più sotto) */
static int canboard_open_socketcan(canboard_t *cbr, int nch, int speed, int debug)
{
	int s, i; 
	int nbytes;
	socketcan_t *socketcan = NULL;
	pthread_attr_t thr_attr;
	struct ifreq ifr;
	struct sockaddr_can addr;
	/*
	struct can_frame frame;
	*/

	memset(&ifr, 0x0, sizeof(ifr));
	memset(&addr, 0x0, sizeof(addr));
	/*
	memset(&frame, 0x0, sizeof(frame));	
	*/

	/* collezionare i dati delle porte can esistenti */
	socketcan_nchan = SOCKETCAN_NUM_CHAN;

	if (nch >= socketcan_nchan) {
		return 0;
	}

	socketcan = &socketcan_channels[nch];
	socketcan->nch = nch;

	/* apertura socket CAN */
	if ((socketcan->s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		if (cbr -> debug) {
			printf("SOCKETCAN (%d): cannot open socket - %s\n", nch, strerror(errno));
		}
		return 0;
	}

	sprintf(ifr.ifr_name, "can%d",nch );
	ioctl(socketcan->s, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(socketcan->s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if (cbr -> debug) {
			printf("SOCKETCAN (%d): cannot bind socket - %s\n", nch, strerror(errno));
		}
		return 1;
	}

	if (pipe(socketcan -> pipe_rx) < 0) {
		if (cbr -> debug) {
			printf("SOCKETCAN (%d): cannot create rx pipe - %s\n", nch, strerror(errno));
		}
		return 0;
	}


	cbr -> fd_rx = socketcan->pipe_rx[0];
	cbr -> fd_tx = socketcan->pipe_rx[0]; // Non usato
	cbr -> lldrv = &socketcan_lldrv;
	cbr -> llarg = socketcan;

	/* Lancio del thread ricevitore. */
	pthread_attr_init(&thr_attr);
	pthread_attr_setinheritsched(&thr_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&thr_attr,SCHED_FIFO);

	if (pthread_create(&(socketcan -> rx_thread), &thr_attr, socketcan_rx_thread,(void *)cbr) != 0) {
		// Proviamo con parametri meno impegnativi...
		if (pthread_create(&(socketcan -> rx_thread), NULL, socketcan_rx_thread,(void *)cbr) < 0) {
			pthread_attr_destroy(&thr_attr);
			return 0;
		}
	}

	pthread_attr_destroy(&thr_attr);

	if (cbr -> debug & 1) {
		printf("SOCKETCAN (%d): opened\n", nch);
	}
	return 1;

}

static int canboard_send_socketcan(canboard_t *cbr, canmsg_t *msg)
{
	socketcan_t *socketcan;

#ifdef _doc_
/**
* The CAN message structure.
* Used for all data transfers between the application and the driver
* using read() or write().
*/
typedef struct {
    /** flags, indicating or controlling special message properties */
    int             flags;
    int             cob;	 /**< CAN object number, used in Full CAN  */
    unsigned   long id;		 /**< CAN message ID, 4 bytes  */
    struct timeval  timestamp;	 /**< time stamp for received messages */
    short      int  length;	 /**< number of bytes in the CAN message */
    unsigned   char data[CAN_MSG_LENGTH]; /**< data, 0...8 bytes */
} canmsg_t;
#endif

#ifdef _doc_
struct can_frame {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __u8    can_dlc; /* frame payload length in byte (0 .. 8) */
    __u8    __pad;   /* padding */
    __u8    __res0;  /* reserved / padding */
    __u8    __res1;  /* reserved / padding */
    __u8    data[8] __attribute__((aligned(8)));
};

#define 	CAN_EFF_FLAG   0x80000000U /* EFF/SFF is set in the MSB */
#define 	CAN_RTR_FLAG   0x40000000U /* remote transmission request */
#define 	CAN_ERR_FLAG   0x20000000U /* error message frame */
#define 	CAN_SFF_MASK   0x000007FFU /* standard frame format (SFF) */
#define 	CAN_EFF_MASK   0x1FFFFFFFU /* extended frame format (EFF) */
#define 	CAN_ERR_MASK   0x1FFFFFFFU /* omit EFF, RTR, ERR flags */
#define 	CAN_SFF_ID_BITS   11
#define 	CAN_EFF_ID_BITS   29
#define 	CAN_MAX_DLC   8
#define 	CAN_MAX_DLEN   8
#define 	CANFD_MAX_DLC   15
#define 	CANFD_MAX_DLEN   64
#define 	CANFD_BRS   0x01 /* bit rate switch (second bitrate for payload data) */
#define 	CANFD_ESI   0x02 /* error state indicator of the transmitting node */
#define 	CAN_MTU   (sizeof(struct can_frame))
#define 	CANFD_MTU   (sizeof(struct canfd_frame))
#define 	CAN_RAW   1 /* RAW sockets */
#define 	CAN_BCM   2 /* Broadcast Manager */
#define 	CAN_TP16   3 /* VAG Transport Protocol v1.6 */
#define 	CAN_TP20   4 /* VAG Transport Protocol v2.0 */
#define 	CAN_MCNET   5 /* Bosch MCNet */
#define 	CAN_ISOTP   6 /* ISO 15765-2 Transport Protocol */
#define 	CAN_NPROTO   7
#define 	SOL_CAN_BASE   100
#define 	CAN_INV_FILTER   0x20000000U /* to be set in can_filter.can_id */

#endif

	struct can_frame frame;

	socketcan = (socketcan_t *)cbr->llarg;
	
	frame.can_id = msg->id;
	frame.can_dlc = msg->length;
	memcpy (frame.data, msg->data, msg->length);

	if (write(socketcan->s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		if (cbr -> debug) {
			printf("SOCKETCAN (%d): write error - %s\n", socketcan->nch, strerror(errno));
		}
		return 0;
	}
	/*
	if (cbr -> debug & 9) {
		int i;

		printf("TX - 0x%03X [%d] ",frame.can_id, frame.can_dlc);

		for (i = 0; i < frame.can_dlc; i++){
			printf("%02X ",frame.data[i]);
		}
		printf("\n");
	}
	*/

	return 1;
}

static int canboard_recv_socketcan(canboard_t *cbr, canmsg_t *msg)
{
	return (read(cbr -> fd_rx, msg, sizeof(canmsg_t)) > 0);
}


/**********************************************************************/
/**********************************************************************/

/*****************************************/

static int canboard_recv(canboard_t *cbr, canmsg_t *msg)
{
	int rv;

	rv = cbr->lldrv->recv(cbr, msg);
	canboard_dump(cbr,"RX",msg);
	if (msg->length > 8) {
		msg->length = 8;
	}
	if (msg->flags & MSG_ERR_MASK) {
		msg->length = 0;
	}
	if (msg->flags & MSG_EXT) {
		msg->id &=CAN_EFF_MASK;
	} else {
		msg->id &=CAN_SFF_MASK;
	}
	return rv;
}

static void canboard_handle_pdo(canboard_t *cbr, canmsg_t *msg, void *arg)
{
	unsigned int pdo_idx;
	int mod;
	int i;
	canpdo_t *p;
	canmodule_t *cm;

	if (msg == NULL) {
		return;
	}

	pdo_idx = (unsigned int) arg;

	mod = CANOPEN_GET_ID(msg -> id);
	cm = &(cbr -> module[mod]);
	if (cm -> state == CANMOD_STATE_PRESENT) {
		p = &cm -> txpdo[pdo_idx];
		if (p->data != NULL) {
			/* 
			* Aggiorniamo l'immagine degli ingressi.
			* Dobbiamo fare una copia byte per byte, perche` non possiamo fidarci dei dati
			* oltre la lunghezza del messaggio. 
			*/
			for (i = 0; i < msg -> length; ++i) {
				// TODO: tenere conto di p->wordsz.
				((unsigned char *)p->data)[i] = msg -> data[i];
			}
		}
	}
}

static void canboard_handle_msg(canboard_t *cbr, canmsg_t *msg)
{
	if (msg == NULL) {
		// TODO: potrebbe essere usato per codificare un timeout.
		printf("Messaggio nullo ignorato\n");
	} else if (msg -> flags & MSG_ERR_MASK) {
		// TODO: Errore di comunicazione
		printf("Errore di comunicazione ignorato\n");
	} else if (msg -> flags & MSG_EXT) {
		// Messaggio esteso, ignorare.
		printf("Messaggio esteso ignorato\n");
	} else if (msg -> flags & MSG_RTR) {
		// Messaggio remoto, ignorare.
		printf("Messaggio remoto ignorato\n");
	} else {
		// printf("Messaggio con ID=0x%03x\n",msg -> id);
		cbr->msgcb[msg -> id].fn(cbr, msg, cbr->msgcb[msg -> id].arg);
	}
}

/* Thread ricevitore. */
static void *canboard_rx_thread(void *arg)
{
	canboard_t *cbr = (canboard_t *) arg;
	canmsg_t msg;
	int rv;
	fd_set set;
	char cmd;

	if (cbr -> debug & 8) {
		printf("Receiver started\n");
	}

	for (;;) {

		/* 
		* Attesa di eventi (messaggi CAN o comandi di sospensione
		* della ricezione). 
		*/

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
			/* 
			* Comando di sospensione o di ripresa della ricezione.
			* Lo decifriamo, e ci regoliamo di conseguenza. 
			*/
			if (read(cbr -> pipe_mr[0], &cmd, sizeof(cmd)) <= 0) {
				break;
			}
			if (cmd == 'd') {
				/* Richiesta di sospensione della ricezione. */
				if (cbr -> debug & 8) {
					printf("Receiver disabled\n");
				}
				cbr -> rx_sleeping = 1;
			} else {
				/* Richiesta di ripresa della ricezione. */
				if (cbr -> debug & 8) {
					printf("Receiver enabled\n");
				}
				cbr -> rx_sleeping = 0;
			}
			/* Risposta al richiedente. */
			write(cbr -> pipe_rm[1],"\n",1);
		}

		if ((! cbr -> rx_sleeping) && FD_ISSET(cbr -> fd_rx,&set)) {
			rv = canboard_recv(cbr, &msg);
			if (rv <= 0) {
				break;
			}
			canboard_handle_msg(cbr, &msg);
		}
	}

	return NULL;
}

static void canboard_disable_receiver(canboard_t *cbr)
{
	char c;

	write(cbr -> pipe_mr[1], "d", 1);
	read(cbr -> pipe_rm[0], &c, 1);
}
static void canboard_enable_receiver(canboard_t *cbr)
{
	char c;

	write(cbr -> pipe_mr[1], "e", 1);
	read(cbr -> pipe_rm[0], &c, 1);
}

static void canboard_close_core(canboard_t *cbr)
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

static void default_msg_cb(canboard_t *cbr, canmsg_t *msg, void *arg)
{
}

/* Apertura di un gestore di bus CANOpen. */
canboard_t *canboard_open(int nch, int speed, int debug)
{
	canboard_t *cbr = NULL;
	char dev[40];
	int i;
	pthread_attr_t thr_attr;
	struct sched_param sch;
	int policy;

	cbr = (canboard_t *) malloc(sizeof(*cbr));
	if (! cbr) {
		goto canboard_init_error;
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
	cbr -> noremap_pdo_cniinf = DEFAULT_NOREMAP_PDO_CNIINF;

	/* 
	* Pool delle richieste SDO libere (array preallocato e lista
	* degli elementi non in uso). 
	*/
	sdo_t sdo_pool[SDO_POOL_SIZE];
	for (i = 0; i < sizeof(cbr->sdo_pool)/sizeof(cbr->sdo_pool[0]); ++i) {
		cbr -> sdo_pool[i].cbr = cbr;
		sdo_init(&cbr -> sdo_pool[i]);
		cbr -> sdo_pool[i].next = &cbr -> sdo_pool[i+1];
	}
	cbr -> sdo_pool[i-1].next = NULL;
	cbr -> sdo_pool_free = cbr -> sdo_pool;
	/* Lista degli SDO asincroni in corso. */
	cbr -> sdo_rw_list = NULL;

	for (i = 0; i < sizeof(cbr->msgcb)/sizeof(cbr->msgcb[0]); ++i) {
		cbr -> msgcb[i].fn = default_msg_cb;
		cbr -> msgcb[i].arg = NULL;
	}

	/* Apertura del dispositivo. */
	/*
	if ((! can4linux_lldrv.open(cbr, nch, speed, debug)) && 
		(! ixxatusb_lldrv.open(cbr, nch, speed, debug)) &&
		(! socketcan_lldrv.open(cbr, nch, speed, debug)) &&
		((! (cbr -> debug & 2)) || (! debugpipe_lldrv.open(cbr, nch, speed, debug)))) {
		goto canboard_init_error;
	}
	*/
	if (! socketcan_lldrv.open(cbr, nch, speed, debug)) {
		goto canboard_init_error;
	}	

	/* Aggiunta dei file descriptor di trasmissione e ricezione. */
	cbr -> nfd = cbr -> fd_rx + 1;
	if (cbr -> nfd <= cbr -> fd_tx) {
		cbr -> nfd = cbr -> fd_tx + 1;
	}

	/* Creazione della pipe per comandare il thread ricevitore. */
	if (pipe(cbr -> pipe_mr) < 0) {
		goto canboard_init_error;
	}
	if (cbr -> nfd <= cbr -> pipe_mr[0]) {
		cbr -> nfd = cbr -> pipe_mr[0] + 1;
	}
	if (cbr -> nfd <= cbr -> pipe_mr[1]) {
		cbr -> nfd = cbr -> pipe_mr[1] + 1;
	}
	/* Pipe per le risposte dal ricevitore. */
	if (pipe(cbr -> pipe_rm) < 0) {
		goto canboard_init_error;
	}
	if (cbr -> nfd <= cbr -> pipe_rm[0]) {
		cbr -> nfd = cbr -> pipe_rm[0] + 1;
	}
	if (cbr -> nfd <= cbr -> pipe_rm[1]) {
		cbr -> nfd = cbr -> pipe_rm[1] + 1;
	}

	/* Inizializzazione dello stato dei moduli. */
	for (i = 0; i < CANOPEN_NUM_ID; ++i) {
		int j;
		canmodule_t *cm;

		cm = &cbr -> module[i];
		cm -> id = i;
		cm -> cbr = cbr;
		cm -> profile = 0;
		cm -> info = 0;
		cm -> name[0] = '\0';
		cm -> state = CANMOD_STATE_UNKNOWN;
		cm -> aout_idx = 0x6411;
		cm -> n_ib = 0;
		cm -> n_ob = 0;
		cm -> n_ibit = 0;
		cm -> n_obit = 0;
		cm -> n_ain = 0;
		cm -> n_aout = 0;
		cm -> n_enc = 0;
		memset(cm -> in, 0, sizeof(cm -> in));
		memset(cm -> out, 0, sizeof(cm -> out));
		memset(cm -> ain, 0, sizeof(cm -> ain));
		memset(cm -> aout, 0, sizeof(cm -> aout));
		memset(cm -> enc, 0, sizeof(cm -> enc));
		sdo_rw_init(&cm -> sr, cm);
		cbr -> msgcb[CANOPEN_COBID_SDOTX(i)].fn = sdo_handler;
		cbr -> msgcb[CANOPEN_COBID_SDOTX(i)].arg = (sdo_rw_t *) &cm -> sr;
		cm -> sdo_req_head = NULL;
		cm -> sdo_req_tail = NULL;
		for (j = 0; j < CANOPEN_NUM_PDO; ++j) {
			cm -> txpdo[j].id = CANOPEN_COBID_PDOTX(j,i);
			cm -> txpdo[j].data = NULL;
			cm -> txpdo[j].size = 0;
			cm -> txpdo[j].wordsz = 0;

			cm -> rxpdo[j].id = CANOPEN_COBID_PDORX(j,i);
			cm -> rxpdo[j].data = NULL;
			cm -> rxpdo[j].size = 0;
			cm -> rxpdo[j].wordsz = 0;
		}
	}

	/* 
	* Lancio del thread ricevitore. 
	* Da questo momento lo stato degli ingressi puo` evolvere. 
	*/
	pthread_attr_init(&thr_attr);
	pthread_attr_setinheritsched(&thr_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&thr_attr,SCHED_FIFO);

	if (pthread_create(&(cbr -> rx_thread), &thr_attr, canboard_rx_thread,(void *)cbr) != 0) {
		// Proviamo con parametri meno impegnativi...
		if (pthread_create(&(cbr -> rx_thread), NULL, canboard_rx_thread,(void *)cbr) != 0) {
			pthread_attr_destroy(&thr_attr);
			goto canboard_init_error;
		}
	}

	pthread_attr_destroy(&thr_attr);

#if 0
	/* Reset globale del bus. */
	canboard_reset_module(cbr, 0);
	usleep(DEFAULT_RESET_PAUSE);
#endif

	return cbr;

canboard_init_error:
	canboard_close_core(cbr);
	return NULL;
}

void canboard_close(canboard_t *cbr)
{
	canboard_close_core(cbr);
}

static int canboard_send(canboard_t *cbr, canmsg_t *msg)
{
	canboard_dump(cbr,"TX",msg);
	return cbr->lldrv->send(cbr, msg);
}

/* Invio del PDO per l'aggiornamento delle uscite di un modulo. */
static void canboard_write_pdo(canmodule_t *cm, int pdo)
{
	canmsg_t msg;
	canboard_t *cbr = cm -> cbr;
	canpdo_t *rxpdo;

	rxpdo = &cm -> rxpdo[pdo];
	if (rxpdo -> data == NULL) {
		return;
	}

/* Costruzione del primo PDO standard (DS401). */
	msg.cob = 0;
	msg.flags = 0;
	msg.id = rxpdo -> id;
	msg.length = rxpdo -> size;
#if 0
{
int i;
	switch (rxpdo -> wordsz) {
	case 16:
		for (i = 0; i < size: i += wordsize) {
			msg.data[i] = (*(unsigned short *)rxpdo->data)
			            % 0x100;
			msg.data[i+1] = ((*(unsigned short *)rxpdo->data)
			            / 0x100) % 0x100;
		}
		break;
	case 32:
		for (i = 0; i < size: i += wordsize) {
			msg.data[i] = (*(unsigned long *)rxpdo->data)
			            % 0x100;
			msg.data[i+1] = ((*(unsigned long *)rxpdo->data)
			            / 0x100) % 0x100;
			msg.data[i+2] = ((*(unsigned long *)rxpdo->data)
			            / 0x10000) % 0x100;
			msg.data[i+3] = ((*(unsigned long *)rxpdo->data)
			            / 0x1000000) % 0x100;
		}
		break;
	case 8:
		memcpy(msg.data, rxpdo -> data, rxpdo -> size);
		break;
	default: // TODO
		memcpy(msg.data, rxpdo -> data, rxpdo -> size);
		break;
	}
}
#else
	/* Ottimizzazione orrenda! (e neppure generale) */
	memcpy(msg.data, rxpdo -> data, rxpdo -> size);
#endif
	/* Emissione del messaggio. */
	canboard_send(cbr, &msg);
}

/* Invio del SYNC CANOpen. */
void canboard_send_sync(canboard_t *cbr)
{
canmsg_t msg;

	msg.cob = 0;
	msg.flags = 0;
	msg.id = CANOPEN_COBID_SYNC;
	msg.length = 0;
	canboard_send(cbr, &msg);
}

/* 
* Aggiornamento delle uscite, cioe` trasmissione delle
* nuove immagini delle uscite dei moduli che hanno subito
* una variazione. 
*/
void canboard_flush_output_ex(canboard_t *cbr, int send_sync)
{
	int i;
	int j;
	int bit;
	canmodule_t *cm;

	for (i = 0; i < CANOPEN_NUM_PDO; ++i) {
		for (j = 0; j < UPDATE_SET_DIM; ++j) {
			while (cbr -> update_set[i][j]) {
				bit = ffs(cbr -> update_set[i][j]) - 1;
				cm = &cbr -> module[j * UPDATE_SET_ITEM_NBIT + bit];
				canboard_write_pdo(cm, i);
				cbr -> update_set[i][j] &= ~(1 << bit);
			}
		}
	}

#ifdef TEST_ENCODER
#else
	if (send_sync)
#endif
	{
		canboard_send_sync(cbr);
	}
	/* PROVA lettura encoder. */
#ifdef TEST_ENCODER
{
int i;
unsigned long d;
int rv;
int index;
static int conta = 0;

if (++conta == 100) 
{
conta = 0;

	for (i = 0; i < 4; ++i) {
		d = 0;
#if 0
/* Posizione encoder in impulsi */
	index = 0x6063 + 0x800*i;
#else
/* Posizione encoder in micron */
	index = 0x6064 + 0x800*i;
#endif
		rv = canboard_read_dictionary(cbr, 1,
															 index, 0,
															 &d, sizeof(d));
		if (rv == 0) {
			if (cbr -> debug & 4) {
				printf("Module %d: Encoder %d = %lu = 0x%08lx\n", 1, i, d, d);
			
			}
		}
		else {
		}
	}
	}
}
#endif
}

void canboard_flush_output(canboard_t *cbr)
{
	canboard_flush_output_ex(cbr, 0);
}

/* 
* Scrittura di un bit di un modulo. 
*/
static void canboard_set_output_bit_m(canmodule_t *cm, int bit, int val)
{
	int m;
	unsigned char *op;
	canboard_t *cbr = cm -> cbr;

	/* 
	* Banale accensione di bit, un tantinello ottimizzata. 
	*/
	op = &(cm -> out[bit / 8]);
	m = 1 << (bit % 8);
	if (val) {
		val = m;
	}
	if ((*op & m) != val) {
		*op = (unsigned char)(val | (*op & ~m));
		/* 
		* C'e` stata una modifica: aggiungiamo il modulo all'insieme
		* di quelli che richiedono aggiornamento (cioe`: accendiamo il bit corrispondente). 
		*/
		cbr -> update_set[bit/64][cm->id / UPDATE_SET_ITEM_NBIT] |= 1 << (cm->id % UPDATE_SET_ITEM_NBIT);
	}
}

void canboard_set_output_bit(canboard_t *cbr, int mod, int bit, int val)
{
	canboard_set_output_bit_m(&cbr->module[mod], bit, val);
}

/* 
* Scrittura di un intero byte di un modulo. 
*/
static void canboard_set_output_byte_m(canmodule_t *cm, int b, int val)
{
	unsigned char *op;
	canboard_t *cbr = cm -> cbr;

	/* */
	op = &(cm -> out[b]);
	if (*op != val) {
		*op = (unsigned char) val;
		/* 
		* C'e` stata una modifica: aggiungiamo il modulo all'insieme
		* di quelli che richiedono aggiornamento (cioe`: accendiamo il bit corrispondente). 
		*/
		cbr -> update_set[b/8][cm->id / UPDATE_SET_ITEM_NBIT] |= 1 << (cm->id % UPDATE_SET_ITEM_NBIT);
	}
}

void canboard_set_output_byte(canboard_t *cbr, int mod, int b, int val)
{
	canboard_set_output_byte_m(&cbr->module[mod], b, val);
}

/* Scrittura di un intero byte di un modulo. */
static void canboard_set_output_word_m(canmodule_t *cm, int b, int val)
{
	unsigned short *op;
	canboard_t *cbr = cm -> cbr;

	if (b > 8) {
		return;
	}
	/* */
	op = &(cm -> aout[b]);
	if (*op != val) {
		*op = (unsigned short) val;
		/* 
		* C'e` stata una modifica: aggiungiamo il modulo all'insieme
		* di quelli che richiedono aggiornamento (cioe`: accendiamo il bit corrispondente). 
		*/
		cbr -> update_set[b/4 + 2][cm->id / UPDATE_SET_ITEM_NBIT] |= 1 << (cm->id % UPDATE_SET_ITEM_NBIT);
	}
}

void canboard_set_output_word(canboard_t *cbr, int mod, int b, int val)
{
	canboard_set_output_word_m(&cbr->module[mod], b, val);
}

/*
* Calcolo del puntatore all'immagine delle uscite di un modulo
* (scopo: scrittura ottimizzata). 
*/
unsigned char *canboard_get_output_image(canboard_t *cbr, int mod)
{
	return cbr -> module[mod].out;
}

/*
* Calcolo del puntatore all'immagine delle uscite analogiche di un modulo
* (scopo: scittura ottimizzata). 
*/
unsigned short *canboard_get_aout_image(canboard_t *cbr, int mod)
{
	return cbr -> module[mod].aout;
}

/*
* Calcolo del puntatore all'immagine degli ingressi di un modulo
* (scopo: lettura ottimizzata). 
*/
unsigned char *canboard_get_input_image(canboard_t *cbr, int mod)
{
	return cbr -> module[mod].in;
}

/* 
* Calcolo del puntatore all'immagine degli ingressi analogici di un modulo
 (scopo: lettura ottimizzata). 
 */
unsigned short *canboard_get_ain_image(canboard_t *cbr, int mod)
{
	return cbr -> module[mod].ain;
}

/*
* Calcolo del puntatore all'immagine degli ingressi encoder di un modulo
* (scopo: lettura ottimizzata). 
*/
long *canboard_get_enc_image(canboard_t *cbr, int mod)
{
	return cbr -> module[mod].enc;
}

int canboard_get_input_bit(canboard_t *cbr, int mod, int bit)
{
	return (cbr -> module[mod].in[bit / 8] & (1 << (bit % 8))) != 0;
}

static int canboard_recv_timeout(canboard_t *cbr, canmsg_t *msg, int timeout)
{
	fd_set fds;
	struct timeval tm;
	int n;

	if (timeout < 0) {
		return canboard_recv(cbr,msg);
	}
	FD_ZERO(&fds);
	FD_SET(cbr -> fd_rx,&fds);
	tm.tv_sec = timeout / 1000;
	tm.tv_usec = (timeout % 1000) * 1000;
	n = select(cbr -> fd_rx + 1,&fds,NULL,NULL,&tm);
	if (n < 0) {
		return -1;
	} else if (n == 0) {
		return 0;
	}
	return canboard_recv(cbr,msg);
}

static int canboard_sdo_answer(canboard_t *cbr, canmsg_t *msg, int id)
{
	int rv;
	time_t t0;
	canmsgcb_t old_sdo_cb;

	old_sdo_cb = cbr->msgcb[CANOPEN_COBID_SDOTX(id)];
	cbr -> msgcb[CANOPEN_COBID_SDOTX(id)].fn = default_msg_cb;
	cbr -> msgcb[CANOPEN_COBID_SDOTX(id)].arg = NULL;

	t0 = time(NULL);

	do {
		rv = canboard_recv_timeout(cbr,msg,SDO_TIMEOUT*1000);
		if (rv <= 0) {
			goto FAIL;
		}
		if (time(NULL) - t0 >= 2*SDO_TIMEOUT) {
			goto FAIL;
		}
		canboard_handle_msg(cbr, msg);
		//if (msg -> flags & MSG_ERR_MASK) {
		//	continue;
		//}
	} while ((msg -> flags & (MSG_ERR_MASK | MSG_RTR)) != 0 || msg -> id != CANOPEN_COBID_SDOTX(id));

	if (msg -> length != 8) {
		goto FAIL;
	}
	rv = 0;

END:
	cbr->msgcb[CANOPEN_COBID_SDOTX(id)] = old_sdo_cb;
	return rv;
	
FAIL:
	rv = -1;
	goto END;
}

static int sdo_rw_abort_code(sdo_rw_t *sr, unsigned long code)
{
	sr -> msg.id = CANOPEN_COBID_SDORX(sr -> cm->id);
	sr -> msg.cob = 0;
	sr -> msg.length = 8;
	sr -> msg.flags = 0;
	if (sr -> wr) {
		sr -> msg.data[0] = CANOPEN_SCS_ABORT;
	}
	else {
		sr -> msg.data[0] = CANOPEN_CCS_ABORT;
	}
	sr -> msg.data[1] = sr -> idx % 0x100;
	sr -> msg.data[2] = sr -> idx / 0x100;
	sr -> msg.data[3] = sr -> subidx;
	sr -> msg.data[4] = (code % 0x100);
	sr -> msg.data[5] = ((code / 0x100) % 0x100);
	sr -> msg.data[6] = ((code / 0x10000) % 0x100);
	sr -> msg.data[7] = ((code / 0x1000000) % 0x100);
	return canboard_send(sr -> cbr,&sr -> msg);
}

static int sdo_rw_abort(sdo_rw_t *sr)
{
	return sdo_rw_abort_code(sr, 0x05040002); /* Invalid size */
}

static int sdo_rw_check(sdo_rw_t *sr, unsigned int cond)
{
	if (sr -> msg.data[0] == ((sr -> wr) ? CANOPEN_CCS_ABORT : CANOPEN_SCS_ABORT)) {
		sr -> abort = sr->msg.data[7] * 0x1000000
		            + sr->msg.data[6] * 0x10000
		            + sr->msg.data[5] * 0x100
		            + sr->msg.data[4];
		return -1;
	}
	if ((sr -> msg.data[0] & CANOPEN_SCS_M) != cond) {
		sdo_rw_abort(sr);
		return -1;
	}
	return 0;
}

static int sdo_rw_next(sdo_rw_t *sr, int state) 
{
	sr -> state = state;
	return 1;
}

static int sdo_rw_jump(sdo_rw_t *sr, int state) 
{
	sr -> state = state;
	return 0;
}

static int sdo_rw_end(sdo_rw_t *sr) 
{
	if (sr -> wr) {
		*sr -> len = sr -> tot;
	} else {
		*sr -> len = sr -> n;
	}
	return sdo_rw_jump(sr, 4);
}

static int sdo_rw_fail(sdo_rw_t *sr) 
{
	return sdo_rw_jump(sr, 5);
}

static void sdo_rw_reset(sdo_rw_t *sr, canmodule_t *cm)
{
	sr -> state = -1;
	sr -> wr = -1;
	sr -> cm = cm;
	sr -> idx = 0;
	sr -> subidx = 0;
	sr -> data = NULL;
	sr -> len = 0;
	memset(&sr -> msg, 0, sizeof(sr -> msg));
	sr -> p = 0;
	sr -> tot = 0;
	sr -> cont = 0;
	sr -> toggle = 0;
	sr -> n = 0;
	sr -> abort = 0;
	sr -> cbr = cm -> cbr;
	sr -> timer = 0;
	sr -> rv = 1;
}

static void sdo_rw_init(sdo_rw_t *sr, canmodule_t *cm)
{
	sdo_rw_reset(sr, cm);
	sr -> busy = 0;
	sr -> prev = NULL;
	sr -> next = NULL;
}

static int sdo_rw_start(sdo_rw_t *sr,
                     canmodule_t *cm,
                     int idx, int subidx,
                     void *data, unsigned int *len, int wr)
{
	if (data == NULL || len == NULL || *len == 0) {
		return -1;
	}

	if (sr -> busy) {
		return -2;
	}

	sdo_rw_reset(sr, cm);

	sr -> busy = 1;
	if (wr) {
		sr -> wr = 1;
		sr -> state = 6;
		sr -> n = *len;
	} else {
		sr -> wr = 0;
		sr -> state = 0;
		sr -> n = 0;
	}

	sr -> idx = idx;
	sr -> subidx = subidx;
	sr -> data = data;
	sr -> len = len;
	sr -> p = (unsigned char *)data;

	return 0;
}
static int sdo_read_s0(sdo_rw_t *sr)
{
	sr -> msg.id = CANOPEN_COBID_SDORX(sr->cm->id);
	sr -> msg.cob = 0;
	sr -> msg.length = 8;
	sr -> msg.flags = 0;
	sr -> msg.data[0] = CANOPEN_CCS_STARTUPLOAD;
	sr -> msg.data[1] = sr -> idx % 0x100;
	sr -> msg.data[2] = sr -> idx / 0x100;
	sr -> msg.data[3] = sr -> subidx;
	sr -> msg.data[4] = 0;
	sr -> msg.data[5] = 0;
	sr -> msg.data[6] = 0;
	sr -> msg.data[7] = 0;

	if (canboard_send(sr -> cbr,&sr -> msg) < 0) {
		return sdo_rw_fail(sr);
	}
	return sdo_rw_next(sr, 1);
}

static int sdo_read_s1(sdo_rw_t *sr)
{
	sr -> timer = 0;

	if (sdo_rw_check(sr, CANOPEN_SCS_STARTUPLOAD) < 0) {
		return sdo_rw_fail(sr);
	}

	sr -> tot = 0;
	sr -> cont = 0;

	switch (sr -> msg.data[0] & 0x03) {
		case 0:
			sr -> cont = 1;
			sr -> tot = 0xFFFFFFFF;
		break;
		case 1:
			sr -> tot = sr -> msg.data[4]
					+ sr -> msg.data[5] * 0x100
					+ sr -> msg.data[6] * 0x10000
					+ sr -> msg.data[7] * 0x1000000;
			sr -> cont = 1;
		break;
		case 2:
			sr -> cont = 0;
			sr -> tot = 4;
		break;
		case 3:
			sr -> cont = 0;
			sr -> tot = 4 - ((sr -> msg.data[0] & 0x0C) >> 2);
		break;
		default:
			return sdo_rw_fail(sr);
	}

	if (*(sr -> len) < sr -> tot) {
		sr -> tot = *(sr -> len);
	}

	if (! (sr -> cont && sr -> tot > 0)) {
		int i;
		
		for (i = 0; i < sr -> tot; ++i) {
			sr -> p[i] = sr -> msg.data[i+4];
		}
		if (sr -> tot <= 0) {
			sdo_rw_abort(sr);
		}
		sr -> n = sr -> tot;

		return sdo_rw_end(sr);
	}

	sr -> toggle = 0;
	sr -> n = 0;
	sr -> msg.id = CANOPEN_COBID_SDORX(sr -> cm->id);
	sr -> msg.cob = 0;
	sr -> msg.length = 8;
	sr -> msg.flags = 0;
	sr -> msg.data[0] = CANOPEN_CCS_SEGUPLOAD | sr -> toggle;
#ifdef STRICT_DS301
	sr -> msg.data[1] = 0;
	sr -> msg.data[2] = 0;
	sr -> msg.data[3] = 0;
#else
	sr -> msg.data[1] = sr -> idx % 0x100;
	sr -> msg.data[2] = sr -> idx / 0x100;
	sr -> msg.data[3] = sr -> subidx;
#endif
	sr -> msg.data[4] = 0;
	sr -> msg.data[5] = 0;
	sr -> msg.data[6] = 0;
	sr -> msg.data[7] = 0;
	if (canboard_send(sr -> cbr,&sr -> msg) < 0) {
		return sdo_rw_fail(sr);
	}
	return sdo_rw_next(sr, 2);
}

static int sdo_read_s2(sdo_rw_t *sr)
{
	int nb;
	int i;

	sr -> timer = 0;

	if (sdo_rw_check(sr, CANOPEN_SCS_SEGUPLOAD) < 0) {
		return sdo_rw_fail(sr);
	}

	sr -> toggle ^= 0x10;
	nb = 7 - ((sr -> msg.data[0] & 0x0E) >> 1);
	sr -> cont = !(sr -> msg.data[0] & 0x01);

	// printf("SDO: idx=%d cont=%d tot=%d n=%d nb=%d\n",sr->idx,sr->cont,sr->tot,sr->n,nb);

	for(i = 0;
	    i < nb && sr->tot > 0;
	    ++i, ++(sr->p), ++(sr->n), --(sr->tot)) {
		*sr -> p = sr -> msg.data[i+1];
	}
	if (! sr -> cont) {
		return sdo_rw_end(sr);
	}
	if (sr -> tot <= 0) {
		sdo_rw_abort(sr);
		return sdo_rw_end(sr);
	}

	sr -> msg.id = CANOPEN_COBID_SDORX(sr -> cm->id);
	sr -> msg.cob = 0;
	sr -> msg.length = 8;
	sr -> msg.flags = 0;
	sr -> msg.data[0] = CANOPEN_CCS_SEGUPLOAD | sr -> toggle;
#ifdef STRICT_DS301
	sr -> msg.data[1] = 0;
	sr -> msg.data[2] = 0;
	sr -> msg.data[3] = 0;
#else
	sr -> msg.data[1] = sr -> idx % 0x100;
	sr -> msg.data[2] = sr -> idx / 0x100;
	sr -> msg.data[3] = sr -> subidx;
#endif
	sr -> msg.data[4] = 0;
	sr -> msg.data[5] = 0;
	sr -> msg.data[6] = 0;
	sr -> msg.data[7] = 0;
	if (canboard_send(sr -> cbr,&sr -> msg) < 0) {
		return sdo_rw_fail(sr);
	}

	return sdo_rw_next(sr, 2);
}

static int sdo_write_s0(sdo_rw_t *sr)
{
	int i;

	sr -> msg.id = CANOPEN_COBID_SDORX(sr->cm->id);
	sr -> msg.cob = 0;
	sr -> msg.length = 8;
	sr -> msg.flags = 0;
	sr -> msg.data[0] = CANOPEN_CCS_STARTDOWNLOAD
	                  | 0x01; // CANOPEN_SDO_SIZED
	sr -> msg.data[1] = sr -> idx % 0x100;
	sr -> msg.data[2] = sr -> idx / 0x100;
	sr -> msg.data[3] = sr -> subidx;
	if (*sr -> len > 4) {
		sr -> msg.data[4] = (unsigned char)((*sr -> len)             % 0x100);
		sr -> msg.data[5] = (unsigned char)((*sr -> len /     0x100) % 0x100);
		sr -> msg.data[6] = (unsigned char)((*sr -> len /   0x10000) % 0x100);
		sr -> msg.data[7] = (unsigned char)((*sr -> len / 0x1000000) % 0x100);
	} else {
		sr -> msg.data[0] |= 0x02 | ((4-*sr -> len) << 2); // CANOPEN_SDO_EXPEDITED
		for (i = 0; i < *sr -> len; ++i) {
			sr -> msg.data[4+i] = sr -> p[i];
		}
		for (; i < 4; ++i) {
			sr -> msg.data[4+i] = 0;
		}
		sr -> tot = *sr -> len;
		sr -> n = 0;
	}
	if (canboard_send(sr -> cbr,&sr -> msg) < 0) {
		return sdo_rw_fail(sr);
	}
	return sdo_rw_next(sr, 7);
}

static int sdo_write_core(sdo_rw_t *sr)
{
	unsigned int nb;
	unsigned int i;

	if (sr -> n <= 0) {
		return sdo_rw_end(sr);
	}

	if (sr -> n > 7) {
		nb = 7;
	}
	else {
		nb = sr -> n;
	}
	sr -> msg.id = CANOPEN_COBID_SDORX(sr -> cm->id);
	sr -> msg.cob = 0;
	sr -> msg.length = 8;
	sr -> msg.flags = 0;
	sr -> msg.data[0] = CANOPEN_CCS_SEGDOWNLOAD
	                  | sr -> toggle
	                  | ((7 - nb) << 1);
	if (sr -> n <= 7) {
		sr -> msg.data[0] |= 0x01; // CANOPEN_SDO_NOMORE
	}
	for (i = 0; i < nb; ++i, ++sr -> p) {
		sr -> msg.data[i+1] = *sr -> p;
	}
	for (; i < 7; ++i) {
		sr -> msg.data[i+1] = 0;
	}

	if (canboard_send(sr->cbr,&sr->msg) < 0) {
		return sdo_rw_fail(sr);
	}

	sr -> n -= nb;
	sr -> tot += nb;
	sr -> toggle ^= 0x10;

	return sdo_rw_next(sr, 8);
}

static int sdo_write_s1(sdo_rw_t *sr)
{
	sr -> timer = 0;

	if (sdo_rw_check(sr, CANOPEN_SCS_STARTDOWNLOAD) < 0) {
		return sdo_rw_fail(sr);
	}

	sr -> toggle = 0;

	return sdo_write_core(sr);
}

static int sdo_write_s2(sdo_rw_t *sr)
{
	int nb;
	int i;

	sr -> timer = 0;

	if (sdo_rw_check(sr, CANOPEN_SCS_SEGDOWNLOAD) < 0) {
		return sdo_rw_fail(sr);
	}

	return sdo_write_core(sr);
}


static int sdo_rw_evol(sdo_rw_t *sr)
{
int rv = 0;

	// NOTA: per un corretto funzionamento multithreaded, bisogna
	// essere ragionevolmente sicuri che il campo "rv" della struttura
	// sia scritto atomicamente.

	for (;;) {
		switch (sr -> state) {
			case 0:
				rv = sdo_read_s0(sr);
			break;
			case 1:
				rv = sdo_read_s1(sr);
			break;
			case 2:
				rv = sdo_read_s2(sr);
			break;
			case 4:
			/* Successo. */
			return (sr -> rv = 0);
			case 5:
			/* Fallimento. */
			return (sr -> rv = -1);
			case 6:
				rv = sdo_write_s0(sr);
			break;
			case 7:
				rv = sdo_write_s1(sr);
			break;
			case 8:
				rv = sdo_write_s2(sr);
			break;
			case 9:
				/* Sequenza di timeout. */
				sdo_rw_abort_code(sr, 0x05040000); // timeout
				sr -> state = 5; // Fallimento
				sr -> abort = 0x05040000;
				rv = 0;
			break;
			default:
				/* Imprevisto o non inizializzato. */
				return (sr -> rv = -2);
			break;
		}
		if (rv == 1) {
			return (sr -> rv = 1);
		}
	}
}

static void sdo_handler(canboard_t *cbr, canmsg_t *msg, void *arg)
{
	sdo_rw_t *sr = (sdo_rw_t *)arg;

	sr -> msg = *msg;
	sdo_rw_evol(sr);
}

static int sdo_rw_wait(sdo_rw_t *sr)
{
	int rv;

	for (;;) {
		rv = sdo_rw_evol(sr);
		if (rv < 1) {
			sr -> busy = 0;
			return rv;
		}
		if (canboard_sdo_answer(sr->cbr, &sr->msg, sr->cm->id) < 0) {
			sdo_rw_abort(sr);
			sdo_rw_fail(sr);
		}
	}
}

static int sdo_rw_timeout(sdo_rw_t *sr) 
{
	return sdo_rw_jump(sr, 9);
}

static void sdo_dispose(sdo_t *sdo)
{
	if (sdo == NULL) {
		return;
	}
	/* Rimozione dalla lista delle richieste SDO inevase (per modulo). */
	sdo -> cm -> sdo_req_head = sdo -> next;
	if (sdo -> cm -> sdo_req_head == NULL) {
		sdo -> cm -> sdo_req_tail = NULL;
	}
	/* Liberazione del descrittore di transazione. */
	sdo_free(sdo);
}

static void sdo_complete(sdo_t *sdo)
{
	sdo_rw_t *sr;

	sr = sdo -> sr;
	if (sr == NULL) {
	/* ??? */
		sdo_dispose(sdo);
		return;
	}

	if (sdo -> seqn != 0 && sdo -> cb != NULL) {
		sdo -> cb(sdo, (sr->rv<0), sdo->len, sr->abort, sdo->cb_arg);
	}

	/* Rimozione dalla lista delle transazioni in corso. */
	if (sr -> next != NULL) {
		sr -> next -> prev = sr -> prev;
	}
	if (sr -> prev != NULL) {
		sr -> prev -> next = sr -> next;
	}
	else {
		sr -> cbr -> sdo_rw_list = sr -> next;
	}
	sr -> busy = 0;

	sdo_dispose(sdo);
}

static int sdo_async_start(sdo_t *sdo)
{
	int rv;
	sdo_rw_t *sr;

	sr = &sdo -> cm -> sr;
	sdo -> sr = sr;
	rv = sdo_rw_start(sr, sdo -> cm,
	                  sdo -> idx, sdo -> subidx,
	                  sdo -> data, &sdo -> len, sdo -> wr);

	/* Inserimento nella lista delle transazioni in corso. */
	sr -> prev = NULL;
	sr -> next = sr -> cbr -> sdo_rw_list;
	if (sr -> next != NULL) {
		sr -> next -> prev = sr;
	}
	sr -> cbr -> sdo_rw_list = sr;

	if (rv < 0) {
		sdo_complete(sdo);
	} else {
		rv = sdo_rw_evol(sr);
		if (rv < 1) {
			sdo_complete(sdo);
		}
	}
	return rv;
}

static void sdo_eval_list(canmodule_t *cm)
{
	sdo_t *sdo;
	sdo_t *sdo_next;
	int rv;

	for (sdo = cm -> sdo_req_head; sdo != NULL; sdo = sdo_next) {
		sdo_next = sdo -> next;
		rv = sdo_async_start(sdo);
		if (rv >= 0) {
			break;
		}
	}
}

static void sdo_rw_async_end(sdo_rw_t *sr)
{
	canmodule_t *cm;
	sdo_t *sdo;

	cm = sr -> cm;
	if (cm == NULL || &cm -> sr != sr) {
		/* Non e' asincrono (puo' esserlo solo lo SDO privato del modulo). */
		return;
	}
	sdo = cm -> sdo_req_head;
	if (sdo == NULL || sdo -> sr != sr) {
		return;
	}
	sdo_complete(sdo);
	/* Elaborazione della prossima richiesta pendente. */
	sdo_eval_list(cm);
}

static void sdo_rw_schedule(sdo_rw_t *sr, unsigned long period)
{
	if (sr -> rv < 1) {
		sdo_rw_async_end(sr);
	} else if (period > 0) {
		sr -> timer += period;
		if (sr -> timer >= SDO_TIMEOUT*1000) {
			sdo_rw_timeout(sr);
			sdo_rw_evol(sr);
			sdo_rw_async_end(sr);
		}
	}
}

void canboard_schedule(canboard_t *cbr, unsigned long period)
{
	sdo_rw_t *sr;
	sdo_rw_t *sr_next;

	for (sr = cbr -> sdo_rw_list; sr; sr = sr_next) {
		sr_next = sr -> next;
		sdo_rw_schedule(sr, period);
	}
}

static int canboard_sdo_read(canmodule_t *cm, int idx, int subidx, void *data, unsigned int *len)
{
	sdo_rw_t sr;
	int rv;

	sdo_rw_init(&sr, cm);

	if (sdo_rw_start(&sr, cm, idx, subidx, data, len, 0) < 0) {
		return -1;
	}

	return sdo_rw_wait(&sr);
}

static int canboard_sdo_write(canmodule_t *cm, int idx, int subidx, void *data, unsigned int *len)
{
	sdo_rw_t sr;
	int rv;

	sdo_rw_init(&sr, cm);

	if (sdo_rw_start(&sr, cm, idx, subidx, data, len, 1) < 0) {
		return -1;
	}

	return sdo_rw_wait(&sr);
}

static int canboard_read_dictionary_m(canmodule_t *cm, int idx, int subidx, void *data, int size)
{
	int rv;
	unsigned int sz = (unsigned int) size;

	canboard_disable_receiver(cm->cbr);
	rv = canboard_sdo_read(cm, idx, subidx, data, &sz);
	canboard_enable_receiver(cm->cbr);

	return rv;
}

int canboard_read_dictionary(canboard_t *cbr, int mod, int idx, int subidx, void *data, int size)
{
	return canboard_read_dictionary_m(&cbr->module[mod], idx, subidx, data, size);
}

static int canboard_write_dictionary_m(canmodule_t *cm, int idx, int subidx, void *data, int size)
{
	int rv;
	unsigned int sz = (unsigned int) size;

	canboard_disable_receiver(cm->cbr);
	rv = canboard_sdo_write(cm, idx, subidx, data, &sz);
	canboard_enable_receiver(cm->cbr);

	return rv;
}

int canboard_write_dictionary(canboard_t *cbr, int mod, int idx, int subidx, void *data, int size)
{
	return canboard_write_dictionary_m(&cbr->module[mod], idx, subidx, data, size);
}

int canboard_async_sdo_cancel(sdo_t *sdo)
{
	if (sdo == NULL) {
		return -1;
	}
	sdo -> seqn = 0;
	sdo_complete(sdo);
	return 0;
}

static sdo_t *canboard_async_sdo_m(canmodule_t *cm, int idx, int subidx, void *data, unsigned int len, int wr, void (*cb)(sdo_t *sdo, int err, unsigned int len, unsigned int abort, void *cb_arg), void *cb_arg)
{
	sdo_t *sdo;

	if (data == NULL || len == 0 || cb == NULL || cm == NULL) {
		return NULL;
	}

	sdo = sdo_allocate(cm -> cbr);
	if (sdo == NULL) {
		return NULL;
	}

	sdo -> cm = cm;
	sdo -> wr = wr;
	sdo -> idx = idx;
	sdo -> subidx = subidx;
	sdo -> data = data;
	sdo -> len = len;
	sdo -> cb = cb;
	sdo -> cb_arg = cb_arg;
	if (cm -> sdo_req_tail != NULL) {
		cm -> sdo_req_tail -> next = sdo;
	} else {
		cm -> sdo_req_head = sdo;
	}
	cm -> sdo_req_tail = sdo;
	if (cm -> sdo_req_head == sdo) {
		if (sdo_async_start(sdo) < 0) {
			return NULL;
		}
	}
	return sdo;
}

sdo_t *canboard_async_sdo(canboard_t *cbr, int mod,int idx, int subidx, void *data, unsigned int len, int wr, void (*cb)(sdo_t *sdo, int err, unsigned int len, unsigned int abort, void *cb_arg),void *cb_arg)
{
	if (cbr == NULL || mod < 1 || mod > 127) {
		return NULL;
	}
	return canboard_async_sdo_m(&cbr->module[mod], idx, subidx, data, len, wr, cb, cb_arg);
}

static int canop_remap_pdo(canmodule_t *cm, int rd, unsigned int pdo,
                           unsigned int index, unsigned int subindex,
                           unsigned int nel, void *data, unsigned int elbitsz,
                           unsigned int mode, unsigned int steponindex)
{
	canboard_t *cbr;
	unsigned long x;
	unsigned char c;
	unsigned char m[4];
	unsigned int v;
	unsigned int map_idx, comm_idx;
	unsigned int pdo_id;
	unsigned int i;
	unsigned int nbit;
	canpdo_t *p;

	cbr = cm -> cbr;

	nbit = nel * elbitsz;
	if (nbit > 64) {
		return -1;
	}

	/*
	* Se richiesto, saltiamo l'effettiva riscrittura dei registri di mappa
	* sui moduli CNI Informatica, confidando (poco) nella mappatura di default. 
	*/
	if (cm -> profile != PROFILE_CNIINFORMATICA || ! cbr -> noremap_pdo_cniinf) {
		if (rd) {
			map_idx = 0x1A00;
			comm_idx = 0x1800;
			//pdo_id = CANOPEN_COBID_PDOTX(pdo,cm->id);
			pdo_id = cm -> txpdo[pdo].id;
		} else {
			map_idx = 0x1600;
			comm_idx = 0x1400;
			//pdo_id = CANOPEN_COBID_PDORX(pdo,cm->id);
			pdo_id = cm -> rxpdo[pdo].id;
		}

		map_idx += pdo;
		comm_idx += pdo;

		if (cbr -> debug & 1) {
			if (nbit != 0) {
				printf("Module %d: Mapping PDO %s %u to %04x:%x, %u fields of %u bits, buffer 0x%08lx of %u bytes\n",
					cm->id, rd ? "TX" : "RX",
					pdo+1, index, subindex,
					nel, elbitsz,
					(unsigned long)data, (nel*elbitsz+7)/8);
			} else {
				printf("Module %d: Disabling PDO %s %u\n",
					cm->id, rd ? "TX" : "RX",
					pdo+1);
			}
		}

		/* Inibizione del PDO. */
		m[0] = pdo_id % 0x100;
		m[1] = pdo_id / 0x100;
		m[2] = 0;
		m[3] = 0xC0;
		if (canboard_write_dictionary_m(cm,comm_idx,1,&m,sizeof(m)) < 0) {
			if (cbr -> debug & 1) {
				printf("Module %d: PDO disable failed\n",
					cm->id);
			}
			return -1;
		}

		if (nbit == 0) {
			return 0;
		}

		/* Azzeramento della sua mappa. */
		c = 0;
		if (canboard_write_dictionary_m(cm,map_idx,0,&c,sizeof(c)) < 0) {
			if (cbr -> debug & 1) {
				printf("Module %d: PDO map reset failed\n",
					cm->id);
			}
			return -1;
		}
		/* Mappatura di un canale. */
		for (i = 0; i < nel; ++i) {

			unsigned int rem_idx;
			unsigned int rem_subidx;

			if (steponindex != 0) {
				rem_idx = index + steponindex*i;
				rem_subidx = subindex;
			} else {
				rem_idx = index;
				rem_subidx = subindex + i;
			}

			m[0] = elbitsz;
			m[1] = rem_subidx;
			m[2] = rem_idx % 0x100;
			m[3] = rem_idx / 0x100;
			if (canboard_write_dictionary_m(cm,map_idx,1+i,&m,sizeof(m))
				< 0) {
				if (cbr -> debug & 1) {
					printf("Module %d: PDO map field %d failed\n",
						cm->id,i+1);
				}
				return -1;
			}
		}
		/* Definizione della lunghezza della mappa. */
		c = (unsigned char)nel;
		if (canboard_write_dictionary_m(cm,map_idx,0,&c,sizeof(c)) < 0) {
			if (cbr -> debug & 1) {
				printf("Module %d: PDO map length setup failed\n",
					cm->id);
			}
			return -1;
		}
		/*
		* Impostazione dell'inhibit time sul PDO. Scegliamo un valore dell'ordine del tempo di ciclo. 
		*/
		/* TODO: Parametrizzare questa grandezza. */
		v = 100; // unita` = 100 microsecondi */
		m[0] = v % 0x100;
		m[1] = v / 0x100;
		if (canboard_write_dictionary_m(cm,comm_idx,3,&m,2) < 0) {
			if (cbr -> debug & 1) {
				printf("Module %d: inhibit time not supported\n", cm->id);
			}
		}
		/* Impostazione della modalita` asincrona. */
		//c = 254;
		//c = 255;
		//c = 0;
		//c = 1;
		c = (unsigned char)mode;
		if (canboard_write_dictionary_m(cm,comm_idx,2,&c,sizeof(c)) < 0) {
			if (cbr -> debug & 1) {
				printf("Module %d: PDO mode %d not supported\n", cm->id, mode);
			}
			return -1;
		}
		/* Riattivazione del PDO. */
		m[0] = pdo_id % 0x100;
		m[1] = pdo_id / 0x100;
		m[2] = 0;
		m[3] = 0x40;
		//	printf("comm_idx = 0x%04x pdo_id = 0x%03x\n",comm_idx, pdo_id);
		if (canboard_write_dictionary_m(cm,comm_idx,1,&m,sizeof(m)) < 0) {
			if (cbr -> debug & 1) {
				printf("Module %d: PDO enable failed\n", cm->id);
			}
			return -1;
		}

	} // !CNIINF

	/* Registrazione del target buffer nel descrittore del pdo. */
	if (rd) {
		p = &cm -> txpdo[pdo];
	} else {
		p = &cm -> rxpdo[pdo];
	}

	p -> data = data;
	p -> size = (nel * elbitsz + 7) / 8;
	p -> wordsz = elbitsz;

	return 0;
}

static int canop_remap_pdos_ex(canmodule_t *cm,
                           int rd, unsigned int pdo, unsigned int npdo,
                           unsigned int index, unsigned int subidx,
                           unsigned int nel, void *data, unsigned int elbitsz,
                           unsigned int mode, unsigned int steponindex)
{
	canboard_t *cbr;
	unsigned int i;
	unsigned int nstep;
	unsigned int elperstep;
	unsigned int elcount;
	int rv = 0;
	int v = 0;

	cbr = cm -> cbr;

	if (nel == 0 || elbitsz == 0) {
		return 0;
	}
	if (nel * elbitsz > 64) {
		elperstep = 64 / elbitsz;
	} else {
		elperstep = nel;
	}
	nstep = (nel + elperstep - 1) / elperstep;
	if (nstep > npdo) {
		nstep = npdo;
	}
	for (i = 0, elcount = 0;
	     i < nstep;
	     ++i, nel -= elperstep, elcount += elperstep) {

		unsigned int rem_idx;
		unsigned int rem_subidx;

		if (steponindex != 0) {
			rem_idx = index + steponindex*elcount;
			rem_subidx = subidx;
		} else {
			rem_idx = index;
			rem_subidx = elcount + 1 + subidx;
		}

		v = canop_remap_pdo(cm, rd, pdo + i,
		                rem_idx, rem_subidx,
		                ((nel>elperstep) ? elperstep : nel),
		                ((unsigned char *)data)
		                 + elcount*((elbitsz+7)/8),
		                elbitsz, mode, steponindex);
		if (v != 0) {
			rv = -1;
			if (cbr -> debug & 1) {
				printf("Module %d: PDO %s %d remap failed\n", cm->id, rd ? "TX" : "RX", pdo+i+1);
			}
		}
	}
	for ( ; i < npdo; ++i) {
		v = canop_remap_pdo(cm, rd, pdo + i,
		                0, 0, 0, NULL, 0, mode, steponindex);
		if (v != 0) {
			rv = -1;
			if (cbr -> debug & 1) {
				printf("Module %d: PDO %s %d disable failed\n", cm->id, rd ? "TX" : "RX", pdo+i+1);
			}
		}
	}
	return rv;
}

static int canop_remap_pdos(canmodule_t *cm,
                           int rd, unsigned int pdo, unsigned int npdo,
                           unsigned int index,
                           unsigned int nel, void *data, unsigned int elbitsz,
                           unsigned int mode, unsigned int steponindex)
{
	return canop_remap_pdos_ex(cm, rd, pdo, npdo, index, 0, nel, data, elbitsz, mode, steponindex);
}

static int canboard_setup_cn104(canboard_t *cbr, int mod)
{
	canmodule_t *cm;
	int rv;
	int n;
	unsigned char nc;
	unsigned char d[4];
	int i;

	if (mod < 1 || mod > 127) {
		return 0;
	}

	cm = &(cbr -> module[mod]);

	/*
	* Interpreto la documentazione: c'e` un array non standard
	* di indici di porte da "attivare" (?). Il valore
	* d'uscita va in uno strano posto nella parte custom. 
	*/
	nc = 0;
	/* Sulla K50 c'e` un'uscita analogica, ma il registro 640F non va... */
	if (cm -> model == CNIINF_K50) {
		rv = 0;
		nc = 1;
	} else {
		rv = canboard_read_dictionary_m(cm, 0x640F, 0, &nc, sizeof(nc));
	}
	n = nc;
	if (rv != 0) {
		cm -> n_aout = 0;
		if (cbr -> debug & 1) {
			printf("Module CN104 %d: Cannot read analog output number\n", mod);
		}
	} else {
		if (n > 4) {
			/* Cosi` dice la documentazione. */
			n = 4;
		}
		for (i = 0; i < n; ++i) {
			nc = (unsigned char) i;
			rv = canboard_write_dictionary_m( cm, 0x640F, i+1, &nc, sizeof(nc));
			if (rv != 0) {
				if (cbr -> debug & 1) {
					printf("Module CN104 %d: Analog output %d: configuration failed\n",mod, i);
				}
			}
		}
		cm -> n_aout = i;
	}

	/* Non attiviamo gli assi sulla CN50 (detta k50). */
	if (cm -> model == CNIINF_K50) {
		cm -> n_enc = 1;
	} else {

	/* La "FASE 2" !!! */

#if 1 
		/* Se non funziona cosi`, prova cosa`! */
		d[0] = 2;
		d[1] = 0;
#else
		d[0] = 0;
		d[1] = 2;
#endif
		rv = canboard_write_dictionary_m(cm, 0x2042, 2, d, 2);
		if (rv != 0) {
			if (cbr -> debug & 1) {
				printf("Module CN104 %d:"
					" PHASE 2:"
					" configuration failed\n",
					mod);
			}
		}

		cm -> n_enc = 2; // 3

		/* TODO: probabilmente qui si dovra` immettere una quantita`
		spropositata di dati macchina. */

#ifdef TEST_ENCODER
		/* ...intanto li leggiamo... */
		{
			const struct {
				int index;
				int subindex;
				int size;
			} dmacc[] = {
			{ 0x3003, 0, 2, },
			{ 0x3011, 0, 4, },
			{ 0x3013, 0, 4, },
			{ 0x604f, 0, 4, },
			{ 0x6050, 0, 4, },
			{ 0x6051, 0, 4, },
			{ 0x6054, 0, 4, },
			{ 0x6063, 0, 4, },
			{ 0x6064, 0, 4, },
			{ 0x607B, 1, 4, },
			{ 0x607B, 2, 4, },
			{ 0x607F, 0, 4, },
			{ 0x608F, 1, 4, },
			{ 0x608F, 2, 4, },
			{ 0x6099, 1, 4, },
			{ 0x6099, 2, 4, },
			{ 0x60FB, 1, 4, },
			{ 0x60FB, 2, 4, },
			{ 0x60FB, 3, 4, },
			{ 0x60FB, 4, 4, },
			};
			int i,j;
			int index;

			for (i = 0; i < 5; ++i) {
				if (cbr -> debug & 1) {
					printf("Module CN104 %d: AX %d\n", mod, i);
				}
				for (j = 0; j < sizeof(dmacc)/sizeof(dmacc[0]); ++j) {
					memset(d,0,sizeof(d));
					index = dmacc[j].index+0x800*i;
					rv = canboard_read_dictionary_m(cm, index,
								dmacc[j].subindex, d, dmacc[j].size);
					if (rv == 0) {
						if (cbr -> debug & 1) {
							printf("Module CN104 %d:"
							" obj %04x:%d = 0x%08lx\n",
							mod, index, dmacc[j].subindex,
							*(unsigned long *)d);
						}
					}
				}
#if 0
				d[0] = 1;
				d[1] = 0;
				d[2] = 0;
				d[3] = 0;
				rv = canboard_write_dictionary_m(cm, 0x607B + 0x800*i, 1, d, 4);
				if (rv != 0) {
					if (cbr -> debug & 1) {
						printf("Module CN104 %d:"
						" AX %d: idx=607B:1"
						" configuration failed\n",
						mod, i);
					}
				}
				rv = canboard_write_dictionary_m(cm, 0x607B + 0x800*i, 2, d, 4);
				if (rv != 0) {
					if (cbr -> debug & 1) {
						printf("Module CN104 %d:"
						" AX %d: idx=607B:2"
						" configuration failed\n",
						mod, i);
					}
				}
				rv = canboard_write_dictionary_m(cm, 0x608F + 0x800*i, 1, d, 4);
				if (rv != 0) {
					if (cbr -> debug & 1) {
						printf("Module CN104 %d:"
						" AX %d: idx=608F:1"
						" configuration failed\n",
						mod, i);
					}
				}
				rv = canboard_write_dictionary_m(cm, 0x608F + 0x800*i, 2, d, 4);
				if (rv != 0) {
					if (cbr -> debug & 1) {
						printf("Module CN104 %d:"
						" AX %d: idx=608F:2"
						" configuration failed\n",
						mod, i);
					}
				}
#endif
#if 0
				d[0] = 1 << 3;
				d[1] = 0;
				rv = canboard_write_dictionary_m(cm, 0x3001 + 0x800*i, 0, d, 2);
				if (rv != 0) {
					if (cbr -> debug & 1) {
						printf("Module CN104 %d:"
						" AX %d:"
						" configuration failed\n",
						mod, i);
					}
				}
				d[0] = 1 << 7;
				d[1] = 0;
				rv = canboard_write_dictionary_m(cm, 0x3001 + 0x800*i, 0, d, 2);
				if (rv != 0) {
					if (cbr -> debug & 1) {
						printf("Module CN104 %d:"
						" AX %d:"
						" configuration failed\n",
						mod, i);
					}
				}
#endif
			}
		}
#endif

	} // ! k50
	return 0;
}

static int canop_startup_mod(canmodule_t *cm)
{
	canboard_t *cbr = cm -> cbr;

	/* 
	* Rimappiamo tutti i PDO che sono
	* necessari per ricoprire i segnali disponibili. 
	*/
	canop_remap_pdos(cm, 1, 0, 2, 0x6000, cm -> n_ib, cm -> in, 8, 254, 0);
#ifdef USE_ENCODER
#if 0
	cm -> n_enc = 3;
#endif
	if (cm -> n_enc > 0) {
		canop_remap_pdos(cm, 1, 2, 1, 0x6401, cm -> n_ain, cm -> ain, 16, 0, 0);
		canop_remap_pdos(cm, 1, 3, 1, 0x6864, cm -> n_enc, cm -> enc, 32, 0, 0x800);
	} else {
		canop_remap_pdos(cm, 1, 2, 2, 0x6401, cm -> n_ain, cm -> ain, 16, 0, 0);
	}
#else
	canop_remap_pdos(cm, 1, 2, 2, 0x6401, cm -> n_ain, cm -> ain, 16, 0, 0);
#endif

	/*
	* Altra stranezza dei moduli CNI Informatica: gli output sono
	* sparsi tra PDO1 e PDO4. Mah!... Fingeremo che PDO2 e PDO4 coincidano.
	* Questo naturalmente limitera` il numero di uscite analogiche
	* (che comunque non funzionano...). 
	*/
	if (cm -> profile == PROFILE_CNIINFORMATICA && cbr -> noremap_pdo_cniinf) {
		cm -> rxpdo[1] = cm -> rxpdo[3];
		canop_remap_pdos(cm, 0, 2, 1, cm -> aout_idx, cm -> n_aout, cm -> aout, 16, 254, 0);
	} else {
		canop_remap_pdos(cm, 0, 2, 2, cm -> aout_idx, cm -> n_aout, cm -> aout, 16, 254, 0);
	}

	canop_remap_pdos(cm, 0, 0, 2, 0x6200, cm -> n_ob, cm -> out, 8, 254, 0);


	/*
	* Impostiamo la callback di reazione ai PDO. Il terzo argomento della
	* callback e` qui usato per comunicare l'indice del PDO. Nel caso di moduli
	* CNI Informatica, se la rimappatura e` disattivata, falsifichiamo il
	* secondo PDO (input oltre il 64) in modo che figuri come un PDO2 standard. 
	*/

	if (cm -> profile == PROFILE_CNIINFORMATICA && cbr -> noremap_pdo_cniinf) {
		cbr -> msgcb[CANOPEN_COBID_PDO1TX(cm -> id)].fn = canboard_handle_pdo;
		cbr -> msgcb[CANOPEN_COBID_PDO1TX(cm -> id)].arg = (void *)0;
		/* 
		* Qui non uso volutamente la macro per rimarcare la lontananza
		* dallo standard (sarebbe un PDO di RICEZIONE, questo). 
		*/
		cbr -> msgcb[0x400+(cm -> id)].fn = canboard_handle_pdo;
		cbr -> msgcb[0x400+(cm -> id)].arg = (void *)1;
		/* Inutile attivare altri PDO: non funzionano. */
	} else {
		cbr -> msgcb[CANOPEN_COBID_PDO1TX(cm -> id)].fn = canboard_handle_pdo;
		cbr -> msgcb[CANOPEN_COBID_PDO1TX(cm -> id)].arg = (void *)0;
		cbr -> msgcb[CANOPEN_COBID_PDO2TX(cm -> id)].fn = canboard_handle_pdo;
		cbr -> msgcb[CANOPEN_COBID_PDO2TX(cm -> id)].arg = (void *)1;
		cbr -> msgcb[CANOPEN_COBID_PDO3TX(cm -> id)].fn = canboard_handle_pdo;
		cbr -> msgcb[CANOPEN_COBID_PDO3TX(cm -> id)].arg = (void *)2;
		cbr -> msgcb[CANOPEN_COBID_PDO4TX(cm -> id)].fn = canboard_handle_pdo;
		cbr -> msgcb[CANOPEN_COBID_PDO4TX(cm -> id)].arg = (void *)3;
	}

	return 1;
}

static int canboard_setup_module(canboard_t *cbr, int mod)
{
	canmodule_t *cm;
	int rv;
	int n;
	unsigned char nc;
	unsigned char d[4];
	int i;

	if (mod < 1 || mod > 127) {
		return 0;
	}

	cm = &(cbr -> module[mod]);

#if 0
	/* Reset "tosto" */
	canboard_reset_module(cbr, mod);
	usleep(DEFAULT_RESET_PAUSE);
#endif

	/*
	* Portiamo nello stato PREOPERATIONAL il modulo.
	* Cosi` siamo sicuri di poterlo manipolare via SDO. 
	*/

	for (i = 0; i < 1 /* 3 */; ++i) {
		canboard_preop_module(cbr, mod);
		memset(d, 0, sizeof(d));
		rv = canboard_read_dictionary_m(cm, 0x1000, 0, &d, sizeof(d));
		if (rv == 0) {
			cm -> profile = d[0] + 0x100*d[1];
			cm -> info = d[2] + 0x100*d[3];
			break;
		}
	}

	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Device profile = 0x%04x (%u) info = 0x%04x (%u)\n", mod, cm -> profile, cm -> profile, cm -> info, cm -> info);
		} else {
			printf("Module %d: Device profile unkonwn (%d)\n", mod, rv);
		}
	}

	if (rv != 0) {
		/*
		* Va bene essere elastici, ma tollerare l'assenza di
		* un device profile mi sembra troppo! 
		*/
		cm -> state = CANMOD_STATE_NOTPRESENT;
		return 0;
	}

	memset(cm -> name, 0, sizeof(cm -> name));
	rv = canboard_read_dictionary_m(cm, 0x1008, 0, &cm -> name, sizeof(cm -> name));
	cm -> name[sizeof(cm -> name)-1] = '\0';
	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Device name = \"%s\"\n", mod, cm -> name);
		} else {
			printf("Module %d: Device name unkonwn\n", mod);
		}
	}

	cm -> model = 0;

	if (cm -> profile ==  PROFILE_CNIINFORMATICA && strcmp(cm -> name, "k50") != 0) {
		cm -> model = CNIINF_UNKNOWN;
	} else {
		cm -> model = CNIINF_K50;
	}

	memset(cm -> hwver, 0, sizeof(cm -> hwver));
	rv = canboard_read_dictionary_m(cm, 0x1009, 0, &cm -> hwver, sizeof(cm -> hwver));
	cm -> hwver[sizeof(cm -> hwver)-1] = '\0';
	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Hardware version = \"%s\"\n", mod, cm -> hwver);
		} else {
			printf("Module %d: Hardware version unkonwn\n", mod);
		}
	}

	memset(cm -> swver, 0, sizeof(cm -> swver));
	rv = canboard_read_dictionary_m(cm, 0x100A, 0, &cm -> swver, sizeof(cm -> swver));
	cm -> swver[sizeof(cm -> swver)-1] = '\0';
	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Software version = \"%s\"\n", mod, cm -> swver);
		} else {
			printf("Module %d: Software version unkonwn\n", mod);
		}
	}

#if 1
	/* Per curiosita`, leggiamo i parametri di node guarding ed heartbeat. */
	memset(d, 0, sizeof(d));
	rv = canboard_read_dictionary_m(cm, 0x100C, 0, &d, 2);
	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Guard Time = %d\n", mod, d[0]+0x100*d[1]);
		} else {
			printf("Module %d: Guard Time not supported\n", mod);
		}
	}

	memset(d, 0, sizeof(d));
	rv = canboard_read_dictionary_m(cm, 0x100D, 0, &d, 1);
	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Lifetime Factor = %d\n", mod, d[0]);
		} else {
			printf("Module %d: Lifetime Factor not supported\n", mod);
		}
	}

	memset(d, 0, sizeof(d));
	rv = canboard_read_dictionary_m(cm, 0x1016, 0, &d, 1);
	if (rv == 0) {
	int i,n;
		if (cbr -> debug & 1) {
			printf("Module %d: Consumer Heatbeat Time:%d producers\n", mod, d[0]);
		
		}
		n = d[0];
		for (i = 1; i < n; ++i) {
			memset(d, 0, sizeof(d));
			rv = canboard_read_dictionary_m(cm, 0x1016, i, &d, 4);
			printf("Module %d: Consumer Heatbeat Time %d: ID=%d time=%d\n", mod, i, d[2], d[0]+0x100*d[1]);
		}
	}
	else {
		if (cbr -> debug & 1) {
			printf("Module %d: Consumer Heatbeat Time"
			       " not supported\n", mod);
		
		}
	}

	memset(d, 0, sizeof(d));
	rv = canboard_read_dictionary_m(cm, 0x1017, 0, &d, 2);
	if (cbr -> debug & 1) {
		if (rv == 0) {
			printf("Module %d: Producer Heartbeat Time = %d\n", mod, d[0]+0x100*d[1]);
		} else {
			printf("Module %d: Producer Heartbeat Time not supported\n", mod);
		}
	}
#endif
	/*
	* Leggiamo le voci 0x6000:0 (numero di byte per ingressi digitali)
	* e 0x6200:0 (numero di byte per uscite digitali), come da DS401.
	* Non controlliamo invece la voce 0x1000:0, come dovremmo,
	* perche` si da` il caso che possiamo avere a che fare con
	* moduli (CNi-Informatica) che non riportano il valore
	* atteso, che sarebbe 401.
	* Se entrambe le letture falliscono o danno 0 come risultato,
	* deduciamo che il modulo non e` un dispositivo di I/O digitale,
	* oppure non esiste o non funziona. In ogni caso lo marchiamo
	* come "inesistente". 
	*/
	/*
	* In caso di moduloo k50, dobbiamo dedurre la configurazione
	* da alcuni registri custom, le voci di dizionario standard mentono
	* spudoratamente. 
	*/

	n = 0;
	nc = 0;
	rv = canboard_read_dictionary_m(cm, 0x6000, 0, &nc, sizeof(nc));
	n = nc;
	if (rv != 0) {
		cm -> n_ib = 0;
	} else {
		if (n > 32) {
			n = 32;
		}
		cm -> n_ib = n;
	}

	if (cm -> profile ==  PROFILE_CNIINFORMATICA && cm -> model == CNIINF_K50) {
		cm -> n_ib = 20;
	}

	cm -> n_ibit = cm -> n_ib * 8;

	if (cm -> profile ==  PROFILE_CNIINFORMATICA && cm -> model == CNIINF_K50) {
		unsigned char nslots = 0;
		int i;

		rv = canboard_read_dictionary_m(cm, 0x2000, 0, &nslots, sizeof(nslots));
		if (cbr -> debug & 1) {
			printf("Module %d: nslots=%d\n",mod,nslots);
		}
		if (rv == 0) {
			for (i = 0; i < nslots; ++i) {
				unsigned char type;
				unsigned char subt;
				unsigned char name[32];

				type = 0;
				subt = 0;
				memset(name,0,sizeof(name));
				rv = canboard_read_dictionary_m(cm, 0x2010+i, 1, &type, sizeof(type));
				rv = canboard_read_dictionary_m(cm, 0x2010+i, 2, &subt, sizeof(subt));
				rv = canboard_read_dictionary_m(cm, 0x2010+i, 3, &name, sizeof(name));
				name[sizeof(name)-1] = '\0';
				if (cbr -> debug & 1) {
					printf("Module %d: slot %d: type=%d subtype=%d name=\"%s\"\n", mod,i,type,subt,name);
				}
			}
		}
	}

	nc = 0;
	rv = canboard_read_dictionary_m(cm, 0x6200, 0, &nc, sizeof(nc));
	n = nc;
	if (rv != 0) {
		cm -> n_ob = 0;
	} else {
		if (n > 32) {
			n = 32;
		}
		cm -> n_ob = n;
	}

	if (cm -> profile ==  PROFILE_CNIINFORMATICA && cm -> model == CNIINF_K50) {
		cm -> n_ob = 20;
	}

	cm -> n_obit = cm -> n_ob * 8;


	nc = 0;
	rv = canboard_read_dictionary_m(cm, 0x6401, 0, &nc, sizeof(nc));
	n = nc;
	if (rv != 0) {
		cm -> n_ain = 0;
	} else {
		if (n > 8) {
			n = 8;
		}
		cm -> n_ain = n;
	}


	cm -> aout_idx = 0x6411;
	nc = 0;
	rv = canboard_read_dictionary_m(cm, cm -> aout_idx, 0, &nc, sizeof(nc));
	n = nc;
	if (cm -> profile == PROFILE_CNIINFORMATICA) {
		/* 
		* Dovrebbe trattarsi di un modulo CNI Informatica
		* ("CN104" o simile). Ha bisogno di una programmazione tutta sua. 
		*/
		canboard_setup_cn104(cbr, mod);
 		/* Il valore d'uscita va in uno strano posto nella parte custom. */
		cm -> aout_idx = 0x3C00;
	} else {
		if (n > 8) {
			n = 8;
		}
		cm -> n_aout = n;
	}

	if (cbr -> debug & 1) {
		printf("Module %d: n_in=%d, n_out=%d, n_ain=%d, n_aout=%d\n", mod, cm -> n_ibit, cm -> n_obit, cm -> n_ain, cm -> n_aout);
	}

	if (cm -> n_ib == 0 && cm -> n_ob == 0 && cm -> n_ain == 0 && cm -> n_aout == 0) {
		/* E` andata male. */
		cm -> state = CANMOD_STATE_NOTPRESENT;
	} else {
		/* Il modulo esiste! */
		canop_startup_mod(cm);
		cm -> state = CANMOD_STATE_PRESENT;
	}
	return 1;
}

static int canboard_check_module(canboard_t *cbr, int mod)
{
	canmodule_t *cm;

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
			canboard_setup_module(cbr, mod);
		break;
		default:
		break;
	}

	return (cm -> state == CANMOD_STATE_PRESENT);
}

int canboard_get_n_input(canboard_t *cbr, int mod)
{
	if (! canboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_ib * 8;
}

int canboard_get_n_output(canboard_t *cbr, int mod)
{
	if (! canboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_ob * 8;
}

int canboard_get_n_ain(canboard_t *cbr, int mod)
{
	if (! canboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_ain;
}

int canboard_get_n_aout(canboard_t *cbr, int mod)
{
	if (! canboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_aout;
}

int canboard_get_n_enc(canboard_t *cbr, int mod)
{
	if (! canboard_check_module(cbr, mod)) {
		return -1;
	}

	return cbr -> module[mod].n_enc;
}

/* Lettura (lenta) dello stato corrente degli ingressi. */
int canboard_read_input(canboard_t *cbr, int mod)
{
	canmodule_t *cm;
	int rv, rrv;
	int i;
	unsigned char data[4];

	cm = &(cbr -> module[mod]);
	rrv = 0;
	for (i = 0; i < cm -> n_ib; ++i) {
		rv = canboard_read_dictionary_m(cm, 0x6000, i+1, &data[0], 1);
		if (rv == 0) {
			cm -> in[i] = data[0];
		}
		else {
			rrv = -1;
		}
	}
#if 0
	{
		int v = 0;
	for (;;) {
		++v;
	for (i = 0; i < 4; ++i) {
		data[0] = (unsigned char)v;
		rv = canboard_write_dictionary_m(cm,
					      0x6200, 16+i+1,
					      &data[0], 1);
		if (rv == 0) {
		}
		else {
			printf("Male!\n");
		}
	}
	sleep(1);
	}
	}
#endif
#if 0
	for (;;) {
		printf("------------\n");
	for (i = 0; i < 20; ++i) {
		rv = canboard_read_dictionary_m(cm,
					      0x6000, i+1,
					      &data[0], 1);
		if (rv == 0) {
			printf("IN %d = 0x%02x\n",i,data[0]);
		}
		else {
		}
	}
	sleep(1);
	}
#endif
//	for (;;) {
//		printf("------------\n");
	for (i = 0; i < cm -> n_ain; ++i) {
		rv = canboard_read_dictionary_m(cm, 0x6401, i+1, &data[0], 2);
		if (rv == 0) {
			cm -> ain[i] = data[0] + 0x100*data[1];
//			printf("AIN %d = %d\n",i,cm->ain[i]);
		} else {
			rrv = -1;
		}
	}
//	sleep(1);
//	}
//	for (;;) {
//		printf("------------\n");
	for (i = 0; i < cm -> n_enc; ++i) {
		rv = canboard_read_dictionary_m(cm, 0x6864 + i*0x800, 0, &data[0], 4);
		if (rv == 0) {
			cm -> enc[i] = data[0]
			             + 0x100*data[1]
			             + 0x10000*data[2]
			             + 0x1000000*data[3];
//			printf("ENC %d = %d\n",i,cm->enc[i]);
		} else {
			rrv = -1;
		}
	}
//	sleep(1);
//	}
	return rrv;
}

/*
* Attivazione (cioe` entrata in modo "operational") ed eventuale
* interrogazione di un modulo. 
*/
int canboard_activate_module(canboard_t *cbr, int mod)
{
canmsg_t msg;

	/*
	* Controlliamo la presenza del modulo (e anche la correttezza dell'ID).
	*/
	if (! canboard_check_module(cbr, mod)) {
		return 0;
	}

#if 1
	/* Leggiamo lo stato iniziale degli ingressi. */
	canboard_read_input(cbr, mod);
#endif

	/* Portiamo il modulo nello stato "operational". */
	msg.id = CANOPEN_COBID_NMT;
	msg.cob = 0;
	msg.length = 2;
	msg.flags = 0;
	msg.data[0] = 0x01; /* START REMOTE NODE */
	msg.data[1] = mod;
	canboard_send(cbr, &msg);

	return 1;
}

/* Entrata in PREOPERATIONAL di un modulo (anche inesistente). */
int canboard_preop_module(canboard_t *cbr, int mod)
{
	canmsg_t msg;

	/* Portiamo il modulo nello stato "boot". */
	msg.id = CANOPEN_COBID_NMT;
	msg.cob = 0;
	msg.length = 2;
	msg.flags = 0;
	msg.data[0] = 0x80; /* PREOP REMOTE NODE */
	msg.data[1] = mod;
	canboard_send(cbr, &msg);

	return 1;
}

/* Reset di un modulo (anche inesistente). */
int canboard_reset_module(canboard_t *cbr, int mod)
{
	canmsg_t msg;

	/* Portiamo il modulo nello stato "boot". */
	msg.id = CANOPEN_COBID_NMT;
	msg.cob = 0;
	msg.length = 2;
	msg.flags = 0;
	msg.data[0] = 0x81; /* RESET REMOTE NODE */
	msg.data[1] = mod;
	canboard_send(cbr, &msg);

	return 1;
}

int canboard_set_option(canboard_t *cbr, int option, int value)
{
	int rv = 0;

	switch (option) {
		case CANBOARD_OPTION_NOREMAP_CNIINF:
			cbr -> noremap_pdo_cniinf = (value != 0);
			rv = 1;
		break;
		default:
			rv = 0;
		break;
	}
	return rv;
}

#define CANBOARD_OPTION_NOREMAP_CNIINF 1
#ifdef TEST
int main(int argc, char **argv)
{
	canboard_t *cbr;
	int board = 0;
	int speed = 1000;
	int period = 500;
	int debug = 1;
	int module_in = 1;
	int module_out = 1;
	int n_in = 8;
	int n_ain = 7;
	int n_aout = 5;
	int n_out = 8;
	int rv;
	int out[128];
	int in[128];
	int tmp;
	int i;

	for (i = 1; i < argc; ++i) {
		if ((strcmp(argv[i],"-d") == 0) && argv[i+1]) {
			debug = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-s") == 0) && argv[i+1]) {
			speed = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-b") == 0) && argv[i+1]) {
			board = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-mi") == 0) && argv[i+1]) {
			module_in = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-mo") == 0) && argv[i+1]) {
			module_out = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-ni") == 0) && argv[i+1]) {
			n_in = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-no") == 0) && argv[i+1]) {
			n_out = atoi(argv[++i]);
		} else if ((strcmp(argv[i],"-p") == 0) && argv[i+1]) {
			period = atoi(argv[++i]);
		}
	}

	cbr = canboard_open(board,speed,debug);
	if (! cbr) {
		perror("canboard_open");
		return 1;
	}

	rv = canboard_activate_module(cbr, module_in);
	if (! rv) {
		fprintf(stderr,"Module %d not found.\n",module_in);
		return 1;
	}
	if (module_in != module_out) {
		rv = canboard_activate_module(cbr, module_out);
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
			tmp = canboard_get_input_bit(cbr, module_in, i);
			if (tmp != in[i]) {
				in[i] = tmp;
				printf("Input %d = %d\n",i,tmp);
			}
		}

		printf("AInput =");
		for (i = 0; i < n_ain; ++i) {
			printf(" %05u",
			 (canboard_get_ain_image(cbr, module_in))[i]);
		}
		printf("\n");
		(canboard_get_aout_image(cbr, module_in))[0] += 64;
		(canboard_get_aout_image(cbr, module_in))[1] -= 64;
		(canboard_get_aout_image(cbr, module_in))[2] += 128;
		(canboard_get_aout_image(cbr, module_in))[3] -= 128;
		(canboard_get_aout_image(cbr, module_in))[4] += 16;
		tmp = out[0];
		for (i = 1; i < n_out; ++i) {
			out[i-1] = out[i];
		}
		out[n_out-1] = tmp;

		for (i = 0; i < n_out; ++i) {
			canboard_set_output_bit(cbr, module_out, i, out[i]);
		}

		canboard_flush_output(cbr);

		usleep(period * 1000);

	}

	return 0;
}
#endif

