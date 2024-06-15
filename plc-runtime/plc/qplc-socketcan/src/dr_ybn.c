/*
* @(#) dr_ybn.c 1.13 Tue Mar 12 12:54:08 MET 2002
*
* Driver della scheda CNI IOS per QPLC.
*
* Forma dei parametri specifici :
*
* Ingressi e uscite digitali:
*        1 bit : <canale>.<modulo>.<p260|p270|p810>.<bit>
* Pseudo-ingressi handle di moduli display:
*       32 bit : <canale>.<modulo>.p810
* Ingressi encoder:
*       32 bit : <canale>.<modulo>.p131.<numero_ingresso>
*       32 bit : <canale>.<modulo>.p137.<numero_ingresso>
* Pseudo-ingressi handle di moduli motore:
*       32 bit : <canale>.<modulo>.p137
* Uscite analogiche:
*       16 bit : <canale>.<modulo>.p250.<numero_uscita>
*
* Esempio di nome di variabile completo (1 bit):
*
*        ybn.0.0.2.p260.5
*         |  | | |   | |
*         |  | | |   | bit 5...
*         |  | | |   di un modulo P260...
*         |  | | di indirizzo 2...
*         |  | collegato al canale 0...
*         |  della scheda 0...
*         di tipo Fieldbus YBN.
*
* 14/01/02 GG 1.0 Prima stesura.
* 17/01/02 GG 1.1 Corretto l'accesso ai moduli P810: era sbagliato il
*             codice. Usata apposita macro.
*             Restano da far funzionare i metodi "list" e "show".
* 18/01/02 GG 1.2 Rappezzata l'emissione degli errori.
*             Rappezzato pure un bug della libreria YBN, a causa del quale
*             si ha un crash quando si usano gli output dei moduli
*             P810 senza agganciare anche l'input.
* 22/01/02 GG 1.3 Raffinata ancora la logica di lancio di "protocol",
*             che era invocata con un'informazione errata di "esecutore
*             in HALT".
* 24/01/02 GG 1.4 Eliminati alcuni warning aggiungendo la proprieta`
*             "const" a vari puntatori.
*             Corretta la visualizzazione dell'errore. Erano scambiati
*             l'indirizzo e il canale.
* 25/01/02 GG 1.5 Aggiunta la possibilita` di ottenere l'indice di
*             un modulo display come ingresso a 32 bit omettendo
*             l'indice di bit nella stringa.
* 25/01/02 GG 1.6 Resa utilizzabile la primitiva "list".
*             Si e` anche fatto in modo che, se "maxerr" vale 0,
*             il controllo sugli errori sia disattivato.
*             Migliorata la logica di azzeramento degli errori, per
*             permettere al debugger di mostrare qualcosa di utile
*             almeno in HALT.
*             Reso minimamente utilizzabile il metodo "show".
*             Aggiunto un semaforo per gli accessi in concorrenza
*             alla libreria YBN da parte dei thread non prioritari.
* 28/01/02 GG 1.7 Automatizzata l'assunzione dell'opzione "-do-trigger"
*             se c'e` almeno un fieldbus funzionante.
*             Aggiunta la visualizzazione di una forma ridotta delle
*             versioni YBN come commento nella lista dei dispositivi.
* 28/01/02 GG 1.8 Aggiunto un reset totale degli errori ad ogni ciclo
*             per evitare "falsi positivi".
* 28/01/02 GG 1.9 Eliminate le complicazioni dovute agli errori fasulli
*             riportati dalla libreria: si deve usare la versione 1.17
*             (ybn 1.5). Ora si ha halt irreversibile con un solo errore.
* 01/02/02 GG 1.10 Rivoluzione per eliminare completamente il problema
*             della visibilita` dei moduli con PLC in GO. Ora si alloca
*             subito tutto cio` che puo` essere allocato, e non si conta piu`
*             sull'effetto della "Mount_default", che infatti verra` rimossa
*             libreria YBN.
*             "Silenziata" l'emissione di errore della libreria HSD in fase
*             di inizializzazione.
*             Eliminati i moduli fantasma (P810 con indirizzo > 15).
*             Usato un metodo alternativo per l'acquisizione dello stato
*             di un segnale.
* 05/02/02 GG 1.11 Evitato il reset degli errori se max_err_count e` minore
*             di 2, cosi` plc2 puo` far vedere il modulo responsabile
*             dell'HALT irreversibile.
* 07/02/02 GG 1.12 Aggiunta una disattivazione delle eventuali
*             forzature a basso livello dei segnali a bit al termine
*             di "ybn_attach".
* 12/03/02 GG 1.13 Agguinto il supporto per i motori, gli encoder e le
*             uscite analogiche.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ybnlib.h>
#include <hsdlib.h>

#include "qplc.h"

#include "util.h"

#define LEN_HSD_ALLOC_BLK 16

#define AZZERA_NON_USATI 1
#define MAX_HSD_FB 4

/*
* Definizioni legate alla gerarchia dei dispositivi controllati dal driver.
*/

/* Livello 2: tipo di controllore. */
#define c_chan next.spec[0]
#define t_c_chan tree.spec[0]
#define LEVEL_2_CHAN 0 /* Base per i canali: da 0 a HSD_N_CHAN escluso. */
	#define c_addr next.spec[1]
	#define c_mod next.spec[1] /* alias */
	#define t_c_addr tree.spec[1]
	#define t_c_mod tree.spec[1]
	/* Livello 3: Indice modulo (da 0 a HSD_N_ADDR escluso). */
		#define c_type next.spec[2]
		#define t_c_type tree.spec[2]
		#define c_sts next.spec[3]
		#define t_c_sts tree.spec[3]
		/* Livello 4: tipo di modulo. */
		#define LEVEL_4_P260 0
		#define LEVEL_4_P810 1
			/* Livello 5: bit nel byte. */
			#define LEVEL_5_INBIT 0
			#define LEVEL_5_OUTBIT 16
			#define LEVEL_5_DIG_MAX 32
			#define c_bit next.spec[4]
			#define t_c_bit tree.spec[4]
			#define c_mode next.spec[5]
			#define t_c_mode tree.spec[5]
		#define LEVEL_4_P250 2
			/* Livello 5: uscita analogica (0-3). */
			#define LEVEL_5_AOUT 0
			#define LEVEL_5_AOUT_MAX 4
			#define c_aout next.spec[4]
			#define t_c_aout tree.spec[4]
		#define LEVEL_4_P131 3 /* Ingressi encoder. */
		#define LEVEL_4_P137 4 /* Ingressi encoder+motore. */
			/* Livello 5: ingresso encoder del motore. */
			#define LEVEL_5_ENC 0
			#define LEVEL_5_ENC_MAX 1
			#define c_encin next.spec[4]
			#define t_c_encin tree.spec[4]
		#define LEVEL_4_MAX 5
#define LEVEL_2_QUIT (HSD_N_CHAN)

typedef struct _ybn_bit_t {
	unsigned char chan; /* Solita roba... */
	unsigned char addr;
	unsigned char bit;
	unsigned char * hw_var;/* indirizzo della variabile allocata dalla
	                        libreria YBN. */
	unsigned char * var; /* indirizzo della variabile PLC associata. */
	struct _ybn_bit_t * next; /* Puntatore al prossimo elemento. */
} ybn_bit_t;

typedef struct _ybn_short_t {
	unsigned char chan; /* Solita roba... */
	unsigned char addr;
	unsigned char io;
	short * hw_var;/* indirizzo della variabile allocata dalla
	                        libreria YBN. */
	short * var; /* indirizzo della variabile PLC associata. */
	struct _ybn_short_t * next; /* Puntatore al prossimo elemento. */
} ybn_short_t;

typedef struct _ybn_long_t {
	unsigned char chan; /* Solita roba... */
	unsigned char addr;
	unsigned char io;
	long * hw_var;/* indirizzo della variabile allocata dalla
	                        libreria YBN. */
	long * var; /* indirizzo della variabile PLC associata. */
	struct _ybn_long_t * next; /* Puntatore al prossimo elemento. */
} ybn_long_t;

/* Descrittore di categorie di modulo (insiemi raggruppati per tipo. */
typedef struct _ybn_modset_t ybn_modset_t;

/* Descrittore di modulo, per rappezzare i buchi della
 libreria YBN. */
typedef struct _ybn_mod_t {
	int chan; /* Solita roba... */
	int addr;
	int index; /* Indice associato al modulo (per categoria). */
/* Insieme di appartenenza. */
	ybn_modset_t *set;
/* Indirizzi delle variabili allocate dalla libreria YBN. */
	unsigned char * hw_in[16];
	unsigned char * hw_out[16];
	struct _ybn_mod_t * next; /* Puntatore al prossimo elemento. */
} ybn_mod_t;

/* Descrittore di categorie di modulo (insiemi raggruppati per tipo. */
struct _ybn_modset_t {
	int n_in;
	int n_out;
	int code;
	unsigned long in_code;
	unsigned long out_code;
	int count;
	ybn_mod_t *list;
};

/* Struttura di stato di un'istanza del driver. */
typedef struct {
/* Flag di attivazione del controllo periodico del dispositivo. */
	int do_chk;
/* Flag di abilitazione della modalita` master per il fieldbus. */
	int do_master;
/* Flag indicante che e` stata eseguita almeno una "restart". */
	int go_done;
/* Descrittore di fieldbus HSD e indice di fieldbus. */
	hsdfb_t *fb;
	int nfb;
/* Flag usati per capire quando invocare la "hsd_protocol". */
	int protocol_in_trigger;
	int protocol_in_write;
/* Flag di stato di halt permanente. */
	int unrecoverable;
/* Contatore di errori e massimo valore ammesso. */
	int err_count;
	int max_err_count;
/* Liste di ingresso e di uscita. */
	ybn_bit_t *bit_in_list;
	ybn_bit_t *bit_out_list;
	ybn_short_t *short_out_list;
	ybn_long_t *long_in_list;
/* Liste dei moduli utilizzati. Servono a rappezzare un buco orrendo
 della libreria YBN, che costringe a richiedere praticamente tutto
 cio` che esiste sul fieldbus, anche se non serve. */
	ybn_modset_t p260_set;
	ybn_modset_t p810_set;
	ybn_modset_t p250_set;
	ybn_modset_t p131_set;
	ybn_modset_t p137_set;
/* Tabella dei tipi di modulo disponibili. */
	unsigned char types[HSD_N_CHAN][HSD_N_ADDR];
} dr_ybn_t;


/* Contenitore delle variabili statiche. */
typedef struct {
/* Flag di "libreria inizializzata". */
	int init_done;
/* Array delle strutture di stato (per istanza, cioe` per scheda). */
	dr_ybn_t board[MAX_HSD_FB];
/* Liste dei nodi liberi. */
	ybn_bit_t *bit_free;
	ybn_mod_t *mod_free;
	ybn_short_t *short_free;
	ybn_long_t *long_free;
/* Semaforo per l'accesso serializzato alla libreria YBN. */
/* NOTA: non sara` usato dall'esecutore, perche` e` prioritario e
 initerrompibile rispetto agli altri thread. La libreria (dalla 1.4)
 e` stata resa rientrante per interruzioni di questo tipo. */
	csem_t lib_sem;
/* Stringhe di versione delle librerie e del server. */
	char hsdfb_version[16];
	char hserver_version[16];
	char overall_hsd_version[48];
/* Pattumiera per input e output finti. */
	union { long l; unsigned char b; } dummy;
} ybn_statics_t;

static ybn_statics_t ybn_st = {
	0,
	{ { 0 } },
	(ybn_bit_t *) NULL,
	(ybn_mod_t *) NULL,
	(ybn_short_t *) NULL,
	(ybn_long_t *) NULL,
	0,
	"", "", "",
	{ 0 }
};

/* Ottiene l'insieme di competenza del modulo. */
static ybn_modset_t *_ybn_find_set(dr_ybn_t *dev, int chan, int addr)
{
	switch (dev -> types[chan][addr]) {
	case HSD_P260:
		return &(dev -> p260_set);
	case HSD_P810:
		return &(dev -> p810_set);
	case HSD_P137:
		return &(dev -> p137_set);
	case HSD_P230:
		return &(dev -> p131_set);
	case HSD_P250:
		return &(dev -> p250_set);
	default:
		break;
	}
	return (ybn_modset_t *) NULL;
}

/* Ricerca di un modulo nell'insieme di competenza. */
static ybn_mod_t *_ybn_find_mod(dr_ybn_t *dev, int chan, int addr)
{
ybn_mod_t *dl;
ybn_modset_t *set;

	set = _ybn_find_set(dev, chan, addr);
	if (! set) {
		return (ybn_mod_t *) NULL;
	}

	for (dl = set -> list; dl; dl = dl -> next) {
		if (dl -> chan == chan && dl -> addr == addr) {
		/* Trovato. */
			break;
		}
	}
	return dl;
}

/* Occupazione di un modulo. */
static ybn_mod_t *_ybn_take_mod(dr_ybn_t *dev, int chan, int addr)
{
unsigned char *target;
ybn_mod_t *dl;
int i;
ybn_modset_t *set;

	set = _ybn_find_set(dev, chan, addr);
	if (! set) {
		return (ybn_mod_t *) NULL;
	}

/* Bisogna agganciare tutto. Tentiamo col primo input (se previsto). */
	if (set -> n_in > 0) {
		target = hsd_request_io(dev -> fb, chan,addr,
					set -> code,0,set -> in_code);
		if (! target) {
		/* ERRORE: la libreria non collabora. */
			return (ybn_mod_t *) NULL;
		}
	}
	else {
		target = &ybn_st.dummy.b;
	}

/* Alloca un nuovo nodo per descrivere il modulo. */
	dl = (ybn_mod_t *) util_alloc_cell(
			  (void **) &ybn_st.mod_free,
			  sizeof(ybn_mod_t),
			  GET_STRUCT_OFFSET(ybn_mod_t,next),
			  LEN_HSD_ALLOC_BLK);
	if (! dl) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"YBN");
		return (ybn_mod_t *) NULL;
	}

/* Registra i dati di aggancio. */
	dl -> chan = chan;
	dl -> addr = addr;
	dl -> hw_in[0] = target;
	dl -> set = set;
	dl -> index = set -> count;

/* Aggancio degli altri input. */
	for (i = 1; i < set -> n_in; ++i) {
		target = hsd_request_io(dev -> fb,chan,addr,
		                        set -> code,i,set -> in_code);
		if (! target) {
		/* ERRORE: la libreria non collabora. */
			return (ybn_mod_t *) NULL;
		}
		dl -> hw_in[i] = target;
	}
	for (i = set -> n_in; i < 16; ++i) {
		dl -> hw_in[i] = &ybn_st.dummy.b;
	}
/* Aggancio degli output. */
	for (i = 0; i < set -> n_out; ++i) {
		target = hsd_request_io(dev -> fb,chan,addr,
		                        set -> code,i,set -> out_code);
		if (! target) {
		/* ERRORE: la libreria non collabora. */
			return (ybn_mod_t *) NULL;
		}
		dl -> hw_out[i] = target;
	}
	for (i = set -> n_out; i < 16; ++i) {
		dl -> hw_out[i] = &ybn_st.dummy.b;
	}

	++(set -> count);

/* Aggiunge la nuova cella alla lista che le compete. */
	util_add_cell((void *) dl,
		   (void **) &(set -> list),
		   GET_STRUCT_OFFSET(ybn_mod_t,next));
	return dl;
}

/* Aggancio di tutti i segnali esistenti. */
static void _ybn_take_all(dr_ybn_t *dev)
{
int chan, addr;

	for (chan = 0; chan < HSD_N_CHAN; ++chan) {
		for (addr = 0; addr < HSD_N_ADDR; ++addr) {
			_ybn_take_mod(dev, chan, addr);
		}
	}
}

static void _ybn_null_hsd_error_cb(char *id,char *msg,int sys_errno,void *arg)
{
/* Non fa assolutamente nulla. Serve a "silenziare" la hsd_init(). */
}

static int ybn_install(driver_t * d, int conf)
{
int n = conf;
dr_ybn_t *p;
const char *q;
int check,maxerr,master,enable;

	if (d -> instance >= MAX_HSD_FB) {
	/* ERRORE. Troppe istanze del driver. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	check = 1;
	maxerr = 1;
	master = 1;
	enable = 1;
	for (ioStartResource(d); (q = ioNextResource(d)); ) {
		if (util_strneq(q,"chk=",4)) {
		/* Flag di abilitazione del test periodico
		 del fieldbus. Default=1. */
			q += 4;
			check = ioGetNum((char **)&q);
		}
		else if (util_strneq(q,"maxerr=",7)) {
		/* Flag di abilitazione del test periodico
		 del fieldbus. Default=1. */
			q += 7;
			maxerr = ioGetNum((char **)&q);
		}
		else if (util_strneq(q,"master=",7)) {
		/* Flag di (dis)abilitazione della modalita` master
		 per il fieldbus. Default=1. */
			q += 7;
			master = ioGetNum((char **)&q);
		}
		else if (util_strneq(q,"enable=",7)) {
		/* Flag di (dis)abilitazione totale del supporto fieldbus
		 (valido per un solo fieldbus o anche per tutti)
		 Default=1. */
			q += 7;
			enable = ioGetNum((char **)&q);
		}
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}

	if (!enable) {
	/* Questo fieldbus (o tutti) e` riservato ad altro uso. */
		return 0;
	}

	p = &ybn_st.board[n];
	p -> do_chk = check;
	p -> do_master = master;
	p -> go_done = 0;
	p -> protocol_in_trigger = 0;
	p -> protocol_in_write = 0;
/* Contatore di errori e massimo valore ammesso. */
	p -> err_count = 0;
	p -> max_err_count = maxerr;
	p -> unrecoverable = 0;

	if (!ybn_st.init_done) {
	const char *sv;
	void (*old_cb)(char *id, char *msg, int sys_errno, void *arg);
	void *old_cb_arg;
	void *dummy;
	int rv;

/* Silenziamento della segnalazione di errore della hsd_init. */
		hsd_register_error_cb(_ybn_null_hsd_error_cb, (void *)NULL,
	                              &old_cb, &old_cb_arg);

		rv = hsd_init();

/* D'ora in poi le segnalazioni di errore saranno significative. */
		hsd_register_error_cb(old_cb, old_cb_arg,
	                              (hsd_error_cb_t *)&dummy,
		                      (void **) &dummy);
		if (rv < 0) {
	/* ERRORE. La libreria HSD non ha intenzione di collaborare.
	 Forse il server non c'e`, o non e` ancora partito,
	 o e` morto... */
			return 0;
		}


		ybn_st.init_done = 1;

	/* Col fieldbus HSD questo e` necessario: bisogna assicurare
	 che il metodo "trigger" sia invocato anche se si e` in debug,
	 altrimenti non funzionano le forzature (e` in "trigger" che
	 ha luogo la chiamata di hsd_protocol nello stato di HALT). */
		main_status.do_trigger = 1;

/* Creazione del semaforo di accesso alla libreria. Serve a sincronizzare
 tra loro i thread non prioritari (di solito supervisore e debugger). */
		ybn_st.lib_sem = csem_create_val(1);

/* Registrazione delle versioni di libreria e server. */
		strncpy(ybn_st.hsdfb_version,hsd_lib_version(),
		        sizeof(ybn_st.hsdfb_version)-1);
		sv = hsd_server_version();
		if (sv) {
			strncpy(ybn_st.hserver_version,sv,
			        sizeof(ybn_st.hserver_version)-1);
		}
	/* Costruzione della stringa di versione compatta. */
		{
		char *s,*t;

			t = ybn_st.overall_hsd_version;
			strcpy(t,"lib="); t += 4;
			for (s=ybn_st.hsdfb_version;
			     *s != '\0' && *s != '-';
			     ++s, ++t) {
				*t = *s;
			}
			if (*s == '-')
				++s;
			strcpy(t,",ybn="); t += 5;
			for ( ;
			     *s != '\0' && *s != '-';
			     ++s, ++t) {
				*t = *s;
			}
			strcpy(t,",sv="); t += 4;
			for (s=ybn_st.hserver_version;
			     *s != '\0' && *s != '-';
			     ++s, ++t) {
				*t = *s;
			}
			*t = '\0';
		}
	}

	p -> fb = hsd_open_bus(n,p -> do_master);
	if (!(p -> fb)) {
	/* ERRORE. Il dispositivo non esiste. */
		return 0;
	}
	p -> nfb = n;

/* Lettura della lista dei moduli disponibili. */
	if (hsd_get_module_types(p -> fb, p -> types) < 0) {
	/* ERRORE. La libreria si e` imbizzarrita. */
		return 0;
	}
	{
	int chan, addr;

	/* Eliminazione definitiva dei moduli fantasma. */
	for (chan = 0; chan < HSD_N_CHAN; ++chan) {
		for (addr = 16; addr < HSD_N_ADDR; ++addr) {
			if (p -> types[chan][addr] == HSD_P810) {
				if (p -> types[chan][addr-16] == HSD_P810) {
					p -> types[chan][addr] = HSD_NONE;
				}
				else {
			/* Strano, vero? */
				}
			}
		}
	}
	}

	p -> bit_in_list = (ybn_bit_t *) NULL;
	p -> bit_out_list = (ybn_bit_t *) NULL;
	p -> short_out_list = (ybn_short_t *) NULL;
	p -> long_in_list = (ybn_long_t *) NULL;

	p -> p260_set.n_in = 16;
	p -> p260_set.n_out = 16;
	p -> p260_set.code = YBNP260;
	p -> p260_set.in_code = D_IN_N;
	p -> p260_set.out_code = D_OUT_N;
	p -> p260_set.count = 0;
	p -> p260_set.list = (ybn_mod_t *) NULL;

	p -> p810_set.n_in = 1;
	p -> p810_set.n_out = 4;
	p -> p810_set.code = YBNP810;
	p -> p810_set.in_code = D_IN_N;
	p -> p810_set.out_code = D_OUT_N;
	p -> p810_set.count = 0;
	p -> p810_set.list = (ybn_mod_t *) NULL;

	p -> p250_set.n_in = 0;
	p -> p250_set.n_out = 4;
	p -> p250_set.code = YBNP250;
	p -> p250_set.in_code = 0;
	p -> p250_set.out_code = (unsigned long)A_OUT_N;
	p -> p250_set.count = 0;
	p -> p250_set.list = (ybn_mod_t *) NULL;

	p -> p131_set.n_in = 1;
	p -> p131_set.n_out = 0;
	p -> p131_set.code = YBNP131;
	p -> p131_set.in_code = ENC_N;
	p -> p131_set.out_code = 0;
	p -> p131_set.count = 0;
	p -> p131_set.list = (ybn_mod_t *) NULL;

	p -> p137_set.n_in = 1;
	p -> p137_set.n_out = 0;
	p -> p137_set.code = YBNP137;
	p -> p137_set.in_code = ENC_N;
	p -> p137_set.out_code = 0;
	p -> p137_set.count = 0;
	p -> p137_set.list = (ybn_mod_t *) NULL;

	d -> device = (void *) p;

/* ...e adesso ci prendiamo tutto, alla faccia di tutte le altre
 applicazioni. */
	_ybn_take_all(p);
	hsd_commit_requests(p -> fb);

	return 1;
}

static void ybn_detach_core(dr_ybn_t *dev)
{

/* Rimuove tutti gli assegnamenti di variabile. */
/* NO! Restano allocati per sempre!
	hsd_release_modules(dev -> fb);
*/

	util_add_chain((void *)(dev -> bit_in_list),
	           (void **)&ybn_st.bit_free,
		   GET_STRUCT_OFFSET(ybn_bit_t,next));
	util_add_chain((void *)(dev -> bit_out_list),
	           (void **)&ybn_st.bit_free,
		   GET_STRUCT_OFFSET(ybn_bit_t,next));
	util_add_chain((void *)(dev -> short_out_list),
	           (void **)&ybn_st.short_free,
		   GET_STRUCT_OFFSET(ybn_short_t,next));
	util_add_chain((void *)(dev -> long_in_list),
	           (void **)&ybn_st.long_free,
		   GET_STRUCT_OFFSET(ybn_long_t,next));
	dev -> bit_in_list = (ybn_bit_t *) NULL;
	dev -> bit_out_list = (ybn_bit_t *) NULL;
	dev -> short_out_list = (ybn_short_t *) NULL;
	dev -> long_in_list = (ybn_long_t *) NULL;
}

static void ybn_detach(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

	ybn_detach_core(dev);
}

static void ybn_restart(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

/* Rimouve tutti gli assegnamenti di variabile. */

	ybn_detach_core(dev);

/* Azzera il conteggio degli errori. Ma e` giusto? */

	csem_wait(ybn_st.lib_sem, (struct timeval *) 0);
	YBN_reset_all_errors(dev -> nfb);
	csem_signal(ybn_st.lib_sem);
}

/*
* FUNZIONI ASSOCIATE AI METODI "parse" ed "attach".
*/

/* TODO: Unificare le funzioni attach_bit, _aout e _encin, che sono
 "fotocopiate"... */

static int _ybn_attach_bit(dr_ybn_t *dev, devnode_t *l, void *var)
{
unsigned char *target;
ybn_bit_t *lm;
ybn_bit_t **tl;
ybn_mod_t *dl;
int bit;
int inmode;

/* Cerca il bit tra quelli gia` occupati. */

	bit = l -> c_bit;
	if (bit >= LEVEL_5_OUTBIT) {
		bit -= LEVEL_5_OUTBIT;
		tl = &(dev -> bit_out_list);
		inmode = 0;
	}
	else {
		bit -= LEVEL_5_INBIT;
		tl = &(dev -> bit_in_list);
		inmode = 1;
	}
	for (lm = *tl; lm; lm = lm -> next) {
		if (lm -> chan == l -> c_chan
		 && lm -> addr == l -> c_addr
		 && lm -> bit == bit) {
		/* ERRORE: bit gia` impegnato. */
			return 0;
		}
	}

	if (l -> c_sts != dev -> types[l -> c_chan][l -> c_addr]) {
	/* ERRORE. Non esiste il dispositivo, o non e` del tipo
	 previsto. */
		return 0;
	}

/* Preleviamo il modulo dalla lista opportuna. */
	dl = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (! dl) {
	/* ERRORE: (poco spiegabile...) */
		return 0;
	}

	if (inmode) {
		target = dl -> hw_in[bit];
	}
	else {
		target = dl -> hw_out[bit];
	}

	/* Alloca un nuovo nodo per descrivere il bit. */
	lm = (ybn_bit_t *) util_alloc_cell(
		                          (void **) &ybn_st.bit_free,
		                          sizeof(ybn_bit_t),
		                          GET_STRUCT_OFFSET(ybn_bit_t,next),
		                          LEN_HSD_ALLOC_BLK);
	if (! lm) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"YBN");
		return 0;
	}

/* Annulla l'eventuale forzatura attiva sul bit. */
	YBN_force_io(dev -> nfb, (short)(l -> c_chan), l -> c_addr,
	             (short)(bit),
	             inmode ? D_IN_N : D_OUT_N,(short)(-1));

	lm -> chan = l -> c_chan;
	lm -> addr = l -> c_addr;
	lm -> bit = bit;
	lm -> hw_var = target;
	lm -> var = var;

	/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) lm,
	           (void **) tl,
	           GET_STRUCT_OFFSET(ybn_bit_t,next));

	return 1;
}

static int _ybn_attach_aout(dr_ybn_t *dev, devnode_t *l, void *var)
{
unsigned char *target;
ybn_short_t *lm;
ybn_short_t **tl;
ybn_mod_t *dl;
int aout;

/* Cerca il segnale tra quelli gia` occupati. */

	aout = l -> c_aout;
	aout -= LEVEL_5_AOUT;
	tl = &(dev -> short_out_list);
	for (lm = *tl; lm; lm = lm -> next) {
		if (lm -> chan == l -> c_chan
		 && lm -> addr == l -> c_addr
		 && lm -> io == aout) {
		/* ERRORE: segnale gia` impegnato. */
			return 0;
		}
	}

	if (l -> c_sts != dev -> types[l -> c_chan][l -> c_addr]) {
	/* ERRORE. Non esiste il dispositivo, o non e` del tipo
	 previsto. */
		return 0;
	}

/* Preleviamo il modulo dalla lista opportuna. */
	dl = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (! dl) {
	/* ERRORE: (poco spiegabile...) */
		return 0;
	}

	target = dl -> hw_out[aout];

	/* Alloca un nuovo nodo per descrivere il segnale. */
	lm = (ybn_short_t *) util_alloc_cell(
		                          (void **) &ybn_st.short_free,
		                          sizeof(ybn_short_t),
		                          GET_STRUCT_OFFSET(ybn_short_t,next),
		                          LEN_HSD_ALLOC_BLK);
	if (! lm) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"YBN");
		return 0;
	}

#if 0
/* Annulla l'eventuale forzatura attiva sul segnale (??? Inappicabile
 sugli analogici). */
	YBN_force_io(dev -> nfb, l -> c_chan, l -> c_addr, aout,
	             A_OUT_N,-1);
#endif

	lm -> chan = l -> c_chan;
	lm -> addr = l -> c_addr;
	lm -> io = aout;
	lm -> hw_var = (short *)target;
	lm -> var = var;

	/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) lm,
	           (void **) tl,
	           GET_STRUCT_OFFSET(ybn_short_t,next));

	return 1;
}

static int _ybn_attach_encin(dr_ybn_t *dev, devnode_t *l, void *var)
{
unsigned char *target;
ybn_long_t *lm;
ybn_long_t **tl;
ybn_mod_t *dl;
int encin;

/* Cerca il segnale tra quelli gia` occupati. */

	encin = l -> c_encin;
	encin -= LEVEL_5_ENC;
	tl = &(dev -> long_in_list);
	for (lm = *tl; lm; lm = lm -> next) {
		if (lm -> chan == l -> c_chan
		 && lm -> addr == l -> c_addr
		 && lm -> io == encin) {
		/* ERRORE: segnale gia` impegnato. */
			return 0;
		}
	}

	if (l -> c_sts != dev -> types[l -> c_chan][l -> c_addr]) {
	/* ERRORE. Non esiste il dispositivo, o non e` del tipo
	 previsto. */
		return 0;
	}

/* Preleviamo il modulo dalla lista opportuna. */
	dl = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (! dl) {
	/* ERRORE: (poco spiegabile...) */
		return 0;
	}

	target = dl -> hw_in[encin];

	/* Alloca un nuovo nodo per descrivere il segnale. */
	lm = (ybn_long_t *) util_alloc_cell(
		                          (void **) &ybn_st.long_free,
		                          sizeof(ybn_long_t),
		                          GET_STRUCT_OFFSET(ybn_long_t,next),
		                          LEN_HSD_ALLOC_BLK);
	if (! lm) {
	/* ERRORE. Non c'e` memoria. */
		plcError(108,"YBN");
		return 0;
	}

#if 0
/* Annulla l'eventuale forzatura attiva sul segnale (??? Inappicabile
 sugli encoder). */
	YBN_force_io(dev -> nfb, l -> c_chan, l -> c_addr, encin,
	             ENC_N,-1);
#endif

	lm -> chan = l -> c_chan;
	lm -> addr = l -> c_addr;
	lm -> io = encin;
	lm -> hw_var = (long *)target;
	lm -> var = var;

	/* Aggiunge la nuova cella alla lista che le compete. */

	util_add_cell((void *) lm,
	           (void **) tl,
	           GET_STRUCT_OFFSET(ybn_long_t,next));

	return 1;
}

static int _ybn_attach_p810_or_p137(driver_t *d, devnode_t *l, void *var)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;
long ex;
ybn_mod_t *dl;

	if (l -> c_mode != DRIVER_MODE_INPUT) {
		return 0;
	}

	dl = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (! dl) {
	/* ERRORE: vedere la funzione per i dettagli. */
		return 0;
	}
#if 0
/* Questo sarebbe il modo canonico... */
	/* Registrazione del servizio aggiunto. */
	ex = ioRegisterExtension(d, (void *) pl);
	dev -> ex = ex;
#else
/* ...e invece bisogna fare cosi`. Ci sara` il problema di
 distinguere le varie istanze del driver, dato che la libreria YBN,
 internamente, non gestisce piu` di un fieldbus. Truccaccio:
 dato che l'indice di modulo e` dichiarato di tipo "char" nei prototipi
 delle Draw*, si codifica l'indice di fieldbus nella parte alta del dato
 a 32 bit. Per come funziona ora la libreria, la parte alta dovrebbe
 andare perduta. Sperabilemente, la stessa cosa dovrebbe valere per i
 motori... */
	ex = (dl -> index) | (dev -> nfb << 16);
#endif

/* Alla variabile agganciata e` assegnato un valore che permettera`
 di risalire al dispositivo (indice del modulo, in questo caso). */

	*((long *)var) = ex;

	return 1;
}

static int _ybn_parse_bit(dr_ybn_t * dev, devnode_t *l)
{
int bit,inmode,mode;
char *name;

	name = l -> pname;

/* Controlla che il byte corrisponda ad un modulo funzionante, e che
 la modalita` di accesso richiesta sia compatibile con la natura del
 modulo stesso. */

	mode = l -> flags;
	inmode = mode & DRIVER_MODE_INPUT;

/* Cerca l'indice di bit. */
	bit = ioGetNum(&name);
/* TODO: radunare in un solo posto la conoscenza del numero di ingressi
 e uscite ammissibili per ogni tipo di modulo. */
	if (((l -> c_type == LEVEL_4_P260)
	     && (bit < 0 || bit > 15))
	 || ((l -> c_type == LEVEL_4_P810)
	     && (   ((bit < 0 || bit > 4) && !inmode)
	         || ((bit != 0) && inmode)))) {
	/* ERRORE. L'indicazione del bit non e` conforme. */
		return -1;
	}

	if (name[0]) {
	/* ERRORE: schifezze in fondo al nome. */
		return -1;
	}

	l -> c_bit = bit
	           + ((mode & DRIVER_MODE_INPUT)
	              ? LEVEL_5_INBIT
	              : LEVEL_5_OUTBIT);
	l -> c_mode = mode & ~DRIVER_MODE_LIST;
	l -> ideep = 5;
	l -> pname = name;

/* Fine. */

	return (l -> nbit = 1);
}

static int _ybn_parse_aout(dr_ybn_t * dev, devnode_t *l)
{
int aout;
char *name;

	name = l -> pname;

/* Controlla che il dato corrisponda ad un modulo funzionante, e che
 la modalita` di accesso richiesta sia compatibile con la natura del
 modulo stesso. */

	if (! (l -> flags & DRIVER_MODE_OUTPUT)) {
		return -1;
	}

/* Cerca l'indice dell'uscita. */
	aout = ioGetNum(&name);
/* TODO: radunare in un solo posto la conoscenza del numero di ingressi
 e uscite ammissibili per ogni tipo di modulo. */
	if (aout < 0 || aout > 4) {
	/* ERRORE. L'indicazione dell'uscita non e` conforme. */
		return -1;
	}

	if (name[0]) {
	/* ERRORE: schifezze in fondo al nome. */
		return -1;
	}

	l -> c_aout = aout + LEVEL_5_ENC;
	l -> c_mode = l -> flags & ~DRIVER_MODE_LIST;
	l -> ideep = 5;
	l -> pname = name;

/* Fine. */

	return (l -> nbit = 16);
}

static int _ybn_parse_encin(dr_ybn_t * dev, devnode_t *l)
{
int encin;
char *name;

	name = l -> pname;

/* Controlla che il dato corrisponda ad un modulo funzionante, e che
 la modalita` di accesso richiesta sia compatibile con la natura del
 modulo stesso. */

	if (! (l -> flags & DRIVER_MODE_INPUT)) {
		return -1;
	}

/* Cerca l'indice dell'uscita. */
	encin = ioGetNum(&name);
/* TODO: radunare in un solo posto la conoscenza del numero di ingressi
 e uscite ammissibili per ogni tipo di modulo. */
	if (encin < 0 || encin > 1) {
	/* ERRORE. L'indicazione del bit non e` conforme. */
		return -1;
	}

	if (name[0]) {
	/* ERRORE: schifezze in fondo al nome. */
		return -1;
	}

	l -> c_aout = encin + LEVEL_5_ENC;
	l -> c_mode = l -> flags & ~DRIVER_MODE_LIST;
	l -> ideep = 5;
	l -> pname = name;

/* Fine. */

	return (l -> nbit = 32);
}

static int _ybn_parse_mod(dr_ybn_t * dev, devnode_t *l)
{
int mod;
char *name;
int size;
int sts;
int (*parse_l5)(dr_ybn_t *, devnode_t *)
 = (int (*)(dr_ybn_t *, devnode_t *))NULL;

	name = l -> pname;

	mod = ioGetNum(&name);
	if (mod < 0 || mod >= HSD_N_ADDR) {
	/* No, non interessano i moduli YBN. */
		return 0;
	}

	if (name[0] == '\0') {
		l -> ideep = 3;
		l -> flags |= DRIVER_MODE_LIST;
		return 0;
	}
	l -> c_mod = mod;

	if (util_strneq(name,"p260",4)
	 || util_strneq(name,"p270",4)) {
		l -> c_type = LEVEL_4_P260;
		sts = HSD_P260;
		parse_l5 = _ybn_parse_bit;
	}
	else if (util_strneq(name,"p810",4)) {
		l -> c_type = LEVEL_4_P810;
		sts = HSD_P810;
		parse_l5 = _ybn_parse_bit;
	}
	else if (util_strneq(name,"p250",4)) {
		l -> c_type = LEVEL_4_P250;
		sts = HSD_P250;
		parse_l5 = _ybn_parse_aout;
	}
	else if (util_strneq(name,"p131",4)) {
		l -> c_type = LEVEL_4_P131;
		sts = HSD_P230;
		parse_l5 = _ybn_parse_encin;
	}
	else if (util_strneq(name,"p137",4)) {
		l -> c_type = LEVEL_4_P137;
		sts = HSD_P137;
		parse_l5 = _ybn_parse_encin;
	}
	else {
	/* No, non interessano i moduli YBN. */
		return 0;
	}
	if (name[4] != '\0' && name[4] != '.') {
	/* No, non interessano i moduli YBN. */
		return 0;
	}

	if (sts != dev -> types[l -> c_chan][l -> c_addr]) {
	/* ERRORE. Non esiste il dispositivo, o non e` del tipo
	 previsto. */
		return 0;
	}

	name += 5;

	l -> c_sts = sts;

	l -> pname = name;

	if (name[0] == '\0') {
		l -> ideep = 4;
		l -> flags |= DRIVER_MODE_LIST;
		l -> c_mode = l -> flags & ~DRIVER_MODE_LIST;
		if ((sts == HSD_P810 || sts == HSD_P137)
		 && (l -> flags & DRIVER_MODE_INPUT)) {
			return 32;
		}
		else {
			return 0;
		}
	}
	size = (*parse_l5)(dev,l);
	return size;
}

static int _ybn_parse_bus(dr_ybn_t * dev, devnode_t *l)
{
int chan;
char *name;
int size;

	name = l -> pname;

	chan = ioGetNum(&name);
	if (chan < 0 || chan >= HSD_N_CHAN) {
	/* No, non interessano i moduli HSD. */
		return 0;
	}

	l -> c_chan = chan;

	l -> pname = name;

	if (name[0] == '\0') {
		l -> ideep = 2;
		l -> flags |= DRIVER_MODE_LIST;
		return 0;
	}

	size = _ybn_parse_mod(dev,l);
	return size;
}

static int ybn_parse(driver_t * d, int i_off, devnode_t *l)
{
int size;
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

/* Il driver HSD e` istanziabile, percio` ha poco senso interpretare
 in modo ambiguo il campo "istanza". */

	if (i_off != 0) {
	/* ERRORE. Istanza inesistente. */
		return 0;
	}

	size = _ybn_parse_bus(dev,l);
	return size < 0 ? 0 : size;
}

static int ybn_attach(driver_t * d, devnode_t * l, void * var)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

	if (l -> ideep == 5) {
		switch (l -> c_type) {
		case LEVEL_4_P260:
		case LEVEL_4_P810:
			return _ybn_attach_bit(dev, l, var);
		case LEVEL_4_P250:
			return _ybn_attach_aout(dev, l, var);
		case LEVEL_4_P131:
		case LEVEL_4_P137:
			return _ybn_attach_encin(dev, l, var);
		default:
			break;
		}
	}
	else if (l -> ideep == 4)
		return _ybn_attach_p810_or_p137(d, l, var);
	return 0;
}

static int ybn_check(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;
int rv,mod;
int in_go;

	rv = 1;

	if (! dev -> do_chk)
		return rv;

/* LAVORI IN CORSO */

/* Teoria: si provoca l'halt irreversibile solo quando si e` in GO.
  Per questo ad ogni ciclo devono essere azzerati gli errori dei moduli,
 altrimenti non si potrebbe distinguere tra un errore saltuario
 e uno persistente. Il debugger potra` mostrare l'evoluzione degli errori
 solo a partire dall'entrata in halt (cazzata, comunque). Unica eccezione:
 quando max_err_count vale meno di 2 (1 e` il caso normale). La presenza
 di un solo errore provoca comunque l'HALT irreversibile, percio` e` bello
 che rimanga memoria dell'accaduto, in modo che il debugger mostri lo
 stato delle cose. */
	in_go = (!plcIsHalt());
	mod = YBN_error(dev -> nfb,(in_go && (dev -> max_err_count > 1)));
	if (mod != -1 && in_go && (dev -> max_err_count > 0)) {
		++(dev -> err_count);
		if (dev -> err_count >= dev -> max_err_count) {
			ioAddError(d,1,
				((mod & 0xFF) << 16) | ((mod & 0xFF00) >> 8));
			rv = 0;
		}
	}
	else {
		dev -> err_count = 0;
	}

	return rv;
}

static void ybn_trigger(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

/* LAVORI IN CORSO */
	if (dev -> unrecoverable) /* Per sicurezza. */
		return;

	if (!(dev -> protocol_in_write)) {
		if (dev -> do_master) {
			hsd_protocol(dev -> fb,1);
		}
		dev -> protocol_in_trigger = 1;
	}
	dev -> protocol_in_write = 0;
}

static void ybn_up(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

/* LAVORI IN CORSO */

/* Alla prima entrata in GO in assoluto, si attiva la modalita` master. */

	if (!(dev -> go_done)) {
		dev -> go_done = 1;
		if (dev -> do_master) {
			hsd_start_master(dev -> fb, plcPeriod());
		}
	}
}

static void _ybn_clear_out(dr_ybn_t *dev)
{
{
ybn_bit_t *p = dev -> bit_out_list;

	while (p) {
		*(p -> hw_var) = 0;
		p = p -> next;
	}
}
{
ybn_short_t *p = dev -> short_out_list;

	while (p) {
		*(p -> hw_var) = 0;
		p = p -> next;
	}
}
}

static void ybn_down(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

/* Bisogna mettere in sicurezza tutte le uscite a bit, almeno. */
	_ybn_clear_out(dev);

	dev -> err_count = 0;
}

static void ybn_fatal(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;


/* LAVORI IN CORSO */

/* Direi che la cosa piu` saggia e` smettere di gestire il fieldbus. */

	dev -> unrecoverable = 1;

	if (dev -> do_master) {
		hsd_end_master(dev -> fb);
	}
	else {
	/* ...e se non lo gestiamo noi? Si azzera tutto, sperando
	 di prenderci. */
		_ybn_clear_out(dev);
	}
}

static void ybn_read(driver_t * d)
{
{
ybn_bit_t *p = ((dr_ybn_t *) d -> device) -> bit_in_list;

	while (p) {
		*(p -> var) = (*(p -> hw_var) != 0);
		p = p -> next;
	}
}
{
ybn_long_t *p = ((dr_ybn_t *) d -> device) -> long_in_list;

	while (p) {
		*(p -> var) = *(p -> hw_var);
		p = p -> next;
	}
}
}

static void ybn_write(driver_t * d)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

	if (dev -> unrecoverable) /* Per sicurezza. */
		return;

{
ybn_bit_t *p = dev -> bit_out_list;

	while (p) {
		*(p -> hw_var) = (*(p -> var) != 0);
		p = p -> next;
	}
}
{
ybn_short_t *p = dev -> short_out_list;

	while (p) {
		*(p -> hw_var) = *(p -> var);
		p = p -> next;
	}
}


/* LAVORI IN CORSO */
/* Vedere la nota in ybn_trigger. */
	if (!(dev -> protocol_in_trigger)) {
		if (dev -> do_master) {
			hsd_protocol(dev -> fb,1);
		}
	}
	dev -> protocol_in_write = 1;
	dev -> protocol_in_trigger = 0;
}

/*
* Metodo "list". Complicato assai.
*/

/* Funzione di costruzione del nome (livello 1: DRIVER.ISTANZA). */
static void _ybn_list_1_print(driver_t * d, bstring_t *b)
{
	util_sprintf_bstring(b,"%s.%d", d -> op.name, d -> instance);
}

static int _ybn_list_1(driver_t * d, devnode_t * l, bstring_t *b)
{
	util_safe_sprintf(l -> comment,
			  sizeof(l -> comment),
			  "%s", ybn_st.overall_hsd_version);
	return 0;
}

/* Funzione di costruzione del nome
 (livello 2: DRIVER.ISTANZA.CANALE). */
static void _ybn_list_2_print(driver_t * d, bstring_t *b, int param)
{
	_ybn_list_1_print(d, b);
	util_addch_bstring(b,'.');
	util_ltob_bstring(b,param,10);
}

/* Funzione di esplorazione per il livello 2
 (DRIVER.ISTANZA.CANALE). */
static int _ybn_list_2(driver_t * d, devnode_t * l, bstring_t *b)
{
int quit = 0;
int ok = 0;
int param = -1;

	while (!quit && !ok) {
		param = l -> c_chan;
		if (l -> c_chan < HSD_N_CHAN) {
			ok = 1;
			++(l -> c_chan);
		}
		else {
			quit = 1;
		}
	}
	if (ok) {
		l -> flags = DRIVER_MODE_LIST;
		l -> t_c_chan = param;
		l -> t_c_mod = 0;
		_ybn_list_2_print(d, b, param);
	}
	return 0;
}

/* Funzione di costruzione del nome (livello 3:
 DRIVER.ISTANZA.CANALE.MODULO  */
static void _ybn_list_3_print(driver_t * d, bstring_t *b, int p1, int p2)
{
	_ybn_list_2_print(d, b, p1);
	util_addch_bstring(b,'.');
	util_ltob_bstring(b,p2,10);
}

/* Funzione di esplorazione per il livello 3
 (DRIVER.ISTANZA.CANALE.MODULO) */
static int _ybn_list_3(driver_t * d, devnode_t * l, bstring_t *b)
{
int quit = 0;
int ok = 0;
int p1 = -1;
int p2 = -1;
int sts = 0;
dr_ybn_t *dev = (dr_ybn_t *) d -> device;

	p1 = l -> c_chan;
	while (!quit && !ok) {
		p2 = l -> c_mod;
		if (p1 < HSD_N_CHAN && p2 < HSD_N_ADDR) {
			sts = dev -> types[p1][p2];
			if (sts != HSD_NONE) {
				l -> flags = DRIVER_MODE_LIST;
				ok = 1;
			}
			++(l -> c_mod);
		}
		else {
			quit = 1;
		}
	}
	if (ok) {
		l -> t_c_mod = p2;
		l -> t_c_type = 0;
		l -> t_c_sts = sts;
		_ybn_list_3_print(d, b, p1, p2);
	}
	return 0;
}

/* Funzione di costruzione del nome (livello 4:
 DRIVER.ISTANZA.CPU.MODULO.TIPO) */
static void _ybn_list_4_print(driver_t * d, bstring_t *b,int p1,int p2,int p3)
{
static const char *nt[] = { "P260","P810","P250","P131","P137" };

	_ybn_list_3_print(d, b, p1, p2);
	util_addch_bstring(b,'.');
	util_cat_bstring(b,nt[p3]);
}

/* Funzione di esplorazione per il livello 4
 (DRIVER.ISTANZA.CPU.MODULO.TIPO) */
static int _ybn_list_4(driver_t * d, devnode_t * l, bstring_t *b)
{
int ok = 0;
int p3 = -1;
int sts;

	sts = l -> t_c_sts;
	/* sts = dev -> types[l -> c_chan][l -> c_mod]; */
	if (sts == HSD_NONE)
		return -1;

	while (!ok) {
		p3 = l -> c_type;
		if (p3 == LEVEL_4_P260) {
			if (sts == HSD_P260) {
				ok = 1;
				l -> flags = DRIVER_MODE_LIST;
			}
			l -> nbit = 0;
		}
		else if (p3 == LEVEL_4_P810) {
			if (sts == HSD_P810) {
				ok = 1;
				l -> flags = DRIVER_MODE_LIST
				           | DRIVER_MODE_INPUT;
			}
			l -> nbit = 32;
		}
		else if (p3 == LEVEL_4_P250) {
			if (sts == HSD_P250) {
				ok = 1;
				l -> flags = DRIVER_MODE_LIST;
			}
			l -> nbit = 0;
		}
		else if (p3 == LEVEL_4_P137) {
			if (sts == HSD_P137) {
				ok = 1;
				l -> flags = DRIVER_MODE_LIST
				           | DRIVER_MODE_INPUT;
			}
			l -> nbit = 32;
		}
		else if (p3 == LEVEL_4_P131) {
			if (sts == HSD_P230) {
				ok = 1;
				l -> flags = DRIVER_MODE_LIST;
			}
			l -> nbit = 0;
		}
		else {
			break;
		}
		++(l -> c_type);
	}
	if (ok) {
		l -> t_c_type = p3;
		l -> t_c_sts = sts;
		l -> t_c_bit = 0;
		l -> t_c_mode = l -> flags & ~DRIVER_MODE_LIST;
		_ybn_list_4_print(d, b, l -> c_chan,
		                  l -> c_mod, p3);
	}
	return 0;
}

static int _ybn_list_5(driver_t * d, devnode_t * l, bstring_t *b)
{
int p3,p4,nb;
int aux = 0;
int inmode = 0;
int step = 1;
int sz = 0;

	p3 = l -> c_type;
	p4 = l -> c_bit;
	switch (p3) {
	case LEVEL_4_P260:
	case LEVEL_4_P810:
		if (p4 < LEVEL_5_OUTBIT) {
			aux = p4 - LEVEL_5_INBIT;
			inmode = 1;
		}
		else if (p4 < LEVEL_5_DIG_MAX) {
			aux = p4 - LEVEL_5_OUTBIT;
			inmode = 0;
		}
		else {
			return 0;
		}
		if (p3 == LEVEL_4_P260) {
			nb = 16;
			step = 1;
		}
		else {
			if (inmode) {
				nb = 1;
				step = 16;
			}
			else {
				nb = 4;
				step = 13;
			}
		}
		sz = 1;
		break;
	case LEVEL_4_P250:
		inmode = 0;
		aux = p4 - LEVEL_5_AOUT;
		nb = 4;
		step = 1;
		sz = 16;
		break;
	case LEVEL_4_P131:
	case LEVEL_4_P137:
		inmode = 1;
		aux = p4 - LEVEL_5_ENC;
		nb = 1;
		step = 1;
		sz = 32;
		break;
	default:
		nb = 0;
		sz = 0;
		break;
	}
	if (aux < nb) {
		l -> c_bit += (aux == nb-1) ? step : 1;
		l -> nbit = sz;
		l -> flags = inmode ? DRIVER_MODE_INPUT : DRIVER_MODE_OUTPUT;
		_ybn_list_4_print(d,b,l -> c_chan,
				  l -> c_mod,l -> c_type);
		util_addch_bstring(b,'.');
		util_ltob_bstring(b,aux,10);
	}
	return 0;
}

static int ybn_list(driver_t * d, devnode_t * l)
{
int rv;
bstring_t b;

	l -> flags = 0;
	l -> nbit = 0;
	l -> name[0] = '\0';

	util_init_bstring(&b,l -> name, sizeof(l -> name));

	switch (l -> ideep) {
	case 1:
		rv = _ybn_list_1(d,l,&b);
		break;
	case 2:
		rv = _ybn_list_2(d,l,&b);
		break;
	case 3:
		rv = _ybn_list_3(d,l,&b);
		break;
	case 4:
		rv = _ybn_list_4(d,l,&b);
		break;
	case 5:
		rv = _ybn_list_5(d,l,&b);
		break;
	default:
		rv = -1;
		break;
	}
	return rv;
}

static int _ybn_show_bit(dr_ybn_t *dev, devnode_t *l, void *dest)
{
int bit,p4,inmode;
ybn_mod_t *dl;

	p4 = l -> c_bit;
	if (p4 < LEVEL_5_OUTBIT) {
		bit = p4 - LEVEL_5_INBIT;
		inmode = 1;
	}
	else if (p4 < LEVEL_5_DIG_MAX) {
		bit = p4 - LEVEL_5_OUTBIT;
		inmode = 0;
	}
	else {
		return 0;
	}

	dl  = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (dl) {
		*(char *)dest = *(inmode
				  ? dl -> hw_in[bit]
				  : dl -> hw_out[bit])
			      != 0;
		return 1;
	}

	*(char *)dest = 0;

	return 0;
}

static int _ybn_show_aout(dr_ybn_t *dev, devnode_t *l, void *dest)
{
int p4;
ybn_mod_t *dl;

	p4 = l -> c_aout;
	dl  = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (dl) {
		*(short *)dest = *(short *)(dl -> hw_out[p4 - LEVEL_5_AOUT]);
		return 1;
	}

	*(short *)dest = 0;

	return 0;
}

static int _ybn_show_encin(dr_ybn_t *dev, devnode_t *l, void *dest)
{
int p4;
ybn_mod_t *dl;

	p4 = l -> c_encin;
	dl  = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
	if (dl) {
		*(long *)dest = *(long *)(dl -> hw_out[p4 - LEVEL_5_ENC]);
		return 1;
	}

	*(long *)dest = 0;

	return 0;
}

static int ybn_show(driver_t * d, devnode_t *l, void *dest)
{
dr_ybn_t *dev = (dr_ybn_t *) d -> device;
ybn_mod_t *dl;

	l -> nbit = 1;

	switch (l -> ideep) {

	case 5:
		switch (l -> c_type) {
		case LEVEL_4_P260:
		case LEVEL_4_P810:
			return _ybn_show_bit(dev, l, dest);
		case LEVEL_4_P250:
			return _ybn_show_aout(dev, l, dest);
		case LEVEL_4_P131:
		case LEVEL_4_P137:
			return _ybn_show_encin(dev, l, dest);
		default:
			break;
		}
		break;

	case 4:
		if (l -> c_type == LEVEL_4_P810
		 || l -> c_type == LEVEL_4_P137) {
			dl  = _ybn_find_mod(dev, l -> c_chan, l -> c_addr);
			if (dl) {
				l -> nbit = 32;
				*(long *)dest = dl->index | (dev->nfb << 16);
				return 1;
			}
		}
		break;

	case 3:
		break;

	case 2:
		break;

	default:
		break;
	}

	return 0;
}

driver_op_t driver_ybn = {
	"ybn",
	ybn_install,
	ybn_restart,
	ybn_parse,
	ybn_list,
	ybn_attach,
	ybn_detach,
	ybn_check,
	ybn_trigger,
	ybn_up,
	ybn_down,
	(void (*)(driver_t *)) 0, /* ybn_close */
	ybn_fatal,
	ybn_read,
	ybn_write,
	ybn_show,
	(int (*)(driver_t *,void *,int,void *)) 0, /* ybn_extension */
};

