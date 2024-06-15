/*
* @(#) dr_ax.c 2.1 Fri Apr  3 19:48:00 MET DST 1998
*
* Driver della scheda CNI CPAX per QPLC.
* Il dispositivo "cpax" permette di controllare DAC o ADC posti sul
* bus CABX controllato da una CPAX o da un programma che la emuli.
* Per le CPAX reali e` eseguito anche il controllo periodico per
* identificare lo stato di fault.
* Per ogni "linea" (la "linea" e` un'estensione del concetto di "scheda CPAX",
* le linee sono definite in apposite tabelle dei dati macchina) e` possibile
* avere fino a quattro collegamenti di input o di output con variabili a 16
* bit. Gli input controllano DAC, gli output ADC. La stringa che descrive
* un segnale di input o di output ha la forma seguente :
*
*   ax.<linea>.<posto CABX>.<tipo scheda>.<porta>
*
* dove
*
*    linea : vedere l'apposita tabella dei dati macchina ("protee"),
*    posto CABX : un numero da 0 a 15, indicante il dispositivo
*            collegato al bus CABX coinvolto,
*    tipo scheda : "1" o "rcx" per RCX,
*                  "2" o "adx" per ADX,
*                  "3" o "mpp" per MPP3,
*    porta : 1 o 2 : primo o secondo canale di input o di output.
*
* Esempio
*
*    ax.0.12.rcx.2 : secondo input od output della scheda RCX
*                    nel posto 12 della linea 0.
*
* --------------------------------------------------------------------
* Organizzazione dei dati descriventi i segnali:
*
*  ideep = 2 : spec[0] : linea
*              spec[1] : posto CABX
*              spec[2] : tipo scheda
*              spec[3] : porta
*
* --------------------------------------------------------------------
* 
* --/--/-- 1.0 Prima stesura.
* 26/06/96 1.1 Corretto buco clamoroso : la "read_parm" riempie un
*          array di 16 elementi ! (e mi risolve automaticamente il problema
*          delle CPAX multiple). Chissa` che cosa succedeva nella versione
*          precedente...
* 28/08/96 GG 1.2 Modifiche per parametro aggiuntivo di "parse".
* 04/09/96 GG 1.3 Completato il driver, per dare accesso ai canali della
*             scheda CPAX. Il nome ufficiale del driver e` ora "ax".
* 16/09/96 GG 1.4 Corretti problemi nei metodi "parse" (valore di ritorno
*             errato) e "attach" (omesso l'indice di linea).
* 19/09/96 GG 1.5 Aggiunte adeguate segnalazioni di errore in caso di
*             timeout.
* 05/12/96 GG 1.6 Il tempo di risposta massimo prima del fault e` stato reso
*             indipendente dal periodo del Plc.
* 26/02/97 GG 1.7 Il driver si installa anche se non ci sono dispositivi
*             che ammettano watchdog.
* 15/04/97 GG 1.8 Aggiunta la gestione della variabile di configurazione
*             "wd=numero", che imposta il periodo d'intervento del watchdog.
* 13/05/97 GG 1.9 Cambiato nome al file.
* 23/05/97 GG 1.10 Micro-modifica per gestione della ripetizione della
*             fase di halt. Non era indispensabile, ma non fa male.
* 16/07/97 GG 1.11 Aggiunti i metodi "up", "close" ed "extension".
* 11/03/98 GG 1.12 Aggiunto il metodo "detach".
* 24/03/98 GG 1.13 Aggiunto il metodo "list".
*             Portato ad 8 il numero massimo di canali gestito.
* 01/04/98 GG 2.0 Rivoluzione dovuta a modifiche profonde in "drivers.h".
*             E, gia` che c'ero, ho anche aggiunto il supporto per l'uscita
*             analogica su scheda MPP3.
* 03/04/98 GG 2.1 Aggiunto il metodo "show" (vuoto).
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <cpax.h>
#include <hwcni.h>

#include "qplc.h"
#include "util.h"

#define DEFAULT_AX_TIMEOUT 120 /* Millisecondi */

#if MAX_LINEE > MAX_CPAX_BOARD
#define MAX_CPAX MAX_LINEE
#else
#define MAX_CPAX MAX_CPAX_BOARD
#endif

#ifndef CHAN_PER_CPAX
#define CHAN_PER_CPAX 8
#endif

#ifdef TEST_EMERGENZA
unsigned char *test_prima_cpax = NULL;
static unsigned char test_dummy = 0;
#endif

#define c_mode flags
#define c_addr next.spec[0]
#define c_line next.spec[1]
#define c_type next.spec[2]
#define c_chan next.spec[3]

typedef struct {
	int n_cpax;
	unsigned char * wdin[MAX_CPAX];
	int curr_count[MAX_CPAX];
	int max_count[MAX_CPAX];
	int n_target_in;
	unsigned short *target_in[MAX_CPAX * CHAN_PER_CPAX];
	unsigned short *var_in[MAX_CPAX * CHAN_PER_CPAX];
	int n_target_out;
	unsigned short *target_out[MAX_CPAX * CHAN_PER_CPAX];
	unsigned short *var_out[MAX_CPAX * CHAN_PER_CPAX];
	signed char rev[MAX_CPAX];
} dr_cpax_t;

static dr_cpax_t cpax_sts;

/* Funzione di inizializzazione della lista di I/O. */

static void _cpax_initiolist(void)
{
	cpax_sts.n_target_in = 0;
	memset((char *)cpax_sts.target_in, 0, sizeof(cpax_sts.target_in));
	cpax_sts.n_target_out = 0;
	memset((char *)cpax_sts.target_out, 0, sizeof(cpax_sts.target_out));
}

static int cpax_install(driver_t * d, int conf)
{
int n = d -> instance;
int i,j;
int wd_timeout_val;
char *p;

	if (n >= 1) {
	/* ERRORE. Troppe istanze del driver. Piu` di una non ha senso. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	wd_timeout_val = -1;
	for (ioStartResource(d); (p = ioNextResource(d)); ) {
		if (util_strneq(p,"wd=",3)) {
			p += 3;
			wd_timeout_val = ioGetNum(&p);
		}
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}
	if (wd_timeout_val == -1)
		wd_timeout_val = DEFAULT_AX_TIMEOUT;
	
	if (! *main_status.mod_hatsh_op.cpax) {
	/* ERRORE. A quanto pare, per qualche motivo, la CPAX non e`
	 stata inizializzata. */
		return 0;
	}

	d -> device = (void *) &cpax_sts;
	read_parm(*main_status.mod_hatsh_op.cpax,
	          SAX_WATCH_DOG,0,0,0,(int *)(cpax_sts.wdin));

/* Compatta l'array delle CPAX attive. */

	for (i = j = 0; i < MAX_CPAX; ++i) {
		if (cpax_sts.wdin[i]) {
			cpax_sts.wdin[j] = cpax_sts.wdin[i];
			cpax_sts.curr_count[j] = 0;
			cpax_sts.max_count[j] = wd_timeout_val / plcPeriod();
			cpax_sts.rev[i] = j;
			++j;
		}
		else {
			cpax_sts.rev[i] = -1;
		}
	}

	cpax_sts.n_cpax = j;

#ifdef TEST_EMERGENZA
	if (j)
		test_prima_cpax = cpax_sts.wdin[0];
	else {
		fprintf(stderr,"PLC: No CPAX/INAX\n");
		test_prima_cpax = &test_dummy;
	}
#endif

/* Inizializza la lista di associazione degli I/O. */

	_cpax_initiolist();

	return 1;
}

static void cpax_restart(driver_t * d)
{
int i;

/* Rilascia i canali precedentemente allocati. */

	for (i = 0; i < cpax_sts.n_target_in; ++i)
		write_parm(*main_status.mod_hatsh_op.cpax,
		           SAX_RELEASE_ANALOGICA, -1, -1, -1,
		           (void *)(&cpax_sts.target_in[i]));
	for (i = 0; i < cpax_sts.n_target_out; ++i)
		write_parm(*main_status.mod_hatsh_op.cpax,
		           SAX_RELEASE_ANALOGICA, -1, -1, -1,
		           (void *)(&cpax_sts.target_out[i]));

/* Inizializza la lista di associazione degli I/O. */

	_cpax_initiolist();
}

static int cpax_parse(driver_t * d, int i_off, devnode_t *l)
{
int linea;
int cabxaddr;
int tipo;
int canale;
char *name;
int mode;

/* "Pesca" nome e modo dalla struttura. */

	mode = l -> flags;
	name = l -> pname;

/* Il driver CPAX non e` istanziabile. Il numero di istanza e`
 interpretato come "linea". */

	linea = i_off;

/* Controlla l'indice del posto scheda nel bus CABX. */

	cabxaddr = ioGetNum(&name);
	if (cabxaddr < 0 || cabxaddr >= 16) {
	/* ERRORE. Manca l'indicazione del posto scheda, oppure la stringa
	 non e` conforme. */
		return 0;
	}

/* Cerca il tipo di scheda. */

	if (util_strneq(name,"rcx",3)) {
		tipo = 1;
		name += 4;
	}
	else if (util_strneq(name,"adx",3)) {
		tipo = 2;
		name += 4;
	}
	else if (util_strneq(name,"mpp",3)) {
		tipo = 3;
		name += 4;
	}
	else {
		tipo = ioGetNum(&name);
		if (tipo < 1 || tipo > 3) {
		/* ERRORE. Tipo sconosciuto. */
			return 0;
		}
	}

/* Controlla l'indice di canale sulla scheda. */

	canale = ioGetNum(&name);
	if (canale != 1 && canale != 2) {
	/* ERRORE. Canale sconosciuto. */
		return 0;
	}

	if (((mode & DRIVER_MODE_INPUT) && (mode & DRIVER_MODE_OUTPUT))
	    || (mode & (DRIVER_MODE_INPUT | DRIVER_MODE_OUTPUT)) == 0) {
	/* ERRORE. Non esiste niente del genere in CNi. */
		return 0;
	}

	if (mode & DRIVER_MODE_INPUT) {
		canale |= 0x80;
	}

/* Se richiesto, registra i dati nel "posto giusto". */

	l -> c_addr = cabxaddr;
	l -> c_line = linea;
	l -> c_type = tipo;
	l -> c_chan = canale;
	l -> pname = name;
	l -> ideep = 2;

	return sizeof(unsigned short)*8;
}

static int cpax_attach(driver_t * d, devnode_t *l, void *var)
{
dr_cpax_t *dev = (dr_cpax_t *) d -> device;
int n,lim;
unsigned short *t;

/* Controlla se c'e` posto nelle tabelle. */

	if (l -> c_mode & DRIVER_MODE_INPUT) {
		n = dev -> n_target_in;
		lim = sizeof(dev -> target_in)/sizeof(dev -> target_in[0]);
	}
	else {
		n = dev -> n_target_out;
		lim = sizeof(dev -> target_out)/sizeof(dev -> target_out[0]);
	}

	if (n >= lim) {
	/* ERRORE. Lista associazioni piena. */
		return 0;
	}

/* Richiede un canale al gestore AX. */

	t = (unsigned short *)
	     (((l->c_type) << 8) | ((l->c_addr) << 12) | l->c_chan);
	if (write_parm(*main_status.mod_hatsh_op.cpax,
	               SAX_INIT_ANALOGICA, -1, l -> c_line, -1, (void *)&t)
	    != SAX_TUTTO_OK) {
	/* ERRORE. Qualcosa e` andato storto. */
		return 0;
	}

/* Registra l'associazione nella tabella apposita. */

	if (l -> c_mode & DRIVER_MODE_INPUT) {
		dev -> target_in[n] = t;
		dev -> var_in[n] = (unsigned short *) var;
		++(dev -> n_target_in);
	}
	else {
		dev -> target_out[n] = t;
		dev -> var_out[n] = (unsigned short *) var;
		++(dev -> n_target_out);
	}

	return 1;
}

static int cpax_check(driver_t * d)
{
register dr_cpax_t *dev = (dr_cpax_t *) d -> device;
register int i;
int rv = 1;

#ifdef TEST_EMERGENZA
	return 1;
#else
	for (i = 0; i < dev -> n_cpax; ++i) {
		if (*(dev -> wdin[i])) {
			if (++(dev -> curr_count[i]) >= dev -> max_count[i]) {
			/* ERRORE. Timeout nella CPAX. */
				ioAddError(d,0, - dev -> rev[i]);
				rv = 0;
			}
		}
		else {
			dev -> curr_count[i] = 0;
		}

		*(dev -> wdin[i]) = 0x55;
	}

	return rv;
#endif
}

static void cpax_down(driver_t * d)
{
dr_cpax_t *dev = (dr_cpax_t *) d -> device;
int i;

/* Esce se la funzione e` gia` stata chiamata. */

	if (plcHaltCycle())
		return;

/* Azzera i DAC. Ma sara` giusto ? */

	for (i = 0; i < dev -> n_target_out; ++i)
		*(dev -> target_out[i]) = 0;
}

static void cpax_fatal(driver_t * d)
{
/* Idem. */
	cpax_down(d);
}

static void cpax_read(driver_t * d)
{
register unsigned short **p = ((dr_cpax_t *) d -> device) -> target_in;
register unsigned short **q = ((dr_cpax_t *) d -> device) -> var_in;
register int n;

	for (n = ((dr_cpax_t *) d -> device) -> n_target_in; n--; )
		**(q++) = **(p++);
}

static void cpax_write(driver_t * d)
{
register unsigned short **p = ((dr_cpax_t *) d -> device) -> target_out;
register unsigned short **q = ((dr_cpax_t *) d -> device) -> var_out;
register int n;

	for (n = ((dr_cpax_t *) d -> device) -> n_target_out; n--; )
		**(p++) = **(q++);
}

#if 0
static int cpax_list(driver_t * d, devnode_t * l)
{
	if (l -> ideep != 2)
		return -1;
	util_safe_sprintf(l -> name, sizeof(l -> name),
	                  "%s.%d", d -> op.name, d -> instance);
	l -> flags = l -> spec[0] ? DRIVER_MODE_INPUT : DRIVER_MODE_OUTPUT;
	l -> nbit = 16;
	++(l -> next.spec[0]);
	return 0;
}
#endif

driver_op_t driver_cpax = {
	"ax",
	cpax_install,
	cpax_restart,
	cpax_parse,
	(int (*)(driver_t *, devnode_t *)) 0, /* cpax_list */
	cpax_attach,
	cpax_restart, /* cpax_detach */
	cpax_check,
	(void (*)(driver_t *)) 0, /* cpax_trigger */
	(void (*)(driver_t *)) 0, /* cpax_up */
	cpax_down,
	(void (*)(driver_t *)) 0, /* cpax_close */
	cpax_fatal,
	cpax_read,
	cpax_write,
	(int (*)(driver_t *,devnode_t *,void *)) 0, /* cpax_show */
	(int (*)(driver_t *,void *,int,void *)) 0, /* cpax_extension */
};

