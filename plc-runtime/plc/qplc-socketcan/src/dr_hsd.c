/*
* @(#) dr_hsd.c 1.0 Tue Dec 18 15:07:47 MET 2001
*
* Driver della scheda CNI IOS per QPLC.
*
* Forma dei parametri specifici :
*
*        1 bit : <canale>.<modulo>.<byte>.<bit>
*
* Esempio di nome di variabile completo (1 bit):
*
*        hsd.0.0.2.1.5
*         |  | | | | |
*         |  | | | | bit 5...
*         |  | | | del byte 1...
*         |  | | del modulo 2...
*         |  | collegato al canale 0...
*         |  della scheda 0...
*         di tipo Fieldbus HSD.
*
* 18/12/01 GG 1.0 Prima stesura.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		/* Livello 4: */
		#define c_byte next.spec[2]
		#define t_c_byte tree.spec[2]
		#define c_sts next.spec[3]
		#define t_c_sts tree.spec[3]
		/* Byte del modulo (0-7). */
			#define c_bit next.spec[4]
			#define t_c_bit tree.spec[4]
			#define c_mode next.spec[5]
			#define t_c_mode tree.spec[5]
			/* Livello 5: bit nel byte (0-7). */
#define LEVEL_2_QUIT (HSD_N_CHAN)

typedef struct {
	unsigned long m; /* Maschera. */
	unsigned char * var; /* indirizzo della variabile. */
} hsd_bit_t;

typedef struct _hsd_byte {
	unsigned char * addr;	/* Indirizzo del byte sulla scheda. */
#if !AZZERA_NON_USATI
	unsigned int m;		/* Maschera globale per il byte. */
#endif
	hsd_bit_t bit[9];	/* 8 bit (+1 per terminare la lista). */
	struct _hsd_byte * next; /* Puntatore al prossimo elemento. */
} hsd_byte_t;

typedef struct _hsd_module {
	int chan; /* Indirizzo/canale. */
	int addr;
	unsigned char *in; /* Puntatori alle zone di trasmissione */
	unsigned char *out; /* e ricezione. */
	hsdfb_module_t * mod; /* Descrittore offerto dalla libreria. */
	struct _hsd_module * next; /* Puntatore al prossimo elemento. */
} hsd_module_t;

typedef struct {
/* Flag di attivazione del controllo periodico del dispositivo. */
	int do_chk;
/* Flag indicante che e` stata eseguita almeno una "restart". */
	int go_done;
/* Descrittore di fieldbus HSD. */
	hsdfb_t *fb;
/* Liste di ingresso e di uscita. */
	hsd_byte_t *bit_in_list;
	hsd_byte_t *bit_out_list;
/* Lista dei moduli correntemente in uso. */
	hsd_module_t *mod_list;
/* Tabella dei tipi di modulo disponibili. */
	unsigned char types[HSD_N_CHAN][HSD_N_ADDR];
} dr_hsd_t;

/* Array delle strutture di stato (per istanza, cioe` per scheda). */

static dr_hsd_t hsd_board[MAX_HSD_FB];
static hsd_byte_t *hsd_byte_free = (hsd_byte_t *) 0;
static hsd_module_t *hsd_module_free = (hsd_module_t *) 0;

static int hsd_install(driver_t * d, int conf)
{
int n = conf;
dr_hsd_t *p;
char *q;
int check;

	if (d -> instance >= MAX_HSD_FB) {
	/* ERRORE. Troppe istanze del driver. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	check = 1;
	for (ioStartResource(d); (q = ioNextResource(d)); ) {
		if (util_strneq(q,"chk=",4)) {
		/* Flag di abilitazione del test periodico
		 del fieldbus. Default=1. */
			q += 4;
			check = ioGetNum(&q);
		}
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}

	p = &hsd_board[n];
	p -> do_chk = check;
	p -> go_done = 0;

	if (n == 0) {
/* LAVORI IN CORSO */
/* Si potrebbe mettere un ciclo di attesa, per dare tempo al server di
 partire. */
		if (hsd_init() < 0) {
		/* ERRORE. La libreria HSD non ha intenzione di collaborare.
		 Forse il server non c'e`, o non e` ancora partito,
		 o e` morto... */
			return 0;
		}
	}

	p -> fb = hsd_open_bus(n);
	if (!(p -> fb)) {
	/* ERRORE. Il dispositivo non esiste. */
		return 0;
	}

/* Lettura della lista dei moduli disponibili. */
	if (hsd_get_module_types(p -> fb, p -> types) < 0) {
	/* ERRORE. La libreria si e` imbizzarrita. */
		return 0;
	}

	p -> bit_in_list = (hsd_byte_t *) 0;
	p -> bit_out_list = (hsd_byte_t *) 0;
	p -> mod_list = (hsd_module_t *) 0;

	d -> device = (void *) p;

	return 1;
}

static void hsd_detach(driver_t * d)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

/* Rimouve tutti gli assegnamenti di variabile. */
	hsd_release_modules(dev -> fb);

	util_add_chain((void *)(dev -> bit_in_list),
	           (void **)&hsd_byte_free,
		   GET_STRUCT_OFFSET(hsd_byte_t,next));
	util_add_chain((void *)(dev -> bit_out_list),
	           (void **)&hsd_byte_free,
		   GET_STRUCT_OFFSET(hsd_byte_t,next));
	util_add_chain((void *)(dev -> mod_list),
	           (void **)&hsd_module_free,
		   GET_STRUCT_OFFSET(hsd_module_t,next));
	dev -> bit_in_list = dev -> bit_out_list = (hsd_byte_t *) 0;
	dev -> mod_list = (hsd_module_t *) 0;
}

static void hsd_restart(driver_t * d)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

/* Rimouve tutti gli assegnamenti di variabile. */

	hsd_detach(d);

/* LAVORI IN CORSO */

/* Alla prima entrata in GO in assoluto, si attiva la modalita` master. */

	if (!(dev -> go_done)) {
		dev -> go_done = 1;
		hsd_start_master(dev -> fb, plcPeriod());
	}
}

/*
* FUNZIONI ASSOCIATE AI METODI "parse" ed "attach".
*/

static int _hsd_attach_bit(dr_hsd_t *dev, devnode_t *l, void *var)
{
int i;
unsigned long m;
hsd_byte_t *p;
unsigned char *target;
hsd_byte_t **list;
hsd_module_t *lm;
hsdfb_module_t *mod;
unsigned char *in,*out;

/* Cerca il modulo tra quelli gia` occupati. */

	for (lm = dev -> mod_list; lm; lm = lm -> next) {
		if (lm -> chan == l -> c_chan
		 && lm -> addr == l -> c_addr) {
			break;
		}
	}

/* Se il modulo non e` ancora stato occupato, lo prende. */

	if (! lm) {
		mod = hsd_request_module_simple(dev -> fb, l -> c_chan,
		                                l -> c_addr, HSD_P260,
		                                &in,&out);
		if (! mod) {
		/* ERRORE: modulo gia` impegnato, o sfighe piu` gravi
		 ancora. */
			return 0;
		}

	/* Alloca un nuovo nodo per descrivere il modulo. */
		lm = (hsd_module_t *) util_alloc_cell(
		                          (void **) &hsd_module_free,
		                          sizeof(hsd_module_t),
		                          GET_STRUCT_OFFSET(hsd_module_t,next),
		                          LEN_HSD_ALLOC_BLK);
		if (! lm) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

		lm -> chan = l -> c_chan;
		lm -> addr = l -> c_addr;
		lm -> in = in;
		lm -> out = out;
		lm -> mod = mod;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) lm,
		           (void **) &(dev -> mod_list),
		           GET_STRUCT_OFFSET(hsd_module_t,next));
	}

	m = 1 << l -> c_bit;

	if (l -> flags & DRIVER_MODE_INPUT) {
		list = &(dev -> bit_in_list);
		target = lm -> in + l -> c_byte;
	}
	else {
		list = &(dev -> bit_out_list);
		target = lm -> out + l -> c_byte;
	}

	for (p = *list; p; p = p -> next) {
		if (p -> addr == target)
			break;
	}

/* Se non c'e`, alloca un nuovo nodo per descriverlo. */

	if (! p) {
		p = (hsd_byte_t *) util_alloc_cell(
		                          (void **) &hsd_byte_free,
		                          sizeof(hsd_byte_t),
		                          GET_STRUCT_OFFSET(hsd_byte_t,next),
		                          LEN_HSD_ALLOC_BLK);
		if (! p) {
		/* ERRORE. Non c'e` memoria. */
			plcError(108,"2");
			return 0;
		}

	/* Assegna il campo "indirizzo". */

		p -> addr = target;

	/* Azzera il campo "maschera" di tutti gli elementi. */

#if !AZZERA_NON_USATI
		p -> m = 0;
#endif
		for (i = 0; i < sizeof(p -> bit)/sizeof(p -> bit[0]); ++i)
			p -> bit[i].m = 0;

	/* Aggiunge la nuova cella alla lista che le compete. */

		util_add_cell((void *) p,
		           (void **) list,
		           GET_STRUCT_OFFSET(hsd_byte_t,next));
	}

/* Controlla la cella e le aggiunge il bit. */

	for (i = 0; p -> bit[i].m; ++i) {
		if (p -> bit[i].m == m) {
		/* ERRORE. Bit gia` assegnato. */
			return 0;
		}
	}

#if !AZZERA_NON_USATI
	p -> m |= m;
#endif
	p -> bit[i].m = m;
	p -> bit[i].var = (unsigned char *) var;

	return 1;
}

static int _hsd_parse_bit(dr_hsd_t * dev, devnode_t *l)
{
int byt,bit,inmode,mode;
unsigned char sts;
char *name;

	name = l -> pname;

/* Cerca l'indice di byte. */
	byt = ioGetNum(&name);
	if (byt < 0 || byt > 1) {
	/* No, grazie, non interessa. */
		return 0;
	}

/* Controlla che il byte corrisponda ad un modulo funzionante, e che
 la modalita` di accesso richiesta sia compatibile con la natura del
 modulo stesso. */

	mode = l -> flags;
	inmode = mode & DRIVER_MODE_INPUT;

	sts = l -> c_sts;

/* Gestione del caso "byte di un modulo". */
	if (name[0] == '\0') {
		l -> ideep = 4;
		l -> flags = DRIVER_MODE_LIST;
		l -> pname = name;
		l -> c_byte = byt;
		return (l -> nbit = 0);
	}

/* Cerca l'indice di bit. */
	bit = ioGetNum(&name);
	if (bit < 0 || bit > 15) {
	/* ERRORE. L'indicazione del bit non e` conforme. */
		return -1;
	}

	if (name[0]) {
	/* ERRORE: schifezze in fondo al nome. */
		return -1;
	}

/* Calcola un eventuale spostamento in avanti del byte. */

	byt += bit / 8;
	bit %= 8;

	if (byt > 1) {
	/* ERRORE. Non esiste niente del genere in CNi. */
		return -1;
	}

	l -> c_byte = byt;
	l -> c_bit = bit;
	l -> c_mode = mode & ~DRIVER_MODE_LIST;
	l -> ideep = 5;
	l -> pname = name;

/* Fine. */

	return (l -> nbit = 1);
}

static int _hsd_parse_mod(dr_hsd_t * dev, devnode_t *l)
{
int mod;
char *name;
int sts;
int size;

	name = l -> pname;

	mod = ioGetNum(&name);
	if (mod < 0 || mod >= HSD_N_ADDR) {
	/* No, non interessano i moduli IOS. */
		return 0;
	}

	l -> c_mod = mod;

	sts = dev -> types[l -> c_chan][mod];
	if (sts != HSD_P260) {
	/* ERRORE. Non esiste il dispositivo, o non e` del tipo previsto. */
		return -1;
	}

	l -> c_sts = sts;

	l -> pname = name;

	if (name[0] == '\0') {
		l -> ideep = 3;
		l -> flags |= DRIVER_MODE_LIST;
		return 0;
	}
	size = _hsd_parse_bit(dev,l);
	return size;
}

static int _hsd_parse_bus(dr_hsd_t * dev, devnode_t *l)
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

	size = _hsd_parse_mod(dev,l);
	return size;
}

static int hsd_parse(driver_t * d, int i_off, devnode_t *l)
{
int size;
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

/* Il driver HSD e` istanziabile, percio` ha poco senso interpretare
 in modo ambiguo il campo "istanza". */

	if (i_off != 0) {
	/* ERRORE. Istanza inesistente. */
		return 0;
	}

	size = _hsd_parse_bus(dev,l);
	return size < 0 ? 0 : size;
}

static int hsd_attach(driver_t * d, devnode_t * l, void * var)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

	switch (l -> c_chan) {
	default:
		if (l -> ideep == 5)
			return _hsd_attach_bit(dev, l, var);
	}
	return 0;
}

static int hsd_check(driver_t * d)
{
int rv;

	rv = 1;

	if (! ((dr_hsd_t *) d -> device) -> do_chk)
		return rv;

/* LAVORI IN CORSO */

/* Bisognerebbe controllare il livello d'errore riportato da
 "hsd_protocol", e magari aggiungere qualcosa di piu` raffinato
 per ottenere la lista dei moduli in errore... */

	return rv;
}

static void hsd_trigger(driver_t * d)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

/* LAVORI IN CORSO */

/* Chiamiamo qui il manager, per ora. Non e` il posto migliore,
 perche` l'esecutore chiama questa routine all'inizio del ciclo, mentre
 sarebbe meglio che l'invocazione avvenisse alla fine. Ci sarebbe
 il metodo "write", ma e` invocato solo nello stato di GO.
  Complicando la logica, forse si riuscirebbe a usare "write" durante
 il GO, e "trigger" durante l'HALT, ma per ora ci si accontenti. */

	hsd_protocol(dev -> fb);
}

static void hsd_up(driver_t * d)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

}

static void hsd_down(driver_t * d)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

/* LAVORI IN CORSO */

}

static void hsd_fatal(driver_t * d)
{
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

/* LAVORI IN CORSO */

	hsd_end_master(dev -> fb);
}

static void hsd_read(driver_t * d)
{
	{
	register hsd_bit_t *q;
	register unsigned char v;
	register unsigned char m;
	hsd_byte_t *p = ((dr_hsd_t *) d -> device) -> bit_in_list;

		while (p) {
			v = *(p -> addr);
			q = p -> bit;
			while ((m = q -> m)) {
				*(q++ -> var) = ((v & m) != 0);
			}
			p = p -> next;
		}
	}
}

static void hsd_write(driver_t * d)
{
dr_hsd_t * dr = ((dr_hsd_t *) d -> device);

/* Aggiorna le uscite a bit ordinarie. */

{
hsd_byte_t *p;
register hsd_bit_t *q;
register unsigned long v;
register unsigned long m;

	for (p = dr -> bit_out_list; p; p = p -> next) {
#if AZZERA_NON_USATI
		v = 0;
#else
		v = *(p -> addr) & ~(p -> m);
#endif
		q = p -> bit;
		while ((m = q -> m)) {
			if (*(q++ -> var))
				v |= m;
		}
		*(p -> addr) = (unsigned char) v;
	}
}

/* LAVORI IN CORSO */
/* Vedere la nota in hsd_trigger. */
}

/*
* Metodo "list". Complicato assai.
*/

/* Funzione di costruzione del nome (livello 1: DRIVER.ISTANZA). */
static void _hsd_list_1_print(driver_t * d, bstring_t *b)
{
	util_sprintf_bstring(b,"%s.%d", d -> op.name, d -> instance);
}

/* Funzione di costruzione del nome
 (livello 2: DRIVER.ISTANZA.n). */
static void _hsd_list_2_print(driver_t * d, bstring_t *b, int param)
{
	_hsd_list_1_print(d, b);
	util_addch_bstring(b,'.');
	util_ltob_bstring(b,param,10);
}

/* Funzione di esplorazione per il livello 2
 (DRIVER.ISTANZA.n). */
static int _hsd_list_2(driver_t * d, devnode_t * l, bstring_t *b)
{
int quit = 0;
int ok = 0;
int param = -1;
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

	while (!quit && !ok) {
		param = l -> c_chan;
		if (l -> c_chan < HSD_N_CHAN) {
			ok = 1;
			util_safe_sprintf(l -> comment,
				  sizeof(l -> comment),
				  "hsd-fb");
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
		_hsd_list_2_print(d, b, param);
	}
	return 0;
}

/* Funzione di costruzione del nome (livello 3:
 DRIVER.ISTANZA.CANALE.MODULO  */
static void _hsd_list_3_print(driver_t * d, bstring_t *b, int p1, int p2)
{
	_hsd_list_2_print(d, b, p1);
	util_addch_bstring(b,'.');
	util_ltob_bstring(b,p2,10);
}

/* Funzione di esplorazione per il livello 3
 (DRIVER.ISTANZA.CANALE.MODULO) */
static int _hsd_list_3(driver_t * d, devnode_t * l, bstring_t *b)
{
int quit = 0;
int ok = 0;
int p1 = -1;
int p2 = -1;
int sts = 0;
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

	p1 = l -> c_chan;
	while (!quit && !ok) {
		p2 = l -> c_mod;
		if (p1 < HSD_N_CHAN && p2 < HSD_N_ADDR) {
			if (dev -> types[p1][p2] == HSD_P260) {
				util_safe_sprintf(l -> comment,
					sizeof(l -> comment),
					"P260");
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
		l -> t_c_byte = 0;
		l -> t_c_sts = sts;
		_hsd_list_3_print(d, b, p1, p2);
	}
	return 0;
}

/* Funzione di costruzione del nome (livello 4:
 DRIVER.ISTANZA.CPU.MODULO.byte) */
static void _hsd_list_4_print(driver_t * d, bstring_t *b,int p1,int p2,int p3)
{
	_hsd_list_3_print(d, b, p1, p2);
	util_addch_bstring(b,'.');
	util_ltob_bstring(b,p3,10);
}

/* Funzione di esplorazione per il livello 4
 (DRIVER.ISTANZA.CPU.MODULO.byte) */
static int _hsd_list_4(driver_t * d, devnode_t * l, bstring_t *b)
{
int ok = 0;
int p3 = -1;
int sts;

	sts = l -> t_c_sts;
/*
	if (_ios_decode_status(sts,fb,&spec) < 0)
		return -1;
*/

	while (!ok) {
		p3 = l -> c_byte;
		if (p3 < 2) {
			ok = 1;
			l -> flags = DRIVER_MODE_LIST;
			l -> nbit = 0;
		}
		else {
			break;
		}
		++(l -> c_byte);
	}
	if (ok) {
		l -> t_c_byte = p3;
		l -> t_c_sts = sts;
		l -> t_c_bit = 0;
		l -> t_c_mode = l -> flags & ~DRIVER_MODE_LIST;
		_hsd_list_4_print(d, b, l -> c_chan,
		                  l -> c_mod, p3);
	}
	return 0;
}

static int _hsd_list_5(driver_t * d, devnode_t * l, bstring_t *b)
{
int p3,p4,nb;
char *s;

	p3 = l -> c_byte;
	p4 = l -> c_bit;
	if (p4 < 8) {
		++(l -> c_bit);
		l -> nbit = 1;
		l -> flags = l -> c_mode;
		_hsd_list_4_print(d,b,l -> c_chan,
				  l -> c_mod,l -> c_byte);
		util_addch_bstring(b,'.');
		util_ltob_bstring(b,p4,10);
	}
	return 0;
}

static int hsd_list(driver_t * d, devnode_t * l)
{
int rv;
bstring_t b;
dr_hsd_t *dev = (dr_hsd_t *) d -> device;

	l -> flags = 0;
	l -> nbit = 0;
	l -> name[0] = '\0';

	util_init_bstring(&b,l -> name, sizeof(l -> name));

	switch (l -> ideep) {
	case 1:
		util_safe_sprintf(l -> comment,
		                  sizeof(l -> comment),
		                  "l1");
		rv = 0;
		break;
	case 2:
	/* Livello degli oggetti CPU, LED, UDC ed MPP. */
		rv = _hsd_list_2(d,l,&b);
		break;
	case 3:
	/* Livello degli oggetti CPU.MODULO o {LED|UDC|MPP}.SEGNALE. */
		rv = _hsd_list_3(d,l,&b);
		break;
	case 4:
	/* Livello degli oggetti CPU.MODULO.{BYTE|DAC}. */
		rv = _hsd_list_4(d,l,&b);
		break;
	case 5:
	/* Livello degli oggetti CPU.MODULO.BYTE.BIT */
		rv = _hsd_list_5(d,l,&b);
		break;
	default:
		rv = -1;
		break;
	}
	return rv;
}

static int hsd_show(driver_t * d, devnode_t *l, void *dest)
{
unsigned char *target;
dr_hsd_t *dev = (dr_hsd_t *) d -> device;
int p1,p2,p3;

	l -> nbit = 1;

	switch (l -> ideep) {

	case 5:

		p3 = l -> c_byte;
		if (p3 >= 16) {
			return 0;
		}
		else {

		/* Normali ingressi o uscite a bit. Si pesca direttamente
		 dalla mappa sulla scheda IOS. */
	
			*(char *)dest = 0;
			return 1;
		}

	case 4:
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

driver_op_t driver_hsd = {
	"hsd",
	hsd_install,
	hsd_restart,
	hsd_parse,
	hsd_list,
	hsd_attach,
	hsd_detach,
	hsd_check,
	hsd_trigger,
	hsd_up,
	hsd_down,
	(void (*)(driver_t *)) 0, /* hsd_close */
	hsd_fatal,
	hsd_read,
	hsd_write,
	hsd_show,
	(int (*)(driver_t *,void *,int,void *)) 0, /* hsd_extension */
};

