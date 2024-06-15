/*
* @(#) dr_bgage.c 2.1 Fri Apr  3 19:49:28 MET DST 1998
*
* Driver della scheda Bus Gage (uso Bobbio).
*
* Forma dei parametri specifici :
*
*   Ingresso ADC o uscita DAC (16 bit) :
*
* Esempio di nome di variabile completo (16 bit):
*
*       bgage.0
*         |   | 
*         |   DAC o ADC della scheda 0 (l'unica)...
*         di tipo Bus Gage.
*
* --------------------------------------------------------------------
* Organizzazione dei dati descriventi i segnali:
*
*  ideep = 2 : spec[0] : 0 = ingresso ADC, 1 = uscita DAC.
*
* --------------------------------------------------------------------
* 
* 09/02/98 GG 1.0 Prima stesura.
* 11/03/98 GG 1.1 Aggiunto il metodo "detach".
* 13/03/98 GG 1.2 Ridotto il codice: bastava usare "util_ahtol".
* 24/03/98 GG 1.3 Aggiunto il metodo "list". Evitata l'installazione
*             del driver se ioaddr=0.
* 01/04/98 GG 2.0 Rivoluzione dovuta a modifiche profonde in "drivers.h".
* 03/04/98 GG 2.1 Aggiunto il metodo "show".
*/

#include <stdio.h>
#include <string.h>

#include "qplc.h"

#include "util.h"

/*
* Struttura di stato del driver.
*/

typedef struct _dr_bgage {
/* Porta di I/O della scheda. */
	int ioaddr;
/* Indirizzo della variabile associata all'ingresso. */
	short * in_addr;
/* Indirizzo della variabile associata all'uscita. */
	short * out_addr;
/* Flag di dispositivo in timeout. */
	int timeout;
/* Valore corrente dell'uscita. */
	int out_cache;
} dr_bgage_t;

/*
* Tabella di corrispondenze scancode-variabile tasto.
*/

/*
* Variabile di stato del driver (e` ammessa una sola istanza di
* questo driver, percio` e` inutile scervellarsi per allocazioni
* dinamiche).
*/

static dr_bgage_t bgage_status;

static int bgage_install(driver_t * d, int conf)
{
int ioaddr;
char *p;

	if (d -> instance) {
	/* ERRORE. Troppe istanze del driver. */
		return 0;
	}

/* Lettura dei parametri di configurazione. */

	ioaddr = 0x340;
	for (ioStartResource(d); (p = ioNextResource(d)); ) {
		if (util_strneq(p,"ioaddr=",7)) {
			p += 7;
			ioaddr = util_ahtol(p,&p);
		}
		else {
/* Che fare ? Sprecare un errore ? */
		}
	}
	
	if (ioaddr == 0)
		return 0;

	bgage_status.ioaddr = ioaddr;
	bgage_status.in_addr = (short *)0;
	bgage_status.out_addr = (short *)0;
	bgage_status.timeout = 0;
	bgage_status.out_cache = 0x7FFFFFFF;
	d -> device = (void *) &bgage_status;
	return 1;
}

static void bgage_restart(driver_t * d)
{
dr_bgage_t *dev = (dr_bgage_t *) d -> device;

	dev -> in_addr = (short *)0;
	dev -> out_addr = (short *)0;
	dev -> out_cache = 0x7FFFFFFF;
	dev -> timeout = 0;
}

static int _bgage_in(dr_bgage_t *dev)
{
int i,rv;
int a = dev -> ioaddr;

	util_outb(0x58, a);
	for (i = 0; util_inb(a + 6) & 0x01; ++i) {
		if (i > 100) {
			dev -> timeout = 1;
			break;
		}
	}
	rv = util_inb(a + 1) << 8;
	return rv | (util_inb(a) & 0xFF);
}

static void _bgage_out(dr_bgage_t *dev, int v)
{
int a = dev -> ioaddr;

	util_outb(v & 0xFF, a + 2);
	util_outb((v >> 8) & 0xFF, a + 3);
	util_outb(0, a + 4);
}

static int bgage_parse(driver_t * d, int i_off, devnode_t *l)
{
dr_bgage_t *dev = (dr_bgage_t *) d -> device;

/* Il driver BGAGE prevede una sola istanza, ma non vedo perche` dovrei
 faticare per non scrivere uno "0". */

	if (i_off != 0) {
	/* ERRORE. Istanza inesistente. */
		return 0;
	}

/* Indirizzo di base == 0 indica "dispositivo assente". */

	if (dev -> ioaddr == 0) {
	/* ERRORE. Dispositivo inesistente. */
		return 0;
	}

/* Tenta una lettura per verificare l'esistenza del dispositivo. */
	dev -> timeout = 0;
	_bgage_in(dev);
	if (dev -> timeout) {
	/* ERRORE. Dispositivo inesistente. */
		dev -> timeout = 0;
#if 0
	/* E d'ora in poi non ci si provera` piu`. */
		dev -> ioaddr = 0;
#endif
		return 0;
	}

	l -> next.spec[0] = (l -> flags & DRIVER_MODE_OUTPUT) != 0;
	l -> ideep = 2;
	return (l -> nbit = 16);
}

static int bgage_attach(driver_t * d, devnode_t *l, void * var)
{
dr_bgage_t *dev = (dr_bgage_t *) d -> device;
short ** dest;

	if (l -> next.spec[0])
		dest = &(dev -> out_addr);
	else
		dest = &(dev -> in_addr);

	if (*dest) {
	/* ERRORE. Linea gia` assegnata. */
		return 0;
	}

	*dest = var;

	return 1;
}

static void bgage_read(driver_t * d)
{
register dr_bgage_t *dev = (dr_bgage_t *) d -> device;

	if (dev -> in_addr) {
		*(dev -> in_addr) = _bgage_in(dev);
	}
}

static void bgage_write(driver_t * d)
{
register dr_bgage_t *dev = (dr_bgage_t *) d -> device;

	if (dev -> out_addr && *(dev -> out_addr) != dev -> out_cache) {
		_bgage_out(dev,dev -> out_cache = *(dev -> out_addr));
	}
}

static int bgage_check(driver_t * d)
{
	return ((dr_bgage_t *) d -> device) -> timeout == 0;
}

static int bgage_list(driver_t * d, devnode_t * l)
{
dr_bgage_t *dev = (dr_bgage_t *) d -> device;

	if (l -> ideep == 1) {
		util_safe_sprintf(l -> comment, sizeof(l -> comment),
		                  "BusGage-io=0x%x", dev -> ioaddr);
		return 0;
	}
	if (l -> ideep != 2)
		return -1;
	if (l -> next.spec[0] > 1) {
		l -> name[0] = '\0';
		l -> nbit = 0;
		l -> flags = 0;
		return 0;
	}
	util_safe_sprintf(l -> name, sizeof(l -> name),
	                  "%s.%d", d -> op.name, d -> instance);
	l -> flags = l -> next.spec[0] ? DRIVER_MODE_OUTPUT : DRIVER_MODE_INPUT;
	l -> nbit = 16;
	++(l -> next.spec[0]);
	return 0;
}

static int bgage_show(driver_t * d, devnode_t * l, void *dest)
{
dr_bgage_t *dev = (dr_bgage_t *) d -> device;

	if (l -> next.spec[0])
		*(short *)dest = dev -> out_cache;
	else
		*(short *)dest = _bgage_in(dev); /* Speriamo... */
	l -> nbit = 16;
	return 1;
}

driver_op_t driver_bgage = {
	"bgage",
	bgage_install,
	bgage_restart,
	bgage_parse,
	bgage_list,
	bgage_attach,
	bgage_restart, /* bgage_detach */
	bgage_check,
	(void (*)(driver_t *)) 0, /* bgage_trigger */
	(void (*)(driver_t *)) 0, /* bgage_up */
	(void (*)(driver_t *)) 0, /* bgage_down */
	(void (*)(driver_t *)) 0, /* bgage_close */
	(void (*)(driver_t *)) 0, /* bgage_fatal */
	bgage_read,
	bgage_write,
	bgage_show,
	(int (*)(driver_t *,void *,int,void *)) 0, /* bgage_extension */
};

