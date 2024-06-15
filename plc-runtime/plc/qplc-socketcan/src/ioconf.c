
/*
* @(#) ioconf.c 1.1 Mon Nov  9 11:42:08 MET 1998
*
* Lettura ed elaborazione del file di configurazione degli I/O.
* 
***************************************************************************
*
* 	Descrizione del file di configurazione degli I/O di qplc
* 	--------------------------------------------------------
* 
*  Il file di configurazione degli I/O serve a modificare il comportamento
* di Qplc nei confronti delle richieste di uso dell'hardware da parte
* del programma PLC. Per l'esattezza, file di configurazione influenza il
* conportamento della funzione interna "ioAttach".
*  Qplc, durante il collegamento di un I/O ad una variabile, assegna ai segnali
* una delle seguenti proprieta`:
* 
*     R : (Richiesto) Il segnale deve essere presente, cioe`
* 	in sua assenza la funzione "ioAttach" fallisce.
* 	Per un programma scritto in IL significa l'impossibilita`
* 	di essere eseguito.
*     S : (Simulato) Il segnale, presente o assente che sia,
* 	non viene fisicamente manipolato, e il tentativo di
* 	usarlo non produce alcun effetto ma neppure errori.
*     O : (Opzionale) Il segnale e` utilizzato, se presente, oppure
* 	e` simulato. In ogni caso non si hanno errori.
*     D : (Disattivato) Il tentativo di utilizzare il segnale fallisce sempre.
* 
*  Qplc applica le regole seguenti nel gestire i segnali e il file di 
* configurazione:
* 1) Se e` specificata l'opzione "-simul", tutti i segnali sono
*    considerati "simulati" (proprieta` "S") indipendentemente dal contenuto
*    del file di configurazione e dalle condizioni dell'hardware.
* 2) Se il file di configurazione non e` presente, si assume che tutti
*    i segnali il cui nome inizi con "_" siano opzionali (proprieta` "O")
*    e tutti gli altri hanno la proprieta` indicata dall'opzione "-defaultio"
*    (il valore di default e` "R").
* 3) Se il file e` presente, viene interpretato come segue:
*  3.1) Le linee che iniziano con "#" sono commenti.
*  3.2) Non c'e` distinzione tra caratteri maiuscoli e minuscoli.
*  3.3) Ogni segnale e` descritto da una linea avente la forma
* 
*            <segnale> <proprieta`>
* 
*       dove "segnale" e` il nome del segnale di I/O, preceduto da un
*       carattere "%" opzionale e due caratteri obbligatori che ne
*       identificano il tipo (input o output) e la dimensione. Questi
*       due caratteri sono conformi alla sintassi di descrizione dei
*       segnali di I/O riconosciuta dal compilatore del linguaggio IL.
*       Il primo puo` essere "I" per indicare segnali di INPUT, oppure
*       "Q" o "O" per segnali di OUTPUT. Il secondo rappresenta la
*       dimensione in bit del segnale secondo lo schema seguente:
* 
*            X : 1 bit (tipo BOOL)
*            B : 8 bit (tipi BYTE, CHAR, UCHAR, SINT, USINT)
*            W : 16 bit (tipi WORD, INT, UINT)
*            D : 32 bit (tipi DWORD, DINT, UDINT, REAL)
*            L : 64 bit (tipi LWORD, LINT, ULINT, LREAL)
* 
*        L'eventuale "%" iniziale non ha alcun effetto, ma serve solo per
*       avvicinare la sintassi di questo file a quella del compilatore IL.
*        Il campo "proprieta`" descrive il comportamento di Qplc quando
*       il programma plc tenta di agganciarsi al segnale. Le proprieta`
*       dei segnali sono state descritte sopra.
*        I segnali il cui nome inizia con "_" non provocano in ogni caso
*       errore.
* 4) I segnali assenti sono trattati diversamente a seconda del valore dato
*    con l'opzione da linea di comando "-defaultio x", dove "x" e` uno dei
*    possibili valori di proprieta`. In assanza di quest'opzione, i segnali
*    assenti sono trattati come RICHIESTI (proprieta` R).
*
*  Ecco un esempio di file di configurazione di qplc:
* 
* # Segnali obbligatori
* %QXios.0.0.31.0.0	R
* %QXios.0.0.31.0.1	R
* %QXios.0.0.31.0.2	R
* %QXios.0.0.31.0.3	R
* 
* # Segnali simulati
* %QXios.0.0.31.0.4	S
* %QXios.0.0.31.0.5	S
* %QXios.0.0.31.0.6	S
* %QXios.0.0.31.0.7	S
* 
* # Segnali opzionali
* %QXios.0.0.31.0.8	O
* %QXios.0.0.31.0.9	O
* %QXios.0.0.31.0.10	O
* %QXios.0.0.31.0.11	O
* 
***************************************************************************
*
* 01/10/98 GG 1.0 Prima stesura.
* 09/11/98 GG 1.1 Aggiunta la distinzione tra segnali proibiti (D) e
*             segnali non citati. Questo permette di modificare il 
*             comportamento di qplc davanti ai segnali non previsti nel
*             file di configurazione. I segnali sono "non citati" se
*             non sono citati nel file, oppure se il file e` assente.
*             Aggiunta la funzione "cioLoad", che serve a rileggere
*             il file di configurazione.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "hsort.h"

#include "qplc.h"

#include "util.h"

#include "ioconf.h"

/* Lunghezza massima della parte significativa di una linea. */
#define MAX_IOCONF_LINE 80

/* Minima unita` di allocazione della tabella degli I/O. */
#define IO_PROPBUFF_MIN 30
/* Margine aggiuntivo di allocazione della tabella degli I/O. */
#define IO_PROPBUFF_MARG 10

typedef struct _ioprop_t {
/* Modo di accesso. (vedi "DRIVER_MODE_..."). */
	char flags;
/* Dimensione in bit del segnale associato. */
	char nbit;
/* Proprieta` del segnale. */
	char prop;
/* Nome del segnale. */
	char name[MAX_DEVLIST_NAME];
} ioprop_t;

static struct _ioaccess_sts_t {
/* Dimensione del blocco fisico allocato. */
	int nmem;
/* Numero di elementi utilizzati nel blocco fisico. */
	int nprop;
/* Indirizzo del blocco contenente le proprieta`. */
	ioprop_t *prop;
/* Linea correntemente interpretata. */
	int linenum;
/* Valore del primo campo della linea (segnale). */
	char field1[MAX_IOCONF_LINE];
/* Valore del secondo campo della linea (proprieta`). */
	char field2[2];
/* Nome del file di configurazione. */
	char *filename;
/* Passo di interpretazione del file. */
	int pass;
/* Numero di entry correntemente incontrate nel file. */
	int currprop;
} ioacc_sts = { 0 };

enum {
	CIO_E_NULL = 0,
	CIO_E_TOOLONG,
	CIO_E_NEEDPROP,
	CIO_E_BADIOQ,
	CIO_E_BADSZQ,
	CIO_E_BADPROP,
	CIO_E_DUP,

	CIO_E_MAX,
};

#ifdef IOCONF_TEST /* Per debug. */
int ioCanonify(char *d, int n, char *s)
{
	strcpy(d,s);
	return 0;
}
void util_fatal(char *s)
{
	fprintf(stderr,"FATAL: %s\n",s);
	exit(1);
}
void main(int argc, char **argv)
{
int rv,i;

	rv = cioInit("sample.ioconf");
	printf("cioInit = %d\n",rv);
	if (rv == 0)
		exit(0);
	for (i = 0; i < ioacc_sts.nprop; ++i)
		printf("%d %s %c %d %d\n",i,
		       ioacc_sts.prop[i].name,
		       ioacc_sts.prop[i].prop,
		       ioacc_sts.prop[i].flags,
		       ioacc_sts.prop[i].nbit);
	for (;;) {
	char name[40];
	int sz,flags;
		printf("Scrivi un segnale. Minuscolo.\n");
		scanf("%s %d %d",name,&sz,&flags);
		printf("cioGetProp(\"%s\",%d,%d) = %c\n",
		       name,sz,flags,cioGetProp(name,sz,flags));
	}
	exit(0);
}
#endif /* Fine debug. */

/*
* Funzione "cioError"
* -------------------
*
* Emissione di un errore.
*/

static void cioError(int err)
{
char *msg;
int ln;
static char *errs[] = {
"",
"line too long",
"property required",
"bad I/O qualifier",
"bad size qualifier",
"bad property qualifier",
"duplicated entry",
};

/* Niente errori al passo 1 (tranne CIO_E_DUP). */
	if (ioacc_sts.pass && err != CIO_E_DUP)
		return;

	ln = ioacc_sts.linenum;

	if (err == CIO_E_NULL)
		return;
	else if (err < 0 || err >= CIO_E_MAX)
		msg = "internal error";
	else
		msg = errs[err];
	if (err == CIO_E_TOOLONG)
		--ln;
	fprintf(stderr,"QPLC: %s:%d: %s\n",ioacc_sts.filename,ln,msg);
}

/*
* Funzione "cioEval"
* ------------------
*
* Questa funzione controlla la correttezza dei campi letti e
* li elabora.
*/

static void cioEval(void)
{
char *p;
ioprop_t prop;
int err,i;

	p = ioacc_sts.field1;
	if (*p == '%') {
	/* Ignora il "%" opzionale. */
		++p;
	}

/* Controlla il primo carattere (modo di accesso). */

	switch (p[0]) {
	case 'i': case 'I': prop.flags = DRIVER_MODE_INPUT; break;
	case 'o': case 'O':
	case 'q': case 'Q': prop.flags = DRIVER_MODE_OUTPUT; break;
	default: err = CIO_E_BADIOQ; goto ERROR;
	}

/* Controlla il secondo carattere (numero di bit). */

	switch (p[1]) {
	case 'x': case 'X': prop.nbit = 1; break;
	case 'b': case 'B': prop.nbit = 8; break;
	case 'w': case 'W': prop.nbit = 16; break;
	case 'd': case 'D': prop.nbit = 32; break;
	case 'l': case 'L': prop.nbit = 64; break;
	default: err = CIO_E_BADSZQ; goto ERROR;
	}

/* Controlla il campo proprieta`. */

	switch (ioacc_sts.field2[0]) {
	case 'R': case 'O': case 'D': case 'S':
	case 'r': case 'o': case 'd': case 's':
		prop.prop = toupper(ioacc_sts.field2[0]);
		break;
	default: err = CIO_E_BADPROP; goto ERROR;
	}

/* Elaborazione del dato. */

/* Conversione in forma canonica del nome. */
	ioCanonify(prop.name,sizeof(prop.name),&p[2]);

	if (ioacc_sts.pass) {

	/* Al passo 1 si registra il dato in una cella. */

	/* Controllo di congruenza: il nome e` ripetuto? Se si`,
	 qualcuno si e` probabilmente sbagliato. Meglio segnalarlo. */

		for (i = 0; i < ioacc_sts.currprop; ++i) {
			if (ioacc_sts.prop[i].flags == prop.flags
			 && ioacc_sts.prop[i].nbit == prop.nbit
			 && strcmp(ioacc_sts.prop[i].name,prop.name) == 0) {

			/* Trovato: vi siete sbagliati, vero? */

				err = CIO_E_DUP;
				goto ERROR;
			}
		}

		ioacc_sts.prop[ioacc_sts.currprop] = prop;
		++ioacc_sts.currprop;

	}
	else {

	/* Al passo 0 ci si limita a contare la roba. */

		++ioacc_sts.nprop;

	}

	return;

ERROR:
	cioError(err);
}

/*
* Funzione "cioParse"
* -------------------
*
* Questa funzione analizza il file di configurazione e passa le linee
* valide alla procedura di caricamento ("ioEval").
* Produce anche messaggi di errore (su stderr) in caso di sintassi errata.
* Vale 1 se tutto e` andato bene, -1 se il file non esiste, 0 in caso di
* altri errori.
*/

enum {
	CIOSTS_BAD = 0,
	CIOSTS_START,
	CIOSTS_FIELD1,
	CIOSTS_SEP12,
	CIOSTS_FIELD2,
	CIOSTS_COMMENT,
};
/* Funzioncine di utilita`. Trovano spazi, commenti... */
static int cioSpace(int c)
{
	return (c == ' ' || c == '\t' || c == '\f' || c == '\r');
}
static int cioEndLine(int c)
{
	if (c == '#' || c == '\n') {
		++ioacc_sts.linenum;
		return c == '\n' ? CIOSTS_START : CIOSTS_COMMENT;
	}
	else {
		return CIOSTS_BAD;
	}
}

static int cioParse(void)
{
FILE *f;
char *p = NULL;
int c,rv,sts;

/* Apertura del file. */

	f = fopen(ioacc_sts.filename,"r");
	if (! f) {
		if (errno == ENOENT)
			return -1;
		else
			return 0;
	}

/* Ciclo di lettura delle linee. */

	ioacc_sts.linenum = 0;
	sts = CIOSTS_START;

	while ((c = fgetc(f)) != EOF) {

		switch (sts) {

		case CIOSTS_START:

		/* Stato "inizio linea". */

			if (cioSpace(c)) {
			/* Spazi. Buttare via. */
			}
			else if ((rv = cioEndLine(c)) != CIOSTS_BAD) {
				sts = rv;
			}
			else {
			/* Primo carattere del primo campo:
			 lo si registra, e si cambia stato. */
				p = &ioacc_sts.field1[0];
				*p = c;
				*++p = '\0';
				sts = CIOSTS_FIELD1;
			}
			break;

		case CIOSTS_FIELD1:

			if (cioSpace(c)) {
			/* Spazi. Il campo e` finito. */
				sts = CIOSTS_SEP12;
			}
			else if ((rv = cioEndLine(c)) != CIOSTS_BAD) {
				cioError(CIO_E_NEEDPROP);
				sts = rv;
			}
			else {
			/* Carattere del primo campo:
			 lo si registra, e si cambia stato. */
				*p = c;
				*++p = '\0';
			/* Se il campo e` troppo lungo, si passa
			 allo stato "commento". Si emette un errore,
			 e la linea viene ignorata. */
				if (p - &ioacc_sts.field1[0]
				 >= sizeof(ioacc_sts.field1) - 1) {
					cioError(CIO_E_TOOLONG);
					sts = CIOSTS_COMMENT;
				}
			}
			break;

		case CIOSTS_SEP12:

		/* Separatore tra primo e secondo campo. */

			if (cioSpace(c)) {
			/* Spazi. Buttare via. */
			}
			else if ((rv = cioEndLine(c)) != CIOSTS_BAD) {
				cioError(CIO_E_NEEDPROP);
				sts = rv;
			}
			else {
			/* Primo carattere del secondo campo:
			 lo si registra, e si cambia stato. */
				ioacc_sts.field2[0] = c;
				ioacc_sts.field2[1] = '\0';
				sts = CIOSTS_FIELD2;
			}
			break;

		case CIOSTS_FIELD2:

			if (cioSpace(c)) {
			/* Spazi. Buttare via. */
			}
			else if ((rv = cioEndLine(c)) != CIOSTS_BAD) {

				cioEval();

				sts = rv;
			}
			else {
			/* Non sono ammessi altri caratteri. Errore. */
				cioError(CIO_E_TOOLONG);
				sts = CIOSTS_COMMENT;
			}
			break;

		case CIOSTS_COMMENT:
			if (c == '\n') {
			/* Fine linea. Si passa allo stato iniziale. */
				sts = CIOSTS_START;
			}
			break;
		}
	}
	
	fclose(f);

	return 1;
}

/*
* Funzione "cioCompare"
* ---------------------
*
*  Funzione di confronto tra elementi di tipo "ioprop_t". Serve
* ad ordinare la tabella e a trovare elementi.
*/

static int cioCompare(ioprop_t *e1, ioprop_t *e2)
{
register int v;

	v = e1 -> nbit - e2 -> nbit;
	if (v)
		return v;
	v = e1 -> flags - e2 -> flags;
	if (v)
		return v;
	return strcmp(e1 -> name, e2 -> name);
}

/*
* Funzione "cioLoad"
* ------------------
*
*  Questa funzione legge il file di configurazione e lo elabora.
*  Il parametro "init" vale 1 se ci si trova in una fase di inizializzazione,
* altrimenti (cioe` normalmente) vale 0.
*  Vale 1 in caso di successo, 0 in caso di errore.
*/

int cioLoad(int init)
{
ioprop_t tmp;
int rv;

/* Inizializzazioni. */
	ioacc_sts.nprop = 0;
	ioacc_sts.linenum = 0;
	ioacc_sts.field1[0] = '\0';
	ioacc_sts.field2[0] = '\0';
	ioacc_sts.pass = 0;
	ioacc_sts.currprop = 0;

/* Passo 0: analisi sintattica. */

	rv = cioParse();
	if (rv == -1)
		return 1;
	else if (rv == 0)
		return 0;

/* Allocazione della tabella. */

	if (ioacc_sts.nprop == 0) /* Esce se non ci sono linee. */
		return 1;

/* Alloca (o ricicla) la memoria necessaria a contenere la tabella. */

	if (ioacc_sts.nprop > ioacc_sts.nmem) {
	/* Se l'area gia` allocata e` troppo piccola, la libera (ammesso
	 che esista). Bisognera` allocarne una piu` grande. */
		if (ioacc_sts.prop) {
			free((void *)(ioacc_sts.prop));
			ioacc_sts.prop = (ioprop_t *) NULL;
		}
	}

/* Se non e` ancora stata allocata un'area adatta (o quella allocata e`
 stata liberata perche` troppo piccola), ne alloca una nuova, tenendo un po'
 di margine. Potrebbe fare comodo piu` avanti. Non si sa mai. */

	if (! ioacc_sts.prop) {

		if (ioacc_sts.nprop < IO_PROPBUFF_MIN)
			ioacc_sts.nmem = IO_PROPBUFF_MIN + IO_PROPBUFF_MARG;
		else
			ioacc_sts.nmem = ioacc_sts.nprop + IO_PROPBUFF_MARG;

		ioacc_sts.prop = (ioprop_t *) malloc(sizeof(ioprop_t)
		                                   * ioacc_sts.nmem);
		if (! ioacc_sts.prop) {
			if (init)
				util_fatal(ioacc_sts.filename);
			else
				plcError(108,"%s",ioacc_sts.filename);
			ioacc_sts.nprop = ioacc_sts.nmem = 0;
			return 0;
		}
	}

/* Passo 1: riempimento della tabella. */

	ioacc_sts.pass = 1;

	if (cioParse() <= 0)
		return 0;

/* Tiene conto di eventuali entry scartate. */
	ioacc_sts.nprop = ioacc_sts.currprop;

/* Ordinamento della tabella. */

	hsort((char *) ioacc_sts.prop,
	      ioacc_sts.nprop,
	      sizeof(ioacc_sts.prop[0]),
	      (int (*)(void *, void *))cioCompare,
	      (char *)&tmp);
	

	return 1;
}

/*
* Funzione "cioInit"
* ------------------
*
*  Questa funzione legge il file di configurazione e lo elabora.
*  Vale 1 in caso di successo, 0 in caso di errore.
*/

int cioInit(char *file)
{
ioprop_t tmp;

/* Inizializzazioni paranoiche. */
	ioacc_sts.nmem = 0;
	ioacc_sts.prop = (ioprop_t *) 0;
	ioacc_sts.filename = file;

/* Caricamento del file. */
	return cioLoad(1);
}

/*
* Funzione "cioGetProp"
* ---------------------
*
*  Questa funzione restituisce la proprieta` del segnale dato
* ottenendola dal database di configurazione. Restituisce uno dei
* valori di proprieta` "CIO_PROP_...".
*  Se il database non e` disponibile (cioe` non c'era il file
* di configurazione) il segnale e` sempre di tipo "CIO_PROP_REQUIRED".
* NOTA: Il nome del segnale deve essere convertito in forma canonica
* (usare la funzione "ioCanonify").
*/

int cioGetProp(char *name, int size, int flags)
{
ioprop_t *p,k;

	if (! ioacc_sts.prop)
		return CIO_PROP_UNKNOWN;

/* Ricerca binaria nel database. */

	strcpy(k.name, name);
	k.nbit = size;
	k.flags = flags;
	k.prop = 0;
	p = bsearch((char *)&k,
	            (char *)ioacc_sts.prop,
	            ioacc_sts.nprop,
	            sizeof(k),
	            cioCompare);

/* I segnali non previsti nel database
 sono marcati con una proprieta` apposita. */

	if (! p)
		return CIO_PROP_UNKNOWN;
	else
		return p -> prop;
}

