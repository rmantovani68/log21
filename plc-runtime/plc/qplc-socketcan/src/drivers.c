/*
* @(#) drivers.c 3.6 Thu Nov 19 19:39:50 MET 1998
*
*  Gestione dei dispositivi di QPLC.
*  I dispositivi sono rappresentati da una stringa cosi` composta :
*
*     <nome del dispositivo>.[<istanza>.]<stringa>.[<stringa>...]
*
*  Il nome del dispositivo identifica il paricolare driver
* che si intende attivare, "istanza" e` un numero che identifica l'istanza
* del dispositivo, cio` che segue ha un significato
* che dipende esclusivamente dal dispositivo.
*
*  Tutti i driver riconoscono i seguenti servizi :
*
*  install  : chiamata al lancio di QPLC, esegue l'allocazione delle
*             aree necessarie al driver.
*  restart  : chiamata ad ogni GO per inizializzare il dispositivo,
*             prima delle "attach".
*  check    : chiamata ad ogni ciclo di PLC per controllare lo stato del
*             dispositivo.
*  trigger  : Chiamata ad ogni ciclo per operazioni di "mantenimento"
*             del dispositivo (tipicamente, rinfresco di watchdog).
*             La funzione e` chiamata anche quando si e` in HALT, a meno
*             che non si sia verificato un errore fatale.
*  parse    : controlla la correttezza del nome di variabile dato (parte
*             dipendente dal dispositivo), ed eventualmente prepara il
*             terreno per la "attach".
*  attach   : associa una variabile di input o di output
*             alla "linea" descritta dall'ultima "parse".
*  detach   : ritira tutte le associazioni realizzate da "attach".
*  up       : chiamata all'entrata in GO, dopo la "restart" e le "attach".
*  down     : chiamata tipicamente all'entrata in HALT, per portare in uno
*             stato sicuro gli I/O.
*  close    : chiamata nella fase finale dell'entrata in HALT dal supervisore,
*             per eseguire azioni legate al sistema e potenzialmente bloccanti.
*  fatal    : chiamata in caso di emergenza per portare in uno stato
*             sicuro i dispositivi. Il suo effetto puo` essere irreversibile.
*  read     : chiamata all'inizio di ogni ciclo per acquisire lo stato
*             degli input.
*  write    : chiamata alla fine del ciclo per aggiornare gli output.
*  list     : chiamata dalla ioList per ottenere la descrizione dei
*             fratelli e dei figli di un nodo. 
*  show     : chiamata dalla ioShow per ottenere in modo non invasivo,
*             se possibile, lo stato di un segnale descritto nella forma
*             data da "ioList" o "ioParse".
*  extension: chiamata durente il ciclo dalla funzione "ioExtendedOp" per
*             eseguire operazioni che non rientrano nel modello ordinario
*             del PLC. Vale 1 in caso di successo, 0 in caso di errore.
*             Ha come perametri il driver, il valore registrato con
*             "ioRegisterExtension", e due parametri dati dal chiamante
*             (un codice di comando ed un dato arbitrario).
*
*
*  Ogni driver e` descritto da un handle, che contiene, tra le altre
* cose, i metodi associati.
*
* 20/05/96 GG 1.0 Prima stesura.
* 10/06/96 GG 1.1 Corretto un buco clamoroso nella "ioGetNum".
* 23/06/96 GG 1.2 Resa "case insensitive" la ricerca del nome del driver.
* 28/08/96 GG 1.3 Aggiunto un parameto al metodo "parse", per permettere
*                 una certa elasticita` nell'interpretazione del campo
*                 "istanza" della stringa di descrizione del driver.
* 19/09/96 GG 1.4 Aggiunto un meccanismo di visualizzazione degli errori
*                 rilevati dalle "check".
* 15/04/97 GG 1.5 Aggiunta la gestione delle variabili di configurazione
*                 provenienti dalla linea di comando (funzioni ioStartResource
*                 ed ioNextResource).
* 10/07/97 GG 1.6 L'assenza dell'indice di istanza in "ioAttach" non e` piu`
*                 un errore. Si assume che sia la 0.
* 16/07/97 GG 2.0 Aggiunti i metodo "up" e "close" dei driver, e le funzioni
*                 "ioUp" ed "ioClose" per chiamarli.
*                 Aggiunto un controllo che permette di
*                 non definire i metodi "read" e "write" anche se il driver
*                 accetta associazioni rispettivamente in lettura e scrittura.
* 05/09/97 GG 2.1 Portato a 100 il numero di errori di I/O registrabili.
* 11/03/98 GG 2.2 Aggiunto il metodo "detach", che ritira tutte le
*                 associazioni realizzate con "attach". Aggiunta ovviamente
*                 anche la funzione "ioDetach".
* 24/03/98 GG 2.3 Aggiunto il metodo "list", in vista dell'aggiunta del
*                 servizio di lista dei dispositivi collegati.
*                 Ridotto a 16 il numero di errori di I/O registrabili.
*                 Bastano.
*                 Aggiunta la funzione "ioList", interfaccia verso i
*                 metodi "list" dei driver.
*                 Aggiunto il parametro "conf" al metodo "install" e
*                 alla funzione "ioInstall".
* 01/04/98 GG 3.0 Sconvolto tutto. I metodi "attach", "parse" e "list"
*                 ora usano gli stessi tipi. Pericolo di sfighe.
* 03/04/98 GG 3.1 Aggiunto il metodo "show", e le funzioni "ioShow"
*                 e "ioParse".
* 17/04/98 GG 3.2 La funzione "ioAttach" (e solo lei) riconosce ora la
*                 stringa "NULL" come dispositivo speciale, che e` associabile
*                 sempre e comunque a qualsiasi cosa.
* 23/09/98 GG 3.3 Raccolte in una struttura di stato tutte le variabili
*                 statiche sparse qua e la`.
*                 Aggiunta la gestione del flag "main_status.simul", che
*                 fa si` che ogni "ioAttach" sia realizzata col dispositivo
*                 nullo anziche` con quello richiesto.
*                 Aggiunta una lista di associazione di I/O "in chiaro",
*                 ad uso del debugger. La funzione "ioGetBindings" ne fornisce
*                 il puntatore. La funzione "ioFindBinding" controlla
*                 che il descrittore di associazione dato faccia parte della
*                 lista.
* 01/10/98 GG 3.4 Resa pubblica la funzione "ioCanonify".
*                 Aggiunta l'interrogazione del database dei segnali di I/O
*                 (funzioni del nuovo oggetto "ioconf.c").
* 09/11/98 GG 3.5 Modificato il comportamento di "ioAttach" in presenza di
*                 segnali non previsti dal file di configurazione: ora e`
*                 assegnata loro la proprieta` indicata dall'opzione
*                 "-defaultio", o, in sua assenza, "R". L'opzione ha effetto
*                 anche in assenza totale di faile di configurazione.
* 19/11/98 GG 3.6 Modificata la visualizzazione degli errori di I/O per
*                 poter segnalare anche il modulo IOS che sbacchetta.
*                 L'indirizzo del modulo e` codificato nei sedici bit alti
*                 del dato. Trattasi di una rivoltante pezza.
* 11/05/01 GG 3.7 Aggiunta la funzione "ioClearErrors", che cancella
*                 gli errori di I/O memorizzati. Deve essere chiamata
*                 all'entrata in GO e all'entrata in HALT.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "qplc.h"

#include "util.h"

#include "ioconf.h"

/* Numero di elementi della lista dei driver allocati in un sol colpo. */
#define LEN_DRIVER_BLOCK 10
/* Numero di elementi della lista delle associazioni allocati
 in un sol colpo. */
#define LEN_IOBIND_BLOCK 100
/* Dimensione della tabella degli errori di I/O. */
#define MAX_DR_ERRS 16

/*
* Tabella di transito dei dati associati agli errori rilevati dalle
* funzioni "check".
*/
struct dr_err_t {
	driver_t *driver;
	unsigned int code;
	long data;
	int new_entry;
};

/*
* Struttura di stato del sistema di I/O.
*/
struct drio_status_t {
/*
* Campi associati alla memorizzazione degli errori di I/O.
*/
	struct dr_err_t driver_err_list[MAX_DR_ERRS];
	int driver_err_count;
	int driver_pred_err_count;
/*
* Freelist per la gestione dinamica dei descrittori di driver.
*/
	driver_t * io_freelist;
/*
* Puntatore alla fine della stringa delle risorse proveniente dalla
* linea di comando. E` inizializzato alla prima chiamata di "ioInstall".
*  E` necessaria perche` la stringa e` spezzata nelle sue componenti,
* che risultano cosi` terminanti nel carattere '\0'.
*/
	char * io_resource_end;
/*
* Freelist per la gestione dinamica delle associazioni di I/O.
*/
	iobind_t * iobind_freelist;
/*
* Base della lista delle associazioni di I/O.
*/
	iobind_t * iobind_list;
};
static struct drio_status_t drio_sts = { { { 0 } }, 0, 0, 0, 0, 0, 0, };

/*
* Funzione "ioClearErrors"
* ------------------------
*
*  Questa funzione cancella gli errori registrati nella lista degli
* errori di I/O rilevato dai driver. Deve essere chiamata all'entrata
* in GO.
*/

void ioClearErrors(void)
{
	drio_sts.driver_pred_err_count = drio_sts.driver_err_count = 0;
}

/*
* Funzione "ioAddError"
* ---------------------
*
*  Questa funzione registra i dati relativi ad un problema hardware
* rilevato dal driver "dr". Se l'errore non e` ancora stato segnalato,
* e` aggiunto alla lista, e provochera` una visualizzazione.
*/

void ioAddError(driver_t *dr, int code, long data)
{
register int i;
register struct dr_err_t *p;

	for (i = drio_sts.driver_err_count, p = drio_sts.driver_err_list;
	     i--;
	     ++p) {
		if (p -> driver == dr
		    && p -> code == code
		    && p -> data == data)
			return;
	}

	if (drio_sts.driver_err_count
	    < sizeof(drio_sts.driver_err_list)
	     /sizeof(drio_sts.driver_err_list[0])) {
		p -> driver = dr;
		p -> code = code;
		p -> data = data;
		p -> new_entry = 1;
		++drio_sts.driver_err_count;
	}
}

/*
* Funzione "ioStartResource"
* --------------------------
*
*  Questa funzione inizia una sequenza di letture di variabili di
* configurazione inerenti al dispositivo "d".
*/

void ioStartResource(driver_t *d)
{
	d -> resource_pointer = main_status.devconf;
}

/*
* Funzione "ioNextResource"
* -------------------------
*
*  Questa funzione fornisce il puntatore alla prima variabile di
* configurazione inerente al dispositivo "d".
*/

char * ioNextResource(driver_t *d)
{
char *p,*name,*rv;
int n,l;

	name = d -> op.name;
	l = strlen(name);
	for (rv = (char *) 0, p = d -> resource_pointer;
	     p < drio_sts.io_resource_end && !rv; ) {
		if (util_strneq(name,p,l) && *(p += l) == '.') {
			++p;
			n = ioGetNum(&p);
			if (n == -1 || n == d -> instance)
				rv = p;
		}
		while (*(p++))
			;
		d -> resource_pointer = p;
	}
	return rv;
}

/*
* Funzione "ioClearBindings"
* --------------------------
*
*  Questa funzione svuota la lista delle associazioni. Deve essere
* chiamata all'interno di "ioRestart".
*/

static void ioClearBindings(void)
{
	util_add_chain((void *)(drio_sts.iobind_list),
	           (void **)&drio_sts.iobind_freelist,
		   GET_STRUCT_OFFSET(iobind_t,next));
	drio_sts.iobind_list = (iobind_t *) NULL;
}

/*
* Funzione "ioCanonify"
* ---------------------
*
*  Normalizzazione di un nome di I/O: "src" e` convertito in minuscolo e sono
* eliminati gli zeri superflui dei campi numerici. Il risultato e`
* scaricato in "dest", di dimensioene "len" (compreso NUL finale).
*  La funzione elimina anche l'eventuale "_" iniziale dal nome, e restituisce
* 1 se l'eliminazione e` avvenuta, altrimenti 0.
*/

int ioCanonify(char *dest, int len, char *src)
{
int rv,sts;
char c;

/* Eliminazione dell'eventuale "_" iniziale. */
	if ( (rv = (*src == '_')) )
		++src;

	sts = 0;

	while (len > 1) {

		c = *(src++);

	/* Trasforma le sequenze del tipo ".00000" in ".0". */
		if (c == '0') {
			if (sts == 0) { /* Questo e` lo stato "inizio campo". */
				sts = 1; /* Passa a "zero sospeso". */
				continue;
			}
			else if (sts == 1) /* Stato "zero sospeso". */
				continue;
			/* Negli altri stati, lo zero puo` passare subito. */
		}
		else if (c == '.' || c == '\0') {
			if (sts == 1) {
			/* Siamo nello stato "zero sospeso". E siamo in fondo
			 al campo. Bisogna emettere lo zero mancante. */
				*(dest++) = '0';
			/* ...e pazienza se non c'e` piu` spazio. */
				if (--len <= 1)
					break;
			}
			if (c == '\0') /* NUL finale: si esce. */
				break;
			sts = 0; /* "." -> stato "inizio campo". */
		}
		else
			sts = 2; /* Altro carattere. Eliminazione sospesa. */

	/* Trasferisce il carattere e lo converte in minuscolo. */

		*(dest++) = tolower(c);
		--len;
	}

	*dest = '\0';

	return rv;
}

/*
* Funzione "ioStoreBinding"
* -------------------------
*
*  Questa funzione aggiunge un elemento alla lista delle associazioni di
* I/O "in chiaro". Dovrebbe essere chiamata all'interno di "ioAttach"
* in caso di successo dell'associazione.
*  Per il significato dei parametri si rimanda alla descrizione di "ioAttach".
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
* NOTA: La registrazione avviene solo se main_status.remote_debug e`
* diverso da zero, altrimenti la funzione vale 1 ma non fa nulla.
*/

static int ioStoreBinding(char * name, void * addr, int size, int mode)
{
iobind_t *p;
char *s;
int i;

/* Deve essere abilitato il debugger remoto, altrimenti questa
 funzionalita` non serve a niente. */
	if (! main_status.remote_debug)
		return 1;

/* Alloca un elemento. */
	p = (iobind_t *) util_alloc_cell((void **) &drio_sts.iobind_freelist,
	                             sizeof(iobind_t),
	                             GET_STRUCT_OFFSET(iobind_t,next),
	                             LEN_IOBIND_BLOCK);
	if (! p) {
	/* ERRORE. Non c'e` abbastanza memoria. */
		plcError(108,"ioStoreBinding");
		return 0;
	}

/* Registra i dati. si suppone che il nome sia gia` in forma canonica. */

	p -> flags = mode;
	p -> nbit = size;
	strncpy(p -> name, name, sizeof(p -> name) - 1);
	p -> name[sizeof(p -> name) - 1] = '\0';
	p -> addr = addr;

/* Aggiunge l'elemento alla lista. */

	util_add_cell((void *) p, 
		   (void **) &drio_sts.iobind_list,
		   GET_STRUCT_OFFSET(iobind_t,next));

	return 1;
}

/*
* Funzione "ioGetBindings"
* ------------------------
*
*  Questa funzione fornisce il puntatore alla lista delle associazioni
* in chiaro.
*/

iobind_t * ioGetBindings(void)
{
	return drio_sts.iobind_list;
}

/*
* Funzione "ioFindBinding"
* ------------------------
*
*  Questa funzione fornisce il puntatore all'elemento di chiave "key"
* della lista di associazione, oppure NULL se la chiave non e` presente.
* Attualmente la chiave non e` altro che l'indirizzo stesso.
*/

iobind_t * ioFindBinding(unsigned long key)
{
iobind_t *d;

	d = (iobind_t *) key;

	if (util_find_cell((void *)d,
	                   (void **)&drio_sts.iobind_list,
	                   GET_STRUCT_OFFSET(iobind_t,next))) {
		return d;
	}
	else {
		return (iobind_t *) NULL;
	}
}

/*
* Funzione "ioRead"
* -----------------
*
*  Questa funzione e` chiamata all'inizio di ogni ciclo di PLC
* per leggere gli input.
*/

void ioRead(void)
{
register driver_t *p;

	for (p = main_status.drivers.in_list; p; p = p -> next_in)
		p -> op.read(p);
}


/*
* Funzione "ioWrite"
* -----------------
*
*  Questa funzione e` chiamata alla fine di ogni ciclo di PLC
* per aggiornare gli output.
*/

void ioWrite(void)
{
register driver_t *p;

	for (p = main_status.drivers.out_list; p; p = p -> next_out)
		p -> op.write(p);
}

/*
* Funzione "ioCheck"
* -----------------
*
*  Questa funzione e` chiamata alla fine di ogni ciclo di PLC, dopo
* l'aggiornamento degli output, per controllare lo stato dei dispositivi.
*  Vale 0 in caso di errore, altrimenti il valore e` diverso da zero.
*/

int ioCheck(void)
{
register int rv = 1;
register driver_t *p;

	for (p = main_status.drivers.check_list; p; p = p -> next_check)
		rv = (p -> error = p -> op.check(p)) && rv;

	if ((! rv)
	 && (drio_sts.driver_err_count != drio_sts.driver_pred_err_count)) {

	register int i;
	register struct dr_err_t *q;

		q = &drio_sts.driver_err_list[drio_sts.driver_pred_err_count];

		for (i = drio_sts.driver_err_count
		       - drio_sts.driver_pred_err_count;
		     i--;
		     ++q) {
			if (q -> new_entry) {
				q -> new_entry = 0;
				if (q -> data < 0) {
					plcError(130 + q -> code,
					         "%s.%d",
						 q -> driver -> op.name,
						 (long)(q->driver->instance
					                - q->data));
				}
				else {
					plcError(130 + q -> code,
					         "%s.%d.%d.%d",
						 q -> driver -> op.name,
						 (long)(q->driver->instance),
					         (q -> data) & 0xFFFF,
					         ((q -> data)>>16) & 0xFFFF);
				}
			}
		}

		drio_sts.driver_pred_err_count = drio_sts.driver_err_count;
	}


	return rv;
}

/*
* Funzione "ioRestart"
* --------------------
*
*  Questa funzione e` chiamata ogni volta che il PLC entra in GO.
*/

void ioRestart(void)
{
register driver_t *p;

/* Ripulisce la lista delle associazioni "in chiaro". */
	ioClearBindings();

/* Ad ogni buon conto, vuota la tabella delle associazioni
 a segnali speciali. */
	main_status.drivers.n_ext = 0;

	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p -> op.restart)
			p -> op.restart(p);
/* Azzera le liste globali di input e di output. */
	main_status.drivers.in_list
	 = main_status.drivers.out_list
	 = (driver_t *) 0;
}

/*
* Funzione "ioTrigger"
* --------------------
*
*  Questa funzione e` chiamata alla fine di ogni ciclo di PLC, dopo
* l'aggiornamento degli output, per il refresh dei dispositivi che lo
* richiedano. La funzione e` chiamata anche quando il PLC e` in HALT.
*/

void ioTrigger(void)
{
register driver_t *p;

	for (p = main_status.drivers.trigger_list; p; p = p -> next_trigger)
		p -> op.trigger(p);
}

/*
* Funzione "ioDetach"
* -------------------
*
*  Questa funzione e` chiamata ogni volta che sia necessario ridefinire
* le associazionei di I/O.
*/

void ioDetach(void)
{
register driver_t *p;

	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p -> op.detach)
			p -> op.detach(p);
/* Azzera le liste globali di input e di output. */
	main_status.drivers.in_list
	 = main_status.drivers.out_list
	 = (driver_t *) 0;
}

/*
* Funzione "ioUp"
* ---------------
*
*  Questa funzione e` chiamata immediatamente prima dall'entrata in RUN
* del PLC, dopo che sono state realizzate tutte le associazioni di I/O.
*/

void ioUp(void)
{
register driver_t *p;

	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p -> op.up)
			p -> op.up(p);
}

/*
* Funzione "ioDown"
* -----------------
*
*  Questa funzione e` chiamata all'uscita dalla modalita` RUN del PLC,
* per azzerare gli output in modo sicuro.
*/

void ioDown(void)
{
register driver_t *p;

	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p -> op.down)
			p -> op.down(p);
}

/*
* Funzione "ioClose"
* ------------------
*
*  Questa funzione e` chiamata all'uscita dalla modalita` RUN del PLC,
* per eseguire le operazioni "lente" che possono coinvolgere risorse di
* sistema, attese indefinite... A differenza della "ioDown", e` eseguita
* dal thread supervisore, percio` non risente delle limitazioni di
* tempo tipiche dell'esecutore.
*/

void ioClose(void)
{
register driver_t *p;

	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p -> op.close)
			p -> op.close(p);
}

/*
* Funzione "ioFatal"
* ------------------
*
*  Questa funzione e` chiamata in caso di emergenza, errori fatali
* ed altro. Il suo effetto puo` essere irreversibile.
*/

void ioFatal(void)
{
register driver_t *p;

	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p -> op.fatal)
			p -> op.fatal(p);
}

/*
* Funzione "ioInstall"
* --------------------
*
*  Questa funzione e` chiamata al lancio di QPLC.
*  Installa un'istanza del driver indicato dalla struttura data
* come parametro.
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioInstall(driver_op_t * self, int conf)
{
register driver_t *p;
int instance = 0;
int rv;

/* Isola le componenti della stringa di configurazione, una volta per tutte. */
	if (! drio_sts.io_resource_end) {
	char *q;
		for (q = main_status.devconf; *q; ++q)
			if (*q == ':') /* Il separatore di componenti e` ':' */
				*q = '\0';
		drio_sts.io_resource_end = q;
	}

	if (! self -> install) {
	/* ERRORE. Il driver non dispone del metodo "install". */
		return 0;
	}


	for (p = main_status.drivers.driver_list; p; p = p -> next) {
		if (strcmp(p -> op.name, self -> name) == 0)
			++instance;
	}
	p = (driver_t *) util_alloc_cell((void **) &drio_sts.io_freelist,
	                             sizeof(driver_t),
	                             GET_STRUCT_OFFSET(driver_t,next),
	                             LEN_DRIVER_BLOCK);
	if (! p) {
	/* ERRORE. Non c'e` abbastanza memoria. */
		plcError(108,"ioInstall");
		return 0;
	}
	p -> op = *self;
	p -> error = 0;
	p -> instance = instance;
	p -> next = (driver_t *) 0;
	p -> next_in = (driver_t *) 0;
	p -> next_out = (driver_t *) 0;
	p -> next_check = (driver_t *) 0;
	p -> next_trigger = (driver_t *) 0;
	p -> device = (void *) 0;
	p -> resource_pointer = main_status.devconf;
	p -> index = main_status.drivers.ndrvrs;
	rv = p -> op.install(p,conf);
	if (! rv) {
	/* ERRORE. Il driver ha rifiutato i parametri. */
		util_add_cell((void *) p, 
	                   (void **) &drio_sts.io_freelist,
			   GET_STRUCT_OFFSET(driver_t,next));
		return 0;
	}

/* Aggiunge il driver alla lista dei driver installati. */

	util_add_cell((void *) p, 
		   (void **) & main_status.drivers.driver_list,
		   GET_STRUCT_OFFSET(driver_t,next));

/* Incrementa il numero di driver installati. */

	++main_status.drivers.ndrvrs;

/* Se il driver possiede il metodo "check", lo aggiunge all'apposita lista. */

	if (p -> op.check) {
		util_add_cell((void *) p, 
			   (void **) & main_status.drivers.check_list,
			   GET_STRUCT_OFFSET(driver_t,next_check));
	}

/* Stessa cosa per il metodo "trigger". */

	if (p -> op.trigger) {
		util_add_cell((void *) p, 
			   (void **) & main_status.drivers.trigger_list,
			   GET_STRUCT_OFFSET(driver_t,next_trigger));
	}

	return 1;
}

/*
* Funzione "ioGetInstance"
* ------------------------
*
*  Questa funzione fornisce l'istanza "n" del driver di nome "name",
* oppure NULL se la combinazione non esiste.
*/

driver_t * ioGetInstance(char * name, int n)
{
driver_t *p;

	for (p = main_status.drivers.driver_list; p; p = p -> next) {
		if (util_streq(p -> op.name, name) && n == p -> instance) {
			return p;
		}
	}

	return (driver_t *) 0;
}

/*
* Funzione "ioGetLastInstance"
* ----------------------------
*
*  Questa funzione fornisce l'ultima istanza del driver di nome "name",
* oppure NULL se il driver non esiste.
*/

driver_t * ioGetLastInstance(char * name)
{
driver_t *p,*q;

	q = (driver_t *) 0;

	for (p = main_status.drivers.driver_list; p; p = p -> next) {
		if (util_streq(p -> op.name, name)) {
			q = p;
		}
	}

	return (driver_t *) q;
}

/*
* Funzione "ioGetNum"
* -------------------
*
*  Questa funzione estrae dalla stringa data il primo numero, e calcola
* la posizione di lettura successiva. La stringa deve avere la forma
* <sequenza_di_cifre_decimali>['.'<altro>]. La posizione di lettura
* e` rappresentata da una variabile, che all'ingresso punta alla stringa,
* ed all'uscita punta oltre il "." (o sul '\0' di fine stringa). La funzione
* si aspetta l'indirizzo di questa variabile.
*  Un risultato uguale a -1 e` da interpretare come errore. In quel caso,
* la variabile non viene alterata.
*/

int ioGetNum(char ** p)
{
char *q;
int v;

/* Scarta i casi particolarmente sfigati. */

	if (! p || ! *p)
		return -1;

/* Cerca la fine della sequenza di cifre decimali, e intanto, calcola
 il valore. */

	for (v = 0, q = *p; *q >= '0' && *q <= '9'; ++q)
		v = v * 10 + *q - '0';

/* Scarta le stringhe che non iniziano con una sequenza di cifre decimali
 (q == *p), e quelle che, dopo la sequenza di cifre decimali, non terminano
 o non hanno un "." . */

	if (q == *p || (*q != '.' && *q))
		return -1;

/* Scarta il "." in fondo alla sequenza di cifre, se c'e`. */

	if (*q == '.')
		++q;

/* Registra la nuova posizione di lettura. */

	*p = q;

/* Fine. */

	return v;
}

/*
* Funzione "ioParse"
*
*  Questa funzione traduce la stringa di descrizione di un segnale di
* I/O nella sua descrizione interna.
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioParse(char * name, int mode, devnode_t *l)
{
char dr_name[MAX_DRIVER_NAME+1];
char *ext;
int instance;
driver_t *p;
int i;

	memset((char *)l,0,sizeof(*l));
	l -> flags = mode;
	strncpy(l -> name,name,sizeof(l -> name)-1);

/* Isola il nome del dispositivo e lo copia in dr_name. */

	ext = strchr(l -> name,'.');
	if (! ext) {
	/* ERRORE. Nome incompleto. */
		return 0;
	}
	i = ext - l -> name;
	dr_name[i] = '\0';
	while (i--)
		dr_name[i] = l -> name[i];
	if (! dr_name[0]) {
	/* ERRORE. Nome incompleto. */
		return 0;
	}

/* Isola l'istanza del driver. Se questo ha successo,
 isola la parte dipendente dal dispositivo. Puo` non esistere, nel
 qual caso il driver ricevera` la stringa vuota (""). */

	++ext;
	instance = ioGetNum(&ext);
	if (instance == -1) {
	/* Istanza assente. Si assume la 0. */
		instance = 0;
	}

/* Cerca il dispositivo e l'istanza. */

	p = ioGetInstance(dr_name, instance);
	if (! p) {

	/* Se l'istanza non e` stata trovata, cerca la piu` alta esistente.
	 Alla funzione "parse" arrivera` come secondo parametro la differenza
	 tra l'istanza richiesta e quella trovata. */

		p = ioGetLastInstance(dr_name);
		/* ERRORE. Driver o istanza inesistenti. */
		if (! p) {
			return 0;
		}
	}

/* Comunica al driver la parte del nome di sua competenza, ed ottiene
 la dimensione della variabile compatibile e la chiave per la
 successiva "attach". */

	if (! p -> op.parse) {
	/* ERRORE. Il driver non dispone del metodo "parse". */
		return 0;
	}

	l -> next.driver = p;
	l -> pname = ext;
	return p -> op.parse(p, instance - p -> instance, l);
}

/*
* Funzione "ioCoreAttach"
* -----------------------
*
*  Questa funzione collega una variabile ad un segnale di I/O descritto
* dalla struttura "*l" (ottenuta con "ioList" o "ioParse").
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioCoreAttach(devnode_t *l, void * addr)
{
driver_t *p;
int mode;

	p = l -> next.driver;
	if (!p) {
	/* ERRORE. "l" contiene evidentemente una schifezza. */
		return 0;
	}
	mode = l -> flags;

/* Collega la variabile data con il segnale. */

	if (! p -> op.attach) {
	/* ERRORE. Il driver non dispone del metodo "attach". */
		return 0;
	}

	if (! (p -> op.attach(p, l, addr))) {
	/* ERRORE. L'attach e` fallita (non so perche`, ma e` successo). */
		return 0;
	}

/* Collega, se necessario, l'istanza alle liste di input e di output. */

	if ((mode & DRIVER_MODE_INPUT)
	    && p -> op.read
	    && !util_find_cell((void *)p,
	                   (void **)& main_status.drivers.in_list,
	                   GET_STRUCT_OFFSET(driver_t,next_in))) {
		util_add_cell((void *) p, 
	                   (void **) & main_status.drivers.in_list,
			   GET_STRUCT_OFFSET(driver_t,next_in));
	}

	if ((mode & DRIVER_MODE_OUTPUT)
	    && p -> op.write
	    && !util_find_cell((void *)p,
	                   (void **)& main_status.drivers.out_list,
	                   GET_STRUCT_OFFSET(driver_t,next_out))) {
		util_add_cell((void *) p, 
	                   (void **) & main_status.drivers.out_list,
			   GET_STRUCT_OFFSET(driver_t,next_out));
	}

	return 1;
}

/*
* Funzione "ioAttach"
* --------------------
*
*  Questa funzione e` chiamata tipicamente ad ogni rilancio del PLC.
*  Serve a collegare una variabile di indirizzo e dimensione dati ad un
* segnale di input o di output ("mode" decide). La variabile e` descritta
* da una stringa nella forma "nome.istanza.altro...", dove "nome" e` il nome
* del driver, "istanza" e` il numero di istanza dello stesso, "altro" e` una
* sequenza di caratteri qualsiasi, la cui interpretazione e` a carico del
* driver.
*  "size" e` la dimensione della variabile associata IN BIT.
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioAttach(char * name, void * addr, int size, int mode)
{
devnode_t l;
int sz,rv,opt,access,try;
char nm[MAX_DEVLIST_NAME];

/* Converte il nome in forma canonica:

 - tutti i caratteri sono convertiti in minuscolo,
 - e` eliminato l'eventuale "_" iniziale, che indica "segnale opzionale",
 - sono eliminati tutti i caratteri "0" all'inizio di un campo. Se il campo
   non contiene altri caratteri, e` conservato uno "0".
   Si tratta di una primitiva normalizzazione di valori numerici.
*/

	opt = ioCanonify(nm, sizeof(nm), name);

/* Controlla un caso speciale: dispositivo "NULL", che serve a "far finta"
 di associare qualcosa ad una variabile. Se poi e` impostata l'opzione
 "-simul", tutti i segnali, esistenti e non, fanno la stessa fine. */

	if (main_status.simul || util_streq("NULL", nm)) {

		rv = 1;

	}
	else {

/* Ottenimento del permesso di accesso al segnale dal database dei segnali. */

		access = cioGetProp(nm, size, mode);
		if (access == CIO_PROP_UNKNOWN) {
			access = toupper(main_status.defaultio[0]);
		}

		try = 0; /* Flag che indica che bisogna prima provare
		          ad usare il segnale hardware. */
		rv = 0; /* Risulatato (provvisorio) dell'azione. */

		switch (access) {
		case CIO_PROP_SIMULATED: rv = 1; break;
		case CIO_PROP_OPTIONAL: try = 1; opt = 1; break;
		case CIO_PROP_DISABLED: break;
		case CIO_PROP_REQUIRED: try = 1; break;
		default: break;
		}

		if (try) {

	/* Si`, si puo` usare l'hardware. */

	/* Caso ordinario: traduzione del nome nel formato interno... */

			sz = ioParse(nm, mode, &l);

	/* ...controllo della compatibilita` della variabile... */

			if (sz == 0 || (size != 0 && size != sz)) {

			/* ERRORE. La dimensione della variabile non
			 coincide con quella prevista per il segnale,
			 o la stringa non descrive un segnale utilizzabile. */

				rv = 0;
			}
			else {

		/* ...e collegamento al dispositivo. */

				rv = ioCoreAttach(&l, addr);
			}
		}
	}

/* C'e` ancora una speranza: se il segnale e` opzionale,
 non e` rilevato l'eventuale fallimento. */

	rv = rv || opt;

/* Se il collegamento ha avuto successo, lo si registra in una forma
 facilmente leggibile dal debugger. */

	if (rv)
		ioStoreBinding(nm, addr, size, mode);

	return rv;
}

/*
* Funzione "ioShow"
*
*  Questa funzione riporta nel buffer puntato da "dest" lo stato
* del segnale descritto da "l".
*  Vale 1 se tutto e` andato bene, 0 in caso di errore.
*/

int ioShow(devnode_t *l, void *dest)
{
	if ((!(l -> flags & (DRIVER_MODE_INPUT | DRIVER_MODE_OUTPUT)))
	 || (!(l -> next.driver -> op.show)))
		return 0;

	return l -> next.driver -> op.show(l -> next.driver,l,dest);
}

/*
* Funzione "ioList"
* -----------------
*
* Questa funzione fornisce il nodo "successivo" a quello indicato
* dalla struttura puntata dal parametro "l". Se il campo "ideep"
* vale 0, si intente esplorare la radice dell'albero dei dispositivi.
* Il campo "explore" indica la profondita` con cui si intende esplorare
* la gerarchia. La funzione, ricorrendo eventualmente ai metodi "list"
* specifici dei driver, fornisce il "successivo" elemento al livello
* "explore" di quello descritto dai restanti campi. Il valore dei campi
* si intende prodotto da una precedente chiamata della stessa funzione
* allo stesso livello o a quello immediatamente superiore. La funzione
* non controlla, se non superficialmente, la sensatezza dei dati
* della struttura. Se comunque viene rilevata un'incongruenza, essa vale
* -1, altrimenti vale 0. La fine dell'esplorazione di un livello e`
* segnalata dalla stringa vuota nel campo "name".
*/

int ioList(devnode_t *l)
{
driver_t *p;
int rv;

	if (l -> ideep < 0)
		return -1;
	if (l -> ideep == 0) {
	/* Riempimento della struttura con valori sensati, in
	 particolare con l'inizio della catena dei driver. */
		memset((char *)l,0,sizeof(*l));
		l -> tree.driver = main_status.drivers.driver_list;
		l -> next.driver = (driver_t *) NULL;
	/* Ovviamente, la radice e` esplorabile ed espandibile ! */
		l -> flags = DRIVER_MODE_LIST;
		l -> name[0] = '/';
		return 0;
	}
/* Un minimo di controllo! che almeno il descrittore di driver esista! */
	for (p = main_status.drivers.driver_list; p; p = p -> next)
		if (p == l -> tree.driver)
			break;
	if (! p)
		return -1;
	rv = 0;
	l -> comment[0] = '\0';
	if (l -> ideep == 1) {
	/* Livello 1: quello della lista dei driver installati.
	  Si deve fornire il descrittore del prossimo driver della lista,
	 ed inizializzarlo con valori sensati e prevedibili. */
		memset((char *)l -> tree.spec,0,sizeof(l -> tree.spec));
		memset((char *)l -> next.spec,0,sizeof(l -> next.spec));
		if (p -> op.list) {
			rv = p -> op.list(p,l);
			l -> flags = DRIVER_MODE_LIST;
		}
		else
			l -> flags = 0;
		strcpy(l -> name, p -> op.name);
		l -> tree.driver = p;
		l -> next.driver = p -> next;
	}
	else {
	/* Livello maggiore di 1: e` quasi tutto a carico del driver. */
	/* Invoca il metodo specifico, se c'e`. */
		if (p -> op.list) {
			l -> next = l -> tree;
			rv = p -> op.list(p,l);
		}
		else {
		/* Il driver non supporta "list". Come si e` arrivati
		 a questo punto? Qualcuno ha fatto confusione. Percio`,
		 errore. */
			rv = -1;
		}
	}

	return rv;
}

/*
* Funzione "ioRegisterExtension"
* ------------------------------
*
* Questa funzione fornisce un codice da utilizzare per la chiamata
* di specifici servizi non standard messi a disposizione da certi driver.
* Il codice ottenuto dovra` essere utilizzato per accedere al servizio
* tramite la funzione "ioExtendedOp".
* Questa funzione cerca di produrre un codice tale da essere difficilmente
* confuso, in modo che sia poco probabile l'accesso accidentale a servizi
* per errori di programmazione.
* Il secondo parametro sara` passato al matodo "extension" del driver
* al momento della chiamata.
* Vale 0 in caso di errore.
*/

#define IO_EXT_CODE_MASK 0x00000FFFL
#define IO_EXT_MAGIC_CODE 0xDE19A000L
#define IO_EXT_HASH(n) (((n) & IO_EXT_CODE_MASK) | IO_EXT_MAGIC_CODE)
#define IO_EXT_ISMAGIC(n) (((n) & ~IO_EXT_CODE_MASK) == IO_EXT_MAGIC_CODE)
#define IO_EXT_CODE(n) ((n) & IO_EXT_CODE_MASK)

long ioRegisterExtension(driver_t *d, void *s_p)
{
int n;

	if (! main_status.drivers.extensions) {
		main_status.drivers.extensions = (extended_driver_t *)
			malloc(main_status.max_extensions
			       * sizeof(extended_driver_t));
		if (! main_status.drivers.extensions) {
		/* ERRORE. Pare che non ci sia memoria. */
			return 0;
		}
	}
	n = main_status.drivers.n_ext;
	if (main_status.drivers.n_ext >= main_status.max_extensions) {
	/* ERRORE. Pare che non ci sia piu` posto nella tabella. */
		return 0;
	}
	if (!d -> op.extension) {
	/* ERRORE. Ci dovra` pur essere il metodo, no ? */
		return 0;
	}
	main_status.drivers.extensions[n].driver = d;
	main_status.drivers.extensions[n].param = s_p;
	main_status.drivers.n_ext = n + 1;
	return IO_EXT_HASH(n);
}

/*
* Funzione "ioExtendedOp"
* -----------------------
*
* Questa funzione invoca il metodo "extension" del driver e con i parametri
* indicati dall'indice di estensione dato. L'indice deve essere stato
* ottenuto con "ioRegisterExtension". Il secondo parametro sara` passato al
* metodo come terzo parametro, il terzo come quarto.
* La funzione vale 1 se tutto e` andato bene, 0 in caso di errori.
*/

int ioExtendedOp(long ex, int cmd, void * u_p)
{
int n;
extended_driver_t *e;

	if (! IO_EXT_ISMAGIC(ex))
		return 0;

	n = (int) IO_EXT_CODE(ex);
	if (n >= main_status.drivers.n_ext)
		return 0;
	e = &main_status.drivers.extensions[n];
	if (! e -> driver || ! e -> driver -> op.extension)
		return 0;

	return (*(e -> driver -> op.extension))(e -> driver, e -> param,
	                                        cmd, u_p);
}

