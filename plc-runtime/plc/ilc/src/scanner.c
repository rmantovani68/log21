
/*
* @(#) scanner.c 3.1 Wed Jun  4 10:07:53 MET DST 1997
* @(#) Rif. Guerrini
*
*  Primitive di lettura e riconoscimento degli elementi base del
* linguaggio descritto dallo standard IEC-1131/3.
*
* 24/10/94 1.2 Versione stabilizzata, distribuita in CNI informatica.
* 26/06/96 2.0 Adattamenti per utilizzo nel compilatore PLC-IEC1131.
*          Aggiunto un flag di lettura "case-insensitive".
*          Modificato il default per i marcatori di commento.
* 29/08/96 2.1 Aggiunta la lettura di costanti di tipo "TIME".
* 07/10/96 2.2 Modificato il meccanismo di lettura delle parole chiave
*          e degli identificatori, per poter gestire i casi ambigui, in cui
*          una parola puo` essere interpretata sia come parola chiave che
*          come identificatore, a seconda del contesto.
* 11/10/96 GG 3.0 Aggiunta la gestione delle stringhe.
* 04/06/97 GG 3.1 Aggiunto il riconoscimento delle grandezze "TIME#" in
*          forma decimale.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "conf.h"
#include "scanner.h"

/* VARIABILI */

/* Struttura di controllo del pacchetto. Si inizializza con "scSetScanner". */
static SCNCTL scnCtl = {
	/* Funzione di lettura di una linea. */
	NULL,
	/* Puntatore al buffer di lettura. */
	NULL,
	/* Lunghezza del buffer di lettura. */
	0,
	/* Posizione corrente di lettura. */
	NULL,
	/* Posizione dell'ultimo identificatoreo stringa numerica. */
	NULL,
	/* Numero di linea. */
	0,
	/* Tabella delle parole chiave correntemente utilizzata. */
	(LPCHAR PTR_) 0,
	0,
	/* Cache delle parole chiave. */
	-1,
	/* Funzione di acquisizione dello stato corrente del lettore. */
	NULL,
	/* Funzione di impostazione dello stato corrente del lettore. */
	NULL,
	/* Dimensione dell'oggetto rappresentante lo stato del lettore. */
	0,
	/* Stringa di inizio commento. */
	"(*",
	/* Lunghezza della stringa di inizio commento. */
	sizeof("(*") - 1,
	/* Stringa di fine commento. */
	"*)",
	/* Lunghezza della stringa di fine commento. */
	sizeof("*)") - 1,
	/* Flag di commento aperto. */
	0,
	/* Carattere utilizzato come inizio e fine stringa. */
	'\'',
	/* Carattere utilizzato come escape nelle stringhe. */
	'$',
	/* Flag che rende di tipo "blank" il carattere "\n". */
	1,
	/* Flag di "case insensitive". */
	1,
	/* Flag che fa si` che il carattere "-" possa comparire all'interno
	 di un identificatore. */
	0,
	/* Lunghezza massima di un identificatore. */
	32,
	/* Lunghezza massima di un intero senza segno.
	  La lunghezza massima di un intero con segno e` maxnumlen+1.
	  La lunghezza massima di un float e` 3*maxnumlen + 4. */
	32,
	/* Lunghezza massima di una costante stringa. */
	80,
};

/* Puntatore GLOBALE associato alla struttura di controllo. */

LPSCNCTL pScCtl = & scnCtl;

/* FUNZIONI */

/*
* Funzione scIsBlank
* -------------------
*
* Parametri :
*
*	char c : Carattere da controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere e` "blank", 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere significativo dello stream di lettura
* e` da considerare uno spazio.
*
* NOTA : Il file "scanner.h" contiene la definizione della macro "LF_IS_BLANK".
*  Se tale macro equivale a "0", il carattere di fine linea ("\n") non e`
* considerato di spaziatura. Puo` quindi essere manipolato dallo scanner
* come qualsiasi altro carattere ordinario.
*/

static int scIsBlank(char c)
{
	if (scnCtl.lf_blank)
		return (! isprint(c) || c == ' ');
	else
		return (! (isprint(c) || c == '\n') || c == ' ');
}

/*
* Funzione scIsSymF
* ------------------
*
* Parametri :
*
*	char c : Carattere da controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere puo` essere l'inizio di un
*		identificatore, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere significativo dello stream di lettura
* puo` essere l'inizio di un identificatore, secondo una sintassi simil-C.
*/

static int scIsSymF(char c)
{
	return (isalpha(c) || c == '_');
}

/*
* Funzione scIsSymM
* ------------------
*
* Parametri :
*
*	char c : Carattere da controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere puo` comparire al'interno di
*		identificatore, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere significativo dello stream di lettura
* puo` comparire all'interno di un identificatore, secondo una sintassi
* simil-C.
*/

static int scIsSymM(char c)
{
	if (scnCtl.minus_into_sym)
		return (isalpha(c) || c == '_' || c == '-' || isdigit(c));
	else
		return (isalpha(c) || c == '_' || isdigit(c));
}

/*
* Funzione scInit
* ---------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Questa funzione deve essere chiamata prima di qualsiasi altra di questo
* file. Inizializza la parte non indipendente della struttura di controllo
* dello scanner, la parte indipendente essendo stata inizializzata dal
* chiamante tramite il puntatore globale "pScCtl".
*/

void scInit(void)
{

/* Inizializza alcuni campi non indipendenti. */

	scnCtl.pred = scnCtl.curr = scnCtl.line;
	scnCtl.line[0] = '\0';
	scnCtl.n_line = 0;
	scnCtl.incomment = 0;
	scnCtl.l_start_co = strlen(scnCtl.start_co);
	scnCtl.l_end_co = strlen(scnCtl.end_co);
	scnCtl.kwt = (LPSCNKW) 0;
	scnCtl.kwt_len = 0;
	scnCtl.last_kw = -1;
}

/*
* Funzione scGetSts
* -----------------
*
* Parametri :
*
*	SCNSTS * sts : puntatore alla struttura di stato (vedi "scanner.h").
*
* Valori calcolati :
*
*	Valore di ritorno : 0 se non e` definita la primitiva di acquisizione
*		dello stato del lettore, o se questa fallisce, altrimenti il numero
*		di byte copiati.
*
* Descrizione :
*
*  Questa funzione scrive nella struttura puntata da "sts" la posizione
* di lettura a livello di scanner (campo "curr") e di primitive di
* lettura (campo "rdsts", dimensione dipendente dal sistema. Vedere
* il commento in "scanner.h" per una discussione dettagliata).
*/

int scGetSts(LPSCNSTS sts)
{
	if (! scnCtl.getrdsts || ! (*scnCtl.getrdsts)((LPVOID)(sts -> rdsts)))
		return 0;
	sts -> curr = scnCtl.curr;
	sts -> pred = scnCtl.pred;
	sts -> n_line = scnCtl.n_line;
	return scnCtl.l_rdsts;
}

/*
* Funzione scSetSts
* -----------------
*
* Parametri :
*
*	SCNSTS * sts : puntatore alla struttura di stato (vedi "scanner.h").
*
* Valori calcolati :
*
*	Valore di ritorno : 0 se non e` definita la primitiva di impostazione
*		dello stato del lettore, o se questa fallisce, altrimenti il numero
*		di byte copiati.
*
* Descrizione :
*
*  Questa funzione imposta lo stato dello scanner e del lettore in accordo
* col contenuto della struttura puntata da "sts". E` modificata la posizione
* di lettura a dello scanner (campo "curr") e delle primitive di
* lettura (campo "rdsts", dimensione dipendente dal sistema. Vedere
* il commento in "scanner.h" per una discussione dettagliata).
*/

int scSetSts(LPSCNSTS sts)
{
	if (! scnCtl.setrdsts || ! (*scnCtl.setrdsts)((LPVOID)(sts -> rdsts)))
		return 0;
	scnCtl.curr = sts -> curr;
	scnCtl.pred = sts -> pred;
	scnCtl.n_line = sts -> n_line;
	return scnCtl.l_rdsts;
}

/*
* Funzione scPreprocess
* ----------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : 1 in caso di successo, 0 se errore o EOF.
*
* Descrizione :
*
*  Legge una linea, se si e` in modalita` "case-insensitive" converte in
* maiuscolo, e rimuove i commenti.
*  Ogni commento e` sostituido da uno spazio. Le stringhe di inizio e
* fine commento si trovano nei campi "start_co" ed "end_co" della
* struttura di controllo. I commenti possono essere su piu` righe :
* il campo "incomment" della struttura di controllo tiene traccia dello
* stato dei commenti tra una linea e l'altra.
*/


static int scPreprocess(void)
{
register LPCHAR dest,src,p;
register char c,esc;

/* Legge una linea. Esce se EOF. */

	if (! (*(scnCtl.readln))()) {
		return 0;
	}

	++scnCtl.n_line;

/* Nucleo della procedura.
  Ad ogni ciclo si controlla se ci si trova all'interno di un commento.
  Se cio` accade, si cerca la fine del commento. Se non esiste, si produce
 una stringa vuota, altrimenti si passa a considerare cio` che segue.
  Si cerca poi l'inizio di un nuovo commento. Se esiste, lo si sostituisce con
 uno spazio, e si ricominca, altrimenti si esce. */

/* Per la rimozione dei commenti, sara` necessario ricompattare la
 linea. La variabile "src" punta alla sottostringa da spostare perche`
 preceduta da commenti, mentre "dest" punta alla posizione finale della
 sottostringa. */

	src = dest = scnCtl.line;

	for (;;) {

	/* Se si e` all'interno di un commento (eventualmente aperto in un'altra
	 linea) se ne cerca la fine. Se c'e`, si sposta "src" in avanti, altrimenti
	 si chiude la stringa e si esce. */

		if (scnCtl.incomment) {

		/* Cerca la fine del commento. */

			p = strstr(src, scnCtl.end_co);
			if (p) {

			/* Trovata. Registra il fatto nella struttura di stato. */

				scnCtl.incomment = 0;

			/* Porta il puntatore alla sottostringa da muovere al
			 primo carattere che segue il commento. */

				src = p + scnCtl.l_end_co;
			}
			else {

			/* Il commento proseguira` nella prossima linea.
			  Chiude la stringa con un LF e termina. */

				*dest = '\n';
				*++dest = '\0';
				return 1;
			}
		}

	/* Se nella posizioe corrente c'e` un inizio commento, lo si
	 sostituisce con uno spazio, e si riprende il ciclo dall'inizio. */

		if (strncmp(src,scnCtl.start_co,scnCtl.l_start_co) == 0) {

		/* Aggiunge uno spazio, a sostituire il commento. */

			*(dest++) = ' ';

		/* Sposta il puntatore temporaneo di lettura oltre la sequenza
		 di inizio commento. */

			src += scnCtl.l_start_co;

		/* Registra il nuovo stato nella struttura di controllo. */

			scnCtl.incomment = 1;

		}
		else {

		/* Copia il carattere corrente, eseguendo le conversioni
		 opportune. */

			c = *(src++);
			*(dest++) = scnCtl.case_insensitive ? toupper(c) : c;

		/* Se nella posizione corrente c'e` un inizio stringa, si
		 provvede a copiarla cosi` com'e` (cioe` fino alla fine
		 delle linea o al successivo "'", a meno che questo non sia
		 preceduto da "$". */

			if (c == scnCtl.start_str) {
				esc = 0;
				while ( (c = *(src++)) ) {
					*(dest++) = c;
					if (c == scnCtl.start_str && !esc)
						break;
					esc = (c == scnCtl.esc_str && !esc);
				}
			}

		/* Esce alla fine della linea. */

			if (! c)
				return 1;
		}
	}
}

/*
* Funzione scBlanks
* ------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : L'indirizzo del primo carattere non-blank trovato,
*		oppure NULL se EOF.
*
* Descrizione :
*
*  Porta il puntatore di lettura sul primo carattere non-blank disponibile.
*/

static LPCHAR scBlanks(void)
{
register char c;

	for (c = *scnCtl.curr; scIsBlank(c); c = *scnCtl.curr) {
		if (c == '\0') {
			scnCtl.curr = scnCtl.line;
			if (scPreprocess() == 0) {
				return NULL;
			}
		}
		else {
			++scnCtl.curr;
		}
	}
	return scnCtl.curr;
}

#ifdef FFSS
/*
* Funzione scBGetCh
* -----------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : il prossimo carattere, eventualmente 0.
*
* Descrizione :
*
*  Fornisce prossimo carattere nella linea corrente, oppure '\0' se si e` a
* fine linea. La posizione di lettura e` spostata in avanti.
*/

static int scBGetCh(void)
{
	if (*scnCtl.curr)
		return *(scnCtl.curr++);
	else
		return '\0';
}
#endif /* FFSS */

/*
* Funzione scGetCh
* ----------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : il prossimo carattere, eventualmente 0.
*
* Descrizione :
*
*  Fornisce prossimo carattere significativo nello stream di lettura,
* oppure '\0' se si e` a fine stream. La posizione di lettura e` spostata
* in avanti.
*/

int scGetCh(void)
{
	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	if (*scnCtl.curr)
		return *(scnCtl.curr++);
	else
		return '\0';
}

#ifdef FFSS
/*
* Funzione scBNextCh
* -------------------
*
* Parametri :
*
*	char c : Carattere la cui presenza si vuole controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere e` presente, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere dello stream di lettura, spazio e fine
* linea compresi, e` "c".
*/

static int scBNextCh(char c)
{
	return *scnCtl.curr == c;
}
#endif /* FFSS */

/*
* Funzione scNextCh
* -------------------
*
* Parametri :
*
*	char c : Carattere la cui presenza si vuole controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere e` presente, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere significativo dello stream di lettura
* e` "c".
*/

int scNextCh(char c)
{
	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	return *scnCtl.curr == c;
}

/*
* Funzione scBackCh
* -----------------
*
* Parametri :
*
*	char c : Carattere da reinserire nello stream di ingresso.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Reintroduce il carattere "c" nello stream di ingresso, in modo che
* sia riletto.
*
* NOTA : il comportamento della funzione e` indefinito se si tenta di
* retocedere fino ad oltrepassare l'inizio del buffer di lettura.
*/

void scBackCh(char c)
{
	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	*--scnCtl.curr = c;
}

/*
* Funzione scBack
* ---------------
*
* Parametri :
*
*	char * s : Stringa da reinserire nello stream di ingresso.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Reintroduce la stringa "s" nello stream di ingresso, in modo che
* sia riletta.
*
* NOTA : il comportamento della funzione e` indefinito se si tenta di
* retocedere fino ad oltrepassare l'inizio del buffer di lettura.
*/

void scBack(LPCHAR s)
{
int l;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	for (l = strlen(s); l; *--scnCtl.curr = s[--l]) {
	}
}

/*
* Funzione scBackToken
* --------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Riporta il puntatore di lettura sulla posizione del token (identificatore,
* parola chiave o stringa numerica) appena letto. E` opportuno
* utilizzare questa funzione ogni volta che sia possibile, perche` piu`
* veloce della "scBack". Per contro, la "scBack" puo` essere utilizzata in
* un numero maggiore di casi.
*
* NOTA : il comportamento della funzione e` indefinito se, dopo la lettura
* del token, sono state eseguite altre funzioni che abbiano implicato la
* chiamata della "scBlanks".
*/

void scBackToken(void)
{
	scnCtl.curr = scnCtl.pred;
/* Invalida la cache. */
	scnCtl.last_kw = -1;
}

#ifdef FFSS
/*
* Funzione scBMatchCh
* -------------------
*
* Parametri :
*
*	char c : Carattere la cui presenza si vuole controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere e` presente, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere dello stream di lettura, spazi o fine linea
* compresi, e` "c". Se il carattere e` presente, sposta in avanti la posizione
* di lettura.
*/

static int scBMatchCh(int c)
{
	if (*scnCtl.curr == c) {
		++scnCtl.curr;
		return 1;
	}
	else {
		return 0;
	}
}
#endif /* FFSS */


/*
* Funzione scMatchCh
* -------------------
*
* Parametri :
*
*	char c : Carattere la cui presenza si vuole controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se il carattere e` presente, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se il prossimo carattere significativo dello stream di lettura
* e` "c". Se il carattere e` presente, sposta in avanti la posizione di
* lettura.
*/

int scMatchCh(int c)
{
	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	if (*scnCtl.curr == c) {
		++scnCtl.curr;
		return 1;
	}
	else {
		return 0;
	}
}

/*
* Funzione scMatchSet
* -------------------
*
* Parametri :
*
*	char * s : Insieme di caratteri la cui presenza si vuole controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : il carattere, se presente, altrimenti 0.
*
* Descrizione :
*
*  Verifica se il prossimo carattere significativo dello stream di lettura
* e` uno di quelli in "s". Se il carattere e` presente, sposta in avanti
* la posizione di lettura.
*/

int scMatchSet(LPCHAR s)
{
register LPCHAR p;
register char c;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	c = *scnCtl.curr;
	for (p = s; *p; ++p) {
		if (c == *p) {
			++scnCtl.curr;
			return c;
		}
	}
	return 0;
}

/*
* Funzione scMatch
* -----------------
*
* Parametri :
*
*	char * s : Stringa la cui presenza si vuole controllare.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se la stringa e` presente, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura e` "s". Se la stringa e` presente, sposta in avanti
* la posizione di lettura.
*/

int scMatch(LPCHAR s)
{
register LPCHAR p,q;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	for (p = s, q = scnCtl.curr; *p; ++p, ++q) {
		if (*q != *p)
			return 0;
	}
	scnCtl.pred = scnCtl.curr;
	scnCtl.curr = q;
	return 1;
}

/*
* Funzione scBIdent
* -----------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare l'identificatore.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovato un identificatore, la sua
*		lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se nella corrente posizione di lettura e` presente un
* identificatore (sintassi simil-C), nel qual caso la copia in "s",
* sposta in avanti il puntatore di lettura e termina con valore = lunghezza.
*  La lunghezza massima dell'identificatore e` decisa dal campo
* "maxidlen" di "scnCtl". Identificatori piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*  Se l'identificatore e` stato trovato, lo copia in "s", sposta in avanti
* il puntatore di lettura e termina con valore = lunghezza. In caso contrario,
* "s" resta invariata, la posizione di lettura non cambia, ed il valore di
* ritorno e` 0.
*
*  NOTA : Questa funzione agisce esclusivamente sulla linea corrente, e non
* salta gli spazi. L'utilizzo di questa funzione dovrebbe rimanere limitato
* alla costruzione di altre primitive di questo modulo.
*/

static int scBIdent(LPCHAR s)
{
register char c;
register int l,n;

	c = *scnCtl.curr;
	if (! scIsSymF( (char)c ))
		return 0;
	scnCtl.pred = scnCtl.curr;
	l = scnCtl.maxidlen;
	n = 0;
	do {
		if (l) {
			--l;
			*(s++) = c;
			++n;
		}
		c = *++scnCtl.curr;
	} while (scIsSymM(c));
	*s = '\0';
	return n;
}

/*
* Funzione scRawIdent
* -------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare l'identificatore.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovato un identificatore, la sua
*		lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura e` un identificatore (sintassi simil-C), e lo copia
* in "s". La lunghezza massima dell'identificatore e` decisa dal campo
* "maxidlen" di "scnCtl". Identificatori piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*  Se l'identificatore e` stato trovato, lo copia in "s", sposta in avanti
* il puntatore di lettura e termina con valore = lunghezza. In caso contrario,
* "s" resta invariata, la posizione di lettura non cambia, ed il valore di
* ritorno e` 0.
*/

static int scRawIdent(LPCHAR s)
{
	scBlanks();
	return scBIdent(s);
}

/*
* Funzione scBUInteger
* --------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa numerica.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stata trovata una stringa numerica,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se nella corrente posizione di lettura e` presente una sequenza
* composta di cifre decimali, nel qual caso la copia in "s", sposta in avanti
* il puntatore di lettura e termina con valore = lunghezza.
*  In caso contrario, "s" resta invariata, la posizione
* di lettura non cambia, ed il valore di ritorno e` 0.
*  La lunghezza massima della stringa e` decisa dal campo
* "maxnumlen" di "scnCtl". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*
*  NOTA : Questa funzione agisce esclusivamente sulla linea corrente, e non
* salta gli spazi. L'utilizzo di questa funzione dovrebbe rimanere limitato
* alla costruzione di altre primitive di questo modulo.
*/

static int scBUInteger(LPCHAR s)
{
register char c;
register int l,n;

	c = *scnCtl.curr;
	if (! isdigit(c))
		return 0;
	scnCtl.pred = scnCtl.curr;
	l = scnCtl.maxnumlen;
	n = 0;
	do {
		if (l) {
			--l;
			*(s++) = c;
			++n;
		}
		c = *++scnCtl.curr;
	} while (isdigit(c));
	*s = '\0';
	return n;
}

/*
* Funzione scUInteger
* --------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa numerica.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stata trovata una stringa numerica,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura e` composta da sole cifre decimali, nel qual caso
* la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza. In caso contrario, "s" resta invariata, la posizione
* di lettura non cambia, ed il valore di ritorno e` 0.
*  La lunghezza massima della stringa e` decisa dal campo
* "maxnumlen" di "scnCtl". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*/

int scUInteger(LPCHAR s)
{
	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	return scBUInteger(s);
}

/*
* Funzione scBInteger
* -------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa numerica.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stata trovata una stringa numerica,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se nella corrente posizione di lettura e` presente una sequenza
* composta di cifre decimali precedute opzionalmente da "+" o "-", nel qual
* caso la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza.
*  In caso contrario, "s" resta invariata, la posizione
* di lettura non cambia, ed il valore di ritorno e` 0.
*  La lunghezza massima della stringa e` decisa dall'espressione
* "scnCtl.maxnumlen + 1". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*
*  NOTA : Questa funzione agisce esclusivamente sulla linea corrente, e non
* salta gli spazi. L'utilizzo di questa funzione dovrebbe rimanere limitato
* alla costruzione di altre primitive di questo modulo.
*/

static int scBInteger(LPCHAR s)
{
register char c;
register int n;

	c = *scnCtl.curr;
	if (c == '+' || c == '-') {
		++scnCtl.curr;
		n = scBUInteger(s + 1);
		if (n) {
			--scnCtl.pred;
			*s = c;
		}
		else
			--scnCtl.curr;
	}
	else
		n = scBUInteger(s);
	return n;
}

/*
* Funzione scInteger
* -------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa numerica.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stata trovata una stringa numerica,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura puo` essere interpretata come un intero con segno
* secondo la rappresentazione in base 10 usuale, nel qual caso
* la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza. In caso contrario, "s" resta invariata, la posizione
* di lettura non cambia, ed il valore di ritorno e` 0.
*  La lunghezza massima della stringa e` decisa dall'espressione
* "scnCtl.maxnumlen + 1". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*/

int scInteger(LPCHAR s)
{
	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

	scBlanks();
	return scBInteger(s);
}

/*
* Funzione scIECDirectVar
* -----------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa.
*	char type : tipo della variabile ('I', 'Q', 'M' o 0 per tutti).
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovata una stringa accettabile,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica la presenza di una stringa compatibile con la sintassi delle
* "variabili dirette" di IEC 1131-3, piu` le estensioni CNI.
*/

int scIECDirectVar(LPCHAR s, char type)
{
register char c,d;
register int n;
LPCHAR p;
LPCHAR q;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

/* "p" conserva la posizione di lettura iniziale, che dovra` essere
 ripristinata in caso di errore, oppure sara` assegnata al campo
 "pred" in caso di successo. */

	scBlanks();
	q = p = scnCtl.curr;

/* Controlla intanto se si ha di fronte una stringa nella forma "%{IQM}..."
 "n" e` la lunghezza totale della stringa generata. */

	if (*q != '%')
		return 0;
	c = *++q;
	if (type && type != c)
		return 0;
	if (!type && c != 'I' && c != 'Q' && c != 'M')
		return 0;

	*s = '%';
	*++s = c;

	d = *++q;

	if (d != 'X' && d != 'B' && d != 'W' && d != 'D' && d != 'L')
		return 0;

	*++s = d;

	if (c == 'M') {
		if (*++q == '*') {
			*++s = '*';
			*++s = '\0';
			n = 1;
			scnCtl.curr = ++q;
		}
		else {
			scnCtl.curr = q;
			n = scBInteger(++s);
		}
	}
	else {
		n = 0;
		while ( (c = *++q)
		       && ((c >= '0' && c <= '9')
                           || (c >= 'A' && c <= 'Z')
                           || (c == '_')
                           || (c == '.')) ) {
			*++s = c;
			++n;
		}
		if (n) {
			scnCtl.curr = q;
			*++s = '\0';
		}
	}

	if (! n) {
		scnCtl.curr = p;
		return 0;
	}

	scnCtl.pred = p;

	return n + 3;
}

/*
* Funzione scIECInteger
* ---------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa numerica.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovata una stringa numerica,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura puo` essere interpretata come un intero nella 
* notazione prevista nel documento IEC1131-3, nel qual caso
* la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza. In caso contrario, la posizione di lettura
* non cambia, il valore di ritorno e` 0, e la stringa "s" puo` contenere 
* caratteri non significativi.
*  La lunghezza massima della stringa e` data dall'espressione
* "scnCtl.maxnumlen*3 + 4". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*  La stringa prodotta e` scomposta il due parti, separate da uno spazio.
*  La prima e` costituita un caratterie codificante la base, la seconda
* contiene la parte che segue '#'.
*/

int scIECInteger(LPCHAR s)
{
register char c;
register int n,base;
LPCHAR p;
LPCHAR q;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

/* "p" conserva la posizione di lettura iniziale, che dovra` essere
 ripristinata in caso di errore, oppure sara` assegnata al campo
 "pred" in caso di successo. */

	scBlanks();
	p = scnCtl.curr;

/* Controlla intanto se si ha di fronte una stringa nella forma "[0-9]+#..."
 "n" e` la lunghezza totale della stringa generata. */

	q = s + 1; /* Lascia 1 carattere per la base. */
	n = scBInteger(q);

	if (! n) {
		scnCtl.curr = p;
		return 0;
	}

/* C'e` "#" : l'intero appena letto era la base. Deve essere 2, 8 o 16. */

	if (*scnCtl.curr != '#') {
	/* Non c'e` : e` un volgare numero decimale. */
		base = 10;
	}
	else {
		base = atoi(q);
	}

	*(--q) = base;

	switch (base) {
	case 2:
		do {
			*++q = c = *++scnCtl.curr;
		} while (c == '0' || c == '1');
		*q = '\0';
		break;
	case 8:
		do {
			*++q = c = *++scnCtl.curr;
		} while (c >= '0' && c <= '7');
		*q = '\0';
		break;
	case 16:
		do {
			*++q = c = *++scnCtl.curr;
		} while ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
		*q = '\0';
		break;
	case 10:
		if (*scnCtl.curr == '#') {
			do {
				*++q = c = *++scnCtl.curr;
			} while (c >= '0' && c <= '9');
			*q = '\0';
		}
		break;
	default:
		q = s + 1;
		break;
	}

	if (q == s + 1) {
		scnCtl.curr = p;
		return 0;
	}

	scnCtl.pred = p;

	return n + 1;
}


/*
* Funzione scFloat
* -----------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa numerica.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovata una stringa numerica,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura puo` essere interpretata come un float con segno
* secondo la rappresentazione usuale, nel qual caso
* la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza. In caso contrario, la posizione di lettura
* non cambia, il valore di ritorno e` 0, e la stringa "s" puo` contenere 
* caratteri non significativi.
*  La lunghezza massima della stringa e` data dall'espressione
* "scnCtl.maxnumlen*3 + 4". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*/

int scFloat(LPCHAR s)
{
register char c;
register int l,n,mant;
LPCHAR p;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

/* "p" conserva la posizione di lettura iniziale, che dovra` essere
 ripristinata in caso di errore, oppure sara` assegnata al campo
 "pred" in caso di successo. */

	scBlanks();
	p = scnCtl.curr;

/* "n" e` la lunghezza totale della stringa generata,
   "l" e` la lunghezza dell'ultima frazione letta,
   "mant" vale 0 solo se non sono state trovate cifre nella mantissa. */

/* Cerca la parte prima del punto decimale (opzionale). */

	mant = n = l = scBInteger(s);
	s += l;

/* Cerca il punto decimale (opzionale). */
/* Per evitare ambiguita` a piu` alto livello, si scartano subito
 le sequenze composte da due '.' consecutivi (e` il separatore di
 subrange). */

	c = *scnCtl.curr;
	if (c == '.' && scnCtl.curr[1] != '.') {

	/* Se c'e` il punto decimale, cerca altre cifre. */

		*s = c;
		*++s = '\0';
		++n;
		c = *++scnCtl.curr;

	/* Le cifre devono seguire immediatamente il punto decimale.
	  Non sono ammessi caratteri di spaziatura all'interno della stringa. */

		l = scBUInteger(s);

	/* "mant" sara` diversa da zero solo se e` stata trovata almeno una cifra
	 decimale prima o dopo il punto. Una stringa float corretta deve sempre
	 rispettare questa regola. */

		mant += l;
		n += l;
		s += l;
		c = *scnCtl.curr;
	}
	else {

/* IEC 1131-C sembra pretendere il punto decimale. Tanto meglio. */

		scnCtl.curr = p;
		return 0;
	}

/* Errore : nessuna cifra nella mantissa. Il puntatore di lettura e`
 riportato nella posizione iniziale, e la funzione fallisce. */

	if (! mant) {
		scnCtl.curr = p;
		return 0;
	}

/* La stringa e`, per il momento, corretta.
  Controlla se e` stato specificato l'esponente (opzionale). */

	if (c == 'e' || c == 'E') {
		*s = c;
		*++s = '\0';
		++n;
		c = *++scnCtl.curr;

	/* La "E" dell'esponente deve essere seguita immediatamente da
	 una stringa intera con o senza segno. */

		l = scBInteger(s);
		if (! l) {

		/* Errore. L'esponente e` incompleto. Riporta all'inizio il
		 puntatore di lettura e segnala fallimento. */

			scnCtl.curr = p;
			return 0;
		}

		n += l;
	}

	scnCtl.pred = p;

	return n;
}

/*
* Funzione scIECTime
* ------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa "tempo".
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovata una stringa "tempo",
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura puo` essere interpretata come un valore "TIME" nella 
* notazione prevista nel documento IEC1131-3, nel qual caso
* la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza. In caso contrario, la posizione di lettura
* non cambia, il valore di ritorno e` 0, e la stringa "s" puo` contenere 
* caratteri non significativi.
*  La lunghezza massima della stringa e` data dall'espressione
* "scnCtl.maxnumlen*3 + 4". Numeri piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*  La stringa prodotta e` composta da campi numerici separati da spazi,
* contenenti le componenti del valore nelle unita` di misura "giorni",
* "ore", "minuti", "secondi" e "millisecondi". Solo il campo "giorni"
* e` sempre presente, gli altri possono mancare (se manca un campo, mancano
* anche tutti i successivi).
*/

int scIECTime(LPCHAR s)
{
int i,n,ln,unit,found,quit;
LPCHAR p;
LPCHAR q;
static char * tunits[] = { "D", "H", "M", "S", "MS" };
static char  ltunits[] = { 1,1,1,1,2 };

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

/* "p" conserva la posizione di lettura iniziale, che dovra` essere
 ripristinata in caso di errore, oppure sara` assegnata al campo
 "pred" in caso di successo. */

	scBlanks();
	p = scnCtl.curr;

/* Controlla intanto se si ha di fronte una stringa nella forma "TIME#..."
 o "T#...". */

	if (strncmp(p,"T#",2) == 0)
		scnCtl.curr += 2;
	else if (strncmp(p,"TIME#",5) == 0)
		scnCtl.curr += 5;
	else {
		scnCtl.curr = p;
		return 0;
	}

/* Inizia la costruzione della stringa formattata. */
/* "n" e` la lunghezza totale della stringa generata. */

	ln = 0;
	q = s;
	found = 0;
	quit = 0;

/* Ciclo di ricerca delle unita` e costruzione del valore formattato. */

	for (unit = 0; unit < sizeof(tunits)/sizeof(tunits[0]) && !quit; ) {

	/* Cerca una costante intera. Esce dal ciclo se non la trova. */

		n = scBInteger(q);
		if (n == 0) {
			break;
		}
		else {
		/* Cerca l'eventuale parte decimale. Se c'e`, la trasferisce
		 e segnala che non saranno ammesse altre componenti. */
			if (*(scnCtl.curr) == '.') {
			int nn;

				++scnCtl.curr;
				q[n] = '.';
				++n;
				nn = scBUInteger(q + n);
				if (nn == 0) {
				/* Se c'e` il punto decimale, ci deve
				 anche essere qualcosa dopo ! */
					found = 0;
					break;
				}
				n += nn;
				quit = 1; /* Forza la fine del ciclo. */
			}
		/* Aggiunge uno spazio dopo il numero. */
			q[n] = ' ';
			++n;
			found = 1;
		}

		ln += n;

	/* Cerca l'unita` di misura. Comincia dalla fine della tabella
	 per risolvere l'ambiguita` tra "M" ed "MS". Sono escluse
	 le unita` di misura gia` incontrate ("unit" ne tiene traccia). */

		for (i = sizeof(tunits)/sizeof(tunits[0]) - 1; i >= unit; --i) {
			if (strncmp(tunits[i],scnCtl.curr,(int)ltunits[i]) == 0) {
				break;
			}
		}

	/* Nessuna unita` di misura nota. Uscita e fallimento. */

		if (i < 0) {
			found = 0;
			break;
		}

	/* Elimina l'unita`. */

		scnCtl.curr += ltunits[i];

	/* Se l'unita` di misura non e` quella "corrente", fa spazio
	 per ospitare i campi delle unita` omesse (varranno 0). */

		if (unit != i) {
			memmove(q + (i - unit) * 2, q, (size_t)n);
		}

	/* Aggiunge i campi delle unita` omesse. */

		for (; unit < i; ++unit) {
			*q = '0';
			*++q = ' ';
			++q;
			ln += 2;
		}

	/* Le unita` ammesse d'ora in poi saranno solo quelle inferiori
	 a quella corrente. */

		unit = i + 1;

	/* Porta avanti il puntatore di fine stringa. */

		q += n;

	/* Scarta eventuali "_", che il linguaggio ammette per separare
	 i campi. */

		while (*scnCtl.curr == '_')
			++scnCtl.curr;
	}

	if (! found) {
	/* Nessuna unita` trovata. Non e` una costante "TIME"
	 (e forse non e` niente di sensato in IL). */
		scnCtl.curr = p;
		return 0;
	}

/* Chiude la stringa. */

	*q = '\0';

	scnCtl.pred = p;

	return ln;
}

/*
* Funzione scIECString
* --------------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare la stringa.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovata una stringa,
*		 la sua lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura puo` essere interpretata come una stringa nella 
* notazione prevista nel documento IEC1131-3, nel qual caso
* la copia in "s", sposta in avanti il puntatore di lettura e termina
* con valore = lunghezza. In caso contrario, la posizione di lettura
* non cambia, il valore di ritorno e` -1, e la stringa "s" puo` contenere 
* caratteri non significativi.
*  La lunghezza massima della stringa e` data dall'espressione
* "scnCtl.maxstringlen". Stringhe piu` lunghe sono troncati, ma
* vengono rimossi per intero dallo stream di lettura.
*/

int scIECString(LPCHAR s)
{
register char c,esc,hex,hv;
register int n;
LPCHAR p,q;

	if (scnCtl.last_kw != -1) {
		scBackToken();
	}

/* "p" conserva la posizione di lettura iniziale, che dovra` essere
 ripristinata in caso di errore, oppure sara` assegnata al campo
 "pred" in caso di successo. */

	scBlanks();
	p = q = scnCtl.curr;

/* Fallimento se non c'e` un "'" all'inizio. */

	if (*p != scnCtl.start_str)
		return -1;

	esc = hex = hv = 0;
	n = 0;
	while ( (c = *++p) ) {
		if (esc) {
			esc = 0;
			switch (c) {
			case 'L': case 'l': c = '\n'; break;
			case 'N': case 'n': c = '\n'; break;
			case 'P': case 'p': c = '\f'; break;
			case 'R': case 'r': c = '\r'; break;
			case 'T': case 't': c = '\t'; break;

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				c -= '0'; goto HEX;
			case 'a': case 'b': case 'c': case 'd':
			case 'e': case 'f':
				c -= 'a' - 10; goto HEX;
			case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F':
				c -= 'A' - 10;
			HEX:
				if (++hex == 1) {
					esc = 1;
					hv = c * 16;
					continue;
				}
				else {
					c += hv;
				}
				break;

			default:
				break;
			}
		}
		else if (c == scnCtl.start_str) {
			break;
		}
		else if (c == scnCtl.esc_str) {
			esc = 1;
			hex = 0;
			hv = 0;
			continue;
		}
		else {
		}
		if (n < scnCtl.maxstringlen) {
			*(s++) = c;
			++n;
		}
	}

/* Chiude la stringa. */

	*s = '\0';

	scnCtl.curr = p;
	scnCtl.pred = q;

	return n;
}

/*
* Funzione scRegisterKeywords
* ---------------------------
*
* Parametri :
*
*	struct scn_kw * kws : Indirizzo della tabella delle parole chiave.
*
*	int n : numero di elementi nella tabella.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Registra la tabella delle parole chiavi da utilizzare in seguito.
*
*  NOTA : La tabella "kws" deve essere ordinata in senso crescente.
*/

void scRegisterKeywords(LPSCNKW kws, int n)
{
/* Ricaccia indietro la parola chiave sospesa nella cache. */
	if (scnCtl.last_kw != -1) {
		scBack(scnCtl.kwt[scnCtl.last_kw].name);
	}
/* Invalida la cache. */
	scnCtl.last_kw = -1;
/* Registra la nuova tabella. */
	scnCtl.kwt = kws;
	scnCtl.kwt_len = n;
}

/*
* Funzione scKeyword
* ------------------
*
* Parametri :
*
*	-
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stata trovata una parola chiave,
*		 il suo indice nella tabella, -1 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura e` contenuta nella tabella delle parole chiave corrente.
*  Se la ricerca ha successo, sposta in avanti il puntatore di lettura e
* termina con valore = indice. In caso contrario, la posizione di lettura
* non cambia, il valore di ritorno e` -1. Le stringhe devono rispettare
* la convenzione sintattica degli identificatori (vedi "scIdent").
*/

int scKeyword(void)
{
register LPCHAR p;
int sup,inf;
int i,j,il,ih;
register char c;

/* Controlla la cache. */

	if (scnCtl.last_kw != -1) {
		i = scnCtl.last_kw;
		scnCtl.last_kw = -1;
		return i;
	}

/* Via gli spazi (ed, eventualmente, avanti con la linea successiva). */

	scBlanks();

/* p : posizione temporanea di lettura. Diventera` definitiva se la ricerca
 avra` successo. */

	p = scnCtl.curr;

/* Ci sono regole piu` restrittive per il primo carattere. Se non sono
 rispettate, e` inutile continuare. */

	c = *p;
	if (! scIsSymF(c))
		return -1;

/* i : posizione nella stringa del carattere correntemente analizzato. */

	i = 0;

/* inf : indice della prima parola compatibile con gli "i" caratteri
 analizzati. */

	inf = 0;

/* sup : indice dell'ultima parola compatibile con gli "i" caratteri
 analizzati. */

	sup = scnCtl.kwt_len - 1;

/* Nucleo della procedura. Ad ogni ciclo si eseguono due ricerche binarie
 del carattere corrente (l'i-esimo della stringa) nell'insieme costituito
 dagli i-esimi caratteri delle stringhe da ksw[inf] a kws[sup]. La prima
 ricerca e` condotta in modo che si trovi il primo elemento dell'insieme
 che coincida col carattere, la seconda invece in modo che si trovi l'ultimo.
 In questo modo sono ricalcolati i limiti "inf" e "sup" da utilizzare per
 il ciclo successivo. Questo modo di procedere riduce al minimo i passi di
 ricerca perche`, gia` dopo il primo carattere, l'intervallo di ricerca si
 riduce in genere ad uno o alpiu` due elementi. */
 
	do {

	/* Ricerca binaria del limite inferiore dell'insieme delle stringhe
	 uguali fino all'i-esimo carattere a quella in ingresso. */

		for (il = inf, ih = sup, j = (il + ih)/2;
		     il < ih;
		     j = (il + ih)/2) {
			if (c > scnCtl.kwt[j].name[i])
				il = j + 1;
			else
				ih = j;
		}

	/* Se il carattere corrente non coincide con il corrispondente
	 nella prima stringa compatibile, la ricerca e` fallita. Si esce. */

		if (c != scnCtl.kwt[il].name[i])
			return -1;

	/* Usa il limite inferiore calcolato per restringere ancore il campo di
	 ricerca del limite superiore. Tutto fa brodo. */

		inf = il;

	/* Ricerca binaria del limite superiore dell'insieme delle stringhe
	 uguali fino all'i-esimo carattere a quella in ingresso. */

		for (il = inf, ih = sup, j = (il + ih + 1)/2;
			 il < ih;
			 j = (il + ih + 1)/2) {
			if (c >= scnCtl.kwt[j].name[i])
				il = j;
			else
				ih = j - 1;
		}

	/* Inultile controllare se il carattere corrente coincida o no
	 con il corrispondente nell'ultima stringa compatibile : la condizione
	 e` infatti sempre vera. A questo punto del programma e` sicuro che
	 l'insieme possieda almeno un elemento, che e` il suo limite inferiore,
	 calcolato dalla prima delle due ricerche. Percio` il limite superiore
	 esiste e, nei casi piu` fortunati, coincide con quello inferiore. */

	/*
		if (c != scnCtl.kwt[ih].name[i])
			return -1;
	*/

		sup = ih;

	/* Altro giro, altro carattere... */

		++i;
		c = *++p;

	/* ...finche` ci sono caratteri accettabili. */

	} while (scIsSymM( (char)c));

/* A questo punto e` noto l'indice ("inf") della prima stringa di "kws" che
 coincida con la parola in ingresso per tutta la lunghezza di quest'ultima.
  Perche` si possa dire conclusa la ricerca, bisogna verificare che
 la stringa in "kws" trovata coincida esattamente con quella data.
  Questo e` vero se esse hanno la stessa lunghezza.
  Per come sono ordinate le stringhe, la prima e` sempre la piu` corta,
 percio` non c'e` pericolo di confusione con altre dell'insieme di
 ricerca. La lunghezza della stringa in ingresso e` nota (variabile "i");
 e` percio` sufficiente controllare che l'i-esimo carattere della
 stringa trovata sia il terminatore. */

	if (scnCtl.kwt[inf].name[i] != '\0')
		return -1;

/* Le due stringhe coincidono. Si sposta avanti il puntatore di lettura
 e si termina riportando l'indice della parola nella tabella. */

	scnCtl.pred = scnCtl.curr;
	scnCtl.curr = p;
	return inf;
}

/*
* Funzione scBackKeyword
* ----------------------
*
* Parametri :
*
*	int k : indice della parola chiave.
*
* Valori calcolati :
*
*	-
*
* Descrizione :
*
*  Reintroduce la parola chiave indicata nella cache, in modo che ricerche
* successive la trovino gia` decodificata.
*/

int scBackKeyword(int k)
{
	scnCtl.last_kw = k;
	return( k );
}

/*
* Funzione scMatchKeyword
* -----------------------
*
* Parametri :
*
*	int k : indice della parola chiave.
*
* Valori calcolati :
*
*	Valore di ritorno : 1 se la stringa e` presente, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura e` la parola chiave di indice "key".
*  Se la stringa e` presente, sposta in avanti
* la posizione di lettura.
*/

int scMatchKeyword(int key)
{
int k;

	k = scKeyword();

	if (k != key) {
		scBackKeyword(k);
		return 0;
	}
	else {
		return 1;
	}
}

/*
* Funzione scIdent
* ----------------
*
* Parametri :
*
*	char * s : Indirizzo dell'area in cui copiare l'identificatore.
*
* Valori calcolati :
*
*	Valore di ritorno : se e` stato trovato un identificatore, la sua
*		lunghezza, 0 altrimenti.
*
* Descrizione :
*
*  Verifica se la prossima sequenza di caratteri significativi dello
* stream di lettura e` un identificatore (sintassi simil-C), e lo copia
* in "s". La lunghezza massima dell'identificatore e` decisa dal campo
* "maxidlen" di "scnCtl". Identificatori piu` lunghi sono troncati, ma
* vengono rimossi per intero dallo stream di lettura. Se l'identificatore
* e` contenuto nella tabella delle parole chiave , la ricerca e` considerata
* fallita, e la posizione di lettura non e` modificata.
*  Se l'identificatore e` stato trovato, lo copia in "s", sposta in avanti
* il puntatore di lettura e termina con valore = lunghezza. In caso contrario,
* "s" contiene dati non significativi, la posizione di lettura non cambia,
* ed il valore di ritorno e` 0.
*/

int scIdent(LPCHAR s)
{
int k;

	k = scKeyword();

	if (k == -1) {
		return scRawIdent(s);
	}
	else if (scnCtl.kwt[k].flags) {
		strcpy(s,scnCtl.kwt[k].name);
		return 1;
	}
	else {
		scBackKeyword(k);
		return 0;
	}
}

