/*
* @(#) dr_keyb.c 3.1 Fri Apr  3 19:50:36 MET DST 1998
*
* Driver della tastiera speciale CNI per QPLC.
*
* Forma dei parametri specifici :
*
*       32 bit : <32>.<override>
*        8 bit : <8>.<override>
*        1 bit : <1>.<tasto>
*   Uscite LED : <LED>.<indice_led>
*
* Esempio di nome di variabile completo (1 bit):
*
*       keyb.0.1.7
*         |  | | |
*         |  | | |
*         |  | | |
*         |  | | tasto 7...
*         |  | dato ad 1 bit...
*         |  della tastiera 0 (l'unica)...
*         di tipo KEYB.
*
* 24/05/96 GG 1.0 Prima stesura.
* 28/08/96 GG 1.1 Modifiche per parametro aggiuntivo di "parse".
* 12/09/96 GG 1.2 Migliorata l'inizializzazione del dispositivo. Aggiunto
*             il test di esistenza della tastiera speciale.
* 30/10/96 GG 1.3 Reso indipendente il test di esistenza del dispositivo
*             dalle primitive di sistema disponibili.
* 11/04/97 GG 2.0 Aggiunta la gestione dei LED e della nuova versione di
*             tastiera del compatto. Portato a due secondi il timeout
*             nell'attesa iniziale dei dati override.
* 23/05/97 GG 2.1 Micro-modifica per gestione della ripetizione della
*             fase di halt.
* 02/07/97 GG 2.2 Spostato dopo la pausa di 1.5 secondi l'azzeramento
*             del bit di invio dello stato degli override.
* 16/07/97 GG 2.3 Aggiunti i metodi "up", "close" ed "extension".
* 05/09/97 GG 2.4 Aggiunta una risorsa di configurazione che permette di
*             ignorare l'assenza della tastiera speciale.
* 02/02/98 GG 2.5 Corretto un clamoroso bug nella gestione dei LED: le
*             maschere di accesso venivano calcolate in base all'ordine di
*             definizione anziche` all'indice di bit. Funzionava per caso!
* 11/03/98 GG 2.6 Aggiunto il metodo "detach".
* 13/03/98 GG 2.7 Aggiunta la possibilita` di mantenere lo stato dei bit
*             non utilizzati di un byte di output tra una forzatura e
*             l'altra (attivo solo se AZZERA_NON_USATI_KEYB vale 0).
* 24/03/98 GG 2.8 Aggiunto il metodo "list".
* 01/04/98 GG 3.0 Rivoluzione dovuta a modifiche profonde in "drivers.h".
*             E (incredibile!) corretto un buco nella gestione degli override
*             ad 8 bit (leggeva dalla tabella sbagliata!).
* 03/04/98 GG 3.1 Aggiunto il metodo "show".
*/

#define AZZERA_NON_USATI_KEYB 0

/* Timeout per il test di esistenza della tastiera speciale. */
#define KB_INIT_TIMEOUT 2000

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sem.h>

#include <kbcni.h>

#include "qplc.h"

#include "util.h"

#include "glue.h"

#define DEFAULT_OVR_VAL 255
#define MAX_OVERRIDE 4
#define KEYB_VERSION_CODE 9

#define MAX_PLC_USER_KEY 32

/* Posizione di alcuni tasti con significato particolare. */
#define KBT_START (MAX_PLC_USER_KEY+0)
#define KBT_STOP (MAX_PLC_USER_KEY+1)
#define KBT_CLEAR (MAX_PLC_USER_KEY+2)
#define KBT_RESET (MAX_PLC_USER_KEY+3)

#define MAX_KEY (KBT_RESET+1)
#define MAX_LED 24

/* Intervallo minimo ammissibile tra due cambiamenti di stato dei LED. */
#define LED_UPDATE_RATE 200 /* millisecondi */

/*
* Struttura di stato del driver.
*/

typedef struct _dr_keyb {
/* File descriptor della tastiera speciale. */
	int fd;
/* TID del thread che sorveglia la tastiera. */
	pthread_t tid_rd;
/* TID del thread incaricato di aggiornare i LED. */
	pthread_t tid_wr;
/* Semaforo di controllo del thread di aggiornamento dei LED. */
	csem_t update_led_sem;
/* Flag di "tastiera presente". */
	int ok;
/* Versione della tastiera collegata. */
	int version;
/* Flag di LED presenti. */
	int led_present;
/* Flag di cambiamento di stato dei tasti. */
	int reload;
/* Stato dei tasti. */
	unsigned char keys[MAX_KEY];
/* Stato degli override. */
	unsigned char overrides[MAX_OVERRIDE];
/* Puntatori alle variabili associate ai tasti. */
	unsigned char *in_key_var[MAX_KEY];
/* Indici dei tasti gia` assegnati. */
	unsigned char in_key_idx[MAX_KEY];
/* Numero di tasti associati a variabili. */
	int n_in_key_var;
/* Puntatori alle variabili associate ai LED. */
	unsigned char *out_led_var[MAX_LED];
/* Indici dei LED gia` assegnati. */
	unsigned char out_led_idx[MAX_LED];
/* Maschere di accesso ai LED. */
	unsigned long out_led_mask[MAX_LED];
/* Stato corrente dei 24 LED. */
	unsigned long led_image;
#if !AZZERA_NON_USATI_KEYB
	unsigned long led_mask;
#endif
/* Flag di cambio stato LED pendente (in attesa di essere inoltrato). */
	int pending_led_change;
/* Istante in cui e` stata inoltrata l'ultima richiesta di aggiornamento
 dei LED. */
	unsigned long last_led_change;
/* Numero di tasti associati a variabili. */
	int n_out_led_var;
/* Puntatori alle variabili associate agli override (8 bit). */
	unsigned char *in_ovr_var_8[MAX_OVERRIDE];
/* Indici degli override gia` assegnati. */
	unsigned char in_ovr_8_idx[MAX_OVERRIDE];
/* Numero di tasti associati a variabili. */
	int n_in_ovr_var_8;
/* Puntatori alle variabili associate agli override (32 bit). */
	long *in_ovr_var_32[MAX_OVERRIDE];
/* Indici degli override gia` assegnati. */
	unsigned char in_ovr_32_idx[MAX_OVERRIDE];
/* Numero di tasti associati a variabili. */
	int n_in_ovr_var_32;
} dr_keyb_t;

/*
* Tabella di corrispondenze scancode-variabile tasto.
*/

/* Abbreviazioni di comodo */
#define SP_ KBT_STOP
#define SR_ KBT_START
#define RS_ KBT_RESET
#define CL_ KBT_CLEAR

static unsigned char key_tab[] = {
/*     0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F*/
/*0*/ 255,255,  0,  1,  2,  3,  4,  5, 24, 28,SP_,SR_,RS_,255,255,255,
/*1*/   6,  7,  8,  9, 10, 11, 25, 29,CL_,255,255,255,255,255,255, 12,
/*2*/  13, 14, 15, 16, 22, 26, 30,255,255,255,255,255, 17, 18, 19, 20,
/*3*/  21, 23, 27, 31,255,255,255,255,255,255,255,255,255,255,255,255,
/*4*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*5*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*6*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*7*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

#undef SP_
#undef SR_
#undef RS_
#undef CL_

/*
* Tabella di corrispondenze tasto esadecimale-valore.
*/

static unsigned char key_hex_tab[] = {
/*     0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F*/
/*0*/ 255,255,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,255,255,255,255,
/*1*/ 255,255, 14,255,255,255,255,255,255,255,255,255,255,255, 10,255,
/*2*/  13, 15,255,255,255,255,255,255,255,255,255,255,255,255, 12,255,
/*3*/  11,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*4*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*5*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*6*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
/*7*/ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

/*
* Variabile di stato del driver (e` ammessa una sola istanza di
* questo driver, percio` e` inutile scervellarsi per allocazioni
* dinamiche).
*/

static dr_keyb_t keyb_status;

/*
* Funzione keyb_getkey()
* ----------------------
*
*  Questo e` il cuore del driver di tastiera. Legge ed interpreta
* una sequenza di byte provenienti dal dispositivo. Se la sequenza
* e` un dato "override", viene posto ad 1 il flag "ok", che indica
* che la tastiera speciale "funziona".
*  Ogni volta che si riceve qualcosa, e` posto ad 1 il flag "reload",
* che indica all'esecutore che lo stato dei tasti e` cambiato. L'esecutore,
* attraverso il metodo "read", acquisisce il nuovo stato e spegne il flag.
*/

static void keyb_getkey(void)
{
unsigned char key,index,h,l;

	read(keyb_status.fd,&key,1);
	if (key == KB_CODE_PLCKEY) {
		read(keyb_status.fd,&key,1);
		index = key_tab[key & 0x7F];
		if (index < MAX_KEY)
			keyb_status.keys[index] = (key & 0x80) ? 0 : 1;
	}
	else if (key == KB_CODE_OVERRIDE) {
		read(keyb_status.fd,&index,1);
		read(keyb_status.fd,&h,1);
		read(keyb_status.fd,&l,1);
		index = key_hex_tab[index & 0x7F];
		h = key_hex_tab[h & 0x7F];
		l = key_hex_tab[l & 0x7F];
		if (h < 0x10 && l < 0x10) {
			l += h * 0x10;
			if (index < MAX_OVERRIDE)
				keyb_status.overrides[index] = l;
			else if (index == KEYB_VERSION_CODE)
				keyb_status.version = l;
		}
		keyb_status.ok = 1;
	}
	keyb_status.reload = 1;
}

/*
* Funzione keyb_main_rd()
* -----------------------
*
*  Questo e` il "main" del thread che sorveglia la tastiera speciale.
*/

static void * keyb_main_rd(void * arg)
{
	for (;;) {
		keyb_getkey();
	}
}

/*
* Funzione keyb_main_wr()
* -----------------------
*
*  Questo e` il "main" del thread che aggiorna i LED.
*/

static void * keyb_main_wr(void * arg)
{
	for (;;) {
		csem_wait(keyb_status.update_led_sem, (struct timeval *) 0);
/*
printf("KEYB: LED = 0x%06x\n",keyb_status.led_image);
*/
		ioctl(keyb_status.fd,KDSPCSETLED,(void *)(keyb_status.led_image));
	}
}

static int keyb_install(driver_t * d, int conf)
{
unsigned char c;
int i,rv;
int keyb_ver;
int ignore_err;
char *p;

	if (d -> instance) {
	/* ERRORE. Troppe istanze del driver. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	keyb_ver = -1;
	ignore_err = 0;
	for (ioStartResource(d); (p = ioNextResource(d)); ) {
		if (util_strneq(p,"ver=",4)) {
			p += 4;
			keyb_ver = ioGetNum(&p);
		}
		else if (util_strneq(p,"ignerr=",7)) {
			p += 7;
			ignore_err = ioGetNum(&p);
		}
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}
	
/* Imposta valori di default ragionevoli. */
	for (i = 0; i < MAX_OVERRIDE; ++i)
		keyb_status.overrides[i] = DEFAULT_OVR_VAL;
	for (i = 0; i < MAX_KEY; ++i)
		keyb_status.keys[i] = 0;
	keyb_status.ok = 0;
	keyb_status.version = 0;

	keyb_status.n_in_key_var = 0;
	keyb_status.n_in_ovr_var_8 = 0;
	keyb_status.n_in_ovr_var_32 = 0;
	keyb_status.n_out_led_var = 0;
	keyb_status.led_image = 0;
#if !AZZERA_NON_USATI_KEYB
	keyb_status.led_mask = 0;
#endif
	keyb_status.pending_led_change = 1;
	keyb_status.last_led_change = 0;
	keyb_status.led_present = 0;

/* Forza un caricamento iniziale dei valori. */
	keyb_status.reload = 1;

/* Apre il dispositivo. */
	keyb_status.fd = open("/dev/spckb0",O_RDWR);
	if (keyb_status.fd < 0) {
	/* ERRORE. Driver della tastiera speciale assente. */
		return 0;
	}

/* Invia il comando di update degli override : un fronte di salita
 del bit 4 del registro di stato dei led di tastiera. */
	c = 0x00;
	write(keyb_status.fd,&c,1);
	util_usleep(1L,0L);
	c = 0x10;
	write(keyb_status.fd,&c,1);
/* Attende che arrivi "qualcosa" dalla tastiera speciale. */
	rv = test_fd(keyb_status.fd,KB_INIT_TIMEOUT);
/* Comunque sia andata, spegne il bit 4 del registro di stato dei led. */
	if (rv == 1) { /* Se c'e` qualcosa da leggere, attende che i */
	               /* dati siano stati inviati tutti. */
		util_usleep(1L,500000L);
	}
	c = 0x00;
	write(keyb_status.fd,&c,1);
/* Se c'e` "qualcosa" da leggere, la tastiera e` presente.
  Questo "qualcosa", in genere, e` lo stato degli override. Non e`
 pero` consigliabile controllare che sia proprio questo. Se qualcuno
 giocherellasse con la tastiera proprio durante l'inizializzazione... */

	if (rv != 1 && !ignore_err) {
	/* ERRORE. Tastiera speciale assente. */
		return 0;
	}

/* Lancio del thread di controllo della tastiera speciale. */
	keyb_status.tid_rd
	 = util_start_thread(keyb_main_rd, util_execprio() - 1);
	if (keyb_ver != -1)
		keyb_status.version = keyb_ver;
	if (keyb_status.version != 0) {
		keyb_status.led_present = 1;
	/* Creazione del semaforo di controllo del thread di scrittura. */
		keyb_status.update_led_sem = csem_create_val(0);
	/* Lancio del thread incaricato di aggiornare i LED, se la versione
	 della tastiera lo permette. */
		keyb_status.tid_wr = util_start_thread(keyb_main_wr,util_execprio()-1);
	}

	d -> device = (void *) &keyb_status;
	return 1;
}

static void keyb_clear(driver_t * d)
{
register dr_keyb_t *dev = (dr_keyb_t *) d -> device;

	if (dev -> led_present) {
		dev -> led_image = 0;
		csem_signal(dev -> update_led_sem);
	}
}

static void keyb_down(driver_t * d)
{
	if (plcHaltCycle() == 0)
		keyb_clear(d);
}

static void keyb_detach(driver_t * d)
{
	keyb_status.n_in_key_var = 0;
	keyb_status.n_in_ovr_var_8 = 0;
	keyb_status.n_in_ovr_var_32 = 0;
	keyb_status.reload = 1;
	keyb_status.n_out_led_var = 0;
	keyb_status.pending_led_change = 1;
	keyb_status.last_led_change = 0;
#if !AZZERA_NON_USATI_KEYB
	keyb_status.led_mask = 0;
#endif
}

static void keyb_restart(driver_t * d)
{
	keyb_detach(d);
	keyb_status.led_image = 0;
	keyb_clear(d);
}

static int keyb_parse(driver_t * d, int i_off, devnode_t *l)
{
dr_keyb_t *dev = (dr_keyb_t *) d -> device;
int tipo;
int linea;
int mode;
char *name;

	name = l -> pname;
	mode = l -> flags;

/* Il driver KEYB prevede una sola istanza, ma non vedo perche` dovrei
 faticare per non scrivere uno "0". */

	if (i_off != 0) {
	/* ERRORE. Istanza inesistente. */
		return 0;
	}

	if (util_strneq(name,"led.",4)) {
	/* Sintassi speciale per i LED della versione compatta
	 della scheda. */
		tipo = -1;
		name += 4;
	}
	else {
	
	/* Controlla l'indice di tipo di dato (1,8 o 32), che, guarda caso,
	 e` anche la dimensione in bit della variabile. */
	
		tipo = ioGetNum(&name);
		if (tipo == -1) {
		/* ERRORE. Il tipo non e` conforme. */
			return 0;
		}
	}

/* Controlla l'indice di linea (LED, tasto o override). */

	linea = ioGetNum(&name);
	if (linea == -1) {
	/* ERRORE. Manca l'indicazione del modulo, oppure la stringa
	 non e` conforme. */
		return 0;
	}

/* Controlla la sensatezza dei dati. */

	if ((tipo != -1
	    && ((!(mode & DRIVER_MODE_INPUT)) || (mode & DRIVER_MODE_OUTPUT)))
	   ||
	    (tipo == -1
	    && ((mode & DRIVER_MODE_INPUT) || !(mode & DRIVER_MODE_OUTPUT)))) {
	/* ERRORE. Si e` richiesta una modalita` incompatibile. */
		return 0;
	}
	switch (tipo) {
	case -1:
		if (linea < 0 || linea >= MAX_LED || ! dev -> led_present) {
		/* ERRORE. Codice di LED fuori dai limiti o dispositivo
		 assente. */
			return 0;
		}
		linea += MAX_KEY + 2*MAX_OVERRIDE;
		break;
	case 1:
		if (linea < 0 || linea >= MAX_KEY) {
		/* ERRORE. Codice di tasto fuori dai limiti. */
			return 0;
		}
		break;
	case 8:
	case 32:
		if (linea < 0 || linea >= MAX_OVERRIDE) {
		/* ERRORE. Codice di override fuori dai limiti. */
			return 0;
		}
		if (tipo == 8)
			linea += MAX_KEY;
		else
			linea += MAX_KEY + MAX_OVERRIDE;
		break;
	default:
		/* ERRORE. Codice di tipo imprevisto. */
		return 0;
	}

	l -> pname = name;
	l -> ideep = 3;
	l -> next.spec[0] = 0;
	l -> next.spec[1] = linea;

	return (l -> nbit = (tipo == -1 ? 1 : tipo));
}

static int keyb_attach(driver_t * d, devnode_t *l, void * var)
{
dr_keyb_t *dev = (dr_keyb_t *) d -> device;
int i,*lim,mlim,mo,v;
unsigned char *idx,**dest;
long **ldest;

	ldest = (long **) 0;
	dest = (unsigned char **) 0;

	mo = l -> next.spec[1];

	if (mo < MAX_KEY) {
		v = mo;
		lim = &dev -> n_in_key_var;
		idx = dev -> in_key_idx;
		dest = dev -> in_key_var;
		mlim = MAX_KEY;
	}
	else if (mo < MAX_KEY + MAX_OVERRIDE) {
		v = mo - MAX_KEY;
		lim = &dev -> n_in_ovr_var_8;
		idx = dev -> in_ovr_8_idx;
		dest = dev -> in_ovr_var_8;
		mlim = MAX_OVERRIDE;
	}
	else if (mo < MAX_KEY + 2*MAX_OVERRIDE) {
		v = mo - (MAX_KEY + MAX_OVERRIDE);
		lim = &dev -> n_in_ovr_var_32;
		idx = dev -> in_ovr_32_idx;
		ldest = dev -> in_ovr_var_32;
		mlim = MAX_OVERRIDE;
	}
	else {
		v = mo - (MAX_KEY + 2*MAX_OVERRIDE);
		lim = &dev -> n_out_led_var;
		idx = dev -> out_led_idx;
		dest = dev -> out_led_var;
		mlim = MAX_LED;
	}
	for (i = 0; i < *lim; ++i) {
		if (v == idx[i]) {
		/* ERRORE. Linea gia` assegnata. */
			return 0;
		}
	}
	if (i >= mlim) {
	/* ERRORE. Probabilissimo sputtanamento. */
		return 0;
	}

	if (ldest)
		ldest[i] = (long *) var;
	else
		dest[i] = (unsigned char *) var;

	idx[i] = v;

	++*lim;

/* Per i LED ci vuole un'elaborazioncina aggiuntiva : bisogna costruire
 la maschera di accesso al LED. Fara` comodo in "keyb_write". */

	if (mo >= MAX_KEY + 2*MAX_OVERRIDE) {
#if !AZZERA_NON_USATI_KEYB
		dev -> led_mask |=
#endif
		dev -> out_led_mask[i] = 1 << v;
	}

	return 1;
}

static void keyb_read(driver_t * d)
{
register dr_keyb_t *dev = (dr_keyb_t *) d -> device;
int i;

	if (dev -> reload) {
		for (i = 0; i < dev -> n_in_key_var; ++i)
			*(dev -> in_key_var[i])
				= dev -> keys[dev -> in_key_idx[i]];
		for (i = 0; i < dev -> n_in_ovr_var_8; ++i)
			*(dev -> in_ovr_var_8[i])
				= dev -> overrides[dev -> in_ovr_8_idx[i]];
		for (i = 0; i < dev -> n_in_ovr_var_32; ++i)
			*(dev -> in_ovr_var_32[i])
				= dev -> overrides[dev -> in_ovr_32_idx[i]];
		dev -> reload = 0;
	}
}

static void keyb_write(driver_t * d)
{
register dr_keyb_t *dev = (dr_keyb_t *) d -> device;
register int i;
register unsigned long v;

	if (! dev -> led_present)
		return;

/* Calcola l'immagine dei LED impostata dal programma PLC. */
#if AZZERA_NON_USATI_KEYB
	v = 0;
#else
	v = dev -> led_image & ~(dev -> led_mask);
#endif
	for (i = dev -> n_out_led_var; --i >= 0; )
		if (*(dev -> out_led_var[i]))
			v |= dev -> out_led_mask[i];

/* Se diversa da quella corrente, ne prenota l'aggiornamento. */
	if (v != dev -> led_image) {
		dev -> pending_led_change = 1;
		dev -> led_image = v;
	}

/* Se e` stato prenotato un aggiornamento, ed e` trasorso abbastanza tempo
 dall'aggiornamento precedente, attiva il thread di scrittura, ritira
 la prenotazione e memoriza l'ora corrente. Il thread chiamera` l'apposito
 servizio di kernel. */

	if (dev -> pending_led_change) {
		v = plcTime(); /* v = cache dell'ora corrente */
		if (v - dev -> last_led_change >= LED_UPDATE_RATE) {
			dev -> last_led_change = v;
			csem_signal(dev -> update_led_sem);
			dev -> pending_led_change = 0;
		}
	}
}

static int keyb_list(driver_t * d, devnode_t * l)
{
register dr_keyb_t *dev = (dr_keyb_t *) d -> device;
int led,c,p1,ok;

	l -> name[0] = '\0';
	l -> nbit = 0;
	l -> flags = 0;

	switch (l -> ideep) {
	case 1:
		util_safe_sprintf(l -> comment,sizeof(l -> comment),
		                  "ver=%x",dev -> version);
		break;
	case 2:
		if (l -> next.spec[0] == 0
		 && (dev -> ok || dev -> led_present)) {
			l -> nbit = 0;
			l -> tree.spec[0] = 0;
			l -> tree.spec[1] = 0;
			l -> flags = DRIVER_MODE_LIST;
			util_safe_sprintf(l -> name, sizeof(l -> name),
				  "%s.%d", d -> op.name, d -> instance);
			++(l -> next.spec[0]);
		}
		break;
	case 3:
		ok = c = 0;
		while (!ok) {
			led = 0;
			p1 = l -> next.spec[1];
			if (p1 < MAX_KEY) {
				if ( (ok = dev -> ok) ) {
					l -> nbit = 1;
					c = p1;
					++(l -> next.spec[1]);
				}
				else {
					l -> next.spec[1] = MAX_KEY+2*MAX_OVERRIDE;
				}
			}
			else if (p1 < MAX_KEY + MAX_OVERRIDE) {
				ok = 1;
				l -> nbit = 8;
				c = p1 - MAX_KEY;
				++(l -> next.spec[1]);
			}
			else if (p1 < MAX_KEY + 2*MAX_OVERRIDE) {
				ok = 1;
				l -> nbit = 32;
				c = p1 - MAX_KEY - MAX_OVERRIDE;
				++(l -> next.spec[1]);
			}
			else if (p1 < MAX_KEY + 2*MAX_OVERRIDE + MAX_LED) {
				if ( (ok = dev -> led_present) ) {
					l -> nbit = 1;
					c = p1 - MAX_KEY - 2*MAX_OVERRIDE;
					led = 1;
					++(l -> next.spec[1]);
				}
				else {
					break;
				}
			}
			else {
				break;
			}
		}
		if (ok) {
			l -> flags = led ? DRIVER_MODE_OUTPUT
			                 : DRIVER_MODE_INPUT;
			if (led)
				util_safe_sprintf(l -> name,sizeof(l -> name),
					"%s.%d.LED.%d",
					d -> op.name, d -> instance, c);
			else
				util_safe_sprintf(l -> name,sizeof(l -> name),
					"%s.%d.%d.%d",
					d->op.name,d->instance,l->nbit,c);
		}
		break;
	default:
		return -1;
	}
	return 0;
}

static int keyb_show(driver_t * d, devnode_t *l, void *dest)
{
register dr_keyb_t *dev = (dr_keyb_t *) d -> device;
int p1;

	p1 = l -> next.spec[1];
	if (p1 < MAX_KEY) {
		*(char *)dest = dev -> keys[p1];
		l -> nbit = 1;
		return 1;
	}
	else if (p1 < MAX_KEY + MAX_OVERRIDE) {
		p1 -= MAX_KEY;
		*(unsigned char *)dest = dev -> overrides[p1];
		l -> nbit = 8;
		return 1;
	}
	else if (p1 < MAX_KEY + 2*MAX_OVERRIDE) {
		p1 -= MAX_KEY + MAX_OVERRIDE;
		*(long *)dest = dev -> overrides[p1];
		l -> nbit = 32;
		return 1;
	}
	else if (p1 < MAX_KEY + 2*MAX_OVERRIDE + MAX_LED) {
		p1 -= MAX_KEY + 2*MAX_OVERRIDE;
		*(char *)dest = (dev -> led_image & (1L << p1)) != 0;
		l -> nbit = 1;
		return 1;
	}
	return 0;
}


driver_op_t driver_keyb = {
	"keyb",
	keyb_install,
	keyb_restart,
	keyb_parse,
	keyb_list,
	keyb_attach,
	keyb_detach,
	(int (*)(driver_t *)) 0, /* keyb_check */
	(void (*)(driver_t *)) 0, /* keyb_trigger */
	(void (*)(driver_t *)) 0, /* keyb_up */
	keyb_down,
	(void (*)(driver_t *)) 0, /* keyb_close */
	keyb_clear, /* keyb_fatal */
	keyb_read,
	keyb_write,
	keyb_show,
	(int (*)(driver_t *,void *,int,void *)) 0, /* keyb_extension */
};

